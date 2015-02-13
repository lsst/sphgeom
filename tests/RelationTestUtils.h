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

#ifndef LSST_SPHGEOM_RELATIONTESTUTILS_H_
#define LSST_SPHGEOM_RELATIONTESTUTILS_H_

/// \file
/// \brief This file contains utility code for testing spatial
///        relations between regions. It is useful both for
///        spherical regions as well as for 1D intervals.

#include "SpatialRelation.h"


namespace lsst {
namespace sphgeom {

// `checkRelations` checks that evaluating the spatial predicates gives the
// expected outcomes.
template <typename U, typename V>
void checkRelations(U const & u,
                    V const & v,
                    int expectedRelations)
{
    bool shouldContain = (expectedRelations & CONTAINS) != 0;
    bool shouldIntersect = (expectedRelations & INTERSECTS) != 0;
    bool shouldBeWithin = (expectedRelations & WITHIN) != 0;
    bool shouldBeDisjointFrom = (expectedRelations & DISJOINT) != 0;
    CHECK(u.contains(v) == shouldContain);
    CHECK(u.intersects(v) == shouldIntersect);
    CHECK(u.isWithin(v) == shouldBeWithin);
    CHECK(u.isDisjointFrom(v) == shouldBeDisjointFrom);
    CHECK(u.relate(v) == expectedRelations);
}

template <typename U, typename V>
void checkDisjoint(U const & u, V const & v) {
    checkRelations(u, v, DISJOINT);
    checkRelations(v, u, DISJOINT);
}

template <typename U, typename V>
void checkIntersects(U const & u, V const & v) {
    checkRelations(u, v, INTERSECTS);
    checkRelations(v, u, INTERSECTS);
}

template <typename U, typename V>
void checkContains(U const & u, V const & v) {
    checkRelations(u, v, CONTAINS | INTERSECTS);
    checkRelations(v, u, INTERSECTS | WITHIN);
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
    checkRelations(r, R(), CONTAINS | DISJOINT);
    // ∅ should be within and disjoint from a non-empty region.
    checkRelations(R(), r, WITHIN | DISJOINT);
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
        checkRelations(r, p, CONTAINS | INTERSECTS);
        checkContains(r, R(p));
    }
    for (unsigned i = 0; i < outLength; ++i) {
        Point p = out[i];
        checkRelations(r, p, DISJOINT);
        checkDisjoint(r, R(p));
    }
}

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_RELATIONTESTUTILS_H_
