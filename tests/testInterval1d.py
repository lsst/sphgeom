from __future__ import absolute_import, division

import sys
import unittest

from lsst.sphgeom import Interval1d, CONTAINS, DISJOINT, WITHIN


class Interval1dTestCase(unittest.TestCase):
    def testConstruction(self):
        i = Interval1d(1)
        self.assertEqual(i.getA(), i.getB())
        self.assertEqual(i.getA(), 1)
        i = Interval1d(1, 2)
        self.assertEqual(i, Interval1d(1, 2))
        self.assertTrue(Interval1d.empty().isEmpty())

    def testComparisonOperators(self):
        self.assertEqual(Interval1d(1), Interval1d(1, 1))
        self.assertEqual(Interval1d(1), 1)
        self.assertNotEqual(Interval1d(1, 1), Interval1d(2, 2))
        self.assertNotEqual(Interval1d(2, 2), 1)

    def testCenterAndSize(self):
        i = Interval1d(1, 2)
        self.assertEqual(i.getSize(), 1)
        self.assertEqual(i.getCenter(), 1.5)

    def testRelationships(self):
        i02 = Interval1d(0, 2)
        i13 = Interval1d(1, 3)
        i46 = Interval1d(4, 6)
        i06 = Interval1d(0, 6)
        self.assertTrue(i02.contains(1))
        self.assertTrue(i02.contains(Interval1d(0.5, 1.5)))
        self.assertTrue(i02.isDisjointFrom(3))
        self.assertTrue(i02.isDisjointFrom(i46))
        self.assertTrue(i02.intersects(1))
        self.assertTrue(i02.intersects(i13))
        self.assertTrue(Interval1d(1, 1).isWithin(i02))
        self.assertTrue(i02.isWithin(i06))
        r = i02.relate(1)
        self.assertEqual(r, CONTAINS)
        r = i46.relate(i02)
        self.assertEqual(r, DISJOINT)

    def testExpandingAndClipping(self):
        a = Interval1d(1, 2)
        b = (
            a.expandedTo(3)
            .expandedTo(Interval1d(2, 4))
            .clippedTo(Interval1d(0, 2))
            .clippedTo(1)
        )
        a.expandTo(3).expandTo(Interval1d(2, 4))
        a.clipTo(Interval1d(0, 2)).clipTo(1)
        self.assertEqual(a, b)
        self.assertEqual(a, 1)

    def testDilationAndErosion(self):
        a = Interval1d(1, 3)
        b = a.dilatedBy(1).erodedBy(2)
        a.dilateBy(1).erodeBy(2)
        self.assertEqual(a, b)
        self.assertEqual(a, 2)


def suite():
    return unittest.makeSuite(Interval1dTestCase)

def run(shouldExit=False):
    status = 0 if unittest.TextTestRunner().run(suite()).wasSuccessful() else 1
    if shouldExit:
        sys.exit(status)
    return status

if __name__ == "__main__":
    run(True)
