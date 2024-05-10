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
#

"""Extend any of the C++ Python classes by adding additional methods."""

# Nothing to export.
__all__ = []

import math
import sys
import typing

from ._sphgeom import Angle, Box, Circle, ConvexPolygon, LonLat, Region, UnitVector3d

# Copy and paste from lsst.utils.wrappers:
# * INTRINSIC_SPECIAL_ATTRIBUTES
# * isAttributeSafeToTransfer
# * continueClass
_INTRINSIC_SPECIAL_ATTRIBUTES = frozenset(
    (
        "__qualname__",
        "__module__",
        "__metaclass__",
        "__dict__",
        "__weakref__",
        "__class__",
        "__subclasshook__",
        "__name__",
        "__doc__",
    )
)


def _isAttributeSafeToTransfer(name: str, value: typing.Any) -> bool:
    if name.startswith("__") and (
        value is getattr(object, name, None) or name in _INTRINSIC_SPECIAL_ATTRIBUTES
    ):
        return False
    return True


def _continueClass(cls):
    orig = getattr(sys.modules[cls.__module__], cls.__name__)
    for name in dir(cls):
        # Common descriptors like classmethod and staticmethod can only be
        # accessed without invoking their magic if we use __dict__; if we use
        # getattr on those we'll get e.g. a bound method instance on the dummy
        # class rather than a classmethod instance we can put on the target
        # class.
        attr = cls.__dict__.get(name, None) or getattr(cls, name)
        if _isAttributeSafeToTransfer(name, attr):
            setattr(orig, name, attr)
    return orig


def _inf_to_limit(value: float, min: float, max: float) -> float:
    """Map a value to a fixed range if infinite."""
    if not math.isinf(value):
        return value
    if value > 0.0:
        return max
    return min


def _inf_to_lat(lat: float) -> float:
    """Map latitude +Inf to +90 and -Inf to -90 degrees."""
    return _inf_to_limit(lat, -90.0, 90.0)


def _inf_to_lon(lat: float) -> float:
    """Map longitude +Inf to +360 and -Inf to 0 degrees."""
    return _inf_to_limit(lat, 0.0, 360.0)


@_continueClass
class Region:
    """A minimal interface for 2-dimensional regions on the unit sphere."""

    @classmethod
    def from_ivoa_pos(cls, pos: str) -> Region:
        """Create a Region from an IVOA POS string.

        Parameters
        ----------
        pos : `str`
            A string using the IVOA SIAv2 POS syntax.

        Returns
        -------
        region : `Region`
            A region equivalent to the POS string.

        Notes
        -----
        See
        https://ivoa.net/documents/SIA/20151223/REC-SIA-2.0-20151223.html#toc12
        for a description of the POS parameter but in summary the options are:

        * ``CIRCLE <longitude> <latitude> <radius>``
        * ``RANGE <longitude1> <longitude2> <latitude1> <latitude2>``
        * ``POLYGON <longitude1> <latitude1> ... (at least 3 pairs)``

        Units are degrees in all coordinates.
        """
        shape, *coordinates = pos.split()
        coordinates = tuple(float(c) for c in coordinates)
        n_floats = len(coordinates)
        if shape == "CIRCLE":
            if n_floats != 3:
                raise ValueError(f"CIRCLE requires 3 numbers but got {n_floats} in '{pos}'.")
            center = LonLat.fromDegrees(coordinates[0], coordinates[1])
            radius = Angle.fromDegrees(coordinates[2])
            return Circle(UnitVector3d(center), radius)

        if shape == "RANGE":
            if n_floats != 4:
                raise ValueError(f"RANGE requires 4 numbers but got {n_floats} in '{pos}'.")
            # POS allows +Inf and -Inf in ranges. These are not allowed by
            # Box and so must be converted.
            return Box(
                LonLat.fromDegrees(_inf_to_lon(coordinates[0]), _inf_to_lat(coordinates[2])),
                LonLat.fromDegrees(_inf_to_lon(coordinates[1]), _inf_to_lat(coordinates[3])),
            )

        if shape == "POLYGON":
            if n_floats % 2 != 0:
                raise ValueError(f"POLYGON requires even number of floats but got {n_floats} in '{pos}'.")
            if n_floats < 6:
                raise ValueError(
                    f"POLYGON specification requires at least 3 coordinates, got {n_floats // 2} in '{pos}'"
                )
            # Coordinates are x1, y1, x2, y2, x3, y3...
            # Get pairs by skipping every other value.
            pairs = list(zip(coordinates[0::2], coordinates[1::2]))
            vertices = [LonLat.fromDegrees(lon, lat) for lon, lat in pairs]
            return ConvexPolygon([UnitVector3d(c) for c in vertices])

        raise ValueError(f"Unrecognized shape in POS string '{pos}'")

    def to_ivoa_pos(self) -> str:
        """Represent the region as an IVOA POS string.

        Returns
        -------
        pos : `str`
            The region in ``POS`` format.
        """
        raise NotImplementedError("This region can not be converted to an IVOA POS string.")


@_continueClass
class Circle:  # noqa: F811
    """A circular region on the unit sphere that contains its boundary."""

    def to_ivoa_pos(self) -> str:
        # Docstring inherited.
        center = LonLat(self.getCenter())
        lon = center.getLon().asDegrees()
        lat = center.getLat().asDegrees()
        rad = self.getOpeningAngle().asDegrees()
        return f"CIRCLE {lon} {lat} {rad}"


@_continueClass
class Box:  # noqa: F811
    """A rectangle in spherical coordinate space that contains its boundary."""

    def to_ivoa_pos(self) -> str:
        # Docstring inherited.
        lon_range = self.getLon()
        lat_range = self.getLat()

        lon1 = lon_range.getA().asDegrees()
        lon2 = lon_range.getB().asDegrees()
        lat1 = lat_range.getA().asDegrees()
        lat2 = lat_range.getB().asDegrees()

        # Do not attempt to map to +/- Inf -- there is no way to know if
        # that is any better than 0. -> 360.
        return f"RANGE {lon1} {lon2} {lat1} {lat2}"


@_continueClass
class ConvexPolygon:  # noqa: F811
    """A rectangle in spherical coordinate space that contains its boundary."""

    def to_ivoa_pos(self) -> str:
        # Docstring inherited.
        coords = (LonLat(v) for v in self.getVertices())
        coord_strings = [f"{c.getLon().asDegrees()} {c.getLat().asDegrees()}" for c in coords]

        return f"POLYGON {' '.join(coord_strings)}"
