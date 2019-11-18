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

import pickle
import unittest

from lsst.sphgeom import Angle


class AngleTestCase(unittest.TestCase):

    def testConstruction(self):
        a1 = Angle(1.0)
        a2 = Angle.fromRadians(1.0)
        a3 = Angle.fromDegrees(57.29577951308232)
        self.assertEqual(a1, a2)
        self.assertEqual(a1.asRadians(), 1.0)
        self.assertEqual(a1, a3)
        self.assertEqual(a1.asDegrees(), 57.29577951308232)

    def testComparisonOperators(self):
        a1 = Angle(1)
        a2 = Angle(2)
        self.assertNotEqual(a1, a2)
        self.assertLess(a1, a2)
        self.assertLessEqual(a1, a2)
        self.assertGreater(a2, a1)
        self.assertGreaterEqual(a2, a1)

    def testArithmeticOperators(self):
        a = Angle(1)
        b = -a
        self.assertEqual(a + b, Angle(0))
        self.assertEqual(a - b, 2.0 * a)
        self.assertEqual(a - b, a * 2.0)
        self.assertEqual(a / 1.0, a)
        self.assertEqual(a / a, 1.0)
        a += a
        a *= 2
        a -= b
        a /= 5
        self.assertEqual(a.asRadians(), 1)

    def testString(self):
        self.assertEqual(str(Angle(1)), '1.0')
        self.assertEqual(repr(Angle(1)), 'Angle(1.0)')
        a = Angle(2.5)
        self.assertEqual(a, eval(repr(a), dict(Angle=Angle)))

    def testPickle(self):
        a = Angle(1.5)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)


if __name__ == '__main__':
    unittest.main()
