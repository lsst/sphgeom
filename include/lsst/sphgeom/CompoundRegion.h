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

#ifndef LSST_SPHGEOM_COMPOUND_REGION_H_
#define LSST_SPHGEOM_COMPOUND_REGION_H_

/// \file
/// \brief This file declares classes for representing compound
///        regions on the unit sphere.

#include <iosfwd>
#include <iterator>
#include <vector>
#include <cstdint>

#include "Region.h"
#include "UnitVector3d.h"

namespace lsst {
namespace sphgeom {

class Box;
class Circle;
class ConvexPolygon;
class Ellipse;

/// CompoundRegion is an intermediate base class for spherical regions that are
/// comprised of a point-set operation on other nested regions.
class CompoundRegion : public Region {
public:
    //@{
    /// Construct by taking ownership of operands.
    explicit CompoundRegion(std::vector<std::unique_ptr<Region>> operands) noexcept;
    //@}

    CompoundRegion(CompoundRegion const &);
    CompoundRegion(CompoundRegion &&) noexcept = default;

    // Disable assignment (including for subclasses) because it makes it hard
    // to guarantee memory safety for operand accessors in Python.
    CompoundRegion &operator=(CompoundRegion const &) = delete;
    CompoundRegion &operator=(CompoundRegion &&) = delete;

    // Return number of operands
    size_t nOperands() const { return _operands.size(); }

    // Return references to the operands.
    Region const & getOperand(std::size_t n) const {
        return *_operands[n];
    }

    // Region interface.
    virtual Relationship relate(Region const &r) const = 0; // still unimplemented; avoid shadowing
    Relationship relate(Box const &b) const override;
    Relationship relate(Circle const &c) const override;
    Relationship relate(ConvexPolygon const &p) const override;
    Relationship relate(Ellipse const &e) const override;

    ///@{
    /// `decode` deserializes a CompoundRegion from a byte string produced by
    /// encode.
    static std::unique_ptr<CompoundRegion> decode(std::vector<std::uint8_t> const &s) {
        return decode(s.data(), s.size());
    }
    static std::unique_ptr<CompoundRegion> decode(std::uint8_t const *buffer, size_t n);
    ///@}

protected:

    // Implementation helper for encode().
    std::vector<std::uint8_t> _encode(std::uint8_t tc) const;

    // Implementation helper for decode().
    static std::vector<std::unique_ptr<Region>> _decode(
        std::uint8_t tc, std::uint8_t const *buffer, std::size_t nBytes);

    // Provide read access to all operands for quick iteration.
    std::vector<std::unique_ptr<Region>> const& operands() const { return _operands; }

    // Flatten vector of regions in-place.
    template <typename Compound>
    void flatten_operands();

private:
    std::vector<std::unique_ptr<Region>> _operands;
};

/// UnionRegion is a lazy point-set union of its operands.
///
/// All operations on a UnionRegion are implementing by delegating to its
/// nested operand regions and combining the results.
class UnionRegion : public CompoundRegion {
public:
    static constexpr std::uint8_t TYPE_CODE = 'u';

    /// Construct by taking ownership of operands.
    explicit UnionRegion(std::vector<std::unique_ptr<Region>> operands);

    // Region interface.
    std::unique_ptr<Region> clone() const override { return std::make_unique<UnionRegion>(*this); }
    bool isEmpty() const override;
    Box getBoundingBox() const override;
    Box3d getBoundingBox3d() const override;
    Circle getBoundingCircle() const override;
    using Region::contains;
    bool contains(UnitVector3d const &v) const override;
    Relationship relate(Region const &r) const override;
    TriState overlaps(Region const& other) const override;
    TriState overlaps(Box const &) const override;
    TriState overlaps(Circle const &) const override;
    TriState overlaps(ConvexPolygon const &) const override;
    TriState overlaps(Ellipse const &) const override;
    std::vector<std::uint8_t> encode() const override { return _encode(TYPE_CODE); }

    ///@{
    /// `decode` deserializes a UnionRegion from a byte string produced by
    /// encode.
    static std::unique_ptr<UnionRegion> decode(std::vector<std::uint8_t> const &s) {
        return decode(s.data(), s.size());
    }
    static std::unique_ptr<UnionRegion> decode(std::uint8_t const *buffer, size_t n) {
        return std::make_unique<UnionRegion>(_decode(TYPE_CODE, buffer, n));
    }
    ///@}

};

/// IntersectionRegion is a lazy point-set inersection of its operands.
///
/// All operations on a IntersectionRegion are implementing by delegating to
/// its nested operand regions and combining the results.
class IntersectionRegion : public CompoundRegion {
public:
    static constexpr std::uint8_t TYPE_CODE = 'i';

    /// Construct by taking ownership of operands.
    explicit IntersectionRegion(std::vector<std::unique_ptr<Region>> operands);

    // Region interface.
    std::unique_ptr<Region> clone() const override { return std::make_unique<IntersectionRegion>(*this); }
    bool isEmpty() const override;
    Box getBoundingBox() const override;
    Box3d getBoundingBox3d() const override;
    Circle getBoundingCircle() const override;
    using Region::contains;
    bool contains(UnitVector3d const &v) const override;
    Relationship relate(Region const &r) const override;
    TriState overlaps(Region const& other) const override;
    TriState overlaps(Box const &) const override;
    TriState overlaps(Circle const &) const override;
    TriState overlaps(ConvexPolygon const &) const override;
    TriState overlaps(Ellipse const &) const override;
    std::vector<std::uint8_t> encode() const override { return _encode(TYPE_CODE); }

    ///@{
    /// `decode` deserializes a IntersetionRegion from a byte string produced
    /// by encode.
    static std::unique_ptr<IntersectionRegion> decode(std::vector<std::uint8_t> const &s) {
        return decode(s.data(), s.size());
    }
    static std::unique_ptr<IntersectionRegion> decode(std::uint8_t const *buffer, size_t n) {
        return std::make_unique<IntersectionRegion>(_decode(TYPE_CODE, buffer, n));
    }
    ///@}

};

}  // namespace sphgeom
}  // namespace lsst

#endif  // LSST_SPHGEOM_COMPOUND_REGION_H_
