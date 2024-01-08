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

from lsst.sphgeom import Angle, LonLat, NormalizedAngle, UnitVector3d


class NormalizedAngleTestCase(unittest.TestCase):
    """Test normalized angle."""

    def testConstruction(self):
        a1 = NormalizedAngle(1.0)
        a2 = NormalizedAngle.fromRadians(1.0)
        a3 = NormalizedAngle.fromDegrees(57.29577951308232)
        self.assertEqual(a1, a2)
        self.assertEqual(a1.asRadians(), 1.0)
        self.assertEqual(a1, a3)
        self.assertEqual(a1.asDegrees(), 57.29577951308232)
        self.assertEqual(NormalizedAngle.between(NormalizedAngle(0), NormalizedAngle(1)), NormalizedAngle(1))
        a = NormalizedAngle.center(NormalizedAngle(0), NormalizedAngle(1))
        self.assertAlmostEqual(a.asRadians(), 0.5, places=15)
        a = NormalizedAngle(LonLat.fromDegrees(45, 0), LonLat.fromDegrees(90, 0))
        self.assertAlmostEqual(a.asDegrees(), 45.0, places=13)
        a = NormalizedAngle(UnitVector3d.Y(), UnitVector3d.Z())
        self.assertAlmostEqual(a.asDegrees(), 90.0, places=13)

    def testComparisonOperators(self):
        a1 = NormalizedAngle(1)
        a2 = NormalizedAngle(2)
        self.assertNotEqual(a1, a2)
        self.assertLess(a1, a2)
        self.assertLessEqual(a1, a2)
        self.assertGreater(a2, a1)
        self.assertGreaterEqual(a2, a1)

    def testArithmeticOperators(self):
        a = NormalizedAngle(1)
        b = -a
        self.assertEqual(a + b, Angle(0))
        self.assertEqual(a - b, 2.0 * a)
        self.assertEqual(a - b, a * 2.0)
        self.assertEqual(a / 1.0, a)
        self.assertEqual(a / a, 1.0)

    def testAngleTo(self):
        self.assertEqual(NormalizedAngle(1).getAngleTo(NormalizedAngle(2)), NormalizedAngle(1))

    def testString(self):
        self.assertEqual(str(NormalizedAngle(1)), "1.0")
        self.assertEqual(repr(NormalizedAngle(1)), "NormalizedAngle(1.0)")
        a = NormalizedAngle(0.5)
        self.assertEqual(a, eval(repr(a), {"NormalizedAngle": NormalizedAngle}))

    def testPickle(self):
        a = NormalizedAngle(1.5)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
