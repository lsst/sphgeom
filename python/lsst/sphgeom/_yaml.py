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

"""Add support for YAML serialization of regions."""

__all__ = ()

import yaml
from .region import Region
from .convexPolygon import ConvexPolygon
from .ellipse import Ellipse
from .circle import Circle
from .box import Box
from .htmPixelization import HtmPixelization
from .q3cPixelization import Q3cPixelization
from .mq3cPixelization import Mq3cPixelization

YamlLoaders = (yaml.Loader, yaml.CLoader, yaml.FullLoader, yaml.SafeLoader, yaml.UnsafeLoader)


# Regions

def region_representer(dumper, data):
    """Represent a sphgeom region object in a form suitable for YAML.

    Stores the region as a mapping with a single ``encoded`` key
    storing the hex encoded byte string.
    """
    encoded = data.encode()
    return dumper.represent_mapping(f"lsst.sphgeom.{type(data).__name__}",
                                    {"encoded": encoded.hex()})


def region_constructor(loader, node):
    """Construct a sphgeom region from YAML"""
    mapping = loader.construct_mapping(node)
    encoded = bytes.fromhex(mapping["encoded"])
    # The generic Region base class can instantiate a region of the
    # correct type.
    return Region.decode(encoded)


# Register all the region classes with the same constructor and representer
for region_class in (ConvexPolygon, Ellipse, Circle, Box):
    yaml.add_representer(region_class, region_representer)

    for loader in YamlLoaders:
        yaml.add_constructor(f"lsst.sphgeom.{region_class.__name__}", region_constructor, Loader=loader)


# Pixelization schemes

def pixel_representer(dumper, data):
    """Represent a pixelization in YAML

    Stored as the pixelization level in a mapping with a single key
    ``level``.
    """
    return dumper.represent_mapping(f"lsst.sphgeom.{type(data).__name__}",
                                    {"level": data.getLevel()})


def pixel_constructor(loader, node):
    """Construct a pixelization object from YAML.
    """
    mapping = loader.construct_mapping(node)

    className = node.tag
    pixelMap = {"lsst.sphgeom.Q3cPixelization": Q3cPixelization,
                "lsst.sphgeom.Mq3cPixelization": Mq3cPixelization,
                "lsst.sphgeom.HtmPixelization": HtmPixelization,
                }

    if className not in pixelMap:
        raise RuntimeError(f"Encountered unexpected class {className} associated with"
                           " sphgeom pixelization YAML constructor")

    return pixelMap[className](mapping["level"])


# All the pixelization schemes use the same approach with getLevel
for pixelSchemeCls in (HtmPixelization, Q3cPixelization, Mq3cPixelization):
    yaml.add_representer(pixelSchemeCls, pixel_representer)
    for loader in YamlLoaders:
        yaml.add_constructor(f"lsst.sphgeom.{pixelSchemeCls.__name__}", pixel_constructor, Loader=loader)
