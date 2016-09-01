/*
 * LSST Data Management System
 * Copyright 2016 AURA/LSST.
 *
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program.  If not,
 * see <https://www.lsstcorp.org/LegalNotices/>.
 */

#ifndef LSST_SPHGEOM_PIXELIZATION_H_
#define LSST_SPHGEOM_PIXELIZATION_H_

/// \file
/// \brief This file defines an interface for pixelizations of the sphere.

#include <string>

#include "RangeSet.h"


namespace lsst {
namespace sphgeom {

class Region;
class UnitVector3d;


/// A `Pixelization` (or partitioning) of the sphere is a mapping between
/// points on the sphere and a set of pixels (a.k.a. cells or partitions)
/// with 64 bit integer labels (indexes), where each point is assigned to
/// exactly one pixel.
///
/// A pixelization is capable of:
///
/// - mapping a unit vector to its assigned pixel
/// - computing the indexes of pixels intersecting a region
/// - computing the indexes of pixels within a region
/// - returning the set of all possible pixel indexes
/// - returning a geometric representation of a pixel
///
/// One use case for pixelizations is spatial search in an RDBMS. Given a
/// table of points in S² indexed by pixel, one can quickly retrieve points
/// inside of a region r by computing the indexes of pixels intersecting
/// r:
///
///     RangeSet pixels = pixelization.envelope(r);
///
/// and then performing range lookups on the table. The range lookup results
/// may include points outside of r but close to its boundary, so additional
/// filtering is necessary if one wishes to obtain exactly those points inside
/// r.
///
/// To mitigate this cost, which can be significant for large regions with
/// complex boundaries, one can compute the indexes of pixels completely
/// contained in R. Only points belonging to pixels in:
///
///     RangeSet s = pixelization.envelope(r) - pixelization.interior(r);
///
/// must be tested for membership in r. Note that the indexes of pixels
/// disjoint from r can be computed as follows:
///
///      RangeSet exterior = pixelization.universe() -
///                          pixelization.envelope(r);
class Pixelization {
public:
    virtual ~Pixelization() {}

    /// `universe` returns the set of all pixel indexes for this pixelization.
    virtual RangeSet universe() const = 0;

    /// `pixel` returns the spherical region corresponding to the pixel with
    /// index i.
    ///
    /// This region will contain all unit vectors v with `index(v) == i`. But
    /// it may also contain points with index not equal to i. To see why,
    /// consider a point that lies on the edge of a polygonal pixel - it is
    /// inside the polygons for both pixels sharing the edge, but must be
    /// assigned to exactly one pixel by the pixelization.
    ///
    /// If i is not a valid pixel index, a std::invalid_argument is thrown.
    virtual std::unique_ptr<Region> pixel(uint64_t i) const = 0;

    /// `index` computes the index of the pixel for v.
    virtual uint64_t index(UnitVector3d const & v) const = 0;

    /// `toString` converts the given pixel index to a human-readable string.
    virtual std::string toString(uint64_t i) const = 0;

    /// `envelope` returns the indexes of the pixels intersecting the
    /// spherical region r.
    ///
    /// For hierarchical pixelizations, a good way to implement this is by
    /// top down tree traversal. Starting with the root pixels (e.g. Q3C cube
    /// faces, or HTM root triangles), a pixel P is tested for intersection
    /// with the region r. If P is already at the desired subdivision level
    /// and intersects r, its index is added to the output. If r contains P,
    /// the indexes of all children of P at the target subdivision level are
    /// output. Finally, if P intersects r, then P is subdivided and the
    /// algorithm recurses on its child pixels.
    ///
    /// Using higher subdivision levels allows a region to be more closely
    /// approximated by smaller pixels, but for large input regions the cost
    /// of computing and storing their indexes can quickly become prohibitive.
    ///
    /// The `maxRanges` parameter can be used to limit both these costs -
    /// setting it to a non-zero value sets a cap on the number of ranges
    /// returned by this method. To meet this constraint, implementations are
    /// allowed to return pixels that do not intersect r along with those that
    /// do. This allows two ranges [a, b) and [c, d), a < b < c < d, to be
    /// merged into one range [a, d) (by adding in the pixels [b, c)). Since
    /// simplification proceeds by adding pixels, the return value will always
    /// be a superset of the intersecting pixels.
    ///
    /// In practice, the implementation of this method for a hierarchical
    /// pixelization like Q3C or HTM will lower the subdivision level when
    /// too many ranges have been found. Each coarse pixel I at level L - n
    /// corresponds to pixels [I*4ⁿ, (I + 1)*4ⁿ) at level L.
    RangeSet envelope(Region const & r, size_t maxRanges = 0) const {
        return _envelope(r, maxRanges);
    }

    /// `interior` returns the indexes of the pixels within the spherical
    /// region r.
    ///
    /// The `maxRanges` argument is analogous to the identically named
    /// envelope() argument. The only difference is that implementations must
    /// remove interior pixels to keep the number of ranges at or below the
    /// maximum. The return value is therefore always a subset of the interior
    /// pixels.
    RangeSet interior(Region const & r, size_t maxRanges = 0) const {
        return _interior(r, maxRanges);
    }

private:
    virtual RangeSet _envelope(Region const & r, size_t maxRanges) const = 0;
    virtual RangeSet _interior(Region const & r, size_t maxRanges) const = 0;
};

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_PIXELIZATION_H_
