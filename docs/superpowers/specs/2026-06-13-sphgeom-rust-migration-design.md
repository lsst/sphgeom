# sphgeom C++ → Rust migration design

Branch: `u/timj/rust`
Date: 2026-06-13
Status: design, pending implementation plan

## 1. Motivation

sphgeom is currently a C++ library (`libsphgeom`) with pybind11 Python bindings and a small pure-Python layer.
We want to investigate minimizing the maintained C++ codebase by moving the implementation to Rust.
Memory safety and performance are both first-class motivations.
We are willing to drop pybind11 entirely and have the Python bindings sit directly on the Rust implementation.

The constraint that shapes everything is that the existing C++ public interface must be retained for downstream C++ consumers (for example `afw`).
That means the `include/lsst/sphgeom/*.h` headers remain the public C++ interface and `libsphgeom` keeps exporting a compatible C++ API.

## 2. Goals and non-goals

### Goals

- Make a pure-Rust crate the canonical implementation of sphgeom's algorithms.
- Keep the existing C++ headers as the public C++ interface, with the C++ object model (classes, vtables, RTTI, inline value-type math) emitted by the C++ compiler.
- Bind Python directly to the Rust core with PyO3 and retire the pybind11 layer.
- Preserve bit-identical results for everything that is persisted or shared across the ecosystem (serialized region bytes, spatial pixel indices, exact predicates).
- Keep the existing C++ and Python test suites green at every step of the migration.
- Reduce the volume of hand-maintained C++ to the header veneer plus thin delegation glue.

### Non-goals

- We do not require binary ABI drop-in replacement of an old `libsphgeom` under already-compiled downstream binaries.
  Downstream recompiles against the headers, consistent with the normal rebuild-the-whole-stack workflow.
- We do not aim to expose a brand-new Rust-shaped C++ API.
  The C++ API stays broadly the same so downstream churn is minimal.
- We do not port trivial inline header arithmetic into FFI calls.
  That arithmetic stays inline in the C++ headers for performance and clarity.
- This document specifies the architecture, the parity contract, the roadmap, and the first phase in detail.
  Later phases get their own specs as we reach them.

## 3. Constraints

### ABI strictness

The agreed level is "header API may evolve."
Downstream recompiles against the headers, so we do not have to hand-match Itanium-mangled symbol names or preserve exact vtable layouts and class sizes.
The headers may change in source-compatible ways as long as the C++ API stays broadly the same.
This is what makes a tool-generated C ABI plus a thin hand-written C++ veneer tractable.

### Bit-identical parity

Three classes of output must be bit-identical to the current C++ implementation because they are persisted or shared:

- The `Region::encode()` byte format, which is stored in databases and used by Python pickling.
- The HTM, Q3C, Mq3c, and MOC pixel index values, which are used as spatial index keys across the ecosystem.
- The exact predicate `orientationExact`, which is integer arithmetic and must match exactly.

Floating-point results from transcendental functions (`sin`, `cos`, `sqrt`) may differ at the ULP level between the C++ standard library and Rust.
ULP-level differences are acceptable for outputs that are not persisted or shared as keys, and we explicitly do not contort the Rust interface or call back into C++ to chase bit-exactness there.
The bit-identical bar applies only to the three persisted or shared classes of output above (see Section 6).

## 4. Target architecture

One Rust core serves three consumers: native Rust, Python via PyO3, and C++ via the retained headers plus a thin veneer.

```
                 ┌──────────────────────────┐
                 │   sphgeom-core (Rust)     │  canonical implementation
                 │   pure Rust, no FFI       │  + native Rust API
                 └──────────────────────────┘
                    ▲          ▲          ▲
        ┌───────────┘          │          └────────────┐
        │ (native)             │ C ABI                  │ PyO3
   Rust users          ┌───────┴────────┐        ┌──────┴───────┐
                       │ sphgeom-capi   │        │  sphgeom-py  │
                       │ extern "C" +   │        │  maturin →   │
                       │ cbindgen .h    │        │  _sphgeom.so │
                       └───────┬────────┘        └──────┬───────┘
                  thin C++ glue │                       │
                 ┌──────────────┴───────────┐    lsst.sphgeom (Python pkg)
                 │ libsphgeom (.dylib/.so)   │    + retained pure-Python layer
                 │ existing include/*.h kept │
                 │ .cc bodies delegate→Rust  │
                 └──────────────┬────────────┘
                       downstream C++ (afw, …)
```

### Crates

- `sphgeom-core` is a pure Rust crate holding the canonical implementation and a clean native Rust API.
  No FFI concerns leak into it.
- `sphgeom-capi` exposes a flat C ABI: opaque handles, `#[repr(C)]` POD structs, and `extern "C"` functions.
  A `build.rs` runs cbindgen to emit a C header consumed by the C++ glue.
- `sphgeom-py` is a PyO3 crate built with maturin that binds directly to `sphgeom-core`.
  It replaces the pybind11 `_*.cc` files.

### C++ veneer

The `include/lsst/sphgeom/*.h` headers remain the public interface.
The C++ compiler still emits the classes, vtables, RTTI, and inline value-type arithmetic, so the C++ object model is satisfied without Rust having to fake a vtable.
The out-of-line `.cc` bodies become thin glue that calls the C ABI.

### Migration versus end state

The diagram above is the end state.
During the migration the Rust port is **not** the public build; the existing pybind11-over-C++ package keeps shipping untouched until we deliberately switch people over.
The Rust path is exercised as a development aid: a parallel PyO3 extension, built by maturin and named `_sphgeom2` to sit beside the public `_sphgeom`, is selected per-process by an environment variable so the existing Python test suite can validate the Rust implementations without affecting anything anyone installs.
The C ABI, the C++ glue, and the packaging changes only become public at the cutover.

## 5. The C ABI bridge contract

### Value types

`Vector3d` (`[f64; 3]`), `Matrix3d` (`[f64; 9]`), `Angle` (`f64`), and similar value types are `#[repr(C)]` POD with byte-identical layout on both sides.
They cross the boundary by value or pointer with zero marshaling.
Their trivial inline arithmetic stays in the C++ headers; only non-trivial out-of-line methods delegate to Rust.
This duplication is bounded and deliberate, and it is guarded by differential tests.

### Regions

Region instances are opaque heap handles.
A C++ subclass such as `Circle : Region` holds a raw pointer to a Rust object.
Its virtual method bodies call the C ABI.
`clone()` wraps a Rust-cloned handle in a fresh `unique_ptr`.
The destructor calls the Rust free function.
`decode()` returns a type tag plus a handle, and the glue constructs the matching C++ subclass.

### Double dispatch

The `relate` and `overlaps` overloads keep their header signatures, but the bodies collapse to passing both handles to a single Rust dispatcher that matches internally.
The header API stays broadly the same while the C++ dispatch machinery shrinks.

### Errors and exceptions

Errors originate as a Rust error enum.
The C ABI returns a status code.
The C++ glue translates the status into the existing C++ exception (`IvoaStcsNotImplemented`), and the PyO3 layer maps the same Rust error to the Python exception (`NotImplementedError`).
There is one source of truth with two idiomatic surfaces.

### Strings and vectors

`encode()` returns `std::vector<uint8_t>` and `toIvoaStcs()` returns `std::string`.
These use standard ownership-transfer C ABI patterns: a boxed buffer with a paired free function.
This is the fiddliest part of the glue, and we may borrow the `cxx` crate selectively here to reduce hand-written `unsafe`.
The decision between hand-rolled glue and `cxx` for this layer is settled empirically during Phase 0 and Phase 1.

## 6. Parity contract

### What must match

The three bit-identical classes of output from Section 3 are non-negotiable: the `encode()` byte format, the pixel index values, and the exact predicates.

### Floating-point reproducibility

Transcendental and `sqrt`-based results can differ at the ULP level between C++ `std::` libm and Rust.
The default stance is to use Rust's native floating-point operations and accept ULP-level differences for any output that is not persisted or shared as a key.
We do not complicate the FFI interface or call back into C++ to make these match.

Two facts keep this safe.
First, the adaptive predicates are robust by construction: `orientation()` returns an exact integer sign because it falls back to exact arithmetic whenever the fast `f64` path is inconclusive, so ULP differences in its `f64` intermediates do not change its result.
Second, in practice Rust's `f64` transcendentals on a given target often resolve to the same system math library as the C++ build, so divergences are expected to be uncommon.

If the golden-vector harness ever shows a ULP difference reaching one of the three persisted or shared outputs, we address it inside the Rust core, for example by calling the platform libm via `libc`.
That remains a Rust-internal detail and does not change the FFI interface.

### Validation strategy

- The existing C++ and Python test suites must stay green at every step.
- A differential and golden-vector harness captures outputs of the current C++ implementation over a fuzz corpus and asserts the Rust core reproduces them bit-for-bit.
  Golden vectors are checked into the repository so the comparison is reproducible and does not require keeping the old C++ implementation linked.
- Rust property tests cover invariants of each ported module.

## 7. Python strategy

### End state

PyO3 plus maturin produces the `lsst.sphgeom._sphgeom` extension that replaces the 27 pybind11 files.
At that point the Python module binds the Rust core directly and no longer depends on the C++ library at all.
The cutover must preserve:

- The full Python class and method API.
- numpy-vectorized `contains` on the region types.
- pickle compatibility, including the `encode`/`decode` bytes and the `__reduce__`/`__getstate__` shapes.
- YAML serialization and the pure-Python layer (`_continue_class`, `_healpixPixelization`, `pixelization_abc`).
- The `lsst-sphgeom` distribution name.

One validation item is flagged for the cutover: confirm that `_continue_class`, which reopens a class to add methods, works against PyO3 heap types.
If it does not, the affected augmentations move into Rust or into a thin Python subclass.

### Migration-time development aid

While the port is in progress the public `_sphgeom` (pybind11) extension is unchanged.
A second extension, `_sphgeom2`, is built by maturin from the PyO3 crate and contains only the functions ported to the Rust core so far.
`lsst/sphgeom/__init__.py` imports the full API from `_sphgeom` as the baseline and, when an environment variable (for example `SPHGEOM_RUST`) is set, overrides the ported names with their `_sphgeom2` equivalents.
With the variable unset the behavior is byte-for-byte the current package; with it set, the existing Python test suite runs the ported predicates through Rust.
The `_sphgeom2` predicates accept the same `UnitVector3d`/`Vector3d` Python objects as their pybind counterparts, reading components through the objects' accessors, so the override is transparent.

## 8. Build-system integration

### During the migration

The public build is left exactly as it is: the existing SCons/eups, CMake, and setuptools builds continue to produce the pybind11-over-C++ package, and the C++ sources are untouched.
The only build added is a development aid: cargo builds `sphgeom-core`, and maturin builds the `_sphgeom2` PyO3 extension from the PyO3 crate.
Developers build `_sphgeom2` alongside the installed package and set the environment variable to route the ported predicates through Rust (Section 7).
Nothing in the public build links against Rust during the migration, so there is no SCons, CMake, or setuptools change and no cargo-into-the-C++-build wiring yet.

### End-state packaging (deferred to the cutover)

Once the Rust core covers the whole API, the public builds switch, and the two products separate cleanly by audience:

- The **PyPI wheel** is built by maturin and is pure Rust plus PyO3, with no C++ at all.
  The Python module binds the Rust core directly, so it never needs `libsphgeom`.
- The **conda-forge / eups** product ships that same pure-Rust Python module **and** a CMake-built `libsphgeom` (the C++ veneer linking the Rust static library) plus the installed headers, for downstream C++ consumers such as afw.
  CMake stays as the C++ build driver and invokes cargo for the Rust static library; SCons is dropped, following the rubinoxide convention of a Makefile plus an `ups/eupspkg.cfg.sh` override.

This dissolves the long-standing wrinkle that pip-installed wheels do not ship a linkable `libsphgeom` or headers: downstream C++ links via conda-forge/eups (where the library and headers live), never via PyPI, and the PyPI wheel is Python-only by design.

### Infrastructure asks

- The developer environment needs a Rust toolchain (cargo, plus the rustfmt and clippy components) and maturin for the migration-time aid. All Rust code is kept `cargo fmt`-clean and `cargo clippy -D warnings`-clean, enforced by pre-commit hooks and CI.
- At the cutover, the lsstsw, eups, and conda build environments must provide a Rust toolchain, and the eups build moves from sconsUtils to a Makefile plus `eupspkg.cfg.sh` override.

## 9. Decomposition and phased roadmap

This is a program, not a single change, so it is decomposed into phases that each get their own implementation plan.
During the migration the public build is never touched (Section 8); each phase grows the Rust core and validates it through the development aid, and the public cutover is a single deliberate late phase.

- Phase 0, development-aid spike: port the orientation predicates to `sphgeom-core` and expose them through the `_sphgeom2` PyO3 extension, validated by golden vectors and by running the existing Python suite through Rust with the environment variable set. The public build is unchanged. Specified in detail in Section 10.
- Phases 1 through N, module-by-module port to the Rust core in dependency order (value types, then intervals and `Box3d`, then `curve` and index math, then the regions `Box`/`Circle`/`ConvexPolygon`/`Ellipse`/`CompoundRegion`, then `RangeSet`, then the pixelizations, then `Chunker`), each extending `_sphgeom2` and validated the same way. A `Region` subclass is the first of these, because it de-risks the opaque-handle pattern the eventual C++ veneer also needs.
- Phase V, C++ veneer track: build the `sphgeom-capi` C ABI and the thin C++ glue against the Rust core, validated as a development build against the C++ test suite. This can proceed in parallel once the core is substantially complete; it does not become public until the cutover.
- Phase C, public cutover: switch the public builds (maturin PyPI wheel for Python, CMake plus cargo for `libsphgeom` under conda-forge/eups), drop SCons, setuptools, and pybind11, remove the now-dead C++ (including `BigInteger`), and confirm downstream builds against the unchanged headers.

## 10. Phase 0 detailed specification

Phase 0 proves the Rust toolchain, the numeric port, and the development-aid validation path on the simplest representative slice: the orientation predicates.
They are chosen because they are exact integer arithmetic, no STL containers, no polymorphism, and no transcendental functions.
The fast `orientation()` path is plain `f64`, and the exact `orientationExact()` fallback needs an arbitrary-precision integer.
Phase 0 does **not** touch the public build: the C++ `orientation.cc` and `BigInteger` are left in place, and the Rust port is reached only through the `_sphgeom2` development extension.

### Big integer dependency

The exact fallback uses the `num-bigint` crate rather than a hand-written or fixed-width integer.
This is the faithful choice: the C++ `orientationExact` sizes its accumulator at 512 32-bit words to absorb wide exponent spreads from general (non-unit) `Vector3d` inputs, so a fixed-width stack integer would impose a tighter bound than the original and amount to a heuristic shortcut.
`num-bigint` is arbitrary precision, pure Rust, and well tested.
Because `orientation()` only falls back to `orientationExact` for near-degenerate inputs, the exact path is rare, so heap allocation there is not a performance concern; the hot path uses no big integer at all.
One benign behavioral difference: `num-bigint` never runs out of capacity, whereas the C++ buffer throws if exceeded; producing the true sign instead of throwing is strictly better and is not expected to affect any real input.

### Scope

- Stand up the cargo workspace with `sphgeom-core` (the Rust algorithms) and `sphgeom-py` (the PyO3 crate that builds the `_sphgeom2` extension).
- Port `orientation()`, `orientationExact()`, and the axis variants into `sphgeom-core`, implementing the exact fallback with `num-bigint`.
- Bind those five predicates in `_sphgeom2` so they accept the same `UnitVector3d`/`Vector3d` Python objects as the pybind versions.
- Add the environment-variable override to `lsst/sphgeom/__init__.py`, a no-op when the variable is unset or `_sphgeom2` is absent.
- Validate with a checked-in golden-vector harness (Rust core against captured current-C++ outputs, bit-identical) and a Python parity test comparing `_sphgeom2` against the pybind predicates.

### Acceptance criteria

- `cargo test` passes, including the golden harness, bit-identical to the captured C++ outputs over the fuzz corpus.
- The Python parity test confirms `_sphgeom2` matches the pybind predicates on a fuzz set; it is skipped when `_sphgeom2` is not built.
- Running the existing Python suite with the environment variable set (orientation served by Rust) passes.
- With the variable unset, the package, its build, and its behavior are unchanged; the default test suite is unaffected.

### Out of scope for Phase 0

- Any change to the public build (no SCons, CMake, or setuptools edits; no cargo linked into `libsphgeom`).
- The C ABI (`sphgeom-capi`/cbindgen), the C++ glue swap, and any change to `src/orientation.cc` or `BigInteger`.
- Any `Region` subclass or polymorphic veneer, and `RangeSet`, pixelizations, intervals, and value-type methods.

## 11. Risks and open questions

### Phase 0 (development aid)

- Building `_sphgeom2` so it sits beside the installed `_sphgeom` without maturin clobbering the shared `lsst` namespace package; the fallback is to build the extension and copy it into the package directory.
- ULP-level floating-point differences are accepted by default for non-persisted outputs; for orientation the returned sign is exact by construction, so this does not arise here, but it is the general stance as later modules are ported.
- Fused-multiply-add contraction in the adaptive predicate's `f64` path only reduces the actual error below the assumed Shewchuk bound, so it does not change the returned sign; noted for completeness.

### Later phases (C++ veneer and public cutover)

- Availability of a Rust toolchain in the LSST build and conda environment at the cutover.
- Whether `_continue_class` works against PyO3 heap types.
- Whether the string, vector, and Region-handle layer is best served by hand-rolled glue or by `cxx`.
- Cross-`.so` RTTI for the exception type once it is thrown from glue rather than from a compiled `.cc`.

## 12. Out of scope for this program

- Changing the mathematical behavior or numerical algorithms of sphgeom.
  The Rust implementation is a faithful port; any behavior change is a separate, deliberate decision.
- Redesigning the public C++ or Python API beyond what the migration mechanically requires.
