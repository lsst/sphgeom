# STC-S C++ migration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move `ConvexPolygon._ivoa_stcs_body` from Python into C++ (with a pybind11 binding) so per-vertex pybind11 round-trips disappear and 64-vertex polygons go from ~1,600 ops/sec to ≥50,000 ops/sec.

**Architecture:** Add a non-virtual public method `ConvexPolygon::toIvoaStcsBody() const` on the C++ `ConvexPolygon` class. Bind it to Python as `_ivoa_stcs_body`. Delete the existing Python `ConvexPolygon._ivoa_stcs_body` in `_continue_class.py`. The base-class `Region.to_ivoa_stcs(frame)` is unchanged — it still calls `self._ivoa_stcs_body()` and inserts the frame keyword in Python. Compound regions continue to dispatch dynamically through Python; once their leaf operands run in C++, they speed up too.

**Tech Stack:** C++17 (`std::to_chars` for shortest-round-trip float formatting, locale-independent), pybind11 (existing infrastructure), Python 3 (unittest), scons (build, via `lsst.sconsUtils`).

**Spec:** `docs/superpowers/specs/2026-05-23-stcs-cpp-migration-design.md`

---

## File map

- **Create**: nothing.
- **Modify** `include/lsst/sphgeom/ConvexPolygon.h` — add the public declaration `std::string toIvoaStcsBody() const;`.
- **Modify** `src/ConvexPolygon.cc` — implement `toIvoaStcsBody`. Add `#include <charconv>` and `#include <string>`.
- **Modify** `python/lsst/sphgeom/_convexPolygon.cc` — bind the new method as `_ivoa_stcs_body`.
- **Modify** `python/lsst/sphgeom/_continue_class.py` — delete the `ConvexPolygon._ivoa_stcs_body` method (the C++ binding takes over).
- **Modify** `tests/test_stcs.py` — add one regression test for bit-exact float round-trip.

The branch is `tickets/DM-53569`. Use `DM-53569` in commit messages.

## Environment / build commands

The LSST stack must be sourced before any build or test. Every shell step in this plan assumes:

```bash
source /sdf/group/rubin/sw/loadLSST.bash
setup lsst_distrib
setup -k -r .
```

Do this once per fresh shell session.

The C++ extension is built with scons. After any change to header/source/binding, rebuild:

```bash
scons
```

(`SConstruct` delegates to `lsst.sconsUtils.scripts.BasicSConstruct("sphgeom")`. `scons` from the repo root is the canonical incremental build.)

If `scons` is unavailable in the environment, fall back to `python setup.py build_ext --inplace`. Stop and ask if neither works.

---

## Task 1: Add a bit-exact round-trip regression test

**Files:**
- Modify: `tests/test_stcs.py`

This task adds a regression test that validates lossless float formatting independently of which language emits the string. Python's f-string default and `std::to_chars` both produce shortest-round-trip output, so the test must pass before *and* after the C++ migration. We add it first to lock in the contract.

- [ ] **Step 1: Write the failing-or-passing test**

Append the following test to the `StcsTestCase` class in `tests/test_stcs.py`, immediately after `test_polygon_body`:

```python
    def test_polygon_roundtrip_precision(self):
        """Lon/lat values round-trip through STC-S without precision loss.

        This locks in shortest-round-trip formatting: when the body string is
        re-parsed with ``float()``, every value must equal the polygon's
        actual stored vertex coordinate exactly (no tolerance).
        """
        vertices = [
            UnitVector3d(LonLat.fromDegrees(12.345678901234567, 34.56789012345678)),
            UnitVector3d(LonLat.fromDegrees(56.789012345678901, 78.90123456789012)),
            UnitVector3d(LonLat.fromDegrees(-23.456789012345678, -45.67890123456789)),
        ]
        poly = ConvexPolygon(vertices)
        body = poly._ivoa_stcs_body()
        tokens = body.split()
        self.assertEqual(tokens[0], "Polygon")
        emitted = [float(t) for t in tokens[1:]]
        # Compare to the polygon's stored vertices (ConvexPolygon may reorder
        # or rotate the input, so compare against its actual getVertices()).
        expected = []
        for v in poly.getVertices():
            ll = LonLat(v)
            expected.append(ll.getLon().asDegrees())
            expected.append(ll.getLat().asDegrees())
        self.assertEqual(len(emitted), len(expected))
        for got, want in zip(emitted, expected, strict=True):
            self.assertEqual(got, want, f"got {got!r} expected {want!r}")
```

- [ ] **Step 2: Run test to verify it passes against the current Python implementation**

Run:

```bash
pytest tests/test_stcs.py::StcsTestCase::test_polygon_roundtrip_precision -v
```

Expected: PASS. (Python's f-string uses shortest-round-trip by default, which is bit-exact.) If it fails, stop and investigate before touching C++.

- [ ] **Step 3: Run the full STC-S test file to confirm no regression**

Run:

```bash
pytest tests/test_stcs.py -v
```

Expected: 20 tests pass (19 existing + 1 new).

- [ ] **Step 4: Commit**

```bash
git add tests/test_stcs.py
git commit -m "DM-53569: add bit-exact round-trip regression test for ConvexPolygon STC-S"
```

---

## Task 2: Add the C++ method

**Files:**
- Modify: `include/lsst/sphgeom/ConvexPolygon.h`
- Modify: `src/ConvexPolygon.cc`

Adds the C++ implementation. The method is not yet exposed to Python — that happens in Task 3 — so no Python tests change in this task. Verification is "it compiles".

- [ ] **Step 1: Declare `toIvoaStcsBody` in the header**

Open `include/lsst/sphgeom/ConvexPolygon.h`. Find the existing public method declarations (around line 107, near `getVertices`). Add the new declaration in the public section, alongside the other accessors. The exact location in the public section doesn't matter; place it after `getVertices` for readability:

```cpp
    /// `toIvoaStcsBody` returns this polygon as the body of an IVOA STC-S
    /// string (the form without a frame keyword), e.g.
    /// `"Polygon 12.0 34.0 14.0 34.0 ..."`. Used by `Region.to_ivoa_stcs`
    /// (Python) which inserts the frame keyword after the shape name.
    ///
    /// Floating-point coordinates are formatted with `std::to_chars`
    /// (shortest round-trip), so every emitted number parses back to the
    /// exact same `double` it was emitted from.
    std::string toIvoaStcsBody() const;
```

- [ ] **Step 2: Implement `toIvoaStcsBody` in the source file**

Open `src/ConvexPolygon.cc`. Add the includes near the existing includes at the top of the file:

```cpp
#include <charconv>
#include <string>
```

(If `<string>` is already included transitively, the duplicate include is harmless.)

Then add the implementation. Place it adjacent to other simple accessors — for example, just after `getCentroid` (around line 309 in the current file), inside the `lsst::sphgeom` namespace block:

```cpp
std::string ConvexPolygon::toIvoaStcsBody() const {
    // Reserve a conservative upper bound: prefix + ~24 chars per number,
    // two numbers per vertex, plus separators. std::to_chars never
    // produces more than ~24 chars for a double in shortest-round-trip mode.
    std::string out;
    out.reserve(8 + _vertices.size() * 52);
    out.append("Polygon");
    char buf[32];
    for (auto const & v : _vertices) {
        LonLat const ll(v);
        double const lon = ll.getLon().asDegrees();
        double const lat = ll.getLat().asDegrees();
        out.push_back(' ');
        auto r1 = std::to_chars(buf, buf + sizeof(buf), lon);
        out.append(buf, r1.ptr - buf);
        out.push_back(' ');
        auto r2 = std::to_chars(buf, buf + sizeof(buf), lat);
        out.append(buf, r2.ptr - buf);
    }
    return out;
}
```

- [ ] **Step 3: Build to verify the C++ compiles**

Run:

```bash
scons
```

Expected: clean build, no warnings or errors. The new symbol `lsst::sphgeom::ConvexPolygon::toIvoaStcsBody` is now linked into the extension.

If you get `'to_chars' is not a member of 'std'` for `double`: the toolchain's libstdc++ predates full C++17 floating-point `to_chars` support. Stop and ask before falling back to `snprintf("%.17g", ...)`, since a fallback changes formatting (`%.17g` is full precision, not shortest round-trip).

- [ ] **Step 4: Run the existing test suite**

Run:

```bash
pytest tests/test_stcs.py -v
```

Expected: still 20 tests pass. The C++ method exists but is not yet bound, so the Python `_ivoa_stcs_body` is still the one being called. This step confirms the rebuild didn't break anything else.

- [ ] **Step 5: Commit**

```bash
git add include/lsst/sphgeom/ConvexPolygon.h src/ConvexPolygon.cc
git commit -m "DM-53569: add ConvexPolygon::toIvoaStcsBody C++ implementation"
```

---

## Task 3: Bind to Python and remove the Python override

**Files:**
- Modify: `python/lsst/sphgeom/_convexPolygon.cc`
- Modify: `python/lsst/sphgeom/_continue_class.py`

This is the swap: pybind11 starts exposing the C++ method as `_ivoa_stcs_body`, and the Python override is deleted in the same commit so dispatch goes straight to C++.

- [ ] **Step 1: Add the pybind11 binding**

Open `python/lsst/sphgeom/_convexPolygon.cc`. Find the line that binds `getVertices` (around line 64):

```cpp
    cls.def("getVertices", &ConvexPolygon::getVertices);
```

Add the new binding immediately after it:

```cpp
    cls.def("_ivoa_stcs_body", &ConvexPolygon::toIvoaStcsBody);
```

- [ ] **Step 2: Remove the Python override**

Open `python/lsst/sphgeom/_continue_class.py`. Find the `ConvexPolygon` class block (currently around lines 291–306). It contains `to_ivoa_pos` and `_ivoa_stcs_body`. Delete only the `_ivoa_stcs_body` method, keeping `to_ivoa_pos` and the class decoration intact. The block should change from:

```python
@_continueClass
class ConvexPolygon:  # noqa: F811
    """A rectangle in spherical coordinate space that contains its boundary."""

    def to_ivoa_pos(self) -> str:
        # Docstring inherited.
        coords = (LonLat(v) for v in self.getVertices())
        coord_strings = [f"{c.getLon().asDegrees()} {c.getLat().asDegrees()}" for c in coords]

        return f"POLYGON {' '.join(coord_strings)}"

    def _ivoa_stcs_body(self) -> str:
        # Docstring inherited.
        coords = (LonLat(v) for v in self.getVertices())
        coord_strings = [f"{c.getLon().asDegrees()} {c.getLat().asDegrees()}" for c in coords]
        return f"Polygon {' '.join(coord_strings)}"
```

to:

```python
@_continueClass
class ConvexPolygon:  # noqa: F811
    """A rectangle in spherical coordinate space that contains its boundary."""

    def to_ivoa_pos(self) -> str:
        # Docstring inherited.
        coords = (LonLat(v) for v in self.getVertices())
        coord_strings = [f"{c.getLon().asDegrees()} {c.getLat().asDegrees()}" for c in coords]

        return f"POLYGON {' '.join(coord_strings)}"
```

- [ ] **Step 3: Rebuild**

Run:

```bash
scons
```

Expected: clean rebuild. The binding file changed, so `_convexPolygon.os` is recompiled and the extension is relinked.

- [ ] **Step 4: Run the STC-S test suite**

Run:

```bash
pytest tests/test_stcs.py -v
```

Expected: all 20 tests pass, including the bit-exact round-trip test from Task 1. The implementation language has changed; the behaviour has not.

- [ ] **Step 5: Run the full sphgeom suite**

Run:

```bash
pytest tests/ -v
```

Expected: 219 tests pass (218 pre-migration + 1 added in Task 1).

- [ ] **Step 6: Lint and format check**

Run:

```bash
ruff check python/lsst/sphgeom/_continue_class.py tests/test_stcs.py
ruff format --check python/lsst/sphgeom/_continue_class.py tests/test_stcs.py
```

Expected: no errors, files already formatted.

- [ ] **Step 7: Commit**

```bash
git add python/lsst/sphgeom/_convexPolygon.cc python/lsst/sphgeom/_continue_class.py
git commit -m "DM-53569: bind ConvexPolygon::toIvoaStcsBody as _ivoa_stcs_body"
```

---

## Task 4: Verify the benchmark target

**Files:** none modified (verification only).

The acceptance criterion from the spec is **64-vertex polygon ≥ 50,000 ops/sec** (≥30× the pre-migration 1,590 ops/sec). This task runs the benchmark and confirms.

- [ ] **Step 1: Run the benchmark**

Run:

```bash
python bench/bench_stcs.py
```

Record the output. The line of interest is:

```
ConvexPolygon[ 64].to_ivoa_stcs()              <ops/sec>     <us/op>
```

- [ ] **Step 2: Check the acceptance threshold**

The 64-vertex throughput must be ≥ 50,000 ops/sec.

- If ≥ 50,000 ops/sec: target met. Continue to Step 3.
- If 16,000–50,000 ops/sec (10×–30× speedup): below target but reasonable. Stop and report numbers; ask the user whether to investigate further or accept.
- If < 16,000 ops/sec (less than 10×): something is wrong — likely the binding still routes through the Python override, or `_continue_class.py` was not fully rebuilt and the old Python method still wins MRO. Stop and investigate. Don't proceed.

- [ ] **Step 3: Report numbers**

In your end-of-task summary to the user, include the before/after table for at least the 64-vertex row, and ideally all rows. Example format:

| workload | before (ops/sec) | after (ops/sec) | speedup |
|---|---:|---:|---:|
| ConvexPolygon[64].to_ivoa_stcs() | 1,590 | <new> | <ratio>× |
| Union(Poly64, Poly64).to_ivoa_stcs() | 796 | <new> | <ratio>× |

The "before" numbers come from this plan's spec (`docs/superpowers/specs/2026-05-23-stcs-cpp-migration-design.md`). No commit is required for this task — the verification is operational.

---

## Task 5: News fragment

**Files:**
- Create: `doc/changes/DM-53569.perf.md`

Repository convention (verified: `doc/changes/DM-54274.bugfix.md`) uses `.md` files with one short sentence. Towncrier categories include `perf` for "Performance Enhancement" (see `[tool.towncrier]` in `pyproject.toml`).

- [ ] **Step 1: Write the fragment**

Create `doc/changes/DM-53569.perf.md` with:

```markdown
Reimplemented ``ConvexPolygon._ivoa_stcs_body`` in C++. 64-vertex polygons now serialise to STC-S at well over 50,000 ops/sec, a 30+× speedup that makes bulk conversion of millions of regions tractable.
```

- [ ] **Step 2: Commit**

```bash
git add doc/changes/DM-53569.perf.md
git commit -m "DM-53569: news fragment for ConvexPolygon STC-S C++ migration"
```
