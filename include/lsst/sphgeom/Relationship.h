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
