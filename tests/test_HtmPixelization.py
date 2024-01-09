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

from lsst.sphgeom import Angle, Circle, ConvexPolygon, HtmPixelization, RangeSet, UnitVector3d


class HtmPixelizationTestCase(unittest.TestCase):
    """Test HTM pixels."""

    def test_construction(self):
        with self.assertRaises(ValueError):
            HtmPixelization(-1)
        with self.assertRaises(ValueError):
            HtmPixelization(HtmPixelization.MAX_LEVEL + 1)
        h1 = HtmPixelization(0)
        self.assertEqual(h1.getLevel(), 0)
        h2 = HtmPixelization(1)
        h3 = HtmPixelization(h2)
        self.assertNotEqual(h1, h2)
        self.assertEqual(h2, h3)

    def test_indexing(self):
        h = HtmPixelization(1)
        self.assertEqual(h.index(UnitVector3d(1, 1, 1)), 63)

    def test_pixel(self):
        h = HtmPixelization(1)
        self.assertIsInstance(h.pixel(10), ConvexPolygon)

    def test_level(self):
        for index in (0, 16 * 4**HtmPixelization.MAX_LEVEL):
            self.assertEqual(HtmPixelization.level(index), -1)
        for level in range(HtmPixelization.MAX_LEVEL + 1):
            for root in range(8, 16):
                self.assertEqual(HtmPixelization.level(root * 4**level), level)

    def test_envelope_and_interior(self):
        pixelization = HtmPixelization(3)
        c = Circle(UnitVector3d(1, 1, 1), Angle.fromDegrees(0.1))
        rs = pixelization.envelope(c)
        self.assertTrue(rs == RangeSet(0x3FF))
        rs = pixelization.envelope(c, 1)
        self.assertTrue(rs == RangeSet(0x3FF))
        self.assertTrue(rs.isWithin(pixelization.universe()))
        rs = pixelization.interior(c)
        self.assertTrue(rs.empty())

    def test_index_to_string(self):
        strings = ["S0", "S1", "S2", "S3", "N0", "N1", "N2", "N3"]
        for i in range(8, 16):
            s0 = strings[i - 8]
            self.assertEqual(HtmPixelization.asString(i), s0)
            self.assertEqual(HtmPixelization(0).toString(i), s0)
            for j in range(4):
                s1 = s0 + str(j)
                self.assertEqual(HtmPixelization.asString(i * 4 + j), s1)
                self.assertEqual(HtmPixelization(1).asString(i * 4 + j), s1)

    def test_string(self):
        p = HtmPixelization(3)
        self.assertEqual(str(p), "HtmPixelization(3)")
        self.assertEqual(str(p), repr(p))
        self.assertEqual(p, eval(repr(p), {"HtmPixelization": HtmPixelization}))

    def test_pickle(self):
        a = HtmPixelization(20)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)

    @unittest.skipIf(not yaml, "YAML module can not be imported")
    def test_yaml(self):
        a = HtmPixelization(20)
        b = yaml.safe_load(yaml.dump(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
