from __future__ import absolute_import, division

import sys
import unittest

from lsst.sphgeom import Angle


class AngleTestCase(unittest.TestCase):
    def testConstruction(self):
        a1 = Angle(1.0)
        a2 = Angle.fromRadians(1.0)
        a3 = Angle.fromDegrees(57.29577951308232)
        self.assertEqual(a1, a2)
        self.assertEqual(a1.asRadians(), 1.0)
        self.assertEqual(a1, a3)
        self.assertEqual(a1.asDegrees(), 57.29577951308232)

    def testComparisonOperators(self):
        a1 = Angle(1)
        a2 = Angle(2)
        self.assertNotEqual(a1, a2)
        self.assertLess(a1, a2)
        self.assertLessEqual(a1, a2)
        self.assertGreater(a2, a1)
        self.assertGreaterEqual(a2, a1)

    def testArithmeticOperators(self):
        a = Angle(1)
        b = -a
        self.assertEqual(a + b, Angle(0))
        self.assertEqual(a - b, 2.0 * a)
        self.assertEqual(a - b, a * 2.0)
        self.assertEqual(a / 1.0, a)
        self.assertEqual(a / a, 1.0)
        a += a
        a *= 2
        a -= b
        a /= 5
        self.assertEqual(a.asRadians(), 1)

    def testString(self):
        self.assertEqual(str(Angle(1)), "1.0")
        self.assertEqual(repr(Angle(1)), "Angle(1.0)")


def suite():
    return unittest.makeSuite(AngleTestCase)

def run(shouldExit=False):
    status = 0 if unittest.TextTestRunner().run(suite()).wasSuccessful() else 1
    if shouldExit:
        sys.exit(status)
    return status

if __name__ == "__main__":
    run(True)
