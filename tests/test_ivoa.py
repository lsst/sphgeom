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

from lsst.sphgeom import Region


class IvoaTestCase(unittest.TestCase):
    """Test Box."""

    def test_construction(self):
        # Example POS strings found in IVOA documentation.
        example_pos = (
            "CIRCLE 12.0 34.0 0.5",
            "RANGE 12.0 12.5 34.0 36.0",
            "POLYGON 12.0 34.0 14.0 35.0 14. 36.0 12.0 35.0",
            "RANGE 0 360.0 -2.0 2.0",
            "RANGE 0 360.0 89.0 +Inf",
            "RANGE -Inf +Inf -Inf +Inf",
            "POLYGON 12 34 14 34 14 36 12 36",
            "RANGE 0 360 89 90",
        )
        for pos in example_pos:
            region = Region.from_ivoa_pos(pos)
            self.assertIsInstance(region, Region)

        # Badly formed strings raising ValueError.
        bad_pos = (
            "circle 12 34 0.5",
            "CIRCLE 12 34 1 1",
            "RANGE 0 360",
            "POLYGON 0 1 2 3",
            "POLYGON 0 1 2 3 4 5 6",
        )
        for pos in bad_pos:
            with self.assertRaises(ValueError):
                Region.from_ivoa_pos(pos)


if __name__ == "__main__":
    unittest.main()
