#!/usr/bin/env python
#
# LSST Data Management System
#
# Copyright 2008-2016  AURA/LSST.
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
from __future__ import absolute_import, division

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
        self.assertEqual(repr(p), "ConvexPolygon([UnitVector3d(0.0, 0.0, 1.0), "
                         "UnitVector3d(1.0, 0.0, 0.0), "
                         "UnitVector3d(0.0, 1.0, 0.0)])")


if __name__ == "__main__":
    unittest.main()
