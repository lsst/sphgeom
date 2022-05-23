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

import numpy as np

from ._sphgeom import RangeSet, Region, UnitVector3d
from ._sphgeom import Box, Circle, ConvexPolygon, Ellipse
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
        import healpy as hp

        if level < 0:
            raise ValueError("HealPix level must be >= 0.")

        self._level = level
        self._nside = 2**level

        self._npix = hp.nside2npix(self._nside)

        # Values used to do pixel/region intersections
        self._bit_shift = 8
        self._nside_highres = self._nside*(2**(self._bit_shift//2))

    @property
    def nside(self):
        return self._nside

    def getLevel(self):
        return self._level

    level = property(getLevel)

    def universe(self) -> RangeSet:
        return RangeSet(0, self._npix)

    def pixel(self, i) -> Region:
        import healpy as hp

        # This is arbitrarily returning 4 points on a side
        # to approximate the pixel shape.
        varr = hp.boundaries(self._nside, i, step=4, nest=True).T
        return ConvexPolygon([UnitVector3d(*c) for c in varr])

    def index(self, v: UnitVector3d) -> int:
        import healpy as hp
        return hp.vec2pix(self._nside, v.x(), v.y(), v.z(), nest=True)

    def toString(self, i: int) -> str:
        return str(i)

    def envelope(self, region: Region, maxRanges: int = 0):
        import healpy as hp

        if maxRanges > 0:
            # If this is important, the rangeset can be consolidated.
            raise NotImplementedError("HealpixPixelization: maxRanges not implemented")
        pixels_highres = self._interior_pixels_from_region(self._nside_highres, region)

        # Dilate the high resolution pixels by one to ensure that the full
        # region is completely covered at high resolution.
        neighbors = hp.get_all_neighbours(self._nside_highres,
                                          pixels_highres,
                                          nest=True)
        # Shift back to the original resolution and uniquify
        pixels = np.unique(np.right_shift(neighbors, self._bit_shift))

        return RangeSet(pixels)

    def interior(self, region: Region, maxRanges: int = 0):
        import healpy as hp

        if maxRanges > 0:
            # If this is important, the rangeset can be consolidated.
            raise NotImplementedError("HealpixPixelization: maxRanges not implemented")
        pixels = self._interior_pixels_from_region(self._nside, region)

        # Check that the corners of the pixels are entirely enclosed in
        # the region

        # Returns array [npixels, 3, ncorners], where ncorners is 4, and
        # the center index points to x, y, z.
        corners = hp.boundaries(self._nside, pixels, step=1, nest=True)

        corners_int = region.contains(corners[:, 0, :].ravel(),
                                      corners[:, 1, :].ravel(),
                                      corners[:, 2, :].ravel()).reshape((len(pixels), 4))
        interior = (np.sum(corners_int, axis=1) == 4)
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
        import healpy as hp

        _circle = None
        _poly = None
        if isinstance(region, (Box, Ellipse)):
            _circle = region.getBoundingCircle()
        elif isinstance(region, Circle):
            _circle = region
        elif isinstance(region, ConvexPolygon):
            _poly = region
        else:
            raise ValueError("Invalid region.")

        # Find all pixels at an arbitrarily higher resolution
        if _circle is not None:
            center = _circle.getCenter()
            vec = np.array([center.x(), center.y(), center.z()]).T
            pixels = hp.query_disc(nside,
                                   vec,
                                   _circle.getOpeningAngle().asRadians(),
                                   inclusive=False,
                                   nest=True)
        else:
            vertices = np.array([[v.x(), v.y(), v.z()] for v in _poly.getVertices()])
            pixels = hp.query_polygon(nside,
                                      vertices,
                                      inclusive=False,
                                      nest=True)

        return pixels

    def __eq__(self, other):
        if isinstance(other, HealpixPixelization):
            return (self._level == other._level)

    def __repr__(self):
        return f"HealpixPixelization({self._level})"

    def __reduce__(self):
        return (self.__class__, (self._level, ))
