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

import numpy as np
from lsst.sphgeom import CONTAINS, Circle, ConvexPolygon, Region, UnitVector3d


class ConvexPolygonTestCase(unittest.TestCase):
    """Test ConvexPolygon."""

    def testConstruction(self):
        points = [UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()]
        p1 = ConvexPolygon(points)
        self.assertEqual(points, p1.getVertices())
        p2 = p1.clone()
        self.assertEqual(p1, p2)
        p3 = ConvexPolygon([-UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        self.assertNotEqual(p1, p3)
        p4 = ConvexPolygon.convexHull(
            [UnitVector3d.Y(), UnitVector3d.X(), UnitVector3d(1, 1, 1), UnitVector3d.Z()]
        )
        self.assertEqual(p1, p4)

    def testCodec(self):
        p = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        s = p.encode()
        self.assertEqual(ConvexPolygon.decode(s), p)
        self.assertEqual(Region.decode(s), p)

    def testRelationships(self):
        p = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        self.assertTrue(p.contains(p.getCentroid()))
        boundingCircle = p.getBoundingCircle()
        self.assertEqual(boundingCircle.relate(p), CONTAINS)
        self.assertTrue(p.isWithin(boundingCircle))
        self.assertTrue(p.intersects(boundingCircle))
        self.assertFalse(p.isDisjointFrom(boundingCircle))
        self.assertFalse(p.contains(boundingCircle))
        tinyCircle = Circle(boundingCircle.getCenter())
        self.assertFalse(p.isWithin(tinyCircle))
        self.assertTrue(p.intersects(tinyCircle))
        self.assertFalse(p.isDisjointFrom(tinyCircle))
        self.assertTrue(p.contains(tinyCircle))

    def test_vectorized_contains(self):
        b = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
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

    def testString(self):
        p = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        self.assertEqual(str(p), repr(p))
        self.assertEqual(
            repr(p),
            "ConvexPolygon([UnitVector3d(0.0, 0.0, 1.0), "
            "UnitVector3d(1.0, 0.0, 0.0), "
            "UnitVector3d(0.0, 1.0, 0.0)])",
        )
        self.assertEqual(p, eval(repr(p), {"ConvexPolygon": ConvexPolygon, "UnitVector3d": UnitVector3d}))

    def testPickle(self):
        a = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        b = pickle.loads(pickle.dumps(a, pickle.HIGHEST_PROTOCOL))
        self.assertEqual(a, b)

    @unittest.skipIf(not yaml, "YAML module can not be imported")
    def testYaml(self):
        a = ConvexPolygon([UnitVector3d.Z(), UnitVector3d.X(), UnitVector3d.Y()])
        b = yaml.safe_load(yaml.dump(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
