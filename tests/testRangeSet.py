from __future__ import absolute_import, division

import sys
import unittest

from lsst.sphgeom import RangeSet


class RangeSetTestCase(unittest.TestCase):
    def testConstruction(self):
        s1 = RangeSet(1)
        s2 = RangeSet()
        s3 = RangeSet(2, 1)
        s4 = RangeSet(s3)
        self.assertTrue(s2.empty())
        self.assertEqual(s3, s4)
        self.assertEqual(s1, s3.complement())

    def testComparisonOperators(self):
        s1 = RangeSet(1)
        s2 = RangeSet(2)
        self.assertNotEqual(s1, s2)
        s1.insert(2)
        s2.insert(1)
        self.assertEqual(s1, s2)
        self.assertTrue(RangeSet(2, 1).contains(RangeSet(3, 4)))
        self.assertTrue(RangeSet(2, 1).contains(3, 4))
        self.assertTrue(RangeSet(2, 1).contains(3))
        self.assertTrue(RangeSet(2, 4).isWithin(RangeSet(1, 5)))
        self.assertTrue(RangeSet(2, 4).isWithin(1, 5))
        self.assertFalse(RangeSet(2, 4).isWithin(3))
        self.assertTrue(RangeSet(2, 4).intersects(RangeSet(3, 5)))
        self.assertTrue(RangeSet(2, 4).intersects(3, 5))
        self.assertTrue(RangeSet(2, 4).intersects(3))
        self.assertTrue(RangeSet(2, 4).isDisjointFrom(RangeSet(6, 8)))
        self.assertTrue(RangeSet(2, 4).isDisjointFrom(6, 8))
        self.assertTrue(RangeSet(2, 4).isDisjointFrom(6))

    def testSetOperators(self):
        a = RangeSet(1)
        b = ~a
        self.assertTrue((a | b).full())
        self.assertTrue((a & b).empty())
        self.assertEqual(a - b, a)
        self.assertEqual(b - a, b)
        a &= a
        b &= b
        c = (a ^ b) - RangeSet(2, 4)
        self.assertEqual(c, RangeSet(4, 2))
        c |= b
        self.assertTrue(c.full())
        c ^= c
        self.assertTrue(c.empty())

    def testRanges(self):
        s = RangeSet()
        s.insert(0,1)
        s.insert(2,3)
        self.assertEqual(s.ranges(), [(0, 1), (2, 3)])
        s = RangeSet(4, 2)
        self.assertEqual(s.ranges(), [(0, 2), (4, 2**64)])

    def testString(self):
        self.assertEqual(str(RangeSet(1)), '{"RangeSet": [[1, 2]]}')


def suite():
    return unittest.makeSuite(RangeSetTestCase)

def run(shouldExit=False):
    status = 0 if unittest.TextTestRunner().run(suite()).wasSuccessful() else 1
    if shouldExit:
        sys.exit(status)
    return status

if __name__ == "__main__":
    run(True)
