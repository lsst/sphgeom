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

import hpgeom as hpg
import numpy as np

try:
    import yaml
except ImportError:
    yaml = None

from lsst.sphgeom import Angle, Box, Circle, ConvexPolygon, Ellipse, HealpixPixelization, LonLat, UnitVector3d


class HealpixPixelizationTestCase(unittest.TestCase):
    """Test HEALPix pixelization."""

    def test_construction(self):
        """Test construction of a HealpixPixelization."""
        with self.assertRaises(ValueError):
            HealpixPixelization(-1)
        h1 = HealpixPixelization(5)
        self.assertEqual(h1.level, 5)
        self.assertEqual(h1.getLevel(), 5)
        self.assertEqual(h1.nside, 32)
        h2 = HealpixPixelization(6)
        h3 = HealpixPixelization(h2.level)
        self.assertNotEqual(h1, h2)
        self.assertEqual(h2, h3)

    def test_indexing(self):
        """Test indexing of HealpixPixelization."""
        h = HealpixPixelization(5)
        vec = UnitVector3d(1, 1, 1)
        lonlat = LonLat(vec)
        pix = hpg.angle_to_pixel(h.nside, lonlat.getLon().asDegrees(), lonlat.getLat().asDegrees())
        self.assertEqual(h.index(UnitVector3d(1, 1, 1)), pix)

    def test_pixel(self):
        """Test pixel polygon of HealpixPixelization."""
        h = HealpixPixelization(5)
        pix_poly = h.pixel(10)
        self.assertIsInstance(pix_poly, ConvexPolygon)

    def test_envelope(self):
        """Test envelope method of HealpixPixelization."""
        # Make the hardest intersection: a region that _just_
        # touches a healpix pixel.
        h = HealpixPixelization(5)
        pix = hpg.angle_to_pixel(h.nside, 50.0, 20.0)

        corners_ra, corners_dec = hpg.boundaries(h.nside, pix, step=1)

        # Take the southernmost corner...
        smost = np.argmin(corners_dec)

        # Choose challenging comparison box corners:
        ra_range = np.array([corners_ra[smost] - 0.5, corners_ra[smost] + 0.5])
        dec_range = np.array([corners_dec[smost] - 0.5, corners_dec[smost] + 1e-8])

        # Test the box region
        box = Box(
            point1=LonLat.fromDegrees(ra_range[0], dec_range[0]),
            point2=LonLat.fromDegrees(ra_range[1], dec_range[1]),
        )
        # These pixels have been checked to completely overlap the region
        self._check_envelope(h, box, [98, 99, 104, 105])

        # Try a polygon region:
        poly = ConvexPolygon(
            [
                UnitVector3d(LonLat.fromDegrees(ra_range[0], dec_range[0])),
                UnitVector3d(LonLat.fromDegrees(ra_range[1], dec_range[0])),
                UnitVector3d(LonLat.fromDegrees(ra_range[1], dec_range[1])),
                UnitVector3d(LonLat.fromDegrees(ra_range[0], dec_range[1])),
                UnitVector3d(LonLat.fromDegrees(ra_range[0], dec_range[0])),
            ]
        )
        self._check_envelope(h, poly, [98, 99, 104, 105])

        # Try a circle region
        circle = Circle(
            center=UnitVector3d(
                LonLat.fromDegrees((ra_range[0] + ra_range[1]) / 2.0, (dec_range[0] + dec_range[1]) / 2.0)
            ),
            angle=Angle.fromDegrees((dec_range[1] - dec_range[0]) / 2.0),
        )
        self._check_envelope(h, circle, [98, 99, 104, 105])

    def _check_envelope(self, pixelization, region, check_pixels):
        """Check the envelope from a region.

        Parameters
        ----------
        pixelization : `lsst.sphgeom.HealpixPixelization`
        region : `lsst.sphgeom.Region`
        check_pixels : `list` [`int`]
        """
        pixel_range = pixelization.envelope(region)

        pixels = []
        for r in pixel_range.ranges():
            pixels.extend(range(r[0], r[1]))

        self.assertEqual(pixels, check_pixels)

    def test_interior(self):
        """Test interior method of HealpixPixelization."""
        h = HealpixPixelization(5)
        pix = hpg.angle_to_pixel(h.nside, 50.0, 20.0)

        corners_ra, corners_dec = hpg.boundaries(h.nside, pix, step=1)

        ra_range = np.array([corners_ra.min() - 1.0, corners_ra.max() + 1.0])
        dec_range = np.array([corners_dec.min() - 1.0, corners_dec.max() + 1.0])

        # Test the box region
        box = Box(
            point1=LonLat.fromDegrees(ra_range[0], dec_range[0]),
            point2=LonLat.fromDegrees(ra_range[1], dec_range[1]),
        )
        # These pixels have been checked to completely overlap the region
        self._check_interior(h, box, [pix])

        # Try a polygon region:
        poly = ConvexPolygon(
            [
                UnitVector3d(LonLat.fromDegrees(ra_range[0], dec_range[0])),
                UnitVector3d(LonLat.fromDegrees(ra_range[1], dec_range[0])),
                UnitVector3d(LonLat.fromDegrees(ra_range[1], dec_range[1])),
                UnitVector3d(LonLat.fromDegrees(ra_range[0], dec_range[1])),
                UnitVector3d(LonLat.fromDegrees(ra_range[0], dec_range[0])),
            ]
        )
        self._check_interior(h, poly, [pix])

        # Try a circle region
        circle = Circle(
            center=UnitVector3d(
                LonLat.fromDegrees((ra_range[0] + ra_range[1]) / 2.0, (dec_range[0] + dec_range[1]) / 2.0)
            ),
            angle=Angle.fromDegrees(2.5),
        )
        self._check_interior(h, circle, [pix])

        # Try an ellipse region
        ellipse = Ellipse(
            center=UnitVector3d(
                LonLat.fromDegrees((ra_range[0] + ra_range[1]) / 2.0, (dec_range[0] + dec_range[1]) / 2.0)
            ),
            alpha=Angle.fromDegrees(1.5),
            beta=Angle.fromDegrees(2.5),
            orientation=Angle.fromDegrees(45.0),
        )
        self._check_interior(h, ellipse, [pix])

    def _check_interior(self, pixelization, region, check_pixels):
        """Check the interior from a region.

        Parameters
        ----------
        pixelization : `lsst.sphgeom.HealpixPixelization`
        region : `lsst.sphgeom.Region`
        check_pixels : `list` [`int`]
        """
        pixel_range = pixelization.interior(region)

        pixels = []
        for r in pixel_range.ranges():
            pixels.extend(range(r[0], r[1]))

        self.assertEqual(pixels, check_pixels)

    def test_index_to_string(self):
        """Test converting index to string of HealpixPixelization."""
        h = HealpixPixelization(5)
        self.assertEqual(h.toString(0), str(0))
        self.assertEqual(h.toString(100), str(100))

    def test_string(self):
        """Test string representation of HealpixPixelization."""
        h = HealpixPixelization(5)
        self.assertEqual(str(h), "HealpixPixelization(5)")
        self.assertEqual(str(h), repr(h))
        self.assertEqual(h, eval(repr(h), {"HealpixPixelization": HealpixPixelization}))

    def test_pickle(self):
        """Test pickling of HealpixPixelization."""
        a = HealpixPixelization(5)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)

    @unittest.skipIf(not yaml, "YAML module can not be imported")
    def test_yaml(self):
        """Test yaml representation of HealpixPixelization."""
        a = HealpixPixelization(5)
        b = yaml.safe_load(yaml.dump(a))
        self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
