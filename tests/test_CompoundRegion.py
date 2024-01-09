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

import pickle
import unittest

try:
    import yaml
except ImportError:
    yaml = None

from lsst.sphgeom import (
    CONTAINS,
    DISJOINT,
    INTERSECTS,
    WITHIN,
    Angle,
    AngleInterval,
    Box,
    Circle,
    CompoundRegion,
    IntersectionRegion,
    LonLat,
    NormalizedAngleInterval,
    Region,
    UnionRegion,
    UnitVector3d,
)


class CompoundRegionTestMixin:
    """Tests for both UnionRegion and IntersectionRegion.

    Concrete TestCase subclasses are responsible for adding an `instance`
    attribute with a non-trivial instance of the `CompoundRegion` subclass
    being tested.
    """

    def setUp(self):
        self.point_in_circle = LonLat.fromDegrees(44.0, 45.0)
        self.point_in_box = LonLat.fromDegrees(46.0, 45.0)
        self.point_in_both = LonLat.fromDegrees(45.0, 45.0)
        self.point_in_neither = LonLat.fromDegrees(45.0, 48.0)
        self.circle = Circle(UnitVector3d(self.point_in_circle), Angle.fromDegrees(1.0))
        self.box = Box.fromDegrees(
            self.point_in_box.getLon().asDegrees() - 1.5,
            self.point_in_box.getLat().asDegrees() - 1.5,
            self.point_in_box.getLon().asDegrees() + 1.5,
            self.point_in_box.getLat().asDegrees() + 1.5,
        )
        self.faraway = Circle(UnitVector3d(self.point_in_neither), Angle.fromDegrees(0.1))
        self.operands = (self.circle, self.box)

    def assertOperandsEqual(self, region, operands):
        """Assert that a compound regions operands are equal to the given
        tuple of operands.
        """
        self.assertCountEqual((region.cloneOperand(0), region.cloneOperand(1)), operands)

    def assertCompoundRegionsEqual(self, a, b):
        """Assert that two compound regions are equal.

        CompoundRegion does not implement equality comparison because
        regions in general do not, and hence it cannot delegate that operation
        to its operands.  But the concrete operands (circle and box) we use in
        these tests do implement equality comparison.
        """
        self.assertEqual(type(a), type(b))
        self.assertOperandsEqual(a, (b.cloneOperand(0), b.cloneOperand(1)))

    def testSetUp(self):
        """Test that the points and operand regions being tested have the
        relationships expected.
        """
        self.assertTrue(self.circle.contains(UnitVector3d(self.point_in_circle)))
        self.assertTrue(self.circle.contains(UnitVector3d(self.point_in_both)))
        self.assertFalse(self.circle.contains(UnitVector3d(self.point_in_box)))
        self.assertFalse(self.circle.contains(UnitVector3d(self.point_in_neither)))
        self.assertTrue(self.box.contains(UnitVector3d(self.point_in_box)))
        self.assertTrue(self.box.contains(UnitVector3d(self.point_in_both)))
        self.assertFalse(self.box.contains(UnitVector3d(self.point_in_circle)))
        self.assertFalse(self.box.contains(UnitVector3d(self.point_in_neither)))
        self.assertEqual(self.circle.relate(self.circle), CONTAINS | WITHIN)
        self.assertEqual(self.circle.relate(self.box), INTERSECTS)
        self.assertEqual(self.circle.relate(self.faraway), DISJOINT)
        self.assertEqual(self.box.relate(self.circle), INTERSECTS)
        self.assertEqual(self.box.relate(self.box), CONTAINS | WITHIN)
        self.assertEqual(self.box.relate(self.faraway), DISJOINT)

    def testOperands(self):
        """Test the cloneOperands accessor."""
        self.assertOperandsEqual(self.instance, self.operands)

    def testCodec(self):
        """Test that encode and decode round-trip."""
        s = self.instance.encode()
        self.assertCompoundRegionsEqual(type(self.instance).decode(s), self.instance)
        self.assertCompoundRegionsEqual(CompoundRegion.decode(s), self.instance)
        self.assertCompoundRegionsEqual(Region.decode(s), self.instance)

    def testPickle(self):
        """Test pickling round-trips."""
        s = pickle.dumps(self.instance, pickle.HIGHEST_PROTOCOL)
        self.assertCompoundRegionsEqual(pickle.loads(s), self.instance)

    def testString(self):
        """Test that repr returns a string that can be eval'd to yield an
        equivalent instance.
        """
        self.assertCompoundRegionsEqual(
            self.instance,
            eval(
                repr(self.instance),
                {
                    "UnionRegion": UnionRegion,
                    "IntersectionRegion": IntersectionRegion,
                    "Box": Box,
                    "Circle": Circle,
                    "UnitVector3d": UnitVector3d,
                    "Angle": Angle,
                    "AngleInterval": AngleInterval,
                    "NormalizedAngleInterval": NormalizedAngleInterval,
                },
            ),
        )

    @unittest.skipIf(not yaml, "YAML module can not be imported")
    def testYaml(self):
        """Test that YAML dump and load round-trip."""
        self.assertCompoundRegionsEqual(yaml.safe_load(yaml.dump(self.instance)), self.instance)


class UnionRegionTestCase(CompoundRegionTestMixin, unittest.TestCase):
    """Test UnionRegion."""

    def setUp(self):
        CompoundRegionTestMixin.setUp(self)
        self.instance = UnionRegion(*self.operands)

    def testContains(self):
        """Test point-in-region checks."""
        self.assertTrue(self.instance.contains(UnitVector3d(self.point_in_both)))
        self.assertTrue(self.instance.contains(UnitVector3d(self.point_in_circle)))
        self.assertTrue(self.instance.contains(UnitVector3d(self.point_in_box)))
        self.assertFalse(self.instance.contains(UnitVector3d(self.point_in_neither)))

    def testRelate(self):
        """Test region-region relationship checks."""
        self.assertEqual(self.instance.relate(self.circle), CONTAINS)
        self.assertEqual(self.instance.relate(self.box), CONTAINS)
        self.assertEqual(self.instance.relate(self.faraway), DISJOINT)
        self.assertEqual(self.circle.relate(self.instance), WITHIN)
        self.assertEqual(self.box.relate(self.instance), WITHIN)
        self.assertEqual(self.faraway.relate(self.instance), DISJOINT)


class IntersectionRegionTestCase(CompoundRegionTestMixin, unittest.TestCase):
    """Test intersection region."""

    def setUp(self):
        CompoundRegionTestMixin.setUp(self)
        self.instance = IntersectionRegion(*self.operands)

    def testContains(self):
        """Test point-in-region checks."""
        self.assertTrue(self.instance.contains(UnitVector3d(self.point_in_both)))
        self.assertFalse(self.instance.contains(UnitVector3d(self.point_in_circle)))
        self.assertFalse(self.instance.contains(UnitVector3d(self.point_in_box)))
        self.assertFalse(self.instance.contains(UnitVector3d(self.point_in_neither)))

    def testRelate(self):
        """Test region-region relationship checks."""
        self.assertEqual(self.instance.relate(self.box), WITHIN)
        self.assertEqual(self.instance.relate(self.circle), WITHIN)
        self.assertEqual(self.instance.relate(self.faraway), DISJOINT)
        self.assertEqual(self.circle.relate(self.instance), CONTAINS)
        self.assertEqual(self.box.relate(self.instance), CONTAINS)
        self.assertEqual(self.faraway.relate(self.instance), DISJOINT)


if __name__ == "__main__":
    unittest.main()
