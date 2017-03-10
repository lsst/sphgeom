#!/usr/bin/env python
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
from __future__ import absolute_import, division, print_function

try:
    import cPickle as pickle   # Use cPickle on Python 2.7
except ImportError:
    import pickle

import math
import unittest

from lsst.sphgeom import (Angle, AngleInterval, Box, CONTAINS, DISJOINT,
                          LonLat, NormalizedAngle, NormalizedAngleInterval,
                          Region, UnitVector3d)


class BoxTestCase(unittest.TestCase):

    def test_construction(self):
        b = Box(Box.allLongitudes(), Box.allLatitudes())
        self.assertTrue(b.isFull())
        b = Box.fromDegrees(-90, -45, 90, 45)
        self.assertEqual(b, Box(b.getLon(), b.getLat()))
        a = Box.fromRadians(-0.5 * math.pi, -0.25 * math.pi,
                            0.5 * math.pi, 0.25 * math.pi)
        b = Box(LonLat.fromRadians(-0.5 * math.pi, -0.25 * math.pi),
                LonLat.fromRadians(0.5 * math.pi, 0.25 * math.pi))
        c = Box(LonLat.fromRadians(0, 0),
                Angle(0.5 * math.pi), Angle(0.25 * math.pi))
        d = c.clone()
        self.assertEqual(a, b)
        self.assertEqual(b, c)
        self.assertEqual(c, d)
        self.assertNotEqual(id(c), id(d))
        b = Box()
        self.assertTrue(b.isEmpty())
        self.assertTrue(Box.empty().isEmpty())
        self.assertTrue(Box.full().isFull())

    def test_comparison_operators(self):
        self.assertEqual(Box(LonLat.fromDegrees(45, 45)),
                         LonLat.fromDegrees(45, 45))
        self.assertEqual(Box.fromDegrees(90, -45, 180, 45),
                         Box(NormalizedAngleInterval.fromDegrees(90, 180),
                             AngleInterval.fromDegrees(-45, 45)))
        self.assertNotEqual(Box(LonLat.fromDegrees(45, 45)),
                            LonLat.fromDegrees(45, 90))
        self.assertNotEqual(Box.fromDegrees(90, -45, 180, 45),
                            Box.fromDegrees(90, -45, 180, 90))

    def test_center_and_dimensions(self):
        b = Box.fromDegrees(-90, -45, 90, 45)
        self.assertEqual(b.getCenter(), LonLat.fromDegrees(0, 0))
        self.assertEqual(b.getWidth(), Angle.fromDegrees(180))
        self.assertEqual(b.getHeight(), Angle.fromDegrees(90))
        self.assertEqual(b.getLon().getA(), NormalizedAngle.fromDegrees(-90))
        self.assertEqual(b.getLat().getB(), Angle.fromDegrees(45))

    def test_relationships(self):
        b1 = Box.fromDegrees(90, 0, 180, 45)
        p = LonLat.fromDegrees(135, 10)
        self.assertTrue(p in b1)
        self.assertTrue(b1.contains(p))
        b2 = Box.fromDegrees(135, 15, 135, 30)
        self.assertTrue(b1.contains(b2))
        self.assertTrue(b2.isWithin(b1))
        b3 = Box.fromDegrees(0, -45, 90, 0)
        u = UnitVector3d(1, 1, -1)
        self.assertTrue(b1.intersects(b3))
        self.assertTrue(u in b3)
        self.assertTrue(b3.contains(u))
        b4 = Box.fromDegrees(200, 10, 300, 20)
        self.assertTrue(b1.isDisjointFrom(b4))
        r = b1.relate(LonLat.fromDegrees(135, 10))
        self.assertEqual(r, CONTAINS)
        r = b4.relate(b1)
        self.assertEqual(r, DISJOINT)

    def test_expanding_and_clipping(self):
        a = Box.fromDegrees(0, 0, 10, 10)
        b = (a.expandedTo(LonLat.fromDegrees(20, 20))
              .expandedTo(Box.fromDegrees(0, 0, 30, 10))
              .clippedTo(Box.fromDegrees(10, 10, 15, 15))
              .clippedTo(LonLat.fromDegrees(11, 11)))
        a.expandTo(LonLat.fromDegrees(20, 20))
        a.expandTo(Box.fromDegrees(0, 0, 30, 10))
        a.clipTo(Box.fromDegrees(10, 10, 15, 15))
        a.clipTo(LonLat.fromDegrees(11, 11))
        self.assertEqual(a, b)
        self.assertEqual(a, LonLat.fromDegrees(11, 11))
        a.clipTo(LonLat.fromDegrees(0, 0))
        self.assertTrue(a.isEmpty())

    def test_dilation_and_erosion(self):
        a = Box.fromRadians(0.5, -0.5, 1.5, 0.5)
        b = a.dilatedBy(Angle(0.5), Angle(0.5)).erodedBy(Angle(1), Angle(1))
        a.dilateBy(Angle(0.5), Angle(0.5)).erodeBy(Angle(1), Angle(1))
        self.assertEqual(a, b)
        self.assertEqual(a, LonLat.fromRadians(1, 0))

    def test_codec(self):
        b = Box.fromRadians(0, 0, 1, 1)
        s = b.encode()
        self.assertEqual(Box.decode(s), b)
        self.assertEqual(Region.decode(s), b)

    def test_string(self):
        b = Box.fromRadians(0, 0, 1, 1)
        self.assertEqual(str(b), 'Box([0.0, 1.0], [0.0, 1.0])')
        self.assertEqual(
            repr(b),
            'Box(NormalizedAngleInterval.fromRadians(0.0, 1.0), '
            'AngleInterval.fromRadians(0.0, 1.0))'
        )
        self.assertEqual(b, eval(repr(b), dict(
            AngleInterval=AngleInterval, Box=Box,
            NormalizedAngleInterval=NormalizedAngleInterval
        )))

    def test_pickle(self):
        a = Box.fromDegrees(0, 0, 10, 10)
        b = pickle.loads(pickle.dumps(a, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(a, b)


if __name__ == '__main__':
    unittest.main()
