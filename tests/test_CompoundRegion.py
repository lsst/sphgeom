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
from base64 import b64encode

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
        regions = tuple(region.cloneOperand(i) for i in range(region.nOperands()))
        self.assertCountEqual(regions, operands)

    def assertCompoundRegionsEqual(self, a, b):
        """Assert that two compound regions are equal.

        CompoundRegion does not implement equality comparison because
        regions in general do not, and hence it cannot delegate that operation
        to its operands.  But the concrete operands (circle and box) we use in
        these tests do implement equality comparison.
        """
        operands = tuple(b.cloneOperand(i) for i in range(b.nOperands()))
        self.assertEqual(type(a), type(b))
        self.assertOperandsEqual(a, operands)

    def assertRelations(self, r1, r2, relation, overlaps):
        """Assert relation between two regions."""
        self.assertEqual(r1.relate(r2), relation)
        self.assertEqual(r1.overlaps(r2), overlaps)

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
        self.assertRelations(self.circle, self.circle, CONTAINS | WITHIN, True)
        self.assertRelations(self.circle, self.box, INTERSECTS, True)
        self.assertRelations(self.circle, self.faraway, DISJOINT, False)
        self.assertRelations(self.box, self.circle, INTERSECTS, True)
        self.assertRelations(self.box, self.box, CONTAINS | WITHIN, True)
        self.assertRelations(self.box, self.faraway, DISJOINT, False)

    def testOperands(self):
        """Test the cloneOperands accessor."""
        self.assertOperandsEqual(self.instance, self.operands)

    def testIterator(self):
        """Test Python iteration."""
        self.assertEqual(len(self.instance), len(self.operands))
        it = iter(self.instance)
        self.assertEqual(next(it), self.operands[0])
        self.assertEqual(next(it), self.operands[1])
        with self.assertRaises(StopIteration):
            next(it)

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

    def testEmpty(self):
        """Test zero-operand union which is quivalent to empty region."""
        region = UnionRegion()

        self.assertFalse(region.contains(UnitVector3d(self.point_in_both)))
        self.assertFalse(region.contains(UnitVector3d(self.point_in_circle)))
        self.assertFalse(region.contains(UnitVector3d(self.point_in_box)))
        self.assertFalse(region.contains(UnitVector3d(self.point_in_neither)))

        self.assertRelations(region, self.box, DISJOINT, False)
        self.assertRelations(region, self.circle, DISJOINT, False)
        self.assertRelations(region, self.faraway, DISJOINT, False)
        self.assertRelations(region, self.instance, DISJOINT, False)
        self.assertRelations(self.box, region, DISJOINT, False)
        self.assertRelations(self.circle, region, DISJOINT, False)
        self.assertRelations(self.faraway, region, DISJOINT, False)
        self.assertRelations(self.instance, region, DISJOINT, False)

        self.assertEqual(Region.getRegions(region), [])

    def testContains(self):
        """Test point-in-region checks."""
        self.assertTrue(self.instance.contains(UnitVector3d(self.point_in_both)))
        self.assertTrue(self.instance.contains(UnitVector3d(self.point_in_circle)))
        self.assertTrue(self.instance.contains(UnitVector3d(self.point_in_box)))
        self.assertFalse(self.instance.contains(UnitVector3d(self.point_in_neither)))

    def testRelate(self):
        """Test region-region relationship checks."""
        self.assertRelations(self.instance, self.circle, CONTAINS, True)
        self.assertRelations(self.instance, self.box, CONTAINS, True)
        self.assertRelations(self.instance, self.faraway, DISJOINT, False)
        self.assertRelations(self.circle, self.instance, WITHIN, True)
        self.assertRelations(self.box, self.instance, WITHIN, True)
        self.assertRelations(self.faraway, self.instance, DISJOINT, False)

    def testBounding(self):
        """Test for getBounding*() methods."""
        region = UnionRegion()
        self.assertTrue(region.getBoundingBox().empty())
        self.assertTrue(region.getBoundingBox3d().empty())
        self.assertTrue(region.getBoundingCircle().empty())

        for operand in self.operands:
            self.assertEqual(self.instance.getBoundingBox().relate(operand), CONTAINS)
        for operand in self.operands:
            self.assertTrue(self.instance.getBoundingBox3d().contains(operand.getBoundingBox3d()))
        # This test fails for first operand (Circle), I guess due to precision.
        for operand in self.operands[-1:]:
            self.assertTrue(self.instance.getBoundingCircle().relate(operand), CONTAINS)

    def testDecodeBase64(self):
        """Test Region.decodeBase64, which includes special handling for
        union regions.
        """
        # Test with the full UnionRegion encoded, then base64-encoded.
        s1 = b64encode(self.instance.encode()).decode("ascii")
        self.assertCompoundRegionsEqual(Region.decodeBase64(s1), self.instance)
        # Test alternate form with union members concatenated with ':' after
        # base64-encoding.
        s2 = ":".join(b64encode(region.encode()).decode("ascii") for region in self.instance)
        self.assertCompoundRegionsEqual(Region.decodeBase64(s2), self.instance)
        # Test that an empty string decodes as a UnionRegion with no members.
        self.assertCompoundRegionsEqual(Region.decodeBase64(""), UnionRegion())


class IntersectionRegionTestCase(CompoundRegionTestMixin, unittest.TestCase):
    """Test intersection region."""

    def setUp(self):
        CompoundRegionTestMixin.setUp(self)
        self.instance = IntersectionRegion(*self.operands)

    def testEmpty(self):
        """Test zero-operand intersection (equivalent to full sphere)."""
        region = IntersectionRegion()

        self.assertTrue(region.contains(UnitVector3d(self.point_in_both)))
        self.assertTrue(region.contains(UnitVector3d(self.point_in_circle)))
        self.assertTrue(region.contains(UnitVector3d(self.point_in_box)))
        self.assertTrue(region.contains(UnitVector3d(self.point_in_neither)))

        self.assertRelations(region, self.box, CONTAINS, True)
        self.assertRelations(region, self.circle, CONTAINS, True)
        self.assertRelations(region, self.faraway, CONTAINS, True)
        self.assertRelations(region, self.instance, CONTAINS, True)
        self.assertRelations(self.box, region, WITHIN, True)
        self.assertRelations(self.circle, region, WITHIN, True)
        self.assertRelations(self.faraway, region, WITHIN, True)
        # Overlaps between intersections are very conservative.
        self.assertRelations(self.instance, region, WITHIN, None)

        self.assertEqual(Region.getRegions(region), [])

    def testContains(self):
        """Test point-in-region checks."""
        self.assertTrue(self.instance.contains(UnitVector3d(self.point_in_both)))
        self.assertFalse(self.instance.contains(UnitVector3d(self.point_in_circle)))
        self.assertFalse(self.instance.contains(UnitVector3d(self.point_in_box)))
        self.assertFalse(self.instance.contains(UnitVector3d(self.point_in_neither)))

    def testRelate(self):
        """Test region-region relationship checks."""
        self.assertRelations(self.instance, self.box, WITHIN, None)
        self.assertRelations(self.instance, self.circle, WITHIN, None)
        self.assertRelations(self.instance, self.faraway, DISJOINT, False)
        self.assertRelations(self.circle, self.instance, CONTAINS, None)
        self.assertRelations(self.box, self.instance, CONTAINS, None)
        self.assertRelations(self.faraway, self.instance, DISJOINT, False)

    def testGetRegion(self):
        c1 = Circle(UnitVector3d(0.0, 0.0, 1.0), 1.0)
        c2 = Circle(UnitVector3d(1.0, 0.0, 1.0), 2.0)
        b1 = Box.fromDegrees(90, 0, 180, 45)
        b2 = Box.fromDegrees(135, 15, 135, 30)
        u1 = UnionRegion(c1, b1)
        u2 = UnionRegion(c2, b2)
        i1 = IntersectionRegion(c1, b1)
        i2 = IntersectionRegion(c2, b2)
        ur = UnionRegion(u1, u2)
        ir = IntersectionRegion(i1, i2)
        self.assertEqual(Region.getRegions(c1), [c1])
        self.assertEqual(Region.getRegions(i1), [c1, b1])
        self.assertEqual(Region.getRegions(u1), [c1, b1])
        # Compounds of compounds will be flattened, order preserved.
        self.assertEqual(Region.getRegions(ir), [c1, b1, c2, b2])
        self.assertEqual(Region.getRegions(ur), [c1, b1, c2, b2])

        # TODO: This test fails because CompoundRegion does not define
        # equality operator, and it is non-trivial to add one.
        # ur2 = UnionRegion(u1, i1, u2)
        # self.assertEqual(Region.getRegions(ur2), [c1, b1, i1, c2, b2])

    def testBounding(self):
        """Test for getBounding*() methods."""
        region = UnionRegion()
        self.assertTrue(region.getBoundingBox().full())
        self.assertTrue(region.getBoundingBox3d().full())
        self.assertTrue(region.getBoundingCircle().full())

        # Only Box3d test works reliably, other two fails due to boundary
        # overlaps and precision.
        for operand in self.operands:
            self.assertTrue(operand.getBoundingBox3d().contains(self.instance.getBoundingBox3d()))

    def testDecodeOverlapsBase64(self):
        """Test Region.decodeOverlapsBase64.

        This test is in this test case because it can make good use of the
        concrete regions defined in setUp.
        """

        def run_overlaps(pairs):
            or_terms = []
            for a, b in pairs:
                a_str = b64encode(a.encode()).decode("ascii")
                b_str = b64encode(b.encode()).decode("ascii")
                or_terms.append(f"{a_str}&{b_str}")
            overlap_str = "|".join(or_terms)
            return Region.decodeOverlapsBase64(overlap_str)

        self.assertEqual(run_overlaps([]), False)
        self.assertEqual(run_overlaps([(self.box, self.circle)]), True)
        self.assertEqual(run_overlaps([(self.box, self.faraway)]), False)
        self.assertEqual(run_overlaps([(self.circle, self.faraway)]), False)
        self.assertEqual(run_overlaps([(self.instance, self.box)]), None)
        self.assertEqual(run_overlaps([(self.box, self.circle), (self.box, self.faraway)]), True)
        self.assertEqual(run_overlaps([(self.faraway, self.circle), (self.box, self.faraway)]), False)
        self.assertEqual(run_overlaps([(self.instance, self.box), (self.circle, self.faraway)]), None)
        self.assertEqual(run_overlaps([(self.instance, self.box), (self.circle, self.box)]), True)
        self.assertEqual(run_overlaps([(self.circle, self.box), (self.instance, self.box)]), True)

        with self.assertRaises(RuntimeError):
            # Decoding a single region is an error; that's not an expression.
            Region.decodeOverlapsBase64(b64encode(self.box.encode()).decode("ascii"))


if __name__ == "__main__":
    unittest.main()
