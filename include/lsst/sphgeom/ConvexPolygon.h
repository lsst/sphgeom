/*
 * This file is part of sphgeom.
 *
 * Developed for the LSST Data Management System.
 * This product includes software developed by the LSST Project
 * (http://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This software is dual licensed under the GNU General Public License and also
 * under a 3-clause BSD license. Recipients may choose which of these licenses
 * to use; please see the files gpl-3.0.txt and/or bsd_license.txt,
 * respectively.  If you choose the GPL option then the following text applies
 * (but note that there is still no warranty even if you opt for BSD instead):
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LSST_SPHGEOM_CONVEXPOLYGON_H_
#define LSST_SPHGEOM_CONVEXPOLYGON_H_

/// \file
/// \brief This file declares a class for representing convex
///        polygons with great circle edges on the unit sphere.

#include <iosfwd>
#include <vector>
#include <cstdint>

#include "Region.h"
#include "UnitVector3d.h"


namespace lsst {
namespace sphgeom {

/// `ConvexPolygon` is a closed convex polygon on the unit sphere. Its edges
/// are great circles (geodesics), and the shorter of the two great circle
/// segments between any two points on the polygon boundary is contained in
/// the polygon.
///
/// The vertices of a convex polygon are distinct and have counter-clockwise
/// orientation when viewed from outside the unit sphere. No three consecutive
/// vertices are coplanar and edges do not intersect except at vertices.
///
/// Furthermore, if a convex polygon contains a point p of S², then we require
/// that it be disjoint from point -p. This guarantees the existence of a
/// unique shortest great circle segment between any 2 points contained in the
/// polygon, but means e.g. that hemispheres and lunes cannot be represented
/// by convex polygons.
///
/// Currently, the only way to construct a convex polygon is to compute the
/// convex hull of a point set.
class ConvexPolygon : public Region {
public:
    static constexpr std::uint8_t TYPE_CODE = 'p';

    /// `convexHull` returns the convex hull of the given set of points if it
    /// exists and throws an exception otherwise. Though points are supplied
    /// in a vector, they really are conceptually a set - the ConvexPolygon
    /// returned is invariant under permutation of the input array.
    static ConvexPolygon convexHull(std::vector<UnitVector3d> const & points) {
        return ConvexPolygon(points);
    }

    /// This constructor creates a convex polygon that is the convex hull of
    /// the given set of points.
    explicit ConvexPolygon(std::vector<UnitVector3d> const & points);

    /// This constructor creates a triangle with the given vertices.
    ///
    /// It is assumed that orientation(v0, v1, v2) = 1. Use with caution -
    /// for performance reasons, this is not verified!
    ConvexPolygon(UnitVector3d const & v0,
                  UnitVector3d const & v1,
                  UnitVector3d const & v2) :
        _vertices{v0, v1, v2}
    {}

    /// This constructor creates a quadrilateral with the given vertices.
    ///
    /// It is assumed that orientation(v0, v1, v2), orientation(v1, v2, v3),
    /// orientation(v2, v3, v0), and orientation (v3, v0, v1) are all 1.
    /// Use with caution - for performance reasons, this is not verified!
    ConvexPolygon(UnitVector3d const & v0,
                  UnitVector3d const & v1,
                  UnitVector3d const & v2,
                  UnitVector3d const & v3) :
        _vertices{v0, v1, v2, v3}
    {}

    /// Two convex polygons are equal iff they contain the same points.
    bool operator==(ConvexPolygon const & p) const;
    bool operator!=(ConvexPolygon const & p) const { return !(*this == p); }

    std::vector<UnitVector3d> const & getVertices() const {
        return _vertices;
    }

    /// The centroid of a polygon is its center of mass projected onto
    /// S², assuming a uniform mass distribution over the polygon surface.
    UnitVector3d getCentroid() const;

    // Region interface
    std::unique_ptr<Region> clone() const override {
        return std::unique_ptr<ConvexPolygon>(new ConvexPolygon(*this));
    }

    bool isEmpty() const override;

    Box getBoundingBox() const override;
    Box3d getBoundingBox3d() const override;
    Circle getBoundingCircle() const override;

    ///@{
    /// `contains` returns true if the intersection of this convex polygon and x
    /// is equal to x.
    bool contains(UnitVector3d const & v) const override;
    bool contains(Region const & r) const;
    ///@}

    using Region::contains;

    ///@{
    /// `isDisjointFrom` returns true if the intersection of this convex polygon
    /// and x is empty.
    bool isDisjointFrom(Region const & r) const;
    ///@}

    ///@{
    /// `intersects` returns true if the intersection of this convex polygon and x
    /// is non-empty.
    bool intersects(Region const & r) const;
    ///@}

    ///@{
    /// `isWithin` returns true if the intersection of this convex polygon and x
    /// is this convex polygon.
    bool isWithin(Region const & r) const;
    ///@}

    Relationship relate(Region const & r) const override {
        // Dispatch on the type of r.
        return invert(r.relate(*this));
    }

    Relationship relate(Box const &) const override;
    Relationship relate(Circle const &) const override;
    Relationship relate(ConvexPolygon const &) const override;
    Relationship relate(Ellipse const &) const override;

    TriState overlaps(Region const& other) const override {
        return other.overlaps(*this);
    }
    TriState overlaps(Box const &) const override;
    TriState overlaps(Circle const &) const override;
    TriState overlaps(ConvexPolygon const &) const override;
    TriState overlaps(Ellipse const &) const override;

    std::vector<std::uint8_t> encode() const override;

    ///@{
    /// `decode` deserializes a ConvexPolygon from a byte string produced by encode.
    static std::unique_ptr<ConvexPolygon> decode(std::vector<std::uint8_t> const & s) {
        return decode(s.data(), s.size());
    }
    static std::unique_ptr<ConvexPolygon> decode(std::uint8_t const * buffer, size_t n);
    ///@}

private:
    typedef std::vector<UnitVector3d>::const_iterator VertexIterator;

    ConvexPolygon() : _vertices() {}

    std::vector<UnitVector3d> _vertices;
};

std::ostream & operator<<(std::ostream &, ConvexPolygon const &);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_CONVEXPOLYGON_H_
