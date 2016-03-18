from __future__ import absolute_import, division

import sys
import unittest

from lsst.sphgeom import Angle, LonLat, NormalizedAngle, UnitVector3d


class LonLatTestCase(unittest.TestCase):
    def testConstruction(self):
        p = LonLat.fromDegrees(45, 45)
        self.assertEqual(p, LonLat(NormalizedAngle.fromDegrees(45),
                                   Angle.fromDegrees(45)))
        u = UnitVector3d(p)
        q = LonLat(u)
        self.assertAlmostEqual(p.getLon().asRadians(), q.getLon().asRadians(), places=13)
        self.assertAlmostEqual(p.getLat().asRadians(), q.getLat().asRadians(), places=13)
        self.assertAlmostEqual(p.getLon().asRadians(), LonLat.latitudeOf(u).asRadians(), places=13)
        self.assertAlmostEqual(p.getLon().asRadians(), LonLat.longitudeOf(u).asRadians(), places=13)

    def testComparisonOperators(self):
        self.assertEqual(LonLat.fromDegrees(45, 45), LonLat.fromDegrees(45, 45))
        self.assertNotEqual(LonLat.fromDegrees(0, 0), LonLat.fromDegrees(45, 45))

    def testString(self):
        self.assertEqual(str(LonLat.fromRadians(1, 1)), "[1, 1]")
        self.assertEqual(repr(LonLat.fromRadians(1, 1)), "LonLat.fromRadians(1.0, 1.0)")


def suite():
    return unittest.makeSuite(LonLatTestCase)

def run(shouldExit=False):
    status = 0 if unittest.TextTestRunner().run(suite()).wasSuccessful() else 1
    if shouldExit:
        sys.exit(status)
    return status

if __name__ == "__main__":
    run(True)
