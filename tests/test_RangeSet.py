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

from lsst.sphgeom import RangeSet


class RangeSetTestCase(unittest.TestCase):
    """Test RangeSet."""

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
        self.assertEqual(list(s), [(0, 2), (4, 0)])

    def testString(self):
        s = RangeSet(1, 10)
        self.assertEqual(str(s), "[(1, 10)]")
        self.assertEqual(repr(s), "RangeSet([(1, 10)])")
        self.assertEqual(s, eval(repr(s), {"RangeSet": RangeSet}))

    def testPickle(self):
        r = RangeSet([2, 3, 5, 7, 11, 13, 17, 19])
        s = pickle.loads(pickle.dumps(r))
        self.assertEqual(r, s)


if __name__ == "__main__":
    unittest.main()
