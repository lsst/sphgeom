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

import math
import unittest

import numpy as np

from lsst.sphgeom import (
    CONTAINS,
    DISJOINT,
    Angle,
    AngleInterval,
    Box,
    LonLat,
    NormalizedAngle,
    NormalizedAngleInterval,
    Region,
    UnitVector3d,
)


class BoxTestCase(unittest.TestCase):
    """Test Box."""

    def setUp(self):
        np.random.seed(1)

    def test_construction(self):
        b = Box(Box.allLongitudes(), Box.allLatitudes())
        self.assertTrue(b.isFull())
        b = Box.fromDegrees(-90, -45, 90, 45)
        self.assertEqual(b, Box(b.getLon(), b.getLat()))
        a = Box.fromRadians(-0.5 * math.pi, -0.25 * math.pi, 0.5 * math.pi, 0.25 * math.pi)
        b = Box(
            LonLat.fromRadians(-0.5 * math.pi, -0.25 * math.pi),
            LonLat.fromRadians(0.5 * math.pi, 0.25 * math.pi),
        )
        c = Box(LonLat.fromRadians(0, 0), Angle(0.5 * math.pi), Angle(0.25 * math.pi))
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
        self.assertEqual(Box(LonLat.fromDegrees(45, 45)), LonLat.fromDegrees(45, 45))
        self.assertEqual(
            Box.fromDegrees(90, -45, 180, 45),
            Box(NormalizedAngleInterval.fromDegrees(90, 180), AngleInterval.fromDegrees(-45, 45)),
        )
        self.assertNotEqual(Box(LonLat.fromDegrees(45, 45)), LonLat.fromDegrees(45, 90))
        self.assertNotEqual(Box.fromDegrees(90, -45, 180, 45), Box.fromDegrees(90, -45, 180, 90))

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
        self.assertEqual(b1.overlaps(b4), False)
        r = b1.relate(LonLat.fromDegrees(135, 10))
        self.assertEqual(r, CONTAINS)
        r = b4.relate(b1)
        self.assertEqual(r, DISJOINT)
        self.assertEqual(b4.overlaps(b1), False)

    def test_vectorized_contains(self):
        b = Box.fromDegrees(200, 10, 300, 20)
        x = np.random.rand(5, 3)
        y = np.random.rand(5, 3)
        z = np.random.rand(5, 3)
        c = b.contains(x, y, z)
        lon = np.arctan2(y, x)
        lat = np.arctan2(z, np.hypot(x, y))
        c2 = b.contains(lon, lat)
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                u = UnitVector3d(x[i, j], y[i, j], z[i, j])
                self.assertEqual(c[i, j], b.contains(u))
                self.assertEqual(c2[i, j], b.contains(u))
        # test with non-contiguous memory
        c3 = b.contains(x[::2], y[::2], z[::2])
        c4 = b.contains(lon[::2], lat[::2])
        for i in range(x.shape[0], 2):
            for j in range(x.shape[1]):
                u = UnitVector3d(x[i, j], y[i, j], z[i, j])
                self.assertEqual(c3[i // 2, j], b.contains(u))
                self.assertEqual(c4[i // 2, j], b.contains(u))

    def test_expanding_and_clipping(self):
        a = Box.fromDegrees(0, 0, 10, 10)
        b = (
            a.expandedTo(LonLat.fromDegrees(20, 20))
            .expandedTo(Box.fromDegrees(0, 0, 30, 10))
            .clippedTo(Box.fromDegrees(10, 10, 15, 15))
            .clippedTo(LonLat.fromDegrees(11, 11))
        )
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
        self.assertEqual(str(b), "Box([0.0, 1.0], [0.0, 1.0])")
        self.assertEqual(
            repr(b),
            "Box(NormalizedAngleInterval.fromRadians(0.0, 1.0), AngleInterval.fromRadians(0.0, 1.0))",
        )
        self.assertEqual(
            b,
            eval(
                repr(b),
                {
                    "AngleInterval": AngleInterval,
                    "Box": Box,
                    "NormalizedAngleInterval": NormalizedAngleInterval,
                },
            ),
        )

    def test_pickle(self):
        a = Box.fromDegrees(0, 0, 10, 10)
        b = pickle.loads(pickle.dumps(a, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(a, b)

    @unittest.skipIf(not yaml, "YAML module can not be imported")
    def test_yaml(self):
        a = Box.fromDegrees(0, 0, 10, 10)
        b = yaml.safe_load(yaml.dump(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
