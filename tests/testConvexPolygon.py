from __future__ import absolute_import, division

import sys
import unittest

from lsst.sphgeom import ConvexPolygon, Region, UnitVector3d, CONTAINS


class ConvexPolygonTestCase(unittest.TestCase):
    def testConstruction(self):
        points = [UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()]
        p1 = ConvexPolygon(points)
        self.assertEqual(points, p1.getVertices())
        p2 = ConvexPolygon.cast(p1.clone())
        self.assertEqual(p1, p2)
        p3 = ConvexPolygon([-UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        self.assertNotEqual(p1, p3)

    def testCodec(self):
        p = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        s = p.encode()
        self.assertEqual(ConvexPolygon.decode(s), p)
        self.assertEqual(ConvexPolygon.cast(Region.decode(s)), p)

    def testRelationships(self):
        p = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        self.assertTrue(p.contains(p.getCentroid()))
        self.assertEqual(p.getBoundingCircle().relate(p), CONTAINS)

    def testString(self):
        p = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        self.assertEqual(str(p), '{"ConvexPolygon": [[0, 0, 1], [1, 0, 0], [0, 1, 0]]}')
        self.assertEqual(repr(p),"ConvexPolygon([UnitVector3d(0.0, 0.0, 1.0), "
                                                 "UnitVector3d(1.0, 0.0, 0.0), "
                                                 "UnitVector3d(0.0, 1.0, 0.0)])")


def suite():
    return unittest.makeSuite(ConvexPolygonTestCase)

def run(shouldExit=False):
    status = 0 if unittest.TextTestRunner().run(suite()).wasSuccessful() else 1
    if shouldExit:
        sys.exit(status)
    return status

if __name__ == "__main__":
    run(True)
