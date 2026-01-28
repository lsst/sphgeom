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

        # Try a circle region with zero-radius (representing a point)
        pt_circle = Circle(
            center=UnitVector3d(
                LonLat.fromDegrees((ra_range[0] + ra_range[1]) / 2.0, (dec_range[0] + dec_range[1]) / 2.0)
            ),
        )
        self._check_envelope(h, pt_circle, [98])

    def test_envelope_missing_neighbors(self):
        """Test envelope, with a pixel with missing neighbors.

        Testing DM-47043.
        """
        h = HealpixPixelization(0)
        h_highres = HealpixPixelization(4)

        self.assertEqual(h._nside_highres, h_highres._nside)

        # Find a pixel with a missing neighbor at high res.
        n = hpg.neighbors(h_highres._nside, np.arange(hpg.nside_to_npixel(h_highres._nside)))
        ind = np.argmin(n)
        self.assertEqual(n.ravel()[ind], -1)

        # This is the pixel with a bad (-1) neighbor.
        badpix = ind // 8
        ra, dec = hpg.pixel_to_angle(h_highres._nside, badpix)

        box = Box(
            point1=LonLat.fromDegrees(ra - 0.1, dec - 0.1),
            point2=LonLat.fromDegrees(ra + 0.1, dec + 0.1),
        )

        self._check_envelope(h, box, [0, 1, 5])

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

    def test_interior_nearly_degenerate_polygon(self):
        """Check interior pixels with nearly degenerate polygon.

        See DM-53933.
        """
        poly = ConvexPolygon(
            [
                UnitVector3d(-0.016144196513206758, -0.9235039286924489, -0.3832490816799893),
                UnitVector3d(-0.01988138785117867, -0.9228560783513854, -0.38463149775728545),
                UnitVector3d(-0.022348863683228713, -0.9209177038528237, -0.3891158066983548),
                UnitVector3d(-0.024841110338799998, -0.918545770328335, -0.3945333788782152),
                UnitVector3d(-0.026637426968850634, -0.9164294757774608, -0.39930873194901123),
                UnitVector3d(-0.02509225617228684, -0.9149335621264763, -0.4028237276709774),
                UnitVector3d(-0.0221015078570923, -0.911998223315482, -0.4095982959191202),
                UnitVector3d(-0.020553220684181986, -0.9104571830599119, -0.4130923418972051),
                UnitVector3d(-0.017371565494301272, -0.9072522623419655, -0.4202279871541906),
                UnitVector3d(-0.014347349160349979, -0.9041551644174409, -0.4269632211670476),
                UnitVector3d(-0.014303105070771129, -0.9041094310633312, -0.42706153871271785),
                UnitVector3d(-0.012802279210872009, -0.9025514012895127, -0.4303917630221838),
                UnitVector3d(-0.007862275539177377, -0.9015265103248066, -0.43265244227315125),
                UnitVector3d(-0.0019484762556141095, -0.900471339489277, -0.4349109911219402),
                UnitVector3d(0.0032282853511799436, -0.8996943229589627, -0.436508537613075),
                UnitVector3d(0.006969847869770022, -0.9003377930402403, -0.4351359323752772),
                UnitVector3d(0.022099717156107892, -0.9027944227880381, -0.42950417074160424),
                UnitVector3d(0.033589177231523826, -0.9045049275054283, -0.4251383342999184),
                UnitVector3d(0.03732521827078148, -0.9050320420971739, -0.4237025263772424),
                UnitVector3d(0.039795546158565454, -0.9070052460628258, -0.41923477684998206),
                UnitVector3d(0.04228779372661228, -0.9093772751891867, -0.41381724694752126),
                UnitVector3d(0.04409395578698435, -0.9114581951867685, -0.4090228373696683),
                UnitVector3d(0.04255005806475871, -0.9130743444969162, -0.4055671756691021),
                UnitVector3d(0.04109145924730725, -0.91458405813035, -0.4023027374885082),
                UnitVector3d(0.03791490987091337, -0.9178181221381234, -0.3951864803916365),
                UnitVector3d(0.03335712714052637, -0.9223387473238281, -0.38493965404208763),
                UnitVector3d(0.03180762744041508, -0.9238417869862389, -0.3814506881035668),
                UnitVector3d(0.030248610691484885, -0.9253347742243898, -0.3779425580195124),
                UnitVector3d(0.02530895801451361, -0.9263964395908428, -0.3756981412751856),
                UnitVector3d(0.01939515964342646, -0.9274517060282444, -0.37343963470379626),
                UnitVector3d(0.014215536919345694, -0.9281937989444676, -0.37182548340738003),
                UnitVector3d(0.010475126504663994, -0.9276710642902235, -0.3732514811803906),
            ],
        )

        h = HealpixPixelization(6)
        self._check_interior(h, poly, [28890, 28891, 28912, 28913, 28914])

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
