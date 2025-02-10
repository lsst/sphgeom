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

from lsst.sphgeom import CONTAINS, WITHIN, Angle, Circle, Ellipse, Region, UnitVector3d


class EllipseTestCase(unittest.TestCase):
    """Test Ellipse."""

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
        f = Ellipse(UnitVector3d.X(), Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        self.assertEqual(e, e)
        self.assertNotEqual(e, f)

    def test_center_and_dimensions(self):
        e = Ellipse(UnitVector3d.X(), UnitVector3d.Y(), Angle(2 * math.pi / 3))
        self.assertAlmostEqual(e.getF1().dot(UnitVector3d.X()), 1.0)
        self.assertAlmostEqual(e.getF2().dot(UnitVector3d.Y()), 1.0)
        self.assertAlmostEqual(e.getAlpha(), Angle(2 * math.pi / 3))
        f = Ellipse(UnitVector3d.X(), Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        self.assertEqual(f.getCenter(), UnitVector3d.X())

    def test_relationships(self):
        e = Ellipse(UnitVector3d.X(), Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        self.assertTrue(e.contains(UnitVector3d.X()))
        self.assertTrue(UnitVector3d.X() in e)
        c = Circle(UnitVector3d.X(), Angle(math.pi / 2))
        self.assertEqual(c.relate(e), CONTAINS)
        self.assertEqual(c.overlaps(e), True)
        self.assertEqual(e.relate(c), WITHIN)
        self.assertEqual(e.overlaps(c), True)

    def test_vectorized_contains(self):
        e = Ellipse(UnitVector3d.X(), Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        x = np.random.rand(5, 3)
        y = np.random.rand(5, 3)
        z = np.random.rand(5, 3)
        c = e.contains(x, y, z)
        lon = np.arctan2(y, x)
        lat = np.arctan2(z, np.hypot(x, y))
        c2 = e.contains(lon, lat)
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                u = UnitVector3d(x[i, j], y[i, j], z[i, j])
                self.assertEqual(c[i, j], e.contains(u))
                self.assertEqual(c2[i, j], e.contains(u))
        # test with non-contiguous memory
        c3 = e.contains(x[::2], y[::2], z[::2])
        c4 = e.contains(lon[::2], lat[::2])
        for i in range(x.shape[0], 2):
            for j in range(x.shape[1]):
                u = UnitVector3d(x[i, j], y[i, j], z[i, j])
                self.assertEqual(c3[i // 2, j], e.contains(u))
                self.assertEqual(c4[i // 2, j], e.contains(u))

    def test_complement(self):
        e = Ellipse(UnitVector3d.X(), Angle(math.pi / 3), Angle(math.pi / 6), Angle(0))
        f = e.complemented().complement()
        self.assertEqual(e, f)

    def test_codec(self):
        e = Ellipse(UnitVector3d.X(), UnitVector3d.Y(), Angle(2 * math.pi / 3))
        s = e.encode()
        self.assertEqual(Ellipse.decode(s), e)
        self.assertEqual(Region.decode(s), e)

    def test_string(self):
        c = Ellipse(UnitVector3d.Z(), Angle(1.0))
        self.assertEqual(str(c), "Ellipse([0.0, 0.0, 1.0], [0.0, 0.0, 1.0], 1.0)")
        self.assertEqual(
            repr(c), "Ellipse(UnitVector3d(0.0, 0.0, 1.0), UnitVector3d(0.0, 0.0, 1.0), Angle(1.0))"
        )
        self.assertEqual(c, eval(repr(c), {"Angle": Angle, "Ellipse": Ellipse, "UnitVector3d": UnitVector3d}))

    def test_pickle(self):
        a = Ellipse(UnitVector3d.X(), UnitVector3d.Y(), Angle(2 * math.pi / 3))
        b = pickle.loads(pickle.dumps(a, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(a, b)

    @unittest.skipIf(not yaml, "YAML module can not be imported")
    def test_yaml(self):
        a = Ellipse(UnitVector3d.X(), UnitVector3d.Y(), Angle(2 * math.pi / 3))
        b = yaml.safe_load(yaml.dump(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
