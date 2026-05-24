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
        """The Region base class default ``_ivoa_stcs_body`` raises
        NotImplementedError; ``to_ivoa_stcs`` is defined only on the base
        class and delegates to ``_ivoa_stcs_body``.
        """
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)), Angle.fromDegrees(1.0))
        with self.assertRaises(NotImplementedError):
            Region._ivoa_stcs_body(circle)

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
        # Recover the 4 (lon, lat) pairs as a set of rounded tuples.
        emitted = {(round(coords[i], 6), round(coords[i + 1], 6)) for i in range(0, 8, 2)}
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

    def test_union(self):
        """Union of two circles emits a single ICRS keyword."""
        c1 = Circle(UnitVector3d(LonLat.fromDegrees(180.0, 10.0)), Angle.fromDegrees(2.0))
        c2 = Circle(UnitVector3d(LonLat.fromDegrees(190.0, 20.0)), Angle.fromDegrees(1.0))
        u = UnionRegion(c1, c2)
        self.assert_stcs_equal(
            u.to_ivoa_stcs(),
            "Union ICRS ( Circle 180.0 10.0 2.0 Circle 190.0 20.0 1.0 )",
        )

    def test_union_nested(self):
        """Nested unions are flattened by ``UnionRegion`` at construction
        time, so the resulting STC-S is a single ``Union`` with all three
        operands and a single frame keyword.
        """
        c1 = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)), Angle.fromDegrees(1.0))
        c2 = Circle(UnitVector3d(LonLat.fromDegrees(10.0, 0.0)), Angle.fromDegrees(1.0))
        c3 = Circle(UnitVector3d(LonLat.fromDegrees(20.0, 0.0)), Angle.fromDegrees(1.0))
        nested = UnionRegion(c1, UnionRegion(c2, c3))
        # UnionRegion flattens same-kind operands automatically.
        self.assertEqual(nested.nOperands(), 3)
        stcs = nested.to_ivoa_stcs()
        # ICRS appears exactly once.
        self.assertEqual(stcs.count("ICRS"), 1)
        # Only one Union keyword (flattened).
        self.assertEqual(stcs.count("Union"), 1)
        self.assert_stcs_equal(
            stcs,
            "Union ICRS ( Circle 0.0 0.0 1.0 Circle 10.0 0.0 1.0 Circle 20.0 0.0 1.0 )",
        )

    def test_union_body(self):
        """UnionRegion body helper omits the frame keyword."""
        c1 = Circle(UnitVector3d(LonLat.fromDegrees(180.0, 10.0)), Angle.fromDegrees(2.0))
        c2 = Circle(UnitVector3d(LonLat.fromDegrees(190.0, 20.0)), Angle.fromDegrees(1.0))
        u = UnionRegion(c1, c2)
        self.assert_stcs_equal(
            u._ivoa_stcs_body(),
            "Union ( Circle 180.0 10.0 2.0 Circle 190.0 20.0 1.0 )",
        )

    def test_union_with_unsupported_operand(self):
        """A Union containing a Box raises NotImplementedError."""
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)), Angle.fromDegrees(1.0))
        box = Box(LonLat.fromDegrees(1.0, 2.0), LonLat.fromDegrees(5.0, 6.0))
        u = UnionRegion(circle, box)
        with self.assertRaises(NotImplementedError):
            u.to_ivoa_stcs()

    def test_intersection(self):
        """Intersection of a circle and a polygon."""
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)), Angle.fromDegrees(5.0))
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
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)), Angle.fromDegrees(1.0))
        box = Box(LonLat.fromDegrees(1.0, 2.0), LonLat.fromDegrees(5.0, 6.0))
        inter = IntersectionRegion(circle, box)
        with self.assertRaises(NotImplementedError):
            inter.to_ivoa_stcs()


if __name__ == "__main__":
    unittest.main()
