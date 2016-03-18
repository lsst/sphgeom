from __future__ import absolute_import, division

import sys
import unittest

from lsst.sphgeom import Angle, LonLat, NormalizedAngle, UnitVector3d


class NormalizedAngleTestCase(unittest.TestCase):
    def testConstruction(self):
        a1 = NormalizedAngle(1.0)
        a2 = NormalizedAngle.fromRadians(1.0)
        a3 = NormalizedAngle.fromDegrees(57.29577951308232)
        self.assertEqual(a1, a2)
        self.assertEqual(a1.asRadians(), 1.0)
        self.assertEqual(a1, a3)
        self.assertEqual(a1.asDegrees(), 57.29577951308232)
        self.assertEqual(NormalizedAngle.between(NormalizedAngle(0), NormalizedAngle(1)),
                         NormalizedAngle(1))
        a = NormalizedAngle.center(NormalizedAngle(0), NormalizedAngle(1))
        self.assertAlmostEqual(a.asRadians(), 0.5, places=15)
        a = NormalizedAngle(LonLat.fromDegrees(45, 0), LonLat.fromDegrees(90, 0))
        self.assertAlmostEqual(a.asDegrees(), 45.0, places=13)
        a = NormalizedAngle(UnitVector3d.Y(), UnitVector3d.Z())
        self.assertAlmostEqual(a.asDegrees(), 90.0, places=13)        

    def testComparisonOperators(self):
        a1 = NormalizedAngle(1)
        a2 = NormalizedAngle(2)
        self.assertNotEqual(a1, a2)
        self.assertLess(a1, a2)
        self.assertLessEqual(a1, a2)
        self.assertGreater(a2, a1)
        self.assertGreaterEqual(a2, a1)

    def testArithmeticOperators(self):
        a = NormalizedAngle(1)
        b = -a
        self.assertEqual(a + b, Angle(0))
        self.assertEqual(a - b, 2.0 * a)
        self.assertEqual(a - b, a * 2.0)
        self.assertEqual(a / 1.0, a)
        self.assertEqual(a / a, 1.0)

    def testAngleTo(self):
        self.assertEqual(NormalizedAngle(1).getAngleTo(NormalizedAngle(2)),
                         NormalizedAngle(1))

    def testString(self):
        self.assertEqual(str(NormalizedAngle(1)), "1.0")
        self.assertEqual(repr(NormalizedAngle(1)), "NormalizedAngle(1.0)")


def suite():
    return unittest.makeSuite(NormalizedAngleTestCase)

def run(shouldExit=False):
    status = 0 if unittest.TextTestRunner().run(suite()).wasSuccessful() else 1
    if shouldExit:
        sys.exit(status)
    return status

if __name__ == "__main__":
    run(True)
