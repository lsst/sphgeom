/*
 * LSST Data Management System
 * Copyright 2014-2015 AURA/LSST.
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

/// \file
/// \brief This file contains the CompoundRegion class implementation.

#include "lsst/sphgeom/CompoundRegion.h"

#include <iostream>
#include <stdexcept>

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/codec.h"

namespace lsst {
namespace sphgeom {

namespace {

// A version of decodeU64 from codec.h that checks for buffer overruns and
// increments the buffer pointer it is given.
std::uint64_t consumeDecodeU64(std::uint8_t const *&buffer, std::uint8_t const *end) {
    if (buffer + 8 > end) {
        throw std::runtime_error("Encoded CompoundRegion is truncated.");
    }
    std::uint64_t result = decodeU64(buffer);
    buffer += 8;
    return result;
}

template <typename F>
auto getUnionBounds(UnionRegion const &compound, F func) {
    auto bounds = func(compound.getOperand(0));
    bounds.expandTo(func(compound.getOperand(1)));
    return bounds;
}

template <typename F>
auto getIntersectionBounds(IntersectionRegion const &compound, F func) {
    auto bounds = func(compound.getOperand(0));
    bounds.clipTo(func(compound.getOperand(1)));
    return bounds;
}

}  // namespace

CompoundRegion::CompoundRegion(Region const &first, Region const &second)
        : _operands{first.clone(), second.clone()} {}

CompoundRegion::CompoundRegion(
    std::array<std::unique_ptr<Region>, 2> operands) noexcept
        : _operands(std::move(operands)) {}

CompoundRegion::CompoundRegion(CompoundRegion const &other)
        : _operands{other.getOperand(0).clone(), other.getOperand(1).clone()} {}

Relationship CompoundRegion::relate(Box const &b) const { return relate(static_cast<Region const &>(b)); }
Relationship CompoundRegion::relate(Circle const &c) const { return relate(static_cast<Region const &>(c)); }
Relationship CompoundRegion::relate(ConvexPolygon const &p) const { return relate(static_cast<Region const &>(p)); }
Relationship CompoundRegion::relate(Ellipse const &e) const { return relate(static_cast<Region const &>(e)); }

std::vector<std::uint8_t> CompoundRegion::_encode(std::uint8_t tc) const {
    std::vector<std::uint8_t> buffer;
    buffer.push_back(tc);
    auto buffer1 = getOperand(0).encode();
    encodeU64(buffer1.size(), buffer);
    buffer.insert(buffer.end(), buffer1.begin(), buffer1.end());
    auto buffer2 = getOperand(1).encode();
    encodeU64(buffer2.size(), buffer);
    buffer.insert(buffer.end(), buffer2.begin(), buffer2.end());
    return buffer;
}

std::array<std::unique_ptr<Region>, 2> CompoundRegion::_decode(
    std::uint8_t tc, std::uint8_t const *buffer, std::size_t nBytes) {
    std::uint8_t const *end = buffer + nBytes;
    if (nBytes == 0) {
        throw std::runtime_error("Encoded CompoundRegion is truncated.");
    }
    if (buffer[0] != tc) {
        throw std::runtime_error("Byte string is not an encoded CompoundRegion.");
    }
    ++buffer;
    std::array<std::unique_ptr<Region>, 2> result;
    std::uint64_t nBytes1 = consumeDecodeU64(buffer, end);
    result[0] = Region::decode(buffer, nBytes1);
    buffer += nBytes1;
    std::uint64_t nBytes2 = consumeDecodeU64(buffer, end);
    result[1] = Region::decode(buffer, nBytes2);
    buffer += nBytes2;
    if (buffer != end) {
        throw std::runtime_error("Encoded CompoundRegion is has unexpected additional bytes.");
    }
    return result;
}

std::unique_ptr<CompoundRegion> CompoundRegion::decode(uint8_t const *buffer, size_t n) {
    if (n == 0) {
        throw std::runtime_error("Encoded CompoundRegion is truncated.");
    }
    switch (buffer[0]) {
        case UnionRegion::TYPE_CODE:
            return UnionRegion::decode(buffer, n);
        case IntersectionRegion::TYPE_CODE:
            return IntersectionRegion::decode(buffer, n);
        default:
            throw std::runtime_error("Byte string is not an encoded CompoundRegion.");
    }
}

Box UnionRegion::getBoundingBox() const {
    return getUnionBounds(*this, [](Region const &r) { return r.getBoundingBox(); });
}

Box3d UnionRegion::getBoundingBox3d() const {
    return getUnionBounds(*this, [](Region const &r) { return r.getBoundingBox3d(); });
}

Circle UnionRegion::getBoundingCircle() const {
    return getUnionBounds(*this, [](Region const &r) { return r.getBoundingCircle(); });
}

bool UnionRegion::contains(UnitVector3d const &v) const {
    return getOperand(0).contains(v) || getOperand(1).contains(v);
}

Relationship UnionRegion::relate(Region const &rhs) const {
    auto r1 = getOperand(0).relate(rhs);
    auto r2 = getOperand(1).relate(rhs);
    return
        // Both operands must be disjoint with the given region for the union
        // to be disjoint with it.
        ((r1 & DISJOINT) & (r2 & DISJOINT))
        // Both operands must be within the given region for the union to be
        // within it.
        | ((r1 & WITHIN) & (r2 & WITHIN))
        // If either operand contains the given region, the union contains it.
        | ((r1 & CONTAINS) | (r2 & CONTAINS));
}

Box IntersectionRegion::getBoundingBox() const {
    return getIntersectionBounds(*this, [](Region const &r) { return r.getBoundingBox(); });
}

Box3d IntersectionRegion::getBoundingBox3d() const {
    return getIntersectionBounds(*this, [](Region const &r) { return r.getBoundingBox3d(); });
}

Circle IntersectionRegion::getBoundingCircle() const {
    return getIntersectionBounds(*this, [](Region const &r) { return r.getBoundingCircle(); });
}

bool IntersectionRegion::contains(UnitVector3d const &v) const {
    return getOperand(0).contains(v) && getOperand(1).contains(v);
}

Relationship IntersectionRegion::relate(Region const &rhs) const {
    auto r1 = getOperand(0).relate(rhs);
    auto r2 = getOperand(1).relate(rhs);
    return
        // Both operands must contain the given region for the intersection to
        // contain it.
        ((r1 & CONTAINS) & (r2 & CONTAINS))
        // If either operand is disjoint with the given region, the
        // intersection is disjoint with it.
        | ((r1 & DISJOINT) | (r2 & DISJOINT))
        // If either operand is within the given region, the intersection is
        // within it.
        | ((r1 & WITHIN) | (r2 & WITHIN));
}

}  // namespace sphgeom
}  // namespace lsst
