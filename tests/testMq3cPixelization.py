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

from lsst.sphgeom import (Angle, Circle, Mq3cPixelization, RangeSet,
                          UnitVector3d)


class Mq3cPixelizationTestCase(unittest.TestCase):

    def test_construction(self):
        with self.assertRaises(ValueError):
            Mq3cPixelization(-1)
        with self.assertRaises(ValueError):
            Mq3cPixelization(Mq3cPixelization.MAX_LEVEL + 1)
        pixelization = Mq3cPixelization(0)
        self.assertEqual(pixelization.getLevel(), 0)

    def test_indexing(self):
        pixelization = Mq3cPixelization(1)
        self.assertEqual(pixelization.index(UnitVector3d(0.5, -0.5, 1.0)), 53)

    def test_level(self):
        self.assertEqual(Mq3cPixelization.level(0), -1)
        for level in range(Mq3cPixelization.MAX_LEVEL + 1):
            for root in range(8, 10):
                index = root * 4**level
                self.assertEqual(Mq3cPixelization.level(index), -1)
            for root in range(10, 16):
                index = root * 4**level
                self.assertEqual(Mq3cPixelization.level(index), level)

    def test_envelope_and_interior(self):
        pixelization = Mq3cPixelization(1)
        c = Circle(UnitVector3d(1.0, -0.5, -0.5), Angle.fromDegrees(0.1))
        rs = pixelization.envelope(c)
        self.assertTrue(rs == RangeSet(44))
        rs = pixelization.envelope(c, 1)
        self.assertTrue(rs == RangeSet(44))
        self.assertTrue(rs.isWithin(pixelization.universe()))
        rs = pixelization.interior(c)
        self.assertTrue(rs.empty())


if __name__ == '__main__':
    unittest.main()
