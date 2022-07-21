# This file is part of sphgeom.
#
# Developed for the LSST Data Management System.
# This product includes software developed by the LSST Project
# (http://www.lsst.org).
# See the COPYRIGHT file at the top-level directory of this distribution
# for details of code ownership.
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
__all__ = ["HealpixPixelization"]

import hpgeom as hpg
import numpy as np

from ._sphgeom import Box, Circle, ConvexPolygon, Ellipse, LonLat, RangeSet, Region, UnitVector3d
from .pixelization_abc import PixelizationABC


class HealpixPixelization(PixelizationABC):
    """HEALPix pixelization class.

    Parameters
    ----------
    level : `int`
        Pixelization level.  HEALPix nside = 2**level
    """

    MAX_LEVEL = 17

    def __init__(self, level: int):
        if level < 0:
            raise ValueError("HealPix level must be >= 0.")

        self._level = level
        self._nside = 2**level

        self._npix = hpg.nside_to_npixel(self._nside)

        # Values used to do pixel/region intersections
        self._bit_shift = 8
        self._nside_highres = self._nside * (2 ** (self._bit_shift // 2))

    @property
    def nside(self):
        return self._nside

    def getLevel(self):
        return self._level

    level = property(getLevel)

    def universe(self) -> RangeSet:
        return RangeSet(0, self._npix)

    def pixel(self, i) -> Region:
        # This is arbitrarily returning 4 points on a side
        # to approximate the pixel shape.
        varr = hpg.angle_to_vector(*hpg.boundaries(self._nside, i, step=4))
        return ConvexPolygon([UnitVector3d(*c) for c in varr])

    def index(self, v: UnitVector3d) -> int:
        return hpg.vector_to_pixel(self._nside, v.x(), v.y(), v.z())

    def toString(self, i: int) -> str:
        return str(i)

    def envelope(self, region: Region, maxRanges: int = 0):
        if maxRanges > 0:
            # If this is important, the rangeset can be consolidated.
            raise NotImplementedError("HealpixPixelization: maxRanges not implemented")
        pixels_highres = self._interior_pixels_from_region(self._nside_highres, region)

        # Dilate the high resolution pixels by one to ensure that the full
        # region is completely covered at high resolution.
        neighbors = hpg.neighbors(self._nside_highres, pixels_highres)
        # Shift back to the original resolution and uniquify
        pixels = np.unique(np.right_shift(neighbors.ravel(), self._bit_shift))

        return RangeSet(pixels)

    def interior(self, region: Region, maxRanges: int = 0):
        if maxRanges > 0:
            # If this is important, the rangeset can be consolidated.
            raise NotImplementedError("HealpixPixelization: maxRanges not implemented")
        pixels = self._interior_pixels_from_region(self._nside, region)

        # Check that the corners of the pixels are entirely enclosed in
        # the region

        # Returns arrays [npixels, ncorners], where ncorners is 4.
        corners_lon, corners_lat = hpg.boundaries(self._nside, pixels, step=1, degrees=False)

        corners_int = region.contains(corners_lon.ravel(), corners_lat.ravel()).reshape((len(pixels), 4))
        interior = np.sum(corners_int, axis=1) == 4
        pixels = pixels[interior]

        return RangeSet(pixels)

    def _interior_pixels_from_region(self, nside: int, region: Region):
        """Get interior pixels from a region.

        Parameters
        ----------
        nside : `int`
            Healpix nside to retrieve interior pixels.
        region : `lsst.sphgeom.Region`
            Sphgeom region to find interior pixels.

        Returns
        -------
        pixels : `np.ndarray`
            Array of pixels at resolution nside, nest ordering.
        """
        if isinstance(region, Circle):
            center = LonLat(region.getCenter())
            pixels = hpg.query_circle(
                nside,
                center.getLon().asRadians(),
                center.getLat().asRadians(),
                region.getOpeningAngle().asRadians(),
                degrees=False,
            )
        elif isinstance(region, ConvexPolygon):
            vertices = np.array([[v.x(), v.y(), v.z()] for v in region.getVertices()])
            pixels = hpg.query_polygon_vec(nside, vertices)
        elif isinstance(region, Box):
            pixels = hpg.query_box(
                nside,
                region.getLon().getA().asRadians(),
                region.getLon().getB().asRadians(),
                region.getLat().getA().asRadians(),
                region.getLat().getB().asRadians(),
                degrees=False,
            )
        elif isinstance(region, Ellipse):
            # hpgeom supports query_ellipse given center, alpha, beta,
            # and orientation. However, until we figure out how to get
            # the orientation out of the Ellipse region, we will use the
            # bounding circle as was done with healpy.
            _circle = region.getBoundingCircle()
            center = LonLat(_circle.getCenter())
            pixels = hpg.query_circle(
                nside,
                center.getLon().asRadians(),
                center.getLat().asRadians(),
                _circle.getOpeningAngle().asRadians(),
                degrees=False,
            )
        else:
            raise ValueError("Invalid region.")

        return pixels

    def __eq__(self, other):
        if isinstance(other, HealpixPixelization):
            return self._level == other._level

    def __repr__(self):
        return f"HealpixPixelization({self._level})"

    def __reduce__(self):
        return (self.__class__, (self._level,))
