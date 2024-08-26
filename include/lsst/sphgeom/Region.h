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

#ifndef LSST_SPHGEOM_REGION_H_
#define LSST_SPHGEOM_REGION_H_

/// \file
/// \brief This file defines an interface for spherical regions.

#include <memory>
#include <vector>
#include <cstdint>

#include "Relationship.h"


namespace lsst {
namespace sphgeom {

// Forward declarations
class Box;
class Box3d;
class Circle;
class ConvexPolygon;
class Ellipse;
class UnitVector3d;

/// `Region` is a minimal interface for 2-dimensional regions on the unit
/// sphere. It provides three core pieces of functionality:
///
/// - It allows a region to be approximated with a simpler one.
/// - It allows for inexact (but conservative) computation of the spatial
///   relationships between two regions.
/// - It provides transformation between objects and binary strings.
///
/// Given a pixelization of the unit sphere with pixels that can be
/// bounded by Regions, this provides all the necessary functionality for
/// determining which pixels may intersect a Region.
///
/// When implementing a new concrete region subclass R, the Region interface
/// should be extended with:
///
///     virtual Relationship relate(R const &) const = 0;
///
/// All other Region subclasses must then implement this method. In
/// addition, R is expected to contain the following implementation of the
/// generic relate method:
///
///     virtual Relationship relate(Region const & r) const {
///         return invert(r.relate(*this));
///     }
///
/// where invert is defined in Relationship.h.
///
/// Given two Region references r1 and r2 of type R1 and R2, the net effect
/// is that r1.relate(r2) will end up calling R2::relate(R1 const &). In other
/// words, the call is polymorphic in the types of both r1 and r2.
///
/// One negative consequence of this design is that one cannot implement new
/// Region types outside of this library.
class Region {
public:
    virtual ~Region() {}

    /// `clone` returns a deep copy of this region.
    virtual std::unique_ptr<Region> clone() const = 0;

    /// `getBoundingBox` returns a bounding-box for this region.
    virtual Box getBoundingBox() const = 0;

    /// `getBoundingBox3d` returns a 3-dimensional bounding-box for this region.
    virtual Box3d getBoundingBox3d() const = 0;

    /// `getBoundingCircle` returns a bounding-circle for this region.
    virtual Circle getBoundingCircle() const = 0;

    /// `contains` tests whether the given unit vector is inside this region.
    virtual bool contains(UnitVector3d const &) const = 0;

    /// `contains` tests whether the unit vector defined by the given (not
    /// necessarily normalized) coordinates is inside this region.
    bool contains(double x, double y, double z) const;

    /// `contains` tests whether the unit vector defined by the given longitude
    /// and latitude coordinates (in radians) is inside this region.
    bool contains(double lon, double lat) const;

    ///@{
    /// `relate` computes the spatial relationships between this region A and
    /// another region B. The return value S is a bitset with the following
    /// properties:
    ///
    /// - Bit `S & DISJOINT` is set only if A and B do not have any
    ///   points in common.
    /// - Bit `S & CONTAINS` is set only if A contains all points in B.
    /// - Bit `S & WITHIN` is set only if B contains all points in A.
    ///
    /// Said another way: if the CONTAINS, WITHIN or DISJOINT bit is set, then
    /// the corresponding spatial relationship between the two regions holds
    /// conclusively. If it is not set, the relationship may or may not
    /// hold.
    ///
    /// These semantics allow for conservative relationship computations. In
    /// particular, a Region may choose to implement `relate` by replacing
    /// itself and/or the argument with a simplified bounding region.
    virtual Relationship relate(Region const &) const = 0;
    virtual Relationship relate(Box const &) const = 0;
    virtual Relationship relate(Circle const &) const = 0;
    virtual Relationship relate(ConvexPolygon const &) const = 0;
    virtual Relationship relate(Ellipse const &) const = 0;
    ///@}

    /// `encode` serializes this region into an opaque byte string. Byte strings
    /// emitted by encode can be deserialized with decode.
    virtual std::vector<std::uint8_t> encode() const = 0;

    ///@{
    /// `decode` deserializes a Region from a byte string produced by encode.
    static std::unique_ptr<Region> decode(std::vector<std::uint8_t> const & s) {
        return decode(s.data(), s.size());
    }

    static std::unique_ptr<Region> decode(std::uint8_t const * buffer, size_t n);
    ///@}
};

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_REGION_H_
