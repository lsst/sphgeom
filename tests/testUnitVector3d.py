from __future__ import absolute_import, division

import math
import sys
import unittest

from lsst.sphgeom import Angle, LonLat, UnitVector3d, Vector3d


class UnitVector3dTestCase(unittest.TestCase):
    def testConstruction(self):
        v = Vector3d(1, 1, 1)
        u = UnitVector3d.orthogonalTo(v)
        self.assertAlmostEqual(u.dot(v), 0.0, places=15)
        u = UnitVector3d(1, 1, 1)
        self.assertEqual(u, UnitVector3d(Vector3d(1, 1, 1)))
        self.assertAlmostEqual(u.x(), math.sqrt(3.0) / 3.0, places=15)
        self.assertAlmostEqual(u.y(), math.sqrt(3.0) / 3.0, places=15)
        self.assertAlmostEqual(u.z(), math.sqrt(3.0) / 3.0, places=15)
        u = UnitVector3d(Angle.fromDegrees(45), Angle.fromDegrees(45))
        self.assertEqual(u, UnitVector3d(LonLat.fromDegrees(45, 45)))
        self.assertAlmostEqual(u.x(), 0.5, places=15)
        self.assertAlmostEqual(u.y(), 0.5, places=15)
        self.assertAlmostEqual(u.z(), 0.5 * math.sqrt(2.0), places=15)
        u = UnitVector3d.northFrom(u.asVector3d())
        w = UnitVector3d(LonLat.fromDegrees(225, 45))
        self.assertAlmostEqual(u.x(), w.x(), places=15)
        self.assertAlmostEqual(u.y(), w.y(), places=15)
        self.assertAlmostEqual(u.z(), w.z(), places=15)

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
        self.assertEqual(v[0:2], (1, 0))

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
        self.assertEqual(str(UnitVector3d(1, 0, 0)), "[1, 0, 0]")
        self.assertEqual(repr(UnitVector3d(1, 0, 0)), "UnitVector3d(1.0, 0.0, 0.0)")


def suite():
    return unittest.makeSuite(UnitVector3dTestCase)

def run(shouldExit=False):
    status = 0 if unittest.TextTestRunner().run(suite()).wasSuccessful() else 1
    if shouldExit:
        sys.exit(status)
    return status

if __name__ == "__main__":
    run(True)
