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

Python bindings that expose most of the C++ API are also provided via SWIG.

Points
------

There are 3 different classes for points -
[LonLat](include/lsst/sphgeom/LonLat.h) for spherical coordinates,
[Vector3d](include/lsst/sphgeom/Vector3d.h) for Cartesian vectors in ℝ³
(not constrained to lie on the unit sphere), and
[UnitVector3d](include/lsst/sphgeom/UnitVector3d.h) for vectors in ℝ³ with
unit ℓ² norm.

Regions
-------

Four basic spherical [Region](include/lsst/sphgeom/Region.h) types are
provided:

  - [Box](include/lsst/sphgeom/Box.h), a longitude/latitude angle box
  - [Circle](include/lsst/sphgeom/Circle.h), a small circle defined
    by a center and opening angle/chord length
  - [Ellipse](include/lsst/sphgeom/Ellipse.h), the intersection of an
    elliptical cone with the unit sphere
  - [ConvexPolygon](include/lsst/sphgeom/ConvexPolygon.h), a convex
    spherical polygon with unit vector vertices and great circle edges

In addition to the spherical regions, there is a type for 3-D axis aligned
boxes, [Box3d](include/lsst/sphgeom/Box3d.h). All spherical regions know how
to compute their 3-D bounding boxes, which makes it possible to insert them
into a 3-D [R-tree](https://en.wikipedia.org/wiki/R-tree). This is used by the
exposure indexing task in the [daf_ingest](https://github.com/lsst/daf_ingest)
package to spatially index exposure bounding polygons using the
[SQLite](https://sqlite.org) 3
[R*tree module](https://www.sqlite.org/rtree.html).

A region can also determine its spatial
[relationship](include/lsst/sphgeom/Relationship.h) to another region, and
test whether or not it contains a given unit vector.

Partitioning/Pixelization
-------------------------

This library also implements support for assigning points to cells of
a partitioning (a.k.a pixelization) of the sphere, and for determining which
cells intersect a region.

Currently, the [Chunker](include/lsst/sphgeom/Chunker.h) class implements
the partitioning scheme employed by [Qserv](https://github.com/lsst/qserv).
Support for the HTM (Hierarchical Triangular Mesh) and Q3C (Quad Tree
Cube) pixelizations is planned.

See Also
--------

#### Contributing

For instructions on how to contribute, see http://dm.lsst.org/#contributing
(or just send us a pull request).

#### Support

For help, see http://dm.lsst.org/#support.
