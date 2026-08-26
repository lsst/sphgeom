# sphgeom Rust migration — Phase 0 (orientation dev aid) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port the `orientation`, `orientationExact`, `orientationX/Y/Z` predicates to a Rust core crate and expose them through an optional PyO3 extension (`_sphgeom2`) selected by an environment variable, so the existing Python test suite can validate the Rust implementation — **without changing the public build at all**.

**Architecture:** A cargo workspace under `rust/` holds `sphgeom-core` (pure-Rust algorithms) and `sphgeom-py` (a PyO3 crate built by maturin as the `_sphgeom2` extension). `lsst/sphgeom/__init__.py` imports the full API from the existing pybind `_sphgeom` as the baseline and, when `SPHGEOM_RUST` is set, overrides the ported predicate names with their `_sphgeom2` equivalents. The public C++ sources, headers, pybind bindings, and all build files are untouched.

**Tech Stack:** Rust (edition 2024), `num-bigint` for the exact fallback, `pyo3` + `maturin` for the dev extension, existing Python test suite for validation.

**Design reference:** `docs/superpowers/specs/2026-06-13-sphgeom-rust-migration-design.md`

---

## Background the engineer needs

`orientation(a, b, c)` returns the sign (+1/0/-1) of the determinant of the 3x3 matrix whose rows are the three vectors — the robust geometric orientation predicate. The current C++ lives in `src/orientation.cc`, declared in `include/lsst/sphgeom/orientation.h`, and is exposed to Python by `python/lsst/sphgeom/_orientation.cc`. Five entry points:

- `orientationExact(Vector3d a, b, c)` — exact arbitrary-precision computation. Decomposes each `double` into a signed 53-bit integer mantissa and an exponent (via `frexp`), forms the six triple-products of the determinant exactly as big integers, aligns radix points by shifting, sums, and returns the sign. Accepts general (non-unit) vectors.
- `orientation(UnitVector3d a, b, c)` — fast path: computes the determinant in `f64`, returns early when provably away from zero (Shewchuk error bound), short-circuits degenerate inputs, then falls back to `orientationExact`.
- `orientationX/Y/Z(b, c)` — `orientation` with the first argument fixed to the X/Y/Z axis; tighter 2x2 bound, falling back to `orientationExact` with the axis vector.

**Why the returned sign is bit-stable across C++ and Rust:** the fast path only returns early when the `f64` determinant is provably correct within its error bound; otherwise it defers to the exact integer computation. ULP-level differences in the `f64` intermediates therefore cannot change the returned integer, so the golden-vector tests (which compare the returned integers) are immune to floating-point reproducibility concerns.

**Why `num-bigint`:** the C++ accumulator buffer is `uint32_t[512]` (≈16 kbit), deliberately generous to absorb wide exponent spreads from general `Vector3d` inputs. A fixed-width stack integer would impose a tighter bound than the original — a heuristic shortcut. `num-bigint` is arbitrary precision; since `orientationExact` is only the rare fallback, its heap allocation is not a performance concern.

**Python exposes components as methods:** `UnitVector3d`/`Vector3d` expose `x()`, `y()`, `z()` as methods (e.g. `v.x()`), so `_sphgeom2` reads components by calling those methods on whatever object it is handed — making it a transparent drop-in for the pybind predicates.

**What this plan does NOT touch:** `src/*.cc`, `include/`, `python/lsst/sphgeom/_*.cc`, `BigInteger`, `CMakeLists.txt`, `SConscript`, `setup.py`. With `SPHGEOM_RUST` unset, the package builds and behaves exactly as before.

---

## File structure

Created:
- `rust/Cargo.toml` — workspace (members: `sphgeom-core`, `sphgeom-py`).
- `rust/rust-toolchain.toml` — pins the toolchain channel.
- `rust/sphgeom-core/{Cargo.toml,src/lib.rs,src/bits.rs,src/orientation.rs,tests/golden.rs}` — Rust algorithms and golden test.
- `rust/sphgeom-py/{Cargo.toml,pyproject.toml,src/lib.rs}` — the `_sphgeom2` PyO3 extension.
- `rust/.cargo/config.toml` — macOS linker config so plain `cargo` can build the PyO3 cdylib.
- `rust/tools/gen_orientation_golden.py` — golden-vector generator.
- `rust/tools/install_sphgeom2.py` — builds `_sphgeom2` and copies it into the package.
- `tests/data/orientation_golden_unit.csv`, `tests/data/orientation_exact_golden_general.csv` — committed golden vectors.
- `tests/test_orientation_rust.py` — Python parity test (skipped if `_sphgeom2` absent).
- `.github/workflows/rust-devaid.yaml` — CI job for the dev aid.
- `doc/changes/DM-XXXXX.misc.rst` — news fragment.

Modified:
- `python/lsst/sphgeom/__init__.py` — add the `SPHGEOM_RUST` override (no-op when unset/absent).
- `.pre-commit-config.yaml` — add `cargo fmt --check` and `cargo clippy -D warnings` hooks.
- `.gitignore` — ignore `rust/target/` and the built `_sphgeom2` extension.

Untouched: everything in the public build (`src/`, `include/`, `python/lsst/sphgeom/_*.cc`, `CMakeLists.txt`, `lib/SConscript`, `setup.py`, `BigInteger`).

---

## Task 1: Capture golden vectors from the current implementation

Run before any code change so the golden vectors reflect the current C++ behavior. Doubles are stored as their raw `u64` bit patterns so the Rust side reconstructs identical values with `f64::from_bits`.

**Files:**
- Create: `rust/tools/gen_orientation_golden.py`
- Create (generated, then committed): `tests/data/orientation_golden_unit.csv`, `tests/data/orientation_exact_golden_general.csv`

- [ ] **Step 1: Write the generator script**

Create `rust/tools/gen_orientation_golden.py`:

```python
# Generate golden orientation vectors from the CURRENT lsst.sphgeom build.
# Run this BEFORE porting orientation to Rust. Doubles are emitted as their
# IEEE-754 u64 bit patterns so the Rust side can reconstruct identical values.
import csv
import random
import struct

from lsst.sphgeom import (
    UnitVector3d,
    Vector3d,
    orientation,
    orientationExact,
    orientationX,
    orientationY,
    orientationZ,
)

SEED = 20260613
N_UNIT = 20000
N_GENERAL = 20000


def bits(x: float) -> int:
    return struct.unpack("<Q", struct.pack("<d", x))[0]


def unit_rows(rng):
    axes = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)]
    curated = [(a, b, c) for a in axes for b in axes for c in axes]
    rand = []
    for _ in range(N_UNIT):
        a = (rng.uniform(-1, 1), rng.uniform(-1, 1), rng.uniform(-1, 1))
        b = (rng.uniform(-1, 1), rng.uniform(-1, 1), rng.uniform(-1, 1))
        if rng.random() < 0.3:
            j = 1e-15
            c = (a[0] + rng.uniform(-j, j), a[1] + rng.uniform(-j, j), a[2] + rng.uniform(-j, j))
        else:
            c = (rng.uniform(-1, 1), rng.uniform(-1, 1), rng.uniform(-1, 1))
        rand.append((a, b, c))
    return curated + rand


def main():
    rng = random.Random(SEED)

    with open("tests/data/orientation_golden_unit.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["a0", "a1", "a2", "b0", "b1", "b2", "c0", "c1", "c2",
                    "orientation", "orientationExact",
                    "orientationX", "orientationY", "orientationZ"])
        for a, b, c in unit_rows(rng):
            ua, ub, uc = UnitVector3d(*a), UnitVector3d(*b), UnitVector3d(*c)
            w.writerow([bits(ua.x()), bits(ua.y()), bits(ua.z()),
                        bits(ub.x()), bits(ub.y()), bits(ub.z()),
                        bits(uc.x()), bits(uc.y()), bits(uc.z()),
                        orientation(ua, ub, uc),
                        orientationExact(Vector3d(ua), Vector3d(ub), Vector3d(uc)),
                        orientationX(ub, uc),
                        orientationY(ub, uc),
                        orientationZ(ub, uc)])

    with open("tests/data/orientation_exact_golden_general.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["a0", "a1", "a2", "b0", "b1", "b2", "c0", "c1", "c2", "orientationExact"])
        for _ in range(N_GENERAL):
            def vec():
                s = 10.0 ** rng.uniform(-30, 30)
                return (rng.uniform(-1, 1) * s, rng.uniform(-1, 1) * s, rng.uniform(-1, 1) * s)
            va, vb, vc = Vector3d(*vec()), Vector3d(*vec()), Vector3d(*vec())
            w.writerow([bits(va.x()), bits(va.y()), bits(va.z()),
                        bits(vb.x()), bits(vb.y()), bits(vb.z()),
                        bits(vc.x()), bits(vc.y()), bits(vc.z()),
                        orientationExact(va, vb, vc)])


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Run the generator against the current build**

Run (with `lsst.sphgeom` importable):

```bash
mkdir -p tests/data
python rust/tools/gen_orientation_golden.py
wc -l tests/data/orientation_golden_unit.csv tests/data/orientation_exact_golden_general.csv
```

Expected: each CSV has a header plus tens of thousands of rows (≈20028 and ≈20001).

- [ ] **Step 3: Commit**

```bash
git add rust/tools/gen_orientation_golden.py tests/data/orientation_golden_unit.csv tests/data/orientation_exact_golden_general.csv
git commit -m "DM-XXXXX: capture orientation golden vectors from current C++"
```

---

## Task 2: Cargo workspace and `sphgeom-core` `frexp`

**Files:**
- Create: `rust/Cargo.toml`, `rust/rust-toolchain.toml`, `rust/sphgeom-core/Cargo.toml`, `rust/sphgeom-core/src/lib.rs`, `rust/sphgeom-core/src/bits.rs`
- Modify: `.gitignore`

- [ ] **Step 1: Create the workspace and crate manifests**

`rust/Cargo.toml` (only `sphgeom-core` for now; `sphgeom-py` is added to the members list in Task 5 when that crate is created, so `cargo` does not fail on a missing member in between):

```toml
[workspace]
resolver = "2"
members = ["sphgeom-core"]
```

`rust/rust-toolchain.toml`:

```toml
[toolchain]
channel = "stable"
```

`rust/sphgeom-core/Cargo.toml`:

```toml
[package]
name = "sphgeom-core"
version = "0.1.0"
edition = "2024"
license = "BSD-3-Clause OR GPL-3.0-or-later"

[dependencies]
num-bigint = "0.4"
```

`rust/sphgeom-core/src/lib.rs`:

```rust
//! Pure-Rust core implementation of sphgeom algorithms.

pub mod bits;
pub mod orientation;
```

- [ ] **Step 2: Write `frexp` and its tests**

`rust/sphgeom-core/src/bits.rs`:

```rust
//! Exact bit-level decomposition of IEEE-754 doubles.

/// Decompose `x` into `(frac, exp)` such that `x == frac * 2f64.powi(exp)` and
/// `frac` is in `[0.5, 1.0)` in magnitude (or zero). Matches C `frexp`.
pub fn frexp(x: f64) -> (f64, i32) {
    if x == 0.0 || x.is_nan() || x.is_infinite() {
        return (x, 0);
    }
    let bits = x.to_bits();
    let raw_exp = ((bits >> 52) & 0x7ff) as i32;
    if raw_exp == 0 {
        // Subnormal: scale into the normal range, then correct the exponent.
        let (frac, exp) = frexp(x * 18014398509481984.0); // x * 2^54
        return (frac, exp - 54);
    }
    let exp = raw_exp - 1022;
    // Replace the biased exponent field with 1022 so the value lands in [0.5, 1).
    let frac_bits = (bits & !(0x7ffu64 << 52)) | (1022u64 << 52);
    (f64::from_bits(frac_bits), exp)
}

#[cfg(test)]
mod tests {
    use super::frexp;

    fn check(x: f64, frac: f64, exp: i32) {
        let (f, e) = frexp(x);
        assert_eq!(f, frac, "frac mismatch for {x}");
        assert_eq!(e, exp, "exp mismatch for {x}");
        assert_eq!(f * 2f64.powi(e), x, "reconstruct mismatch for {x}");
    }

    #[test]
    fn known_values() {
        check(6.0, 0.75, 3);
        check(-6.0, -0.75, 3);
        check(1.0, 0.5, 1);
        check(0.5, 0.5, 0);
        check(0.0, 0.0, 0);
    }

    #[test]
    fn subnormal_reconstructs_exactly() {
        let x = f64::from_bits(1); // smallest positive subnormal
        let (f, e) = frexp(x);
        assert_eq!(f * 2f64.powi(e), x);
        assert!((0.5..1.0).contains(&f.abs()));
    }
}
```

- [ ] **Step 3: Add an `orientation` module stub so the crate compiles**

`rust/sphgeom-core/src/orientation.rs`:

```rust
//! Orientation predicates. Implementations land in the next tasks.
```

- [ ] **Step 4: Run the tests**

Run:

```bash
cargo test --manifest-path rust/Cargo.toml -p sphgeom-core
```

Expected: `known_values` and `subnormal_reconstructs_exactly` PASS; the crate builds.

- [ ] **Step 5: Ignore build artifacts**

Append to `.gitignore`:

```
rust/target/
python/lsst/sphgeom/_sphgeom2*.so
python/lsst/sphgeom/_sphgeom2*.pyd
```

- [ ] **Step 6: Add rustfmt and clippy pre-commit hooks**

Append this block to the existing `.pre-commit-config.yaml` (under the top-level `repos:` list, after the ruff repo). It uses the system `cargo`, runs only when Rust files change, and enforces standard formatting and a clippy-clean tree (warnings are errors):

```yaml
  - repo: local
    hooks:
      - id: cargo-fmt
        name: cargo fmt (check)
        entry: cargo fmt --manifest-path rust/Cargo.toml --all -- --check
        language: system
        files: ^rust/.*\.rs$
        pass_filenames: false
      - id: cargo-clippy
        name: cargo clippy
        entry: cargo clippy --manifest-path rust/Cargo.toml --all-targets --all-features -- -D warnings
        language: system
        files: ^rust/.*\.rs$
        pass_filenames: false
```

- [ ] **Step 7: Verify the Rust tree is formatted and clippy-clean**

Run:

```bash
cargo fmt --manifest-path rust/Cargo.toml --all -- --check
cargo clippy --manifest-path rust/Cargo.toml --all-targets --all-features -- -D warnings
```

Expected: both succeed with no output/diff. If `cargo fmt --check` reports a diff, run `cargo fmt --manifest-path rust/Cargo.toml --all` and re-check. Fix any clippy findings rather than allowing them.

- [ ] **Step 8: Commit**

```bash
git add rust/Cargo.toml rust/rust-toolchain.toml rust/sphgeom-core .gitignore .pre-commit-config.yaml
git commit -m "DM-XXXXX: add cargo workspace, exact frexp, and Rust pre-commit hooks"
```

---

## Task 3: `orientation_exact` in `sphgeom-core`

**Files:**
- Modify: `rust/sphgeom-core/src/orientation.rs`

- [ ] **Step 1: Write the implementation and tests**

Replace `rust/sphgeom-core/src/orientation.rs` with:

```rust
//! Orientation predicates.

use num_bigint::{BigInt, Sign};

use crate::bits::frexp;

// 2^53: scales a frexp fraction in [0.5, 1) to an integer in [2^52, 2^53).
const SCALE: f64 = 9007199254740992.0;

/// Exact product of three doubles, as `(mantissa, exponent)` with value
/// `mantissa * 2^exponent`.
fn product(d0: f64, d1: f64, d2: f64) -> (BigInt, i32) {
    let (m0, e0) = frexp(d0);
    let (m1, e1) = frexp(d1);
    let (m2, e2) = frexp(d2);
    let i0 = (m0 * SCALE) as i64;
    let i1 = (m1 * SCALE) as i64;
    let i2 = (m2 * SCALE) as i64;
    let mantissa = BigInt::from(i0) * BigInt::from(i1) * BigInt::from(i2);
    (mantissa, e0 + e1 + e2 - 159)
}

/// Exact orientation of three (not necessarily normalized) vectors.
pub fn orientation_exact(a: [f64; 3], b: [f64; 3], c: [f64; 3]) -> i32 {
    let mut products: [(BigInt, i32); 6] = [
        product(a[0], b[1], c[2]),
        product(a[0], b[2], c[1]),
        product(a[1], b[2], c[0]),
        product(a[1], b[0], c[2]),
        product(a[2], b[0], c[1]),
        product(a[2], b[1], c[0]),
    ];
    // Subtracted terms of the determinant.
    products[1].0 = -&products[1].0;
    products[3].0 = -&products[3].0;
    products[5].0 = -&products[5].0;
    // Sort by exponent, descending.
    products.sort_by(|x, y| y.1.cmp(&x.1));
    // Accumulate, aligning radix points by shifting before each add.
    let mut acc = products[0].0.clone();
    for i in 1..6 {
        let shift = (products[i - 1].1 - products[i].1) as usize;
        acc = (acc << shift) + &products[i].0;
    }
    match acc.sign() {
        Sign::Minus => -1,
        Sign::NoSign => 0,
        Sign::Plus => 1,
    }
}

#[cfg(test)]
mod tests {
    use super::orientation_exact;

    #[test]
    fn axis_triples() {
        assert_eq!(orientation_exact([1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]), 1);
        assert_eq!(orientation_exact([0.0, 1.0, 0.0], [1.0, 0.0, 0.0], [0.0, 0.0, 1.0]), -1);
    }

    #[test]
    fn coplanar_is_zero() {
        assert_eq!(orientation_exact([1.0, 0.0, 0.0], [2.0, 0.0, 0.0], [3.0, 1.0, 0.0]), 0);
        assert_eq!(orientation_exact([1.0, 1.0, 1.0], [2.0, 2.0, 2.0], [5.0, 0.0, 0.0]), 0);
    }

    #[test]
    fn wide_exponent_spread() {
        assert_eq!(orientation_exact([1e30, 0.0, 0.0], [0.0, 1e-30, 0.0], [0.0, 0.0, 1e10]), 1);
    }
}
```

- [ ] **Step 2: Run the tests**

Run:

```bash
cargo test --manifest-path rust/Cargo.toml -p sphgeom-core orientation::tests
```

Expected: `axis_triples`, `coplanar_is_zero`, `wide_exponent_spread` PASS.

- [ ] **Step 3: Commit**

```bash
git add rust/sphgeom-core/src/orientation.rs
git commit -m "DM-XXXXX: implement exact orientation predicate with num-bigint"
```

---

## Task 4: `orientation` and `orientation_x/y/z`, validated against golden vectors

**Files:**
- Modify: `rust/sphgeom-core/src/orientation.rs`
- Create: `rust/sphgeom-core/tests/golden.rs`

- [ ] **Step 1: Add the fast path and axis variants**

Insert into `rust/sphgeom-core/src/orientation.rs` immediately before the `#[cfg(test)]` module:

```rust
fn eq(u: [f64; 3], v: [f64; 3]) -> bool {
    u[0] == v[0] && u[1] == v[1] && u[2] == v[2]
}

fn neg(v: [f64; 3]) -> [f64; 3] {
    [-v[0], -v[1], -v[2]]
}

/// Orientation of three unit vectors. Exact: the fast path only returns early
/// when provably correct, otherwise it falls back to `orientation_exact`.
pub fn orientation(a: [f64; 3], b: [f64; 3], c: [f64; 3]) -> i32 {
    const RELATIVE_ERROR: f64 = 5.6e-16;
    const MAX_ABSOLUTE_ERROR: f64 = 1.7e-15;
    const MIN_ABSOLUTE_ERROR: f64 = 4.0e-307;

    let bycz = b[1] * c[2];
    let bzcy = b[2] * c[1];
    let bzcx = b[2] * c[0];
    let bxcz = b[0] * c[2];
    let bxcy = b[0] * c[1];
    let bycx = b[1] * c[0];
    let determinant =
        a[0] * (bycz - bzcy) + a[1] * (bzcx - bxcz) + a[2] * (bxcy - bycx);
    if determinant > MAX_ABSOLUTE_ERROR {
        return 1;
    } else if determinant < -MAX_ABSOLUTE_ERROR {
        return -1;
    }
    let permanent = a[0].abs() * (bycz.abs() + bzcy.abs())
        + a[1].abs() * (bzcx.abs() + bxcz.abs())
        + a[2].abs() * (bxcy.abs() + bycx.abs());
    let max_error = RELATIVE_ERROR * permanent + MIN_ABSOLUTE_ERROR;
    if determinant > max_error {
        return 1;
    } else if determinant < -max_error {
        return -1;
    }
    if eq(a, b) || eq(b, c) || eq(a, c) || eq(a, neg(b)) || eq(b, neg(c)) || eq(a, neg(c)) {
        return 0;
    }
    orientation_exact(a, b, c)
}

fn orientation_2x2(ab: f64, ba: f64) -> i32 {
    const RELATIVE_ERROR: f64 = 1.12e-16;
    const MAX_ABSOLUTE_ERROR: f64 = 1.12e-16;
    const MIN_ABSOLUTE_ERROR: f64 = 1.0e-307;

    let determinant = ab - ba;
    if determinant > MAX_ABSOLUTE_ERROR {
        return 1;
    } else if determinant < -MAX_ABSOLUTE_ERROR {
        return -1;
    }
    let permanent = ab.abs() + ba.abs();
    let max_error = RELATIVE_ERROR * permanent + MIN_ABSOLUTE_ERROR;
    if determinant > max_error {
        return 1;
    } else if determinant < -max_error {
        return -1;
    }
    0
}

/// Equivalent to `orientation([1,0,0], b, c)`.
pub fn orientation_x(b: [f64; 3], c: [f64; 3]) -> i32 {
    let o = orientation_2x2(b[1] * c[2], b[2] * c[1]);
    if o != 0 { o } else { orientation_exact([1.0, 0.0, 0.0], b, c) }
}

/// Equivalent to `orientation([0,1,0], b, c)`.
pub fn orientation_y(b: [f64; 3], c: [f64; 3]) -> i32 {
    let o = orientation_2x2(b[2] * c[0], b[0] * c[2]);
    if o != 0 { o } else { orientation_exact([0.0, 1.0, 0.0], b, c) }
}

/// Equivalent to `orientation([0,0,1], b, c)`.
pub fn orientation_z(b: [f64; 3], c: [f64; 3]) -> i32 {
    let o = orientation_2x2(b[0] * c[1], b[1] * c[0]);
    if o != 0 { o } else { orientation_exact([0.0, 0.0, 1.0], b, c) }
}
```

- [ ] **Step 2: Write the golden-vector integration test**

Create `rust/sphgeom-core/tests/golden.rs`:

```rust
//! Differential test: the Rust core must reproduce the orientation values
//! captured from the current C++ implementation (bit-for-bit on inputs,
//! exactly on outputs).

use std::path::PathBuf;

use sphgeom_core::orientation::{
    orientation, orientation_exact, orientation_x, orientation_y, orientation_z,
};

fn repo_root() -> PathBuf {
    let mut p = PathBuf::from(env!("CARGO_MANIFEST_DIR")); // rust/sphgeom-core
    p.pop();
    p.pop();
    p
}

fn f(bits: &str) -> f64 {
    f64::from_bits(bits.trim().parse::<u64>().expect("u64 bits"))
}

#[test]
fn golden_unit() {
    let path = repo_root().join("tests/data/orientation_golden_unit.csv");
    let text = std::fs::read_to_string(&path).expect("read golden_unit");
    for (n, line) in text.lines().enumerate().skip(1) {
        let r: Vec<&str> = line.split(',').collect();
        let a = [f(r[0]), f(r[1]), f(r[2])];
        let b = [f(r[3]), f(r[4]), f(r[5])];
        let c = [f(r[6]), f(r[7]), f(r[8])];
        assert_eq!(orientation(a, b, c), r[9].trim().parse().unwrap(), "orientation row {n}");
        assert_eq!(orientation_exact(a, b, c), r[10].trim().parse().unwrap(), "orientationExact row {n}");
        assert_eq!(orientation_x(b, c), r[11].trim().parse().unwrap(), "orientationX row {n}");
        assert_eq!(orientation_y(b, c), r[12].trim().parse().unwrap(), "orientationY row {n}");
        assert_eq!(orientation_z(b, c), r[13].trim().parse().unwrap(), "orientationZ row {n}");
    }
}

#[test]
fn golden_exact_general() {
    let path = repo_root().join("tests/data/orientation_exact_golden_general.csv");
    let text = std::fs::read_to_string(&path).expect("read golden_general");
    for (n, line) in text.lines().enumerate().skip(1) {
        let r: Vec<&str> = line.split(',').collect();
        let a = [f(r[0]), f(r[1]), f(r[2])];
        let b = [f(r[3]), f(r[4]), f(r[5])];
        let c = [f(r[6]), f(r[7]), f(r[8])];
        assert_eq!(orientation_exact(a, b, c), r[9].trim().parse().unwrap(), "orientationExact row {n}");
    }
}
```

- [ ] **Step 3: Run the golden tests**

Run:

```bash
cargo test --manifest-path rust/Cargo.toml -p sphgeom-core --test golden
```

Expected: `golden_unit` and `golden_exact_general` PASS — bit-for-bit parity with the current C++ on tens of thousands of inputs. A golden failure means the Rust port is wrong; fix `orientation.rs`, never edit the golden data.

- [ ] **Step 4: Commit**

```bash
git add rust/sphgeom-core/src/orientation.rs rust/sphgeom-core/tests/golden.rs
git commit -m "DM-XXXXX: implement adaptive orientation and validate against golden vectors"
```

---

## Task 5: `sphgeom-py` PyO3 extension (`_sphgeom2`)

Bind the five predicates so they accept the same `UnitVector3d`/`Vector3d` Python objects as the pybind versions, reading components via their `x()`/`y()`/`z()` methods.

**Files:**
- Create: `rust/sphgeom-py/Cargo.toml`, `rust/sphgeom-py/pyproject.toml`, `rust/sphgeom-py/src/lib.rs`, `rust/.cargo/config.toml`

- [ ] **Step 1: Add the crate to the workspace and create its manifests**

Update `rust/Cargo.toml` to add the new member:

```toml
[workspace]
resolver = "2"
members = ["sphgeom-core", "sphgeom-py"]
```

`rust/sphgeom-py/Cargo.toml`:

```toml
[package]
name = "sphgeom-py"
version = "0.1.0"
edition = "2024"
license = "BSD-3-Clause OR GPL-3.0-or-later"

[lib]
name = "_sphgeom2"
crate-type = ["cdylib"]

[dependencies]
sphgeom-core = { path = "../sphgeom-core" }
pyo3 = { version = "0.22", features = ["extension-module"] }
```

`rust/sphgeom-py/pyproject.toml`:

```toml
[build-system]
requires = ["maturin>=1.5,<2.0"]
build-backend = "maturin"

[project]
name = "sphgeom-rust-devaid"
version = "0.0.0"
requires-python = ">=3.10"

[tool.maturin]
module-name = "lsst.sphgeom._sphgeom2"
```

`rust/.cargo/config.toml` (lets plain `cargo build`/`cargo test` link the PyO3 `extension-module` cdylib on macOS, where unresolved Python symbols must be deferred to load time; harmless for the other crates and ignored by maturin, which sets this itself):

```toml
[target.x86_64-apple-darwin]
rustflags = ["-C", "link-arg=-undefined", "-C", "link-arg=dynamic_lookup"]

[target.aarch64-apple-darwin]
rustflags = ["-C", "link-arg=-undefined", "-C", "link-arg=dynamic_lookup"]
```

- [ ] **Step 2: Write the bindings**

`rust/sphgeom-py/src/lib.rs`:

```rust
//! Development-aid PyO3 extension (`_sphgeom2`) exposing the Rust orientation
//! predicates so the existing Python suite can validate them against pybind.

use pyo3::prelude::*;

use sphgeom_core::orientation as core;

/// Read three components from a UnitVector3d/Vector3d Python object, which
/// expose `x()`, `y()`, `z()` as methods.
fn xyz(v: &Bound<'_, PyAny>) -> PyResult<[f64; 3]> {
    Ok([
        v.call_method0("x")?.extract()?,
        v.call_method0("y")?.extract()?,
        v.call_method0("z")?.extract()?,
    ])
}

#[pyfunction]
fn orientation(a: &Bound<'_, PyAny>, b: &Bound<'_, PyAny>, c: &Bound<'_, PyAny>) -> PyResult<i32> {
    Ok(core::orientation(xyz(a)?, xyz(b)?, xyz(c)?))
}

#[pyfunction]
#[pyo3(name = "orientationExact")]
fn orientation_exact(a: &Bound<'_, PyAny>, b: &Bound<'_, PyAny>, c: &Bound<'_, PyAny>) -> PyResult<i32> {
    Ok(core::orientation_exact(xyz(a)?, xyz(b)?, xyz(c)?))
}

#[pyfunction]
#[pyo3(name = "orientationX")]
fn orientation_x(b: &Bound<'_, PyAny>, c: &Bound<'_, PyAny>) -> PyResult<i32> {
    Ok(core::orientation_x(xyz(b)?, xyz(c)?))
}

#[pyfunction]
#[pyo3(name = "orientationY")]
fn orientation_y(b: &Bound<'_, PyAny>, c: &Bound<'_, PyAny>) -> PyResult<i32> {
    Ok(core::orientation_y(xyz(b)?, xyz(c)?))
}

#[pyfunction]
#[pyo3(name = "orientationZ")]
fn orientation_z(b: &Bound<'_, PyAny>, c: &Bound<'_, PyAny>) -> PyResult<i32> {
    Ok(core::orientation_z(xyz(b)?, xyz(c)?))
}

#[pymodule]
fn _sphgeom2(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(orientation, m)?)?;
    m.add_function(wrap_pyfunction!(orientation_exact, m)?)?;
    m.add_function(wrap_pyfunction!(orientation_x, m)?)?;
    m.add_function(wrap_pyfunction!(orientation_y, m)?)?;
    m.add_function(wrap_pyfunction!(orientation_z, m)?)?;
    Ok(())
}
```

- [ ] **Step 3: Type-check the crate**

Run:

```bash
cargo check --manifest-path rust/Cargo.toml -p sphgeom-py
```

Expected: checks with no errors. (Use `cargo check` here, not `cargo build`: the final `cdylib` link of a PyO3 `extension-module` is done by maturin in Task 6, which sets up the interpreter linkage correctly across platforms.)

- [ ] **Step 4: Commit**

```bash
git add rust/Cargo.toml rust/sphgeom-py rust/.cargo/config.toml
git commit -m "DM-XXXXX: add _sphgeom2 PyO3 extension for the orientation predicates"
```

---

## Task 6: Build `_sphgeom2` into the package

A small helper builds the extension with maturin and copies it into `python/lsst/sphgeom/` so the package (on `PYTHONPATH` or editable-installed) picks it up. This avoids maturin clobbering the shared `lsst` namespace.

**Files:**
- Create: `rust/tools/install_sphgeom2.py`

- [ ] **Step 1: Write the install helper**

Create `rust/tools/install_sphgeom2.py`:

```python
#!/usr/bin/env python
"""Build the _sphgeom2 development extension and place it in the package.

Builds rust/sphgeom-py with maturin and copies the resulting _sphgeom2
extension into python/lsst/sphgeom so PYTHONPATH/editable installs pick it up.
"""
import glob
import os
import subprocess
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CRATE = os.path.join(ROOT, "rust", "sphgeom-py")
DEST = os.path.join(ROOT, "python", "lsst", "sphgeom")
OUT = os.path.join(CRATE, "target", "wheels")


def main():
    subprocess.run(["maturin", "build", "--release", "--out", OUT], cwd=CRATE, check=True)
    wheel = sorted(glob.glob(os.path.join(OUT, "*.whl")))[-1]
    with zipfile.ZipFile(wheel) as z:
        for name in z.namelist():
            base = os.path.basename(name)
            if base.startswith("_sphgeom2") and base.endswith((".so", ".pyd")):
                with open(os.path.join(DEST, base), "wb") as f:
                    f.write(z.read(name))
                print("installed", os.path.join(DEST, base))


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Build and smoke-test (requires `maturin`: `pip install maturin`)**

Run (with the package already built/importable, e.g. `PYTHONPATH=python` after a normal build):

```bash
python rust/tools/install_sphgeom2.py
PYTHONPATH=python python -c "import lsst.sphgeom._sphgeom2 as m; from lsst.sphgeom import UnitVector3d as U; print(m.orientation(U(1,0,0), U(0,1,0), U(0,0,1)))"
```

Expected: the helper prints the installed `_sphgeom2` path, and the command prints `1` — orientation computed in Rust, called from Python via `_sphgeom2`.

> If maturin places the extension differently, the helper still finds it by name inside the built wheel. The extension file (`python/lsst/sphgeom/_sphgeom2*.so`) is gitignored (Task 2).

- [ ] **Step 3: Commit**

```bash
git add rust/tools/install_sphgeom2.py
git commit -m "DM-XXXXX: add helper to build _sphgeom2 into the package"
```

---

## Task 7: Environment-variable override and Python parity test

**Files:**
- Modify: `python/lsst/sphgeom/__init__.py`
- Create: `tests/test_orientation_rust.py`

- [ ] **Step 1: Add the `SPHGEOM_RUST` override to `__init__.py`**

Add `import os` and `import warnings` at the top of `python/lsst/sphgeom/__init__.py` (after the license header), and insert the override block immediately before `PixelizationABC.register(Pixelization)`:

```python
if os.environ.get("SPHGEOM_RUST"):
    # Development aid: route the ported predicates through the Rust core, built
    # as the optional _sphgeom2 extension. No effect when unset or absent.
    try:
        from ._sphgeom2 import (  # noqa: F401
            orientation,
            orientationExact,
            orientationX,
            orientationY,
            orientationZ,
        )
    except ImportError as exc:  # pragma: no cover
        warnings.warn(f"SPHGEOM_RUST is set but _sphgeom2 is unavailable: {exc}")
```

(`__init__.py` is excluded from ruff, so import placement is not linted; placing `import os`/`import warnings` at the top still follows the project's top-level-import preference.)

- [ ] **Step 2: Write the parity test**

Create `tests/test_orientation_rust.py`:

```python
"""Parity test: the Rust _sphgeom2 predicates must match the pybind ones.

Skipped when the optional _sphgeom2 development extension is not built. This
imports the pybind predicates directly from _sphgeom so the comparison is
independent of the SPHGEOM_RUST override.
"""
import random

import pytest

from lsst.sphgeom import UnitVector3d, Vector3d
from lsst.sphgeom._sphgeom import (
    orientation as orientation_cpp,
    orientationExact as orientation_exact_cpp,
    orientationX as orientation_x_cpp,
    orientationY as orientation_y_cpp,
    orientationZ as orientation_z_cpp,
)

rust = pytest.importorskip("lsst.sphgeom._sphgeom2")


def test_orientation_parity():
    rng = random.Random(20260613)
    for _ in range(20000):
        a = UnitVector3d(rng.uniform(-1, 1), rng.uniform(-1, 1), rng.uniform(-1, 1))
        b = UnitVector3d(rng.uniform(-1, 1), rng.uniform(-1, 1), rng.uniform(-1, 1))
        c = UnitVector3d(rng.uniform(-1, 1), rng.uniform(-1, 1), rng.uniform(-1, 1))
        va, vb, vc = Vector3d(a), Vector3d(b), Vector3d(c)
        assert rust.orientation(a, b, c) == orientation_cpp(a, b, c)
        assert rust.orientationExact(va, vb, vc) == orientation_exact_cpp(va, vb, vc)
        assert rust.orientationX(b, c) == orientation_x_cpp(b, c)
        assert rust.orientationY(b, c) == orientation_y_cpp(b, c)
        assert rust.orientationZ(b, c) == orientation_z_cpp(b, c)
```

- [ ] **Step 3: Run the parity test and the suite both ways**

Run (with the package built and `_sphgeom2` installed from Task 6):

```bash
PYTHONPATH=python pytest tests/test_orientation_rust.py -v
PYTHONPATH=python pytest tests/ -q
SPHGEOM_RUST=1 PYTHONPATH=python pytest tests/ -q
```

Expected: the parity test PASSES; the suite PASSES with the variable unset (unchanged behavior); and PASSES with `SPHGEOM_RUST=1` (orientation served by Rust).

- [ ] **Step 4: Commit**

```bash
git add python/lsst/sphgeom/__init__.py tests/test_orientation_rust.py
git commit -m "DM-XXXXX: add SPHGEOM_RUST override and orientation parity test"
```

---

## Task 8: CI for the development aid

A self-contained workflow that builds the package, builds `_sphgeom2`, and runs the Rust core tests plus the parity test and the suite-through-Rust. It does not touch the existing `build.yaml`/`cmake.yaml`.

**Files:**
- Create: `.github/workflows/rust-devaid.yaml`

- [ ] **Step 1: Write the workflow**

Create `.github/workflows/rust-devaid.yaml`:

```yaml
name: rust-dev-aid

on:
  push:
    branches:
      - main
  pull_request:

jobs:
  rust-dev-aid:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v6
        with:
          fetch-depth: 0

      - name: Set up Python
        uses: actions/setup-python@v6
        with:
          python-version: "3.12"

      - name: Set up Rust
        uses: dtolnay/rust-toolchain@stable
        with:
          components: clippy, rustfmt

      - name: Install build dependencies
        run: |
          python -m pip install --upgrade pip
          pip install maturin pytest numpy "pybind11[global]" hpgeom pyyaml

      - name: rustfmt (check)
        run: cargo fmt --manifest-path rust/Cargo.toml --all -- --check

      - name: clippy
        run: cargo clippy --manifest-path rust/Cargo.toml --all-targets --all-features -- -D warnings

      - name: Cargo tests (Rust core + golden vectors)
        run: cargo test --manifest-path rust/Cargo.toml -p sphgeom-core

      - name: Build the public package (unchanged pybind11 + C++)
        run: pip install -e .

      - name: Build the _sphgeom2 dev extension
        run: python rust/tools/install_sphgeom2.py

      - name: Parity test
        run: pytest tests/test_orientation_rust.py -v

      - name: Run the suite through Rust
        run: SPHGEOM_RUST=1 pytest tests/ -q
```

- [ ] **Step 2: Validate the workflow YAML**

Run:

```bash
python -c "import yaml; yaml.safe_load(open('.github/workflows/rust-devaid.yaml')); print('yaml ok')"
```

Expected: prints `yaml ok`.

- [ ] **Step 3: Commit**

```bash
git add .github/workflows/rust-devaid.yaml
git commit -m "DM-XXXXX: CI for the Rust orientation development aid"
```

---

## Task 9: News fragment

**Files:**
- Create: `doc/changes/DM-XXXXX.misc.rst`

- [ ] **Step 1: Add the fragment**

Create `doc/changes/DM-XXXXX.misc.rst`:

```
Added a development-only Rust implementation of the orientation predicates, exposed as the optional ``_sphgeom2`` extension and selected by the ``SPHGEOM_RUST`` environment variable. The default build and behavior are unchanged.
```

- [ ] **Step 2: Commit**

```bash
git add doc/changes/DM-XXXXX.misc.rst
git commit -m "DM-XXXXX: news fragment for the orientation Rust development aid"
```

---

## Notes for the executor

- Replace `DM-XXXXX` throughout with the real Jira ticket once assigned.
- The crates use Rust edition 2024, which requires a Rust toolchain of 1.85 or newer; any recent `rustup` stable provides it (`rust-toolchain.toml` pins `channel = "stable"`).
- Crate versions (`num-bigint = "0.4"`, `pyo3 = "0.22"`, `maturin>=1.5`) are starting points; if a newer compatible release is current, update and keep the API usage shown here in sync.
- Do not edit the golden CSVs to make a test pass. A golden failure means the Rust port diverges from the reference C++ and the Rust code must be fixed.
- Nothing here changes the public build. With `SPHGEOM_RUST` unset and `_sphgeom2` absent, the package is byte-for-byte the current one.
- All Rust code must be `cargo fmt`-clean (standard formatting) and `cargo clippy --all-targets --all-features -- -D warnings`-clean. This is enforced by the pre-commit hooks (Task 2) and the CI workflow (Task 8); keep every Rust commit clean rather than deferring fixes.
- Treat the build/test commands as the source of truth (per the user's Rust workflow rule: cargo/build output is ground truth, not editor diagnostics).
- The C ABI / C++ veneer, the public-build cutover, and the maturin-PyPI / conda-`libsphgeom` packaging are deliberately out of scope here; they are separate later phases in the design.
```
