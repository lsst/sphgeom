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

import unittest

from lsst.sphgeom import Angle, LonLat, NormalizedAngle, UnitVector3d


class LonLatTestCase(unittest.TestCase):

    def testConstruction(self):
        p = LonLat.fromDegrees(45, 45)
        self.assertEqual(p, LonLat(NormalizedAngle.fromDegrees(45),
                                   Angle.fromDegrees(45)))
        u = UnitVector3d(p)
        q = LonLat(u)
        self.assertAlmostEqual(p.getLon().asRadians(), q.getLon().asRadians(), places=13)
        self.assertAlmostEqual(p.getLat().asRadians(), q.getLat().asRadians(), places=13)
        self.assertAlmostEqual(p.getLon().asRadians(), LonLat.latitudeOf(u).asRadians(), places=13)
        self.assertAlmostEqual(p.getLon().asRadians(), LonLat.longitudeOf(u).asRadians(), places=13)

    def testComparisonOperators(self):
        self.assertEqual(LonLat.fromDegrees(45, 45), LonLat.fromDegrees(45, 45))
        self.assertNotEqual(LonLat.fromDegrees(0, 0), LonLat.fromDegrees(45, 45))

    def testString(self):
        self.assertEqual(str(LonLat.fromRadians(1, 1)), "[1, 1]")
        self.assertEqual(repr(LonLat.fromRadians(1, 1)), "LonLat.fromRadians(1.0, 1.0)")


if __name__ == "__main__":
    unittest.main()
