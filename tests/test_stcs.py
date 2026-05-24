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

import unittest

from lsst.sphgeom import (
    Angle,
    Circle,
    LonLat,
    Region,
    UnitVector3d,
)


class StcsTestCase(unittest.TestCase):
    """Test STC-S string generation."""

    def assert_stcs_equal(self, stcs1: str, stcs2: str):
        """Compare two STC-S strings, treating numeric tokens with a
        floating-point tolerance and string tokens (shape/operator/frame
        keywords, parentheses) by exact match.
        """
        toks1 = stcs1.replace("(", " ( ").replace(")", " ) ").split()
        toks2 = stcs2.replace("(", " ( ").replace(")", " ) ").split()
        self.assertEqual(len(toks1), len(toks2), f"{stcs1!r} vs {stcs2!r}")
        for t1, t2 in zip(toks1, toks2, strict=True):
            try:
                f1 = float(t1)
                f2 = float(t2)
            except ValueError:
                self.assertEqual(t1, t2)
            else:
                self.assertAlmostEqual(f1, f2, places=6)

    def test_base_region_raises(self):
        """The Region base class default ``_ivoa_stcs_body`` raises
        NotImplementedError; ``to_ivoa_stcs`` is defined only on the base
        class and delegates to ``_ivoa_stcs_body``.
        """
        circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)), Angle.fromDegrees(1.0))
        with self.assertRaises(NotImplementedError):
            Region._ivoa_stcs_body(circle)


if __name__ == "__main__":
    unittest.main()
