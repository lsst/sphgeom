# STC-S generation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `Region.to_ivoa_stcs(frame="ICRS")` method to `lsst.sphgeom` so regions can be serialised as IVOA STC-S strings (for consumers like `stilts mocshape`).

**Architecture:** Mirror the existing `to_ivoa_pos` pattern in `python/lsst/sphgeom/_continue_class.py`. Each region class gets a public `to_ivoa_stcs(frame)` and a private `_ivoa_stcs_body()` helper. The body returns the shape without the leading frame keyword; `to_ivoa_stcs` inserts the frame after the operator/shape name. Compound regions (Union/Intersection) call `_ivoa_stcs_body()` recursively on operands so the frame appears once at the outermost level.

**Tech Stack:** Python (pybind11 bindings into C++). Tests use `unittest`. No new dependencies.

**Spec:** `docs/superpowers/specs/2026-05-22-stcs-generation-design.md`

---

## File map

- **Modify** `python/lsst/sphgeom/_continue_class.py` — add `Ellipse`, `UnionRegion`, `IntersectionRegion` to the existing import list; add `to_ivoa_stcs` / `_ivoa_stcs_body` to the `Region` base class and to each concrete subclass (`Circle`, `Box`, `ConvexPolygon`, `Ellipse`, `UnionRegion`, `IntersectionRegion`).
- **Create** `tests/test_stcs.py` — unittest module mirroring `tests/test_ivoa.py`.
- **Create** `doc/changes/DM-53569.feature.rst` — towncrier news fragment.

The branch is `tickets/DM-53569`. Use `DM-53569` in commit messages.

---

## Task 1: Test scaffold and base-class method

**Files:**
- Modify: `python/lsst/sphgeom/_continue_class.py:38` (import block) and `python/lsst/sphgeom/_continue_class.py:165-173` (Region base class).
- Create: `tests/test_stcs.py`

This task adds the `to_ivoa_stcs` method on `Region` (raises `NotImplementedError`), grows the import list to cover the new classes we'll touch, and creates a test file with one passing test for the base-class behaviour.

- [ ] **Step 1: Write the failing test**

Create `tests/test_stcs.py` with:

```python
# This file is part of sphgeom.
#
# Developed for the LSST Data Management System.
# This product includes software developed by the LSST Project
# (http://www.lsst.org).
# See the COPYRIGHT file at the top-level directory of this distribution
# for details of code ownership.
#
# This software is dual licensed under the GNU General Public License and also
# under a 3-clause BSD license. Recipients may choose which of these licenses
# to use; please see the files gpl-3.0.txt and/or bsd_license.txt,
# respectively.  If you choose the GPL option then the following text applies
# (but note that there is still no warranty even if you opt for BSD instead):
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import math
import unittest

from lsst.sphgeom import (
    Angle,
    Box,
    Circle,
    ConvexPolygon,
    Ellipse,
    IntersectionRegion,
    LonLat,
    Region,
    UnionRegion,
    UnitVector3d,
)


class StcsTestCase(unittest.TestCase):
    """Test STC-S string generation."""

    def assert_stcs_equal(self, stcs1: str, stcs2: str):
        """Compare two STC-S strings, treating numeric tokens with a
        floating-point tolerance and string tokens (shape/operator/frame
        keywords, parentheses) by exact match.
        """
        toks1 = stcs1.replace("(", " ( ").replace(")", " ) ").split()
        toks2 = stcs2.replace("(", " ( ").replace(")", " ) ").split()
        self.assertEqual(len(toks1), len(toks2), f"{stcs1!r} vs {stcs2!r}")
        for t1, t2 in zip(toks1, toks2, strict=True):
            try:
                f1 = float(t1)
                f2 = float(t2)
            except ValueError:
                self.assertEqual(t1, t2)
            else:
                self.assertAlmostEqual(f1, f2, places=6)

    def test_base_region_raises(self):
        """The Region base class itself raises NotImplementedError."""
        # Region is abstract, but the method is defined on it; we exercise the
        # default implementation via a subclass that does not override it.
        # Box is one such case (see test_box_not_supported), but the base
        # behaviour is independently asserted by calling Region.to_ivoa_stcs
        # bound to a Region instance via the unbound method.
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)),
                        Angle.fromDegrees(1.0))
        with self.assertRaises(NotImplementedError):
            Region.to_ivoa_stcs(circle)


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `pytest tests/test_stcs.py -v`
Expected: FAIL with `AttributeError` on `Region.to_ivoa_stcs` (the attribute does not exist yet).

- [ ] **Step 3: Add the base-class method and grow the import**

In `python/lsst/sphgeom/_continue_class.py`, change the import line:

```python
from ._sphgeom import Angle, Box, Circle, ConvexPolygon, LonLat, Region, UnitVector3d
```

to:

```python
from ._sphgeom import (
    Angle,
    Box,
    Circle,
    ConvexPolygon,
    Ellipse,
    IntersectionRegion,
    LonLat,
    Region,
    UnionRegion,
    UnitVector3d,
)
```

Then in the same file, immediately after the existing `to_ivoa_pos` method on `Region` (around line 173, inside `class Region`), append:

```python
    def to_ivoa_stcs(self, frame: str = "ICRS") -> str:
        """Represent the region as an IVOA STC-S string.

        Parameters
        ----------
        frame : `str`, optional
            STC-S coordinate frame keyword (e.g. ``"ICRS"``, ``"FK5"``,
            ``"GALACTIC"``). Emitted verbatim. Defaults to ``"ICRS"``.

        Returns
        -------
        stcs : `str`
            The region in STC-S format.

        Notes
        -----
        See
        http://www.ivoa.net/Documents/Notes/STC-S/20091030/NOTE-STC-S-1.33-20091030.html
        for the format definition. Supported region types are ``Circle``,
        ``Polygon``, ``Ellipse``, and the ``Union`` / ``Intersection``
        compound operators. ``Box`` regions cannot be converted directly
        because STC-S has no latitude-parallel range region.
        """
        raise NotImplementedError("This region can not be converted to an IVOA STC-S string.")

    def _ivoa_stcs_body(self) -> str:
        """Return the STC-S body of this region without the frame keyword.

        Used internally by compound regions so the frame keyword is emitted
        only once at the outermost level.
        """
        raise NotImplementedError("This region can not be converted to an IVOA STC-S string.")
```

- [ ] **Step 4: Run test to verify it passes**

Run: `pytest tests/test_stcs.py -v`
Expected: PASS (one test).

- [ ] **Step 5: Commit**

```bash
git add tests/test_stcs.py python/lsst/sphgeom/_continue_class.py
git commit -m "DM-53569: scaffold to_ivoa_stcs on Region base class"
```

---

## Task 2: Circle

**Files:**
- Modify: `python/lsst/sphgeom/_continue_class.py` (Circle class block, currently ~lines 176-186).
- Modify: `tests/test_stcs.py`.

- [ ] **Step 1: Write the failing tests**

Append to `StcsTestCase` in `tests/test_stcs.py`:

```python
    def test_circle(self):
        """Circle round-trips through STC-S."""
        circle = Circle(
            UnitVector3d(LonLat.fromDegrees(180.0, 30.0)),
            Angle.fromDegrees(2.0),
        )
        self.assert_stcs_equal(
            circle.to_ivoa_stcs(),
            "Circle ICRS 180.0 30.0 2.0",
        )

    def test_circle_frame_argument(self):
        """The frame argument is emitted verbatim."""
        circle = Circle(
            UnitVector3d(LonLat.fromDegrees(180.0, 30.0)),
            Angle.fromDegrees(2.0),
        )
        self.assert_stcs_equal(
            circle.to_ivoa_stcs(frame="GALACTIC"),
            "Circle GALACTIC 180.0 30.0 2.0",
        )

    def test_circle_body(self):
        """The internal body helper omits the frame keyword."""
        circle = Circle(
            UnitVector3d(LonLat.fromDegrees(180.0, 30.0)),
            Angle.fromDegrees(2.0),
        )
        self.assert_stcs_equal(
            circle._ivoa_stcs_body(),
            "Circle 180.0 30.0 2.0",
        )
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `pytest tests/test_stcs.py -v`
Expected: 3 new tests FAIL with `NotImplementedError`.

- [ ] **Step 3: Implement `Circle.to_ivoa_stcs` and `_ivoa_stcs_body`**

In `python/lsst/sphgeom/_continue_class.py`, inside the `class Circle:` block (near line 180), add the two methods (keep the existing `to_ivoa_pos` method as-is):

```python
    def _ivoa_stcs_body(self) -> str:
        # Docstring inherited.
        center = LonLat(self.getCenter())
        lon = center.getLon().asDegrees()
        lat = center.getLat().asDegrees()
        rad = self.getOpeningAngle().asDegrees()
        return f"Circle {lon} {lat} {rad}"

    def to_ivoa_stcs(self, frame: str = "ICRS") -> str:
        # Docstring inherited.
        center = LonLat(self.getCenter())
        lon = center.getLon().asDegrees()
        lat = center.getLat().asDegrees()
        rad = self.getOpeningAngle().asDegrees()
        return f"Circle {frame} {lon} {lat} {rad}"
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `pytest tests/test_stcs.py -v`
Expected: All 4 tests PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/test_stcs.py python/lsst/sphgeom/_continue_class.py
git commit -m "DM-53569: implement to_ivoa_stcs for Circle"
```

---

## Task 3: ConvexPolygon

**Files:**
- Modify: `python/lsst/sphgeom/_continue_class.py` (ConvexPolygon class block, currently ~lines 208-217).
- Modify: `tests/test_stcs.py`.

- [ ] **Step 1: Write the failing tests**

Append to `StcsTestCase`:

```python
    def test_polygon(self):
        """Polygon emits Polygon <frame> followed by lon/lat pairs."""
        vertices = [
            UnitVector3d(LonLat.fromDegrees(12.0, 34.0)),
            UnitVector3d(LonLat.fromDegrees(14.0, 34.0)),
            UnitVector3d(LonLat.fromDegrees(14.0, 36.0)),
            UnitVector3d(LonLat.fromDegrees(12.0, 36.0)),
        ]
        poly = ConvexPolygon(vertices)
        # ConvexPolygon may rotate the vertex order; compare token counts and
        # numeric values pairwise after extracting (lon, lat) pairs.
        stcs = poly.to_ivoa_stcs()
        tokens = stcs.split()
        self.assertEqual(tokens[0], "Polygon")
        self.assertEqual(tokens[1], "ICRS")
        coords = [float(t) for t in tokens[2:]]
        self.assertEqual(len(coords), 8)
        # Recover the 4 (lon, lat) pairs as a set of rounded tuples and compare.
        emitted = {(round(coords[i], 6), round(coords[i + 1], 6))
                   for i in range(0, 8, 2)}
        expected = {(12.0, 34.0), (14.0, 34.0), (14.0, 36.0), (12.0, 36.0)}
        self.assertEqual(emitted, expected)

    def test_polygon_body(self):
        """ConvexPolygon body helper omits the frame keyword."""
        vertices = [
            UnitVector3d(LonLat.fromDegrees(12.0, 34.0)),
            UnitVector3d(LonLat.fromDegrees(14.0, 34.0)),
            UnitVector3d(LonLat.fromDegrees(14.0, 36.0)),
            UnitVector3d(LonLat.fromDegrees(12.0, 36.0)),
        ]
        poly = ConvexPolygon(vertices)
        body = poly._ivoa_stcs_body()
        tokens = body.split()
        self.assertEqual(tokens[0], "Polygon")
        # No "ICRS" or any non-numeric token after "Polygon".
        for tok in tokens[1:]:
            float(tok)  # raises ValueError if non-numeric
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `pytest tests/test_stcs.py -v`
Expected: 2 new tests FAIL with `NotImplementedError`.

- [ ] **Step 3: Implement `ConvexPolygon.to_ivoa_stcs` and `_ivoa_stcs_body`**

In `python/lsst/sphgeom/_continue_class.py`, inside the `class ConvexPolygon:` block, add:

```python
    def _ivoa_stcs_body(self) -> str:
        # Docstring inherited.
        coords = (LonLat(v) for v in self.getVertices())
        coord_strings = [f"{c.getLon().asDegrees()} {c.getLat().asDegrees()}" for c in coords]
        return f"Polygon {' '.join(coord_strings)}"

    def to_ivoa_stcs(self, frame: str = "ICRS") -> str:
        # Docstring inherited.
        coords = (LonLat(v) for v in self.getVertices())
        coord_strings = [f"{c.getLon().asDegrees()} {c.getLat().asDegrees()}" for c in coords]
        return f"Polygon {frame} {' '.join(coord_strings)}"
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `pytest tests/test_stcs.py -v`
Expected: All tests PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/test_stcs.py python/lsst/sphgeom/_continue_class.py
git commit -m "DM-53569: implement to_ivoa_stcs for ConvexPolygon"
```

---

## Task 4: Ellipse

**Files:**
- Modify: `python/lsst/sphgeom/_continue_class.py` (add a new `class Ellipse:` block at end-of-file).
- Modify: `tests/test_stcs.py`.

The position angle of the first ellipse axis is computed from the transform matrix. `getTransformMatrix().getRow(0)` is the first axis direction in 3-vector form; `getRow(2)` is the centre. PA east-of-north at the centre is `atan2(axis · east, axis · north)`, where:

- `east  = (-sin(λ),  cos(λ),       0)`
- `north = (-sin(φ)·cos(λ), -sin(φ)·sin(λ), cos(φ))`

with `(λ, φ)` the centre's longitude and latitude in radians. The result is normalised to `[0, 360)` for predictable output. This was verified by experimentally constructing `Ellipse(centre, alpha=2°, beta=1°, orientation=45°)` and confirming the formula recovers `45.0`.

- [ ] **Step 1: Write the failing tests**

Append to `StcsTestCase`:

```python
    def test_ellipse(self):
        """Ellipse round-trips through STC-S, including position angle."""
        center = UnitVector3d(LonLat.fromDegrees(180.0, 30.0))
        ellipse = Ellipse(
            center,
            Angle.fromDegrees(2.0),  # alpha
            Angle.fromDegrees(1.0),  # beta
            Angle.fromDegrees(45.0),  # orientation (PA)
        )
        self.assert_stcs_equal(
            ellipse.to_ivoa_stcs(),
            "Ellipse ICRS 180.0 30.0 2.0 1.0 45.0",
        )

    def test_ellipse_pa_zero(self):
        """Ellipse with orientation=0 emits PA close to 0."""
        center = UnitVector3d(LonLat.fromDegrees(45.0, 0.0))
        ellipse = Ellipse(
            center,
            Angle.fromDegrees(2.0),
            Angle.fromDegrees(1.0),
            Angle.fromDegrees(0.0),
        )
        tokens = ellipse.to_ivoa_stcs().split()
        # Last token is the position angle.
        pa = float(tokens[-1])
        # PA may come back as 0 or 180 (axis is unsigned).
        self.assertTrue(
            math.isclose(pa, 0.0, abs_tol=1e-6) or math.isclose(pa, 180.0, abs_tol=1e-6),
            f"unexpected PA {pa}",
        )

    def test_ellipse_body(self):
        """Ellipse body helper omits the frame keyword."""
        center = UnitVector3d(LonLat.fromDegrees(180.0, 30.0))
        ellipse = Ellipse(
            center,
            Angle.fromDegrees(2.0),
            Angle.fromDegrees(1.0),
            Angle.fromDegrees(45.0),
        )
        self.assert_stcs_equal(
            ellipse._ivoa_stcs_body(),
            "Ellipse 180.0 30.0 2.0 1.0 45.0",
        )

    def test_ellipse_full_raises(self):
        """A full ellipse cannot be represented as STC-S."""
        center = UnitVector3d(LonLat.fromDegrees(0.0, 0.0))
        ellipse = Ellipse(center, Angle.fromDegrees(180.0))  # full
        self.assertTrue(ellipse.isFull())
        with self.assertRaises(ValueError):
            ellipse.to_ivoa_stcs()

    def test_ellipse_empty_raises(self):
        """An empty ellipse cannot be represented as STC-S."""
        # Default-constructed Ellipse is empty.
        ellipse = Ellipse()
        self.assertTrue(ellipse.isEmpty())
        with self.assertRaises(ValueError):
            ellipse.to_ivoa_stcs()
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `pytest tests/test_stcs.py -v`
Expected: 5 new tests FAIL (`NotImplementedError` for the first three; the empty/full tests fail too because the unsupported error path doesn't exist yet).

- [ ] **Step 3: Implement Ellipse support**

At the end of `python/lsst/sphgeom/_continue_class.py`, add a new block (after `class ConvexPolygon`):

```python
@_continueClass
class Ellipse:  # noqa: F811
    """An elliptical region on the unit sphere."""

    def _ivoa_stcs_body(self) -> str:
        # Docstring inherited.
        if self.isEmpty():
            raise ValueError("Empty Ellipse has no STC-S representation.")
        if self.isFull():
            raise ValueError("Full Ellipse has no STC-S representation.")
        if self.isGreatCircle():
            raise ValueError("Great-circle Ellipse has no STC-S representation.")
        center = LonLat(self.getCenter())
        lon = center.getLon().asDegrees()
        lat = center.getLat().asDegrees()
        alpha = self.getAlpha().asDegrees()
        beta = self.getBeta().asDegrees()
        pa = _ellipse_position_angle_degrees(self)
        return f"Ellipse {lon} {lat} {alpha} {beta} {pa}"

    def to_ivoa_stcs(self, frame: str = "ICRS") -> str:
        # Docstring inherited.
        body = self._ivoa_stcs_body()
        # Insert frame after the first token ("Ellipse").
        head, _, tail = body.partition(" ")
        return f"{head} {frame} {tail}"
```

Add a module-level helper near the other helpers (around line 95, after `_inf_to_lon`):

```python
def _ellipse_position_angle_degrees(ellipse) -> float:
    """Compute the position angle (east of north, degrees) of the first
    ellipse axis at its centre.

    Returned value is normalised to ``[0, 360)``.
    """
    matrix = ellipse.getTransformMatrix()
    axis = matrix.getRow(0)  # first axis direction (Vector3d)
    center = LonLat(ellipse.getCenter())
    lam = center.getLon().asRadians()
    phi = center.getLat().asRadians()
    sin_lam = math.sin(lam)
    cos_lam = math.cos(lam)
    sin_phi = math.sin(phi)
    cos_phi = math.cos(phi)
    # Local east and north 3-vectors at the centre.
    east_dot = -sin_lam * axis.x() + cos_lam * axis.y()
    north_dot = (
        -sin_phi * cos_lam * axis.x()
        - sin_phi * sin_lam * axis.y()
        + cos_phi * axis.z()
    )
    pa_deg = math.degrees(math.atan2(east_dot, north_dot))
    return pa_deg % 360.0
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `pytest tests/test_stcs.py -v`
Expected: All tests PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/test_stcs.py python/lsst/sphgeom/_continue_class.py
git commit -m "DM-53569: implement to_ivoa_stcs for Ellipse"
```

---

## Task 5: Box raises NotImplementedError

**Files:**
- Modify: `python/lsst/sphgeom/_continue_class.py` (Box class block, currently ~lines 189-205).
- Modify: `tests/test_stcs.py`.

- [ ] **Step 1: Write the failing test**

Append to `StcsTestCase`:

```python
    def test_box_not_supported(self):
        """Box explicitly raises NotImplementedError with a helpful message."""
        box = Box(
            LonLat.fromDegrees(1.0, 2.0),
            LonLat.fromDegrees(5.0, 6.0),
        )
        with self.assertRaises(NotImplementedError) as cm:
            box.to_ivoa_stcs()
        # The error message should mention an alternative for callers.
        self.assertIn("Polygon", str(cm.exception))

    def test_box_body_not_supported(self):
        """Box body helper also raises NotImplementedError."""
        box = Box(
            LonLat.fromDegrees(1.0, 2.0),
            LonLat.fromDegrees(5.0, 6.0),
        )
        with self.assertRaises(NotImplementedError):
            box._ivoa_stcs_body()
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `pytest tests/test_stcs.py -v`
Expected: Both tests FAIL — the messages from the inherited base method don't mention "Polygon".

- [ ] **Step 3: Override `to_ivoa_stcs` and `_ivoa_stcs_body` on Box**

In `python/lsst/sphgeom/_continue_class.py`, inside the `class Box:` block, add (after the existing `to_ivoa_pos`):

```python
    def _ivoa_stcs_body(self) -> str:
        # Docstring inherited.
        raise NotImplementedError(
            "Box cannot be converted to STC-S directly because STC-S has no "
            "latitude-parallel range region; build a Polygon (ConvexPolygon) "
            "from this Box if an STC-S representation is required."
        )

    def to_ivoa_stcs(self, frame: str = "ICRS") -> str:
        # Docstring inherited.
        raise NotImplementedError(
            "Box cannot be converted to STC-S directly because STC-S has no "
            "latitude-parallel range region; build a Polygon (ConvexPolygon) "
            "from this Box if an STC-S representation is required."
        )
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `pytest tests/test_stcs.py -v`
Expected: All tests PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/test_stcs.py python/lsst/sphgeom/_continue_class.py
git commit -m "DM-53569: explicitly reject Box -> STC-S conversion"
```

---

## Task 6: UnionRegion

**Files:**
- Modify: `python/lsst/sphgeom/_continue_class.py` (add a new `class UnionRegion:` block at end-of-file).
- Modify: `tests/test_stcs.py`.

`CompoundRegion` is binary (`nOperands() == 2`), so we always emit two operands. `cloneOperand(i)` returns a `Region` instance.

- [ ] **Step 1: Write the failing tests**

Append to `StcsTestCase`:

```python
    def test_union(self):
        """Union of two circles emits a single ICRS keyword."""
        c1 = Circle(UnitVector3d(LonLat.fromDegrees(180.0, 10.0)),
                    Angle.fromDegrees(2.0))
        c2 = Circle(UnitVector3d(LonLat.fromDegrees(190.0, 20.0)),
                    Angle.fromDegrees(1.0))
        u = UnionRegion(c1, c2)
        self.assert_stcs_equal(
            u.to_ivoa_stcs(),
            "Union ICRS ( Circle 180.0 10.0 2.0 Circle 190.0 20.0 1.0 )",
        )

    def test_union_nested(self):
        """Nested unions: frame keyword appears only at the outermost level."""
        c1 = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)),
                    Angle.fromDegrees(1.0))
        c2 = Circle(UnitVector3d(LonLat.fromDegrees(10.0, 0.0)),
                    Angle.fromDegrees(1.0))
        c3 = Circle(UnitVector3d(LonLat.fromDegrees(20.0, 0.0)),
                    Angle.fromDegrees(1.0))
        nested = UnionRegion(c1, UnionRegion(c2, c3))
        stcs = nested.to_ivoa_stcs()
        # ICRS appears exactly once.
        self.assertEqual(stcs.count("ICRS"), 1)
        # Both Union keywords appear.
        self.assertEqual(stcs.count("Union"), 2)
        self.assert_stcs_equal(
            stcs,
            "Union ICRS ( Circle 0.0 0.0 1.0 Union ( Circle 10.0 0.0 1.0 Circle 20.0 0.0 1.0 ) )",
        )

    def test_union_body(self):
        """UnionRegion body helper omits the frame keyword."""
        c1 = Circle(UnitVector3d(LonLat.fromDegrees(180.0, 10.0)),
                    Angle.fromDegrees(2.0))
        c2 = Circle(UnitVector3d(LonLat.fromDegrees(190.0, 20.0)),
                    Angle.fromDegrees(1.0))
        u = UnionRegion(c1, c2)
        self.assert_stcs_equal(
            u._ivoa_stcs_body(),
            "Union ( Circle 180.0 10.0 2.0 Circle 190.0 20.0 1.0 )",
        )

    def test_union_with_unsupported_operand(self):
        """A Union containing a Box raises NotImplementedError."""
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)),
                        Angle.fromDegrees(1.0))
        box = Box(LonLat.fromDegrees(1.0, 2.0), LonLat.fromDegrees(5.0, 6.0))
        u = UnionRegion(circle, box)
        with self.assertRaises(NotImplementedError):
            u.to_ivoa_stcs()
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `pytest tests/test_stcs.py -v`
Expected: 4 new tests FAIL with `NotImplementedError` from the base class default.

- [ ] **Step 3: Implement UnionRegion support**

At the end of `python/lsst/sphgeom/_continue_class.py`, after the Ellipse block, add:

```python
@_continueClass
class UnionRegion:  # noqa: F811
    """A union of two regions on the unit sphere."""

    def _ivoa_stcs_body(self) -> str:
        # Docstring inherited.
        operands = [self.cloneOperand(i)._ivoa_stcs_body()
                    for i in range(self.nOperands())]
        return f"Union ( {' '.join(operands)} )"

    def to_ivoa_stcs(self, frame: str = "ICRS") -> str:
        # Docstring inherited.
        operands = [self.cloneOperand(i)._ivoa_stcs_body()
                    for i in range(self.nOperands())]
        return f"Union {frame} ( {' '.join(operands)} )"
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `pytest tests/test_stcs.py -v`
Expected: All tests PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/test_stcs.py python/lsst/sphgeom/_continue_class.py
git commit -m "DM-53569: implement to_ivoa_stcs for UnionRegion"
```

---

## Task 7: IntersectionRegion

**Files:**
- Modify: `python/lsst/sphgeom/_continue_class.py` (add a new `class IntersectionRegion:` block at end-of-file).
- Modify: `tests/test_stcs.py`.

- [ ] **Step 1: Write the failing tests**

Append to `StcsTestCase`:

```python
    def test_intersection(self):
        """Intersection of a circle and a polygon."""
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)),
                        Angle.fromDegrees(5.0))
        vertices = [
            UnitVector3d(LonLat.fromDegrees(-1.0, -1.0)),
            UnitVector3d(LonLat.fromDegrees(1.0, -1.0)),
            UnitVector3d(LonLat.fromDegrees(1.0, 1.0)),
            UnitVector3d(LonLat.fromDegrees(-1.0, 1.0)),
        ]
        poly = ConvexPolygon(vertices)
        inter = IntersectionRegion(circle, poly)
        stcs = inter.to_ivoa_stcs()
        # Top-level keyword and frame are deterministic.
        self.assertTrue(stcs.startswith("Intersection ICRS ( "))
        self.assertEqual(stcs.count("ICRS"), 1)
        self.assertIn("Circle 0.0 0.0 5.0", stcs)
        self.assertIn("Polygon ", stcs)

    def test_intersection_with_unsupported_operand(self):
        """Intersection containing a Box raises NotImplementedError."""
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)),
                        Angle.fromDegrees(1.0))
        box = Box(LonLat.fromDegrees(1.0, 2.0), LonLat.fromDegrees(5.0, 6.0))
        inter = IntersectionRegion(circle, box)
        with self.assertRaises(NotImplementedError):
            inter.to_ivoa_stcs()
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `pytest tests/test_stcs.py -v`
Expected: 2 new tests FAIL.

- [ ] **Step 3: Implement IntersectionRegion support**

At the end of `python/lsst/sphgeom/_continue_class.py`, after the UnionRegion block, add:

```python
@_continueClass
class IntersectionRegion:  # noqa: F811
    """An intersection of two regions on the unit sphere."""

    def _ivoa_stcs_body(self) -> str:
        # Docstring inherited.
        operands = [self.cloneOperand(i)._ivoa_stcs_body()
                    for i in range(self.nOperands())]
        return f"Intersection ( {' '.join(operands)} )"

    def to_ivoa_stcs(self, frame: str = "ICRS") -> str:
        # Docstring inherited.
        operands = [self.cloneOperand(i)._ivoa_stcs_body()
                    for i in range(self.nOperands())]
        return f"Intersection {frame} ( {' '.join(operands)} )"
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `pytest tests/test_stcs.py -v`
Expected: All tests PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/test_stcs.py python/lsst/sphgeom/_continue_class.py
git commit -m "DM-53569: implement to_ivoa_stcs for IntersectionRegion"
```

---

## Task 8: News fragment

**Files:**
- Create: `doc/changes/DM-53569.feature.rst`.

- [ ] **Step 1: Write the changelog fragment**

Create `doc/changes/DM-53569.feature.rst` with:

```rst
Added ``Region.to_ivoa_stcs(frame="ICRS")`` for serialising regions as IVOA STC-S strings.
Supports ``Circle``, ``ConvexPolygon``, ``Ellipse``, ``UnionRegion``, and ``IntersectionRegion``.
``Box`` raises ``NotImplementedError`` because STC-S has no latitude-parallel range region; build a ``ConvexPolygon`` from the box if an STC-S representation is required.
```

- [ ] **Step 2: Verify the fragment renders**

Run: `towncrier --draft --version=v30.0.7 2>&1 | head -40`
Expected: the fragment text appears under "New Features". (If `towncrier` is not installed, skip — the format mirrors recent fragments and CI does not gate on towncrier.)

- [ ] **Step 3: Commit**

```bash
git add doc/changes/DM-53569.feature.rst
git commit -m "DM-53569: add news fragment for STC-S generation"
```

---

## Task 9: Final full-suite check

- [ ] **Step 1: Run the entire sphgeom test suite**

Run: `pytest tests/ -v`
Expected: All tests pass, including the existing IVOA POS tests (`tests/test_ivoa.py`) and the new STC-S tests (`tests/test_stcs.py`).

- [ ] **Step 2: Run lint/format checks if configured**

Run: `ruff check python/lsst/sphgeom/_continue_class.py tests/test_stcs.py`
Expected: no errors. Run `ruff format --check` on the same files; reformat if needed.

- [ ] **Step 3: If anything fails, fix and commit a follow-up**

Do not amend earlier commits. Add a fixup commit on top:

```bash
git add <files>
git commit -m "DM-53569: address lint/test fallout"
```
