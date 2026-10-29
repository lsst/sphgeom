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

from lsst.sphgeom import (
    CONTAINS,
    Angle,
    Box,
    Circle,
    ConvexPolygon,
    Ellipse,
    IntersectionRegion,
    LonLat,
    UnionRegion,
    UnitVector3d,
)


class RegionTestCase(unittest.TestCase):
    """Test the methods defined on the Region base class."""

    def setUp(self):
        self.regions = [
            Box(LonLat.fromRadians(0.0, 1.0), Angle(0.2), Angle(0.2)),
            Circle(UnitVector3d.Z(), Angle(0.3)),
            ConvexPolygon(
                [
                    UnitVector3d(1.0, 0.0, 0.1),
                    UnitVector3d(0.0, 1.0, 0.1),
                    UnitVector3d(-1.0, -1.0, 0.1),
                ]
            ),
            Ellipse(UnitVector3d.Z(), Angle(0.3)),
            UnionRegion(Circle(UnitVector3d.X(), Angle(0.2)), Circle(UnitVector3d.Y(), Angle(0.2))),
            IntersectionRegion(Circle(UnitVector3d.X(), Angle(0.4)), Circle(UnitVector3d.X(), Angle(0.2))),
        ]

    def test_contains_region(self):
        outer = Circle(UnitVector3d.Z(), Angle(0.5))
        inner = Circle(UnitVector3d.Z(), Angle(0.1))
        self.assertTrue(outer.contains(inner))
        self.assertFalse(inner.contains(outer))

    def test_contains_region_all_types(self):
        """Every region type accepts a Region argument.

        Subclasses that define their own ``contains`` overloads hide the ones
        bound on Region, so each of them has to rewrap it explicitly.
        """
        for region in self.regions:
            for other in self.regions:
                with self.subTest(region=type(region).__name__, other=type(other).__name__):
                    self.assertIsInstance(region.contains(other), bool)

    def test_contains_region_agrees_with_relate(self):
        """``contains`` is defined in terms of ``relate``, and so is equally
        conservative: a false result only means containment could not be
        established.
        """
        for region in self.regions:
            for other in self.regions:
                with self.subTest(region=type(region).__name__, other=type(other).__name__):
                    self.assertEqual(region.contains(other), (region.relate(other) & CONTAINS) != 0)


if __name__ == "__main__":
    unittest.main()
