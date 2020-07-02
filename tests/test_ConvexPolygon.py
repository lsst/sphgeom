#
# LSST Data Management System
# See COPYRIGHT file at the top of the source tree.
#
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
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
# You should have received a copy of the LSST License Statement and
# the GNU General Public License along with this program.  If not,
# see <https://www.lsstcorp.org/LegalNotices/>.
#

import pickle
import yaml

import unittest

from lsst.sphgeom import CONTAINS, ConvexPolygon, Circle, Region, UnitVector3d


class ConvexPolygonTestCase(unittest.TestCase):

    def testConstruction(self):
        points = [UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()]
        p1 = ConvexPolygon(points)
        self.assertEqual(points, p1.getVertices())
        p2 = p1.clone()
        self.assertEqual(p1, p2)
        p3 = ConvexPolygon([-UnitVector3d.Z(),
                            UnitVector3d.X(),
                            UnitVector3d.Y()])
        self.assertNotEqual(p1, p3)
        p4 = ConvexPolygon.convexHull([UnitVector3d.Y(),
                                       UnitVector3d.X(),
                                       UnitVector3d(1, 1, 1),
                                       UnitVector3d.Z()])
        self.assertEqual(p1, p4)

    def testCodec(self):
        p = ConvexPolygon([UnitVector3d.Z(),
                           UnitVector3d.X(),
                           UnitVector3d.Y()])
        s = p.encode()
        self.assertEqual(ConvexPolygon.decode(s), p)
        self.assertEqual(Region.decode(s), p)

    def testRelationships(self):
        p = ConvexPolygon([UnitVector3d.Z(),
                           UnitVector3d.X(),
                           UnitVector3d.Y()])
        self.assertTrue(p.contains(p.getCentroid()))
        boundingCircle = p.getBoundingCircle()
        self.assertEqual(boundingCircle.relate(p), CONTAINS)
        self.assertTrue(p.isWithin(boundingCircle))
        self.assertTrue(p.intersects(boundingCircle))
        self.assertFalse(p.isDisjointFrom(boundingCircle))
        self.assertFalse(p.contains(boundingCircle))
        tinyCircle = Circle(boundingCircle.getCenter())
        self.assertFalse(p.isWithin(tinyCircle))
        self.assertTrue(p.intersects(tinyCircle))
        self.assertFalse(p.isDisjointFrom(tinyCircle))
        self.assertTrue(p.contains(tinyCircle))

    def testString(self):
        p = ConvexPolygon([UnitVector3d.Z(),
                           UnitVector3d.X(),
                           UnitVector3d.Y()])
        self.assertEqual(str(p), repr(p))
        self.assertEqual(repr(p),
                         'ConvexPolygon([UnitVector3d(0.0, 0.0, 1.0), '
                         'UnitVector3d(1.0, 0.0, 0.0), '
                         'UnitVector3d(0.0, 1.0, 0.0)])')
        self.assertEqual(
            p, eval(repr(p), dict(ConvexPolygon=ConvexPolygon,
                                  UnitVector3d=UnitVector3d)))

    def testPickle(self):
        a = ConvexPolygon([UnitVector3d.Z(),
                           UnitVector3d.X(),
                           UnitVector3d.Y()])
        b = pickle.loads(pickle.dumps(a, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(a, b)

    def testYaml(self):
        a = ConvexPolygon([UnitVector3d.Z(),
                           UnitVector3d.X(),
                           UnitVector3d.Y()])
        b = yaml.safe_load(yaml.dump(a))
        self.assertEqual(a, b)


if __name__ == '__main__':
    unittest.main()
