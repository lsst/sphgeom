# STC-S C++ migration design

**Status:** approved 2026-05-23
**Ticket:** DM-53569 follow-up

## Motivation

The pure-Python `ConvexPolygon._ivoa_stcs_body` added under DM-53569 is the throughput bottleneck for STC-S serialisation. Measured on `sdf` (LSST scipipe 13.0.0, Python 3.13):

| workload | ops/sec | µs/op |
|---|---:|---:|
| 4-vertex polygon → STC-S | 24,789 | 40.3 |
| 8-vertex polygon | 12,486 | 80.1 |
| 16-vertex polygon | 6,294 | 158.9 |
| 64-vertex polygon | 1,590 | 628.9 |
| 256-vertex polygon | 403 | 2,481 |
| Circle | 95,183 | 10.5 |
| Ellipse | 38,738 | 25.8 |
| Union(Poly64, Poly64) | 796 | 1,256 |

Cost scales at ~10 µs per polygon vertex. Profiling confirms this is dominated by the five pybind11 round-trips per vertex: `LonLat(v)`, `LonLat.getLon`, `Angle.asDegrees`, `LonLat.getLat`, `Angle.asDegrees`. For 10⁷ regions of ~64 vertices the Python implementation takes ~100 minutes; a C++ implementation that touches the C++ vertex vector directly should cut this by an order of magnitude.

Circle, Ellipse, Box, UnionRegion, and IntersectionRegion stay in Python. Circle and Ellipse are already fast enough for foreseeable workloads, and the compound regions are dominated by their leaf-operand cost — fixing `ConvexPolygon` fixes them too.

## Approach

Add a C++ method `ConvexPolygon::toIvoaStcsBody()` that returns the STC-S body string for the polygon (the form without a frame keyword). Bind it to Python as `_ivoa_stcs_body` and remove the Python override in `_continue_class.py`. The base-class `Region.to_ivoa_stcs(frame)` (defined in `_continue_class.py`) is unchanged: it still calls `self._ivoa_stcs_body()` and inserts the frame keyword in Python. Frame insertion is microseconds and not worth pushing down.

`ConvexPolygon::toIvoaStcsBody` is **not** virtual on `Region`. We are migrating one class; a per-class method is simpler than a virtual on `Region` with default behaviour that other types would have to opt out of. Compound regions continue to call `operand._ivoa_stcs_body()` and Python dispatch picks up the C++ method automatically when the operand is a `ConvexPolygon`.

## C++ implementation

**Header:** `include/lsst/sphgeom/ConvexPolygon.h` — add a public method declaration:

```cpp
std::string toIvoaStcsBody() const;
```

**Source:** `src/ConvexPolygon.cc` — implementation iterates `_vertices` directly. Each vertex is converted to `LonLat` via the existing `LonLat(UnitVector3d)` constructor; longitude and latitude are emitted in degrees. The output format is `"Polygon <lon0> <lat0> <lon1> <lat1> ..."` with single-space separators.

Number formatting uses `std::to_chars(buf, end, value)` (C++17) — shortest round-trip, locale-independent, no allocations in the formatter itself. Whole-valued doubles emit without a trailing `.0` (e.g. `12` rather than `12.0`); the IVOA STC-S grammar treats numbers as floats regardless of decimal point, and STILTS parses both forms identically.

The implementation builds the result into a `std::string` reserved to a conservative upper bound to avoid reallocations during append.

## Python binding

**File:** `python/lsst/sphgeom/_convexPolygon.cc` — add one line in the existing `py::class_<ConvexPolygon, Region>` block:

```cpp
.def("_ivoa_stcs_body", &ConvexPolygon::toIvoaStcsBody)
```

The leading underscore matches the existing Python helper name and keeps the public Python API (`to_ivoa_stcs(frame)`) on the `Region` base class.

## Python side

`python/lsst/sphgeom/_continue_class.py` — delete the `ConvexPolygon._ivoa_stcs_body` definition (the C++ binding now provides it). The `ConvexPolygon` class block in `_continue_class.py` continues to host `to_ivoa_pos`. The base-class `Region.to_ivoa_stcs(frame)` is unchanged.

## Tests

`tests/test_stcs.py` already uses `assert_stcs_equal`, which tokenises and compares numeric tokens with floating-point tolerance, so existing tests pass regardless of whether `12` or `12.0` is emitted.

Add one new test that covers lossless precision: build a polygon whose vertex coordinates carry significant precision (e.g. `12.345678901234567`), serialise, and parse the lon/lat tokens back with `float()`. Assert the parsed values equal the originals exactly (no `assertAlmostEqual` — use `assertEqual` to confirm round-trip is bit-exact).

Run the existing `tests/` suite end-to-end after the change. Expected: 218 tests still pass.

## Verification

Re-run `bench/bench_stcs.py` after the migration. **Acceptance:** 64-vertex polygon throughput ≥ 50,000 ops/sec (≥30× current 1,590 ops/sec). Record before/after numbers in the commit message of the binding change.

If the measured speedup is less than 10×, treat that as a signal to investigate further before declaring the migration complete (likely cause: residual pybind11 overhead in `_ivoa_stcs_body` dispatch, or the bound method returning a `std::string` that's being copied unnecessarily).

## Risk

Small. One new method, one new binding, one Python deletion. Public API is unchanged. Output format changes are within what STC-S consumers accept and within what the existing test suite tolerates.

## Out of scope

- Migrating Circle, Box, Ellipse, UnionRegion, IntersectionRegion to C++.
- Pushing `to_ivoa_stcs(frame)` (frame insertion) into C++.
- Adding an STC-S parser (this work is generation only; parsing is not on the roadmap).
