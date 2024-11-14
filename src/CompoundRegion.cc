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
    for (unsigned i = 1; i < compound.nOperands(); ++ i) {
        bounds.expandTo(func(compound.getOperand(i)));
    }
    return bounds;
}

template <typename F>
auto getIntersectionBounds(IntersectionRegion const &compound, F func) {
    auto bounds = func(compound.getOperand(0));
    for (unsigned i = 1; i < compound.nOperands(); ++ i) {
        bounds.clipTo(func(compound.getOperand(i)));
    }
    return bounds;
}

}  // namespace

CompoundRegion::CompoundRegion(std::vector<std::unique_ptr<Region>> operands) noexcept
        : _operands(std::move(operands))
{
    if (_operands.empty()) {
        throw std::invalid_argument("CompoundRegion requires non-empty region list.");
    }
}

CompoundRegion::CompoundRegion(CompoundRegion const &other)
        : _operands()
{
    for (auto&& operand: other._operands) {
        _operands.emplace_back(operand->clone());
    }
}

Relationship CompoundRegion::relate(Box const &b) const { return relate(static_cast<Region const &>(b)); }
Relationship CompoundRegion::relate(Circle const &c) const { return relate(static_cast<Region const &>(c)); }
Relationship CompoundRegion::relate(ConvexPolygon const &p) const { return relate(static_cast<Region const &>(p)); }
Relationship CompoundRegion::relate(Ellipse const &e) const { return relate(static_cast<Region const &>(e)); }

std::vector<std::uint8_t> CompoundRegion::_encode(std::uint8_t tc) const {
    std::vector<std::uint8_t> buffer;
    buffer.push_back(tc);
    for (auto&& operand: _operands) {
        auto operand_buffer = operand->encode();
        encodeU64(operand_buffer.size(), buffer);
        buffer.insert(buffer.end(), operand_buffer.begin(), operand_buffer.end());
    }
    return buffer;
}

std::vector<std::unique_ptr<Region>> CompoundRegion::_decode(
    std::uint8_t tc, std::uint8_t const *buffer, std::size_t nBytes) {
    std::uint8_t const *end = buffer + nBytes;
    if (nBytes == 0) {
        throw std::runtime_error("Encoded CompoundRegion is truncated.");
    }
    if (buffer[0] != tc) {
        throw std::runtime_error("Byte string is not an encoded CompoundRegion.");
    }
    ++buffer;
    std::vector<std::unique_ptr<Region>> result;
    while (buffer != end) {
        std::uint64_t nBytes = consumeDecodeU64(buffer, end);
        if (buffer + nBytes > end) {
            throw std::runtime_error("Encoded CompoundRegion is truncated.");
        }
        result.push_back(Region::decode(buffer, nBytes));
        buffer += nBytes;
    }
    return result;
}

std::unique_ptr<CompoundRegion> CompoundRegion::decode(std::uint8_t const *buffer, size_t n) {
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
    for (auto&& operand: operands()) {
        if (operand->contains(v)) {
            return true;
        }
    }
    return false;
}

Relationship UnionRegion::relate(Region const &rhs) const {
    auto result = DISJOINT | WITHIN;
    // When result becomes CONTAINS we can stop checking.
    auto const stop = CONTAINS;
    for (auto&& operand: operands()) {
        auto rel = operand->relate(rhs);
        // All operands must be disjoint with the given region for the union
        // to be disjoint with it.
        if ((rel & DISJOINT) != DISJOINT) {
            result &= ~DISJOINT;
        }
        // All operands must be within the given region for the union to be
        // within it.
        if ((rel & WITHIN) != WITHIN) {
            result &= ~WITHIN;
        }
        // If any operand contains the given region, the union contains it.
        // NOTE: The logic here is incomplete there may be cases when union
        // of regions can fully contain a region even though individual regions
        // do not contain it.
        if ((rel & CONTAINS) == CONTAINS) {
            result |= CONTAINS;
        }
        if (result == stop) {
            break;
        }
    }

    return result;
}

bool UnionRegion::isDisjoint(Region const &rhs) const {
    // Union is disjoint if all operands are disjoint.
    for (auto&& operand: operands()) {
        if (not operand->isDisjoint(rhs)) {
            return false;
        }
    }
    return true;
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
    for (auto&& operand: operands()) {
        if (not operand->contains(v)) {
            return false;
        }
    }
    return true;
}

Relationship IntersectionRegion::relate(Region const &rhs) const {
    auto result = CONTAINS;
    // When result becomes DISJOINT | WITHIN we can stop checking.
    auto const stop = DISJOINT | WITHIN;
    for (auto&& operand: operands()) {
        auto rel = operand->relate(rhs);
        // All operands must contain the given region for the intersection to
        // contain it.
        if ((rel & CONTAINS) != CONTAINS) {
            result &= ~CONTAINS;
        }
        // If any operand is disjoint with the given region, the
        // intersection is disjoint with it.
        // NOTE: there may be cases when intersection is disjoint even though
        // not all operands are disjoint.
        if ((rel & DISJOINT) == DISJOINT) {
            result |= DISJOINT;
        }
        // If any operand is within the given region, the intersection is
        // within it.
        // NOTE: There may be cases when intersectiuon is within the region
        // even though not all operand are within.
        if ((rel & WITHIN) == WITHIN) {
            result |= WITHIN;
        }
        if (result == stop) {
            break;
        }
    }

    return result;
}

bool IntersectionRegion::isDisjoint(Region const &rhs) const {
    // Intersection is disjoint if any operand is disjoint.
    for (auto&& operand: operands()) {
        if (operand->isDisjoint(rhs)) {
            return true;
        }
    }
    // False means it may overlap.
    return false;
}

}  // namespace sphgeom
}  // namespace lsst
