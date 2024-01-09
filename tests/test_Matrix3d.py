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

from lsst.sphgeom import Matrix3d, Vector3d


class Matrix3dTestCase(unittest.TestCase):
    """Test 3D Matrix."""

    def testConstruction(self):
        self.assertEqual(Matrix3d().getSquaredNorm(), 0)
        self.assertEqual(Matrix3d(1, 0, 0, 0, 1, 0, 0, 0, 1), Matrix3d(1))

    def testComparison(self):
        m = Matrix3d(1, 2, 3, 4, 5, 6, 7, 8, 9)
        self.assertEqual(m, m)
        self.assertNotEqual(m, Matrix3d(2))

    def testIter(self):
        m = Matrix3d(1, 2, 3, 4, 5, 6, 7, 8, 9)
        r = 0
        for row in m:
            self.assertEqual(row, Vector3d(*range(r * 3 + 1, r * 3 + 4)))
            r += 1

    def testAccess(self):
        m = Matrix3d(1, 2, 3, 4, 5, 6, 7, 8, 9)
        self.assertEqual(len(m), 3)
        self.assertEqual(m[0], Vector3d(1, 2, 3))
        self.assertEqual(m[1], Vector3d(4, 5, 6))
        self.assertEqual(m[2], Vector3d(7, 8, 9))
        self.assertEqual(m[0], m.getRow(0))
        self.assertEqual(m[1], m.getRow(1))
        self.assertEqual(m[2], m.getRow(2))
        n = m.transpose()
        self.assertEqual(m.getColumn(0), n[0])
        self.assertEqual(m.getColumn(1), n[1])
        self.assertEqual(m.getColumn(2), n[2])
        for r in range(3):
            for c in range(3):
                self.assertEqual(m[r, c], r * 3 + c + 1)
        for i in (-4, 3):
            with self.assertRaises(IndexError):
                m.getRow(i)
            with self.assertRaises(IndexError):
                m.getColumn(i)
        for i in [-4, 3, (-4, 1), (3, 1), (1, -4), (1, 3)]:
            with self.assertRaises(IndexError):
                m[i]

    def testInner(self):
        m = Matrix3d(1, 2, 3, 4, 5, 6, 7, 8, 9)
        i = Matrix3d(1)
        self.assertEqual(m.inner(i), i.inner(m))
        self.assertEqual(m.inner(i), 15.0)

    def testNorm(self):
        m = Matrix3d(1, 2, 3, 4, 5, 6, 7, 0, 2)
        self.assertEqual(m.getSquaredNorm(), 144.0)
        self.assertEqual(m.getNorm(), 12.0)

    def testArithmetic(self):
        v = Vector3d(1, 2, 3)
        m = Matrix3d(1, -1, 0, 1, 1, 0, 0, 0, 1)
        n = Matrix3d(1, 1, 0, -1, 1, 0, 0, 0, 1)
        self.assertEqual(n * (m * v), Vector3d(2, 4, 3))
        self.assertEqual(m + m, m * Matrix3d(2))
        self.assertEqual(m, m * Matrix3d(2) - m)

    def testCwiseProduct(self):
        m = Matrix3d(1, 2, 3, 4, 1, 6, 7, 8, 1)
        self.assertEqual(m.cwiseProduct(Matrix3d(2)), Matrix3d(2))

    def testTranspose(self):
        m = Matrix3d(1, 2, 3, 4, 5, 6, 7, 8, 9)
        n = Matrix3d(1, 4, 7, 2, 5, 8, 3, 6, 9)
        self.assertEqual(m.transpose(), n)
        self.assertEqual(m.transpose().transpose(), m)

    def testInverse(self):
        m = Matrix3d(4, 4, 4, -1, 1, 0, 1, -1, -1)
        n = Matrix3d(0.125, 0, 0.5, 0.125, 1, 0.5, 0, -1, -1)
        i = m.inverse()
        self.assertEqual(i, n)
        self.assertEqual(i.inverse(), m)
        self.assertEqual(i * m, Matrix3d(1))
        self.assertEqual(m * i, Matrix3d(1))

    def testString(self):
        m = Matrix3d(1, 2, 3, 4, 5, 6, 7, 8, 9)
        self.assertEqual(str(m), "[[1.0, 2.0, 3.0],\n [4.0, 5.0, 6.0],\n [7.0, 8.0, 9.0]]")
        self.assertEqual(repr(m), "Matrix3d(1.0, 2.0, 3.0,\n         4.0, 5.0, 6.0,\n         7.0, 8.0, 9.0)")
        self.assertEqual(m, eval(repr(m), {"Matrix3d": Matrix3d}))

    def testPickle(self):
        m = Matrix3d(1, 2, 3, 4, 5, 6, 7, 8, 9)
        n = pickle.loads(pickle.dumps(m))
        self.assertEqual(m, n)


if __name__ == "__main__":
    unittest.main()
