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

from lsst.sphgeom import Angle, Circle, Q3cPixelization, RangeSet, UnitVector3d


class Q3cPixelizationTestCase(unittest.TestCase):

    def test_construction(self):
        with self.assertRaises(ValueError):
            Q3cPixelization(-1)
        with self.assertRaises(ValueError):
            Q3cPixelization(Q3cPixelization.MAX_LEVEL + 1)
        pixelization = Q3cPixelization(0)
        self.assertEqual(pixelization.getLevel(), 0)

    def test_indexing(self):
        pixelization = Q3cPixelization(1)
        self.assertEqual(pixelization.index(UnitVector3d(0.5, -0.5, 1.0)), 0)

    def test_envelope_and_interior(self):
        pixelization = Q3cPixelization(1)
        c = Circle(UnitVector3d(1.0, -0.5, -0.5), Angle.fromDegrees(0.1))
        rs = pixelization.envelope(c)
        self.assertTrue(rs == RangeSet(4))
        rs = pixelization.envelope(c, 1)
        self.assertTrue(rs == RangeSet(4))
        self.assertTrue(rs.isWithin(pixelization.universe()))
        rs = pixelization.interior(c)
        self.assertTrue(rs.empty())


if __name__ == '__main__':
    unittest.main()
