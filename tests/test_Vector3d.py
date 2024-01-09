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
import pickle
import unittest

from lsst.sphgeom import Angle, UnitVector3d, Vector3d


class Vector3dTestCase(unittest.TestCase):
    """Test 3D vector."""

    def testConstruction(self):
        v = Vector3d(1, 2, 3)
        self.assertEqual(v.x(), 1)
        self.assertEqual(v.y(), 2)
        self.assertEqual(v.z(), 3)

    def testComparison(self):
        v = Vector3d(1, 2, 3)
        self.assertEqual(v, Vector3d(1, 2, 3))
        self.assertNotEqual(v, Vector3d(1, 2, 4))

    def testIsZero(self):
        self.assertTrue(Vector3d(0, 0, 0).isZero())
        self.assertFalse(Vector3d(0, 0, 1).isZero())

    def testAccess(self):
        v = Vector3d(1, 2, 3)
        self.assertEqual(len(v), 3)
        self.assertEqual(v[0], 1)
        self.assertEqual(v[1], 2)
        self.assertEqual(v[2], 3)
        self.assertEqual(v[-3], 1)
        self.assertEqual(v[-2], 2)
        self.assertEqual(v[-1], 3)
        with self.assertRaises(IndexError):
            v[-4]
        with self.assertRaises(IndexError):
            v[3]

    def testNormal(self):
        v = Vector3d(0, 2, 0)
        self.assertEqual(v.getSquaredNorm(), 4)
        self.assertEqual(v.getNorm(), 2)
        v.normalize()
        self.assertTrue(v.isNormalized())
        self.assertEqual(v, Vector3d(0, 1, 0))

    def testDot(self):
        x = Vector3d(1, 0, 0)
        y = Vector3d(0, 1, 0)
        self.assertEqual(x.dot(y), 0)

    def testCross(self):
        x = Vector3d(1, 0, 0)
        y = Vector3d(0, 1, 0)
        self.assertEqual(x.cross(y), Vector3d(0, 0, 1))

    def testArithmeticOperators(self):
        self.assertEqual(-Vector3d(1, 1, 1), Vector3d(-1, -1, -1))
        self.assertEqual(Vector3d(1, 1, 1) * 2, Vector3d(2, 2, 2))
        self.assertEqual(Vector3d(2, 2, 2) / 2, Vector3d(1, 1, 1))
        self.assertEqual(Vector3d(1, 1, 1) + Vector3d(1, 1, 1), Vector3d(2, 2, 2))
        self.assertEqual(Vector3d(1, 1, 1) - Vector3d(1, 1, 1), Vector3d())
        v = Vector3d(1, 1, 1)
        v += Vector3d(3, 3, 3)
        v -= Vector3d(2, 2, 2)
        v *= 2.0
        v /= 4.0
        self.assertEqual(v, Vector3d(1, 1, 1))
        self.assertEqual(v.cwiseProduct(Vector3d(2, 3, 4)), Vector3d(2, 3, 4))

    def testRotation(self):
        v = Vector3d(0, 1, 0).rotatedAround(UnitVector3d.X(), Angle(0.5 * math.pi))
        self.assertAlmostEqual(v.x(), 0.0, places=15)
        self.assertAlmostEqual(v.y(), 0.0, places=15)
        self.assertAlmostEqual(v.z(), 1.0, places=15)

    def testString(self):
        v = Vector3d(1, 0, 0)
        self.assertEqual(str(v), "[1.0, 0.0, 0.0]")
        self.assertEqual(repr(v), "Vector3d(1.0, 0.0, 0.0)")
        self.assertEqual(v, eval(repr(v), {"Vector3d": Vector3d}))

    def testPickle(self):
        v = Vector3d(1, 2, 3)
        w = pickle.loads(pickle.dumps(v))
        self.assertEqual(v, w)


if __name__ == "__main__":
    unittest.main()
