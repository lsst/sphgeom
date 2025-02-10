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

from lsst.sphgeom import CONTAINS, DISJOINT, Angle, Circle, Region, UnitVector3d


class CircleTestCase(unittest.TestCase):
    """Test Circle."""

    def setUp(self):
        np.random.seed(1)

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
        self.assertEqual(d.overlaps(c), True)
        self.assertEqual(e.relate(d), DISJOINT)
        self.assertEqual(e.overlaps(d), False)

    def test_vectorized_contains(self):
        b = Circle(UnitVector3d(*np.random.randn(3)), Angle(0.4 * math.pi))
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
        a = Circle.empty()
        b = (
            a.expandedTo(UnitVector3d.X())
            .expandedTo(Circle(UnitVector3d.Y(), 1))
            .clippedTo(Circle(UnitVector3d(1, 1, 0), 1))
            .clippedTo(UnitVector3d.Y())
        )
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
        self.assertEqual(str(c), "Circle([0.0, 0.0, 1.0], 1.0)")
        self.assertEqual(repr(c), "Circle(UnitVector3d(0.0, 0.0, 1.0), Angle(1.0))")
        self.assertEqual(c, eval(repr(c), {"Angle": Angle, "Circle": Circle, "UnitVector3d": UnitVector3d}))

    def test_pickle(self):
        a = Circle(UnitVector3d(1, -1, 1), 1.0)
        b = pickle.loads(pickle.dumps(a, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(a, b)

    @unittest.skipIf(not yaml, "YAML module can not be imported")
    def test_yaml(self):
        a = Circle(UnitVector3d(1, -1, 1), 1.0)
        b = yaml.safe_load(yaml.dump(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
