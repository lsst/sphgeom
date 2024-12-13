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

#include <algorithm>
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
    if (compound.nOperands() == 0) {
        return func(Box::empty());
    }
    auto bounds = func(compound.getOperand(0));
    for (std::size_t i = 1; i < compound.nOperands(); ++ i) {
        bounds.expandTo(func(compound.getOperand(i)));
    }
    return bounds;
}

template <typename F>
auto getIntersectionBounds(IntersectionRegion const &compound, F func) {
    if (compound.nOperands() == 0) {
        return func(Box::full());
    }
    auto bounds = func(compound.getOperand(0));
    for (std::size_t i = 1; i < compound.nOperands(); ++ i) {
        bounds.clipTo(func(compound.getOperand(i)));
    }
    return bounds;
}

}  // namespace

CompoundRegion::CompoundRegion(std::vector<std::unique_ptr<Region>> operands) noexcept
        : _operands(std::move(operands))
{
}

CompoundRegion::CompoundRegion(CompoundRegion const &other)
        : _operands()
{
    for (auto&& operand: other._operands) {
        _operands.emplace_back(operand->clone());
    }
}

// Flatten vector of regions in-place.
template <typename Compound>
void CompoundRegion::flatten_operands() {
    for (size_t i = 0; i != _operands.size(); ) {
        if (auto compound = dynamic_cast<Compound*>(_operands[i].get())) {
            // Move all regions from this operand, then remove it.
            std::move(
                compound->_operands.begin(),
                compound->_operands.end(),
                std::inserter(_operands, _operands.begin() + i + 1)
            );
            _operands.erase(_operands.begin() + i);
        } else {
            ++ i;
        }
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

UnionRegion::UnionRegion(std::vector<std::unique_ptr<Region>> operands)
    : CompoundRegion(std::move(operands))
{
    flatten_operands<UnionRegion>();
}

bool UnionRegion::isEmpty() const {
    // It can be empty when there are no operands or all operands are empty.
    for (auto&& operand: operands()) {
        if (not operand->isEmpty()) {
            return false;
        }
    }
    return true;
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
    if (nOperands() == 0) {
        return DISJOINT;
    }
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
        if ((rel & CONTAINS) == CONTAINS) {
            result |= CONTAINS;
        }
        if (result == stop) {
            break;
        }
    }

    return result;
}

TriState UnionRegion::overlaps(Region const& other) const {
    // Union overlaps if any operand overlaps, and disjoint when all are
    // disjoint. Empty union is disjoint with anyhting.
    if (nOperands() == 0) {
        return TriState(false);
    }
    bool may_overlap = false;
    for (auto&& operand: operands()) {
        auto state = operand->overlaps(other);
        if (state == true) {
            // Definitely overlap.
            return TriState(true);
        } if (not state.known()) {
            // May or may not overlap.
            may_overlap = true;
        }
    }
    if (may_overlap) {
        return TriState();
    }
    // None overlaps.
    return TriState(false);
}

TriState UnionRegion::overlaps(Box const &b) const {
    return overlaps(static_cast<Region const&>(b));
}

TriState UnionRegion::overlaps(Circle const &c) const {
    return overlaps(static_cast<Region const&>(c));
}

TriState UnionRegion::overlaps(ConvexPolygon const &p) const {
    return overlaps(static_cast<Region const&>(p));
}

TriState UnionRegion::overlaps(Ellipse const &e) const {
    return overlaps(static_cast<Region const&>(e));
}

IntersectionRegion::IntersectionRegion(std::vector<std::unique_ptr<Region>> operands)
    : CompoundRegion(std::move(operands))
{
    flatten_operands<IntersectionRegion>();
}

bool IntersectionRegion::isEmpty() const {
    // Intersection is harder to decide - the only clear case is when there are
    // no operands, which we declare to be equivalent to full sphere. Other
    // clear case is when all operands are empty.
    if (nOperands() == 0) {
        return false;
    }
    if (std::all_of(
        operands().begin(), operands().end(), [](auto const& operand) { return operand->isEmpty(); }
    )) {
        return true;
    }
    // Another test is for when any operand is disjoint with any other operands.
    auto begin = operands().begin();
    auto const end = operands().end();
    for (auto op1 = begin; op1 != end; ++ op1) {
        for (auto op2 = op1 + 1; op2 != end; ++ op2) {
            if ((*op1)->overlaps(**op2) == false) {
                return true;
            }
        }
    }
    // Still may be empty but hard to guess.
    return false;
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
        if ((rel & DISJOINT) == DISJOINT) {
            result |= DISJOINT;
        }
        // If any operand is within the given region, the intersection is
        // within it.
        if ((rel & WITHIN) == WITHIN) {
            result |= WITHIN;
        }
        if (result == stop) {
            break;
        }
    }

    return result;
}

TriState IntersectionRegion::overlaps(Region const& other) const {
    // Intersection case is harder, difficult to guess "definitely overlaps"
    // without building actual overlap region. It is easier to check for
    // disjoint - if any operand is disjoint then intersection is disjoint too.
    if (nOperands() == 0) {
        // Empty intersection is equivalent to whole sphere, so it should
        // overlap anything, but there is case of empty regions that overlap
        // nothing.
        return TriState(not other.isEmpty());
    }

    for (auto&& operand: operands()) {
        auto state = operand->overlaps(other);
        if (state == false) {
            return TriState(false);
        }
    }
    // Not disjoint, but may or may not overlap.
    return TriState();
}

TriState IntersectionRegion::overlaps(Box const &b) const {
    return overlaps(static_cast<Region const&>(b));
}

TriState IntersectionRegion::overlaps(Circle const &c) const {
    return overlaps(static_cast<Region const&>(c));
}

TriState IntersectionRegion::overlaps(ConvexPolygon const &p) const {
    return overlaps(static_cast<Region const&>(p));
}

TriState IntersectionRegion::overlaps(Ellipse const &e) const {
    return overlaps(static_cast<Region const&>(e));
}

}  // namespace sphgeom
}  // namespace lsst
