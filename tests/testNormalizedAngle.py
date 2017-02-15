#!/usr/bin/env python
#
# LSST Data Management System
#
# Copyright 2008-2016  AURA/LSST.
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
from __future__ import absolute_import, division

import pickle
import unittest
from builtins import str

from lsst.sphgeom import Angle, LonLat, NormalizedAngle, UnitVector3d


class NormalizedAngleTestCase(unittest.TestCase):

    def testConstruction(self):
        a1 = NormalizedAngle(1.0)
        a2 = NormalizedAngle.fromRadians(1.0)
        a3 = NormalizedAngle.fromDegrees(57.29577951308232)
        self.assertEqual(a1, a2)
        self.assertEqual(a1.asRadians(), 1.0)
        self.assertEqual(a1, a3)
        self.assertEqual(a1.asDegrees(), 57.29577951308232)
        self.assertEqual(
            NormalizedAngle.between(NormalizedAngle(0), NormalizedAngle(1)),
            NormalizedAngle(1)
        )
        a = NormalizedAngle.center(NormalizedAngle(0), NormalizedAngle(1))
        self.assertAlmostEqual(a.asRadians(), 0.5, places=15)
        a = NormalizedAngle(LonLat.fromDegrees(45, 0),
                            LonLat.fromDegrees(90, 0))
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
        self.assertEqual(NormalizedAngle(1).getAngleTo(NormalizedAngle(2)),
                         NormalizedAngle(1))

    def testString(self):
        self.assertEqual(str(NormalizedAngle(1)), '1.0')
        self.assertEqual(repr(NormalizedAngle(1)), 'NormalizedAngle(1.0)')
        a = NormalizedAngle(0.5)
        self.assertEqual(
            a, eval(repr(a), dict(NormalizedAngle=NormalizedAngle)))

    def testPickle(self):
        a = NormalizedAngle(1.5)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)


if __name__ == '__main__':
    unittest.main()
