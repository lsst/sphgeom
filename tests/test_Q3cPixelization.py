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

import pickle
import unittest
from builtins import range

from lsst.sphgeom import Angle, Circle, Q3cPixelization, RangeSet, UnitVector3d


class Q3cPixelizationTestCase(unittest.TestCase):

    def test_construction(self):
        with self.assertRaises(ValueError):
            Q3cPixelization(-1)
        with self.assertRaises(ValueError):
            Q3cPixelization(Q3cPixelization.MAX_LEVEL + 1)
        q1 = Q3cPixelization(0)
        self.assertEqual(q1.getLevel(), 0)
        q2 = Q3cPixelization(1)
        q3 = Q3cPixelization(q2)
        self.assertNotEqual(q1, q2)
        self.assertEqual(q2, q3)

    def test_indexing(self):
        pixelization = Q3cPixelization(1)
        self.assertEqual(pixelization.index(UnitVector3d(0.5, -0.5, 1.0)), 0)

    def test_envelope_and_interior(self):
        pixelization = Q3cPixelization(1)
        c = Circle(UnitVector3d(1.0, -0.5, -0.5), Angle.fromDegrees(0.1))
        rs = pixelization.envelope(c)
        self.assertTrue(rs == RangeSet(4))
        rs = pixelization.envelope(c, 1)
        self.assertTrue(rs == RangeSet(4))
        self.assertTrue(rs.isWithin(pixelization.universe()))
        rs = pixelization.interior(c)
        self.assertTrue(rs.empty())

    def test_index_to_string(self):
        strings = ['+X', '+Y', '+Z', '-X', '-Y', '-Z']
        for i in range(6):
            s = strings[i]
            components = [0.0]*3
            components[i % 3] = 1.0 if i < 3 else -1.0
            v = UnitVector3d(*components)
            f = Q3cPixelization(0).index(v)
            self.assertEqual(Q3cPixelization(0).toString(f), s)
            for j in range(4):
                self.assertEqual(Q3cPixelization(1).toString(f*4 + j),
                                 s + str(j))

    def test_string(self):
        p = Q3cPixelization(3)
        self.assertEqual(str(p), 'Q3cPixelization(3)')
        self.assertEqual(str(p), repr(p))
        self.assertEqual(
            p, eval(repr(p), dict(Q3cPixelization=Q3cPixelization)))

    def test_pickle(self):
        a = Q3cPixelization(20)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)


if __name__ == '__main__':
    unittest.main()
