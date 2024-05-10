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

import unittest

from lsst.sphgeom import Box, Circle, ConvexPolygon, LonLat, Region, UnitVector3d


class IvoaTestCase(unittest.TestCase):
    """Test Box."""

    def test_construction(self):
        # Example POS strings found in IVOA documentation.
        example_pos = (
            "CIRCLE 12.0 34.0 0.5",
            "RANGE 12.0 12.5 34.0 36.0",
            "POLYGON 12.0 34.0 14.0 35.0 14. 36.0 12.0 35.0",
            "RANGE 0 360.0 -2.0 2.0",
            "RANGE 0 360.0 89.0 +Inf",
            "RANGE -Inf +Inf -Inf +Inf",
            "POLYGON 12 34 14 34 14 36 12 36",
            "RANGE 0 360 89 90",
        )
        for pos in example_pos:
            region = Region.from_ivoa_pos(pos)
            self.assertIsInstance(region, Region)

        # Badly formed strings raising ValueError.
        bad_pos = (
            "circle 12 34 0.5",
            "CIRCLE 12 34 1 1",
            "RANGE 0 360",
            "POLYGON 0 1 2 3",
            "POLYGON 0 1 2 3 4 5 6",
            "CONVEXPOLYGON 0 1 2 3 4 5",
        )
        for pos in bad_pos:
            with self.assertRaises(ValueError):
                Region.from_ivoa_pos(pos)

    def _split_pos(self, pos: str):
        """Split POS into type and floats."""
        region_type, *coordstr = pos.split()
        coordinates = [float(c) for c in coordstr]
        return region_type, coordinates

    def assert_pos_equal(self, pos1: str, pos2: str):
        """Compare two POS strings and check for equality taking into account
        floating point differences.
        """
        region_type1, coords1 = self._split_pos(pos1)
        region_type2, coords2 = self._split_pos(pos2)
        self.assertEqual(region_type1, region_type2)
        self.assertEqual(len(coords1), len(coords2))
        for c1, c2 in zip(coords1, coords2):
            self.assertAlmostEqual(c1, c2)

    def test_circle(self):
        """Test circle construction."""
        pos = "CIRCLE 12.0 34.0 5"
        circle = Region.from_ivoa_pos(pos)
        self.assertIsInstance(circle, Circle)
        self.assertTrue(circle.contains(UnitVector3d(LonLat.fromDegrees(13.0, 33.0))))
        self.assertFalse(circle.contains(UnitVector3d(LonLat.fromDegrees(12.0, 40.0))))

        self.assert_pos_equal(circle.to_ivoa_pos(), pos)

    def test_range(self):
        """Test range construction."""
        box = Region.from_ivoa_pos("RANGE 1 2 5 6")
        self.assertIsInstance(box, Box)
        self.assertTrue(box.contains(UnitVector3d(LonLat.fromDegrees(1.5, 5.4))))
        self.assertFalse(box.contains(UnitVector3d(LonLat.fromDegrees(4, 10))))
        self.assert_pos_equal(box.to_ivoa_pos(), "RANGE 1 2 5 6")

        box = Region.from_ivoa_pos("RANGE 1 2 20 +Inf")
        self.assertTrue(box.contains(UnitVector3d(LonLat.fromDegrees(1.7, 80))))
        self.assertFalse(box.contains(UnitVector3d(LonLat.fromDegrees(1.7, 10))))
        self.assert_pos_equal(box.to_ivoa_pos(), "RANGE 1 2 20 90")

        box = Region.from_ivoa_pos("RANGE 50 +Inf 20 30")
        self.assertTrue(box.contains(UnitVector3d(LonLat.fromDegrees(60, 25))))
        self.assertFalse(box.contains(UnitVector3d(LonLat.fromDegrees(49, 21))))
        self.assert_pos_equal(box.to_ivoa_pos(), "RANGE 50 360 20 30")

        box = Region.from_ivoa_pos("RANGE -Inf +50 20 30")
        self.assertTrue(box.contains(UnitVector3d(LonLat.fromDegrees(40, 25))))
        self.assertFalse(box.contains(UnitVector3d(LonLat.fromDegrees(60, 21))))
        self.assert_pos_equal(box.to_ivoa_pos(), "RANGE 0 50 20 30")

        box = Region.from_ivoa_pos("RANGE -Inf +Inf 20 30")
        self.assertTrue(box.contains(UnitVector3d(LonLat.fromDegrees(10, 25))))
        self.assertTrue(box.contains(UnitVector3d(LonLat.fromDegrees(359, 25))))
        self.assertFalse(box.contains(UnitVector3d(LonLat.fromDegrees(49, 19))))
        self.assert_pos_equal(box.to_ivoa_pos(), "RANGE 0 360. 20 30")

    def test_polygon(self):
        """Test polygon construction."""
        pos = "POLYGON 12.0 34.0 14.0 35.0 14. 36.0 12.0 35.0"
        poly = Region.from_ivoa_pos(pos)
        self.assertIsInstance(poly, ConvexPolygon)
        self.assertTrue(poly.contains(UnitVector3d(LonLat.fromDegrees(13, 35))))
        self.assertFalse(poly.contains(UnitVector3d(LonLat.fromDegrees(14, 34))))
        self.assert_pos_equal(poly.to_ivoa_pos(), pos)


if __name__ == "__main__":
    unittest.main()
