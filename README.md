sphgeom: spherical geometry primitives
======================================

Overview
--------

This low-level C++ library provides primitives for representing points and
regions on the unit sphere, as well as support for partitioning the sphere.
It can be used to answer the following sorts of questions:

  - *Is point X inside region Y?*
  - *Do two regions A and B intersect?*
  - *Which pieces of the sphere does region C overlap?*

Regions can be serialized to binary strings, so that they may be stored
efficiently in files or VARBINARY database columns. They can also be
approximated with simpler regions - for example, one can ask for the
bounding circle of a convex polygon.

Python bindings that expose most of the C++ API are also provided via
[pybind11](https://pybind11.readthedocs.io/).

Points
------

There are 3 different classes for points

  - *LonLat* for spherical coordinates,
  - *Vector3d* for Cartesian vectors in ℝ³ (not constrained to lie on the unit sphere)
  - *UnitVector3d* for vectors in ℝ³ with unit ℓ² norm.

Regions
-------

Four basic spherical *Region* types are
provided:

  - *Box*, a longitude/latitude angle box
  - *Circle*, a small circle defined by a center and opening angle/chord length
  - *Ellipse*, the intersection of an elliptical cone with the unit sphere
  - *ConvexPolygon*, a convex spherical polygon with unit vector vertices and great circle edges

In addition to the spherical regions, there is a type for 3-D axis aligned
boxes, *Box3d*. All spherical regions know how
to compute their 3-D bounding boxes, which makes it possible to insert them
into a 3-D [R-tree](https://en.wikipedia.org/wiki/R-tree). This is used by the
exposure indexing task in the [daf_ingest](https://github.com/lsst/daf_ingest)
package to spatially index exposure bounding polygons using the
[SQLite](https://sqlite.org) 3
[R*tree module](https://www.sqlite.org/rtree.html).

A region can also determine its spatial
relationship to another region, and
test whether or not it contains a given unit vector.

Pixelizations
-------------

This library also provides support for assigning points to pixels (a.k.a.
cells or partitions) in a *Pixelization*
(a.k.a. partitioning) of the sphere, and for determining which pixels
intersect a region.

Currently, the *Chunker*  class implements
the partitioning scheme employed by [Qserv](https://github.com/lsst/qserv).
The *HtmPixelization*  class implements
the HTM (Hierarchical Triangular Mesh) pixelization. The
*Q3cPixelization* and *Mq3cPixelization* classes implement
the original Quad Tree Cube indexing scheme and a modified version with
reduced pixel area variation.

Installing with pip
-------------------

A simple pip-compatible installer is available.  This only installs the
Python bindings and the resulting installation is not usable for linking
from C++.  Some metadata (in particular the version number) are not set
properly for the distribution.  The main purpose for now is to allow
other packages to pip install from the GitHub URL in their CI systems
where sphgeom is a dependency.

See Also
--------

#### Contributing

For instructions on how to contribute, see http://dm.lsst.org/#contributing
(or just send us a pull request).

#### Support

For help, see http://dm.lsst.org/#support.
