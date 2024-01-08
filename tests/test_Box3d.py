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
import unittest

import numpy as np
from lsst.sphgeom import CONTAINS, DISJOINT, Box3d, Interval1d, Vector3d


class Box3dTestCase(unittest.TestCase):
    """Test Box3d."""

    def setUp(self):
        np.random.seed(1)

    def test_construction(self):
        a = Box3d(Vector3d(0, 0, 0))
        b = Box3d(a)
        self.assertEqual(a, b)
        self.assertNotEqual(id(a), id(b))
        a = Box3d(Vector3d(1, 2, 3), Vector3d(3, 4, 5))
        b = Box3d(Interval1d(1, 3), Interval1d(2, 4), Interval1d(3, 5))
        c = Box3d(Vector3d(2, 3, 4), 1, 1, 1)
        self.assertEqual(a, b)
        self.assertEqual(b, c)
        i = Interval1d(1, 2)
        self.assertEqual(i, Interval1d(1, 2))
        self.assertTrue(Interval1d.empty().isEmpty())

    def test_comparison_operators(self):
        self.assertEqual(Box3d(Vector3d(1, 1, 1)), Vector3d(1, 1, 1))
        self.assertEqual(Box3d(Vector3d(1, 1, 1)), Box3d(Vector3d(1, 1, 1), Vector3d(1, 1, 1)))
        self.assertEqual(Box3d(Vector3d(0, 0, 0), 1, 1, 1), Box3d(Vector3d(-1, -1, -1), Vector3d(1, 1, 1)))
        self.assertNotEqual(Box3d(Vector3d(0, 0, 0), 1, 1, 1), Box3d(Vector3d(-1, -1, -1), Vector3d(1, 1, 2)))
        self.assertNotEqual(Box3d(Vector3d(0, 0, 0), 1, 1, 1), Vector3d(1, 1, 1))

    def test_center_and_dimensions(self):
        b = Box3d(Vector3d(1.5, 1.5, 1.5), 0.5, 1.0, 1.5)
        self.assertEqual(b[0], b[-3])
        self.assertEqual(b[1], b[-2])
        self.assertEqual(b[2], b[-1])
        self.assertEqual(b[0], b.x())
        self.assertEqual(b[1], b.y())
        self.assertEqual(b[2], b.z())
        with self.assertRaises(IndexError):
            b[-4]
        with self.assertRaises(IndexError):
            b[3]
        self.assertEqual(b.x(), Interval1d(1, 2))
        self.assertEqual(b.y(), Interval1d(0.5, 2.5))
        self.assertEqual(b.z(), Interval1d(0, 3))
        self.assertEqual(b.getCenter(), Vector3d(1.5, 1.5, 1.5))
        self.assertEqual(b.getWidth(), 1)
        self.assertEqual(b.getHeight(), 2)
        self.assertEqual(b.getDepth(), 3)
        self.assertEqual(b.isEmpty(), False)
        self.assertEqual(b.isFull(), False)

    def test_relationships(self):
        b02 = Box3d(Interval1d(0, 2), Interval1d(0, 2), Interval1d(0, 2))
        b13 = Box3d(Interval1d(1, 3), Interval1d(1, 3), Interval1d(1, 3))
        b46 = Box3d(Interval1d(4, 6), Interval1d(4, 6), Interval1d(4, 6))
        b06 = Box3d(Interval1d(0, 6), Interval1d(0, 6), Interval1d(0, 6))
        self.assertTrue(b02.contains(Vector3d(1, 1, 1)))
        self.assertTrue(b02.contains(Box3d(Vector3d(1, 1, 1), 0.5, 0.5, 0.5)))
        self.assertTrue(b02.isDisjointFrom(Vector3d(3, 3, 3)))
        self.assertTrue(b02.isDisjointFrom(b46))
        self.assertTrue(b02.intersects(Vector3d(1, 1, 1)))
        self.assertTrue(b02.intersects(b13))
        self.assertTrue(Box3d(Vector3d(1, 1, 1), 0, 0, 0).isWithin(b02))
        self.assertTrue(b02.isWithin(b06))
        r = b02.relate(Vector3d(1, 1, 1))
        self.assertEqual(r, CONTAINS)
        r = b46.relate(b02)
        self.assertEqual(r, DISJOINT)

    def test_vectorized_contains(self):
        b = Box3d.aroundUnitSphere()
        x = np.random.rand(5, 3)
        y = np.random.rand(5, 3)
        z = np.random.rand(5, 3)
        c = b.contains(x, y, z)
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                u = Vector3d(x[i, j], y[i, j], z[i, j])
                self.assertEqual(c[i, j], b.contains(u))
        # test with non-contiguous memory
        c2 = b.contains(x[::2], y[::2], z[::2])
        for i in range(x.shape[0], 2):
            for j in range(x.shape[1]):
                u = Vector3d(x[i, j], y[i, j], z[i, j])
                self.assertEqual(c2[i // 2, j], b.contains(u))

    def test_expanding_and_clipping(self):
        a = Box3d(Vector3d(1, 1, 1), Vector3d(2, 2, 2))
        b = (
            a.expandedTo(Vector3d(3, 3, 3))
            .expandedTo(Box3d(Vector3d(3, 3, 3), 1, 1, 1))
            .clippedTo(Box3d(Vector3d(1, 1, 1), 1, 1, 1))
            .clippedTo(Vector3d(1, 1, 1))
        )
        a.expandTo(Vector3d(3, 3, 3))
        a.expandTo(Box3d(Vector3d(3, 3, 3), 1, 1, 1))
        a.clipTo(Box3d(Vector3d(1, 1, 1), 1, 1, 1))
        a.clipTo(Vector3d(1, 1, 1))
        self.assertEqual(a, b)
        self.assertEqual(a, Vector3d(1, 1, 1))
        a.clipTo(Vector3d(0, 0, 0))
        self.assertTrue(a.isEmpty())

    def test_dilation_and_erosion(self):
        a = Box3d(Vector3d(0, 0, 0), 1, 1, 1)
        b = a.dilatedBy(1).erodedBy(2)
        a.dilateBy(1).erodeBy(2)
        self.assertEqual(a, b)
        self.assertEqual(a, Vector3d(0, 0, 0))

    def test_string(self):
        b = Box3d(Vector3d(0, 0, 0), 1, 1, 1)
        self.assertEqual(str(b), "[[-1.0, 1.0],\n [-1.0, 1.0],\n [-1.0, 1.0]]")
        self.assertEqual(
            repr(b),
            "Box3d(Interval1d(-1.0, 1.0),\n      Interval1d(-1.0, 1.0),\n      Interval1d(-1.0, 1.0))",
        )
        self.assertEqual(b, eval(repr(b), {"Box3d": Box3d, "Interval1d": Interval1d}))

    def test_pickle(self):
        a = Box3d(Vector3d(0, 0, 0), 1, 1, 1)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
