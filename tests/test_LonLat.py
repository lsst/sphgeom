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


class LonLatTestCase(unittest.TestCase):
    """Test LonLat."""

    def testConstruction(self):
        p = LonLat.fromDegrees(45, 45)
        self.assertEqual(p, LonLat(NormalizedAngle.fromDegrees(45), Angle.fromDegrees(45)))
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
        p = LonLat.fromRadians(1, 1)
        self.assertEqual(str(p), "[1.0, 1.0]")
        self.assertEqual(repr(p), "LonLat.fromRadians(1.0, 1.0)")
        self.assertEqual(p, eval(repr(p), {"LonLat": LonLat}))

    def testPickle(self):
        p = LonLat.fromRadians(2, 1)
        q = pickle.loads(pickle.dumps(p))
        self.assertEqual(p, q)


if __name__ == "__main__":
    unittest.main()
