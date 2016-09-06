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
from builtins import str
from builtins import object

import unittest

from lsst.sphgeom import (Angle, AngleInterval,
                          NormalizedAngle, NormalizedAngleInterval,
                          CONTAINS, DISJOINT)


class IntervalTests(object):

    def testConstruction(self):
        i = self.Interval(self.Scalar(1))
        self.assertEqual(i.getA(), i.getB())
        self.assertEqual(i.getA(), self.Scalar(1))
        i = self.Interval(self.Scalar(1), self.Scalar(2))
        self.assertEqual(i, self.Interval.fromRadians(1, 2))
        self.assertTrue(self.Interval.empty().isEmpty())

    def testComparisonOperators(self):
        self.assertEqual(self.Interval(self.Scalar(1)),
                         self.Interval.fromRadians(1, 1))
        self.assertEqual(self.Interval(self.Scalar(1)), self.Scalar(1))
        self.assertNotEqual(self.Interval.fromDegrees(1, 1),
                            self.Interval.fromRadians(1, 1))
        self.assertNotEqual(self.Interval.fromDegrees(2, 2), self.Scalar(1))

    def testCenterAndSize(self):
        a = self.Interval.fromRadians(1, 2)
        self.assertEqual(a.getSize(), self.Scalar(1))
        self.assertEqual(a.getCenter(), self.Scalar(1.5))

    def testRelationships(self):
        a02 = self.Interval.fromRadians(0, 2)
        a13 = self.Interval.fromRadians(1, 3)
        a46 = self.Interval.fromRadians(4, 6)
        a06 = self.Interval.fromRadians(0, 6)
        self.assertTrue(a02.contains(self.Scalar(1)))
        self.assertTrue(a02.contains(self.Interval.fromRadians(0.5, 1.5)))
        self.assertTrue(a02.isDisjointFrom(self.Scalar(3)))
        self.assertTrue(a02.isDisjointFrom(a46))
        self.assertTrue(a02.intersects(self.Scalar(1)))
        self.assertTrue(a02.intersects(a13))
        self.assertTrue(self.Interval.fromRadians(1, 1).isWithin(a02))
        self.assertTrue(a02.isWithin(a06))
        r = a02.relate(self.Scalar(1))
        self.assertEqual(r, CONTAINS)
        r = a46.relate(a02)
        self.assertEqual(r, DISJOINT)

    def testExpandingAndClipping(self):
        a = self.Interval.fromRadians(1, 2)
        b = (
            a.expandedTo(self.Scalar(3))
            .expandedTo(self.Interval.fromRadians(2, 4))
            .clippedTo(self.Interval.fromRadians(0, 2))
            .clippedTo(self.Scalar(1))
        )
        a.expandTo(self.Scalar(3)).expandTo(self.Interval.fromRadians(2, 4))
        a.clipTo(self.Interval.fromRadians(0, 2)).clipTo(self.Scalar(1))
        self.assertEqual(a, b)
        self.assertEqual(a, self.Scalar(1))

    def testDilationAndErosion(self):
        a = self.Interval.fromRadians(1, 3)
        b = a.dilatedBy(self.Scalar(1)).erodedBy(self.Scalar(2))
        a.dilateBy(self.Scalar(1)).erodeBy(self.Scalar(2))
        self.assertEqual(a, b)
        self.assertEqual(a, self.Scalar(2))


class AngleIntervalTestCase(unittest.TestCase, IntervalTests):

    def setUp(self):
        self.Interval = AngleInterval
        self.Scalar = Angle

    def testString(self):
        self.assertEqual(str(AngleInterval.fromRadians(1, 1)),
                         "[1, 1]")
        self.assertEqual(repr(AngleInterval.fromRadians(1, 1)),
                         "AngleInterval.fromRadians(1.0, 1.0)")


class NormalizedAngleIntervalTestCase(unittest.TestCase, IntervalTests):

    def setUp(self):
        self.Interval = NormalizedAngleInterval
        self.Scalar = NormalizedAngle

    def testString(self):
        self.assertEqual(str(NormalizedAngleInterval.fromRadians(1, 1)),
                         "[1, 1]")
        self.assertEqual(repr(NormalizedAngleInterval.fromRadians(1, 1)),
                         "NormalizedAngleInterval.fromRadians(1.0, 1.0)")


if __name__ == "__main__":
    unittest.main()
