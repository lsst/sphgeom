#!/usr/bin/env python
#
# LSST Data Management System
#
# Copyright 2008-2016  AURA/LSST.
#
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
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
# You should have received a copy of the LSST License Statement and
# the GNU General Public License along with this program.  If not,
# see <https://www.lsstcorp.org/LegalNotices/>.
#
from __future__ import absolute_import, division

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
        s.insert(0, 1)
        s.insert(2, 3)
        self.assertEqual(s.ranges(), [(0, 1), (2, 3)])
        s = RangeSet(4, 2)
        self.assertEqual(s.ranges(), [(0, 2), (4, 2**64)])

    def testString(self):
        self.assertEqual(str(RangeSet(1)), '{"RangeSet": [[1, 2]]}')


if __name__ == "__main__":
    unittest.main()
