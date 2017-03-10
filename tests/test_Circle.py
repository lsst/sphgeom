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
from builtins import str

from lsst.sphgeom import (Angle, CONTAINS, Circle, DISJOINT, Region,
                          UnitVector3d)


class CircleTestCase(unittest.TestCase):

    def test_construction(self):
        self.assertTrue(Circle.empty().isEmpty())
        self.assertTrue(Circle().isEmpty())
        self.assertTrue(Circle.full().isFull())
        c = Circle(UnitVector3d.X())
        self.assertEqual(c.getOpeningAngle(), Angle(0))
        self.assertEqual(c.getSquaredChordLength(), 0)
        c = Circle(UnitVector3d.Z(), 2.0)
        self.assertTrue(c.contains(UnitVector3d.Z()))
        c = Circle(UnitVector3d.Z(), Angle(math.pi))
        self.assertTrue(c.isFull())
        d = c.clone()
        self.assertEqual(c, d)
        self.assertNotEqual(id(c), id(d))
        e = Circle(d)
        self.assertEqual(d, e)

    def test_comparison_operators(self):
        c = Circle(UnitVector3d.X(), 4.0)
        d = Circle(UnitVector3d.Y(), 4.0)
        self.assertEqual(c, d)
        self.assertTrue(c.isFull())
        self.assertNotEqual(c, Circle(UnitVector3d.Z()))

    def test_center_and_dimensions(self):
        c = Circle(UnitVector3d.X(), 1)
        self.assertEqual(c.getCenter(), UnitVector3d.X())
        self.assertEqual(c.getSquaredChordLength(), 1)
        self.assertAlmostEqual(c.getOpeningAngle().asRadians(), math.pi / 3)

    def test_relationships(self):
        c = Circle(UnitVector3d.X(), Angle.fromDegrees(0.1))
        d = Circle(UnitVector3d(1, 1, 1), Angle(math.pi / 2))
        e = Circle(-UnitVector3d.X())
        self.assertTrue(c.contains(UnitVector3d.X()))
        self.assertTrue(UnitVector3d.X() in c)
        self.assertTrue(d.contains(c))
        self.assertTrue(c.isWithin(d))
        self.assertTrue(c.intersects(d))
        self.assertTrue(c.intersects(UnitVector3d.X()))
        self.assertTrue(e.isDisjointFrom(d))
        self.assertEqual(d.relate(c), CONTAINS)
        self.assertEqual(e.relate(d), DISJOINT)

    def test_expanding_and_clipping(self):
        a = Circle.empty()
        b = (a.expandedTo(UnitVector3d.X())
              .expandedTo(Circle(UnitVector3d.Y(), 1))
              .clippedTo(Circle(UnitVector3d(1, 1, 0), 1))
              .clippedTo(UnitVector3d.Y()))
        a.expandTo(UnitVector3d.X())
        a.expandTo(Circle(UnitVector3d.Y(), 1))
        a.clipTo(Circle(UnitVector3d(1, 1, 0), 1))
        a.clipTo(UnitVector3d.Y())
        self.assertEqual(a, b)
        self.assertEqual(a, Circle(UnitVector3d.Y()))
        a.clipTo(UnitVector3d.Z())
        self.assertTrue(a.isEmpty())

    def test_dilation_and_erosion(self):
        a = Angle(math.pi / 2)
        c = Circle(UnitVector3d.X())
        d = c.dilatedBy(a).erodedBy(a)
        c.dilateBy(a).erodeBy(a)
        self.assertEqual(c, d)
        self.assertEqual(c, Circle(UnitVector3d.X()))

    def test_complement(self):
        c = Circle(UnitVector3d.X(), 2.0)
        d = c.complemented()
        c.complement()
        self.assertEqual(c, d)
        self.assertEqual(c.getCenter(), -UnitVector3d.X())
        self.assertEqual(c.getSquaredChordLength(), 2.0)

    def test_area(self):
        c = Circle(UnitVector3d(1, 1, 1), 2.0)
        self.assertAlmostEqual(c.getArea(), 2 * math.pi)

    def test_codec(self):
        c = Circle(UnitVector3d.Y(), 1.0)
        s = c.encode()
        self.assertEqual(Circle.decode(s), c)
        self.assertEqual(Region.decode(s), c)

    def test_string(self):
        c = Circle(UnitVector3d.Z(), Angle(1.0))
        self.assertEqual(str(c), 'Circle([0.0, 0.0, 1.0], 1.0)')
        self.assertEqual(repr(c),
                         'Circle(UnitVector3d(0.0, 0.0, 1.0), Angle(1.0))')
        self.assertEqual(c, eval(repr(c), dict(
            Angle=Angle, Circle=Circle, UnitVector3d=UnitVector3d)))

    def test_pickle(self):
        a = Circle(UnitVector3d(1, -1, 1), 1.0)
        b = pickle.loads(pickle.dumps(a, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(a, b)


if __name__ == '__main__':
    unittest.main()
