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
from __future__ import absolute_import, division

try:
    import cPickle as pickle   # Use cPickle on Python 2.7
except ImportError:
    import pickle

import math
import unittest
from builtins import str

from lsst.sphgeom import (Angle, CONTAINS, Circle, Ellipse, Region,
                          UnitVector3d, WITHIN)


class EllipseTestCase(unittest.TestCase):

    def test_construction(self):
        self.assertTrue(Ellipse.empty().isEmpty())
        self.assertTrue(Ellipse().isEmpty())
        self.assertTrue(Ellipse.full().isFull())
        e = Ellipse(Circle(UnitVector3d.X(), Angle(math.pi / 2)))
        f = Ellipse(UnitVector3d.X(), Angle(math.pi / 2))
        self.assertEqual(e, f)
        self.assertEqual(e.getAlpha(), e.getBeta())
        self.assertTrue(e.isCircle())
        self.assertTrue(e.isGreatCircle())
        g = Ellipse(e)
        h = e.clone()
        self.assertEqual(e, g)
        self.assertEqual(g, h)
        self.assertNotEqual(id(e), id(g))
        self.assertNotEqual(id(g), id(h))

    def test_comparison_operators(self):
        e = Ellipse(UnitVector3d.X(), UnitVector3d.Y(), Angle(2 * math.pi / 3))
        f = Ellipse(UnitVector3d.X(),
                    Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        self.assertEqual(e, e)
        self.assertNotEqual(e, f)

    def test_center_and_dimensions(self):
        e = Ellipse(UnitVector3d.X(), UnitVector3d.Y(), Angle(2 * math.pi / 3))
        self.assertAlmostEqual(e.getF1().dot(UnitVector3d.X()), 1.0)
        self.assertAlmostEqual(e.getF2().dot(UnitVector3d.Y()), 1.0)
        self.assertAlmostEqual(e.getAlpha(), Angle(2 * math.pi / 3))
        f = Ellipse(UnitVector3d.X(),
                    Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        self.assertEqual(f.getCenter(), UnitVector3d.X())

    def test_relationships(self):
        e = Ellipse(UnitVector3d.X(),
                    Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        self.assertTrue(e.contains(UnitVector3d.X()))
        self.assertTrue(UnitVector3d.X() in e)
        c = Circle(UnitVector3d.X(), Angle(math.pi / 2))
        self.assertEqual(c.relate(e), CONTAINS)
        self.assertEqual(e.relate(c), WITHIN)

    def test_complement(self):
        e = Ellipse(UnitVector3d.X(),
                    Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        f = e.complemented().complement()
        self.assertEqual(e, f)

    def test_codec(self):
        e = Ellipse(UnitVector3d.X(), UnitVector3d.Y(), Angle(2 * math.pi / 3))
        s = e.encode()
        self.assertEqual(Ellipse.decode(s), e)
        self.assertEqual(Region.decode(s), e)

    def test_string(self):
        c = Ellipse(UnitVector3d.Z(), Angle(1.0))
        self.assertEqual(str(c),
                         'Ellipse([0.0, 0.0, 1.0], [0.0, 0.0, 1.0], 1.0)')
        self.assertEqual(repr(c),
                         'Ellipse(UnitVector3d(0.0, 0.0, 1.0), '
                         'UnitVector3d(0.0, 0.0, 1.0), Angle(1.0))')
        self.assertEqual(c, eval(repr(c), dict(
            Angle=Angle, Ellipse=Ellipse, UnitVector3d=UnitVector3d)))

    def test_pickle(self):
        a = Ellipse(UnitVector3d.X(), UnitVector3d.Y(), Angle(2 * math.pi / 3))
        b = pickle.loads(pickle.dumps(a, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(a, b)


if __name__ == '__main__':
    unittest.main()
