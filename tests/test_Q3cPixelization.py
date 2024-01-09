# This file is part of sphgeom.
#
# Developed for the LSST Data Management System.
# This product includes software developed by the LSST Project
# (http://www.lsst.org).
# See the COPYRIGHT file at the top-level directory of this distribution
# for details of code ownership.
#
# This software is dual licensed under the GNU General Public License and also
# under a 3-clause BSD license. Recipients may choose which of these licenses
# to use; please see the files gpl-3.0.txt and/or bsd_license.txt,
# respectively.  If you choose the GPL option then the following text applies
# (but note that there is still no warranty even if you opt for BSD instead):
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
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import pickle

try:
    import yaml
except ImportError:
    yaml = None

import unittest

from lsst.sphgeom import Angle, Circle, ConvexPolygon, Q3cPixelization, RangeSet, UnitVector3d


class Q3cPixelizationTestCase(unittest.TestCase):
    """Test Q3C pixelization."""

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

    def test_pixel(self):
        h = Q3cPixelization(1)
        self.assertIsInstance(h.pixel(10), ConvexPolygon)

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
        strings = ["+X", "+Y", "+Z", "-X", "-Y", "-Z"]
        for i in range(6):
            s = strings[i]
            components = [0.0] * 3
            components[i % 3] = 1.0 if i < 3 else -1.0
            v = UnitVector3d(*components)
            f = Q3cPixelization(0).index(v)
            self.assertEqual(Q3cPixelization(0).toString(f), s)
            for j in range(4):
                self.assertEqual(Q3cPixelization(1).toString(f * 4 + j), s + str(j))

    def test_string(self):
        p = Q3cPixelization(3)
        self.assertEqual(str(p), "Q3cPixelization(3)")
        self.assertEqual(str(p), repr(p))
        self.assertEqual(p, eval(repr(p), {"Q3cPixelization": Q3cPixelization}))

    def test_pickle(self):
        a = Q3cPixelization(20)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)

    @unittest.skipIf(not yaml, "YAML module can not be imported")
    def test_yaml(self):
        a = Q3cPixelization(20)
        b = yaml.safe_load(yaml.dump(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
