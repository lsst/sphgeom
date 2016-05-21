/*
 * LSST Data Management System
 * Copyright 2014-2016 AURA/LSST.
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

#ifndef LSST_SPHGEOM_RELATIONSHIP_H_
#define LSST_SPHGEOM_RELATIONSHIP_H_

#include <bitset>

/// \file
/// \brief This file provides a type alias for describing set relationships.

namespace lsst {
namespace sphgeom {

/// `Relationship` describes how two sets are related.
using Relationship = std::bitset<3>;

/// A is disjoint from B  ⇔  A ⋂ B = ∅
static constexpr Relationship DISJOINT(1);

/// A intersects B  ⇔  A ⋂ B ≠ ∅
///
/// This is the complement of DISJOINT, hence no explicit
/// bit is reserved for this relationship.
static constexpr Relationship INTERSECTS(0);

/// A contains B  ⇔  A ⋂ B = B
static constexpr Relationship CONTAINS(2);

/// A is within B  ⇔  A ⋂ B = A
static constexpr Relationship WITHIN(4);

/// Given the relationship between two sets A and B (i.e. the output of
/// `A.relate(B)`), `invert` returns the relationship between B and A
/// (`B.relate(A)`).
inline Relationship invert(Relationship r) {
    // If A is disjoint from B, then B is disjoint from A. But if A contains B
    // then B is within A, so the corresponding bits must be swapped.
    return (r & DISJOINT) | ((r & CONTAINS) << 1) | ((r & WITHIN) >> 1);
}

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_RELATIONSHIP_H_
