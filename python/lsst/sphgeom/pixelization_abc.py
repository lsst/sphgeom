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
__all__ = ["PixelizationABC"]

import abc

from ._sphgeom import RangeSet, Region, UnitVector3d


class PixelizationABC(abc.ABC):
    """Pixelization ABC class that should be a base for
    Python implementations of pixelization.
    """

    @abc.abstractmethod
    def universe(self) -> RangeSet:
        """Return the set of all pixel indexes for this pixelization.

        Returns
        -------
        rangeSet : `lsst.sphgeom.RangeSet`
        """
        pass

    @abc.abstractmethod
    def pixel(self, i) -> Region:
        """Return the spherical region corresponding to the pixel index ``i``.

        This region will contain all unit vectors v with ``index(v) == i``.
        But it may also contain points with index not equal to ``i``.
        To see why, consider a point that lies on the edge of a polygonal
        pixel - it is inside the polygons for both pixels sharing the edge,
        but must be assigned to exactly one pixel by the pixelization.

        Parameters
        ----------
        i : `int`
            Pixel index.

        Returns
        -------
        region : `lsst.sphgeom.Region`
            The spherical region corresponding to the pixel with index ``i``

        Raises
        ------
        `InvalidArgumentException`
            Raised if ``i`` is not a valid pixel index.
        """
        pass

    @abc.abstractmethod
    def index(self, v: UnitVector3d) -> int:
        """Compute the index of the pixel.

        Parameters
        ----------
        v : `lsst.sphgeom.UnitVector3d`

        Returns
        -------
        i : `int`
            The index of the pixel.
        """
        pass

    @abc.abstractmethod
    def toString(self, i: int) -> str:
        """Convert the given pixel index to a human-readable string.

        Parameters
        ----------
        i : `int`

        Returns
        -------
        s : `str`
        """
        pass

    @abc.abstractmethod
    def envelope(self, region: Region, maxRanges: int = 0):
        """Return the indexes of the pixels intersecting the spherical region.

        The ``maxRanges`` parameter can be used to limit both these costs -
        setting it to a non-zero value sets a cap on the number of ranges
        returned by this method. To meet this constraint, implementations are
        allowed to return pixels that do not intersect the region along with
        those, that do.
        This allows two ranges [a, b) and [c, d), a < b < c < d, to be
        merged into one range [a, d) (by adding in the pixels [b, c)). Since
        simplification proceeds by adding pixels, the return value will always
        be a superset of the intersecting pixels.

        Parameters
        ----------
        region : `lsst.sphgeom.Region`
        maxRanges : `int`

        Returns
        -------
        rangeSet : `lsst.sphgeom.RangeSet`
        """
        pass

    @abc.abstractmethod
    def interior(self, region: Region, maxRanges: int = 0):
        """Return the indexes of the pixels within the spherical region.

        The ``maxRanges`` argument is analogous to the identically named
        envelope() argument. The only difference is that implementations must
        remove interior pixels to keep the number of ranges at or below the
        maximum. The return value is therefore always a subset of the interior
        pixels.

        Parameters
        ----------
        region : `lsst.sphgeom.Region`
        maxRanges : `int`

        Returns
        -------
        rangeSet : `lsst.sphgeom.RangeSet`
        """
        pass
