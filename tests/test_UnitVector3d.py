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

from lsst.sphgeom import Angle, LonLat, UnitVector3d, Vector3d


class UnitVector3dTestCase(unittest.TestCase):
    """Test 3D UnitVector."""

    def testConstruction(self):
        v = Vector3d(1, 1, 1)
        u = UnitVector3d.orthogonalTo(v)
        self.assertAlmostEqual(u.dot(v), 0.0, places=15)
        a = UnitVector3d(1, 1, 1)
        self.assertEqual(a, UnitVector3d(Vector3d(1, 1, 1)))
        self.assertAlmostEqual(a.x(), math.sqrt(3.0) / 3.0, places=15)
        self.assertAlmostEqual(a.y(), math.sqrt(3.0) / 3.0, places=15)
        self.assertAlmostEqual(a.z(), math.sqrt(3.0) / 3.0, places=15)
        b = UnitVector3d(Angle.fromDegrees(45), Angle.fromDegrees(45))
        self.assertEqual(b, UnitVector3d(LonLat.fromDegrees(45, 45)))
        self.assertAlmostEqual(b.x(), 0.5, places=15)
        self.assertAlmostEqual(b.y(), 0.5, places=15)
        self.assertAlmostEqual(b.z(), 0.5 * math.sqrt(2.0), places=15)
        c = UnitVector3d.northFrom(b)
        d = UnitVector3d(LonLat.fromDegrees(225, 45))
        self.assertAlmostEqual(c.x(), d.x(), places=15)
        self.assertAlmostEqual(c.y(), d.y(), places=15)
        self.assertAlmostEqual(c.z(), d.z(), places=15)

    def testComparison(self):
        self.assertEqual(UnitVector3d.X(), UnitVector3d.X())
        self.assertNotEqual(UnitVector3d.Y(), UnitVector3d.Z())

    def testAccess(self):
        v = UnitVector3d.X()
        self.assertEqual(len(v), 3)
        self.assertEqual(v[0], 1)
        self.assertEqual(v[1], 0)
        self.assertEqual(v[2], 0)
        self.assertEqual(v[-3], 1)
        self.assertEqual(v[-2], 0)
        self.assertEqual(v[-1], 0)
        with self.assertRaises(IndexError):
            v[-4]
        with self.assertRaises(IndexError):
            v[3]

    def testDot(self):
        self.assertEqual(UnitVector3d.X().dot(UnitVector3d.Z()), 0)

    def testCross(self):
        self.assertEqual(UnitVector3d.X().cross(UnitVector3d.Y()), Vector3d(0, 0, 1))
        self.assertEqual(UnitVector3d.X().robustCross(UnitVector3d.Y()), Vector3d(0, 0, 2))

    def testArithmeticOperators(self):
        self.assertEqual(-UnitVector3d.X(), UnitVector3d(-1, 0, 0))
        self.assertEqual(UnitVector3d.X() - UnitVector3d.X(), Vector3d(0, 0, 0))
        self.assertEqual(UnitVector3d.X() + UnitVector3d(1, 0, 0), UnitVector3d.X() * 2)
        self.assertEqual(UnitVector3d.Y() - Vector3d(0, 0.5, 0), UnitVector3d.Y() / 2)
        self.assertEqual(UnitVector3d.Z().cwiseProduct(Vector3d(2, 3, 4)), Vector3d(0, 0, 4))

    def testRotation(self):
        v = UnitVector3d.Y().rotatedAround(UnitVector3d.X(), Angle(0.5 * math.pi))
        self.assertAlmostEqual(v.x(), 0.0, places=15)
        self.assertAlmostEqual(v.y(), 0.0, places=15)
        self.assertAlmostEqual(v.z(), 1.0, places=15)

    def testString(self):
        v = UnitVector3d.X()
        self.assertEqual(str(v), "[1.0, 0.0, 0.0]")
        self.assertEqual(repr(v), "UnitVector3d(1.0, 0.0, 0.0)")
        self.assertEqual(v, eval(repr(v), {"UnitVector3d": UnitVector3d}))

    def testPickle(self):
        v = UnitVector3d(1, 1, 1)
        w = pickle.loads(pickle.dumps(v, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(v, w)


if __name__ == "__main__":
    unittest.main()
