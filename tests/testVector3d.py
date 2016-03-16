from __future__ import absolute_import, division

import math
import sys
import unittest

from lsst.sphgeom import Angle, UnitVector3d, Vector3d


class Vector3dTestCase(unittest.TestCase):
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
        self.assertEqual(v[0:2], (1, 2))

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
        self.assertEqual(str(Vector3d(1, 0, 0)), "[1, 0, 0]")
        self.assertEqual(repr(Vector3d(1, 0, 0)), "Vector3d(1.0, 0.0, 0.0)")


def suite():
    return unittest.makeSuite(Vector3dTestCase)

def run(shouldExit=False):
    status = 0 if unittest.TextTestRunner().run(suite()).wasSuccessful() else 1
    if shouldExit:
        sys.exit(status)
    return status

if __name__ == "__main__":
    run(True)
