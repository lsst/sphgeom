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

from lsst.sphgeom import CONTAINS, DISJOINT, Interval1d


class Interval1dTestCase(unittest.TestCase):
    """Test 1D intervals."""

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
        b = a.expandedTo(3).expandedTo(Interval1d(2, 4)).clippedTo(Interval1d(0, 2)).clippedTo(1)
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

    def testString(self):
        i = Interval1d(1, 2)
        self.assertEqual(str(i), "[1.0, 2.0]")
        self.assertEqual(repr(i), "Interval1d(1.0, 2.0)")
        self.assertEqual(i, eval(repr(i), {"Interval1d": Interval1d}))

    def testPickle(self):
        a = Interval1d(1.5, 3.5)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
