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

#ifndef LSST_SPHGEOM_RELATIONSHIPTESTUTILS_H_
#define LSST_SPHGEOM_RELATIONSHIPTESTUTILS_H_

/// \file
/// \brief This file contains utility code for testing spatial
///        relations between regions. It is useful both for
///        spherical regions as well as for 1D intervals.

#include "lsst/sphgeom/Relationship.h"


namespace lsst {
namespace sphgeom {

// `checkRelationship` checks that evaluating the spatial predicates gives the
// expected outcomes.
template <typename U, typename V>
void checkRelationship(U const & u,
                       V const & v,
                       Relationship expectedRelationship)
{
    bool shouldBeDisjointFrom = (expectedRelationship & DISJOINT) != 0;
    bool shouldIntersect = (expectedRelationship & DISJOINT) == 0;
    bool shouldContain = (expectedRelationship & CONTAINS) != 0;
    bool shouldBeWithin = (expectedRelationship & WITHIN) != 0;
    CHECK(u.contains(v) == shouldContain);
    CHECK(u.intersects(v) == shouldIntersect);
    CHECK(u.isWithin(v) == shouldBeWithin);
    CHECK(u.isDisjointFrom(v) == shouldBeDisjointFrom);
    CHECK(u.relate(v) == expectedRelationship);
}

template <typename U, typename V>
void checkDisjoint(U const & u, V const & v) {
    checkRelationship(u, v, DISJOINT);
    checkRelationship(v, u, DISJOINT);
}

template <typename U, typename V>
void checkIntersects(U const & u, V const & v) {
    checkRelationship(u, v, INTERSECTS);
    checkRelationship(v, u, INTERSECTS);
}

template <typename U, typename V>
void checkContains(U const & u, V const & v) {
    checkRelationship(u, v, CONTAINS);
    checkRelationship(v, u, WITHIN);
}

// `checkBasicProperties` verifies a few rudimentary identities that should
// hold for a non-empty region r.
template <typename R>
void checkBasicProperties(R const & r) {
    CHECK(!r.isEmpty());
    CHECK(r == r);
    CHECK(!(r != r));
    CHECK(r != R());
    // A non-empty region should contain and be disjoint from ∅.
    checkRelationship(r, R(), CONTAINS | DISJOINT);
    // ∅ should be within and disjoint from a non-empty region.
    checkRelationship(R(), r, WITHIN | DISJOINT);
}

// `checkPoints` verifies that the given lists of points, and regions
// constructed from them, are contained in / disjoint from region r.
template <typename R, typename Point>
void checkPoints(Point const * in,
                 size_t inLength,
                 Point const * out,
                 size_t outLength,
                 R const & r)
{
    for (unsigned i = 0; i < inLength; ++i) {
        Point p = in[i];
        checkRelationship(r, p, CONTAINS);
        checkContains(r, R(p));
    }
    for (unsigned i = 0; i < outLength; ++i) {
        Point p = out[i];
        checkRelationship(r, p, DISJOINT);
        checkDisjoint(r, R(p));
    }
}

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_RELATIONSHIPTESTUTILS_H_
