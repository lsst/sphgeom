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
/// \brief This file contains tests for the AngleInterval class.

#include "AngleInterval.h"
#include "SpatialRelation.h"

#include "Test.h"
#include "RelationTestUtils.h"


using namespace lsst::sphgeom;

void checkProperties(AngleInterval const & i) {
    checkBasicProperties(i);
    // A non-empty interval should contain, be within, and intersect itself.
    checkRelations(i, i, CONTAINS | INTERSECTS | WITHIN);
    // Taking the union with an identical or empty interval should be a no-op.
    CHECK(i.expandedTo(i) == i);
    CHECK(i.expandedTo(AngleInterval()) == i);
    CHECK(AngleInterval().expandedTo(i) == i);
    // Taking the intersection with an identical interval should be a no-op.
    CHECK(i.clippedTo(i) == i);
    // Intersecting with an empty interval should give the empty interval.
    CHECK(i.clippedTo(AngleInterval()).isEmpty());
    CHECK(AngleInterval().clippedTo(i).isEmpty());
}

TEST_CASE(Stream) {
    AngleInterval i = AngleInterval::fromRadians(1,2);
    std::stringstream ss;
    ss << i;
    CHECK(ss.str() == "[1 rad, 2 rad]");
}

TEST_CASE(EmptyInterval) {
    Angle a1(1), a2(2);
    AngleInterval emptyIntervals[5] = {
        AngleInterval(),
        AngleInterval(a1, Angle::nan()),
        AngleInterval(Angle::nan(), a1),
        AngleInterval(Angle::nan(), Angle::nan()),
        AngleInterval(a2, a1)
    };
    for (int j = 0; j < 5; ++j) {
        AngleInterval i = emptyIntervals[j];
        CHECK(i.isEmpty());
        CHECK(i == i);
        CHECK(i == Angle::nan());
        // An empty interval should contain itself, be within itself,
        // and be disjoint from itself.
        checkRelations(i, i, CONTAINS | WITHIN | DISJOINT);
        checkRelations(i, Angle::nan(), CONTAINS | WITHIN | DISJOINT);
        for (int k = 0; k < j; ++k) {
            checkRelations(i, emptyIntervals[k], CONTAINS | WITHIN | DISJOINT);
            checkRelations(emptyIntervals[k], i, CONTAINS | WITHIN | DISJOINT);
        }
        checkRelations(i, AngleInterval(a1), WITHIN | DISJOINT);
        checkRelations(AngleInterval(a1), i, CONTAINS | DISJOINT);
        checkRelations(i, a1, WITHIN | DISJOINT);
        // The union of the empty interval with itself should be empty.
        CHECK(i.expandedTo(i) == i);
        CHECK(i.expandedTo(a1) == AngleInterval(a1));
        CHECK(i.expandedTo(AngleInterval(a1, a2)) == AngleInterval(a1, a2));
        // Intersecting with the empty interval should have no effect.
        CHECK(i.clippedTo(i) == i);
        // Morphological operations should have no effect on empty intervals.
        CHECK(i.dilatedBy(a1) == i);
        CHECK(i.erodedBy(a1) == i);
    }
}

TEST_CASE(BasicPoint) {
    AngleInterval i(Angle(0.1));
    CHECK(i == Angle(0.1));
    CHECK(i != Angle(1));
    CHECK(i.getA() == i.getB());
    CHECK(i.getSize() == Angle(0));
    CHECK(i.getCenter() == i.getA());
    checkProperties(i);
}

TEST_CASE(BasicInterval) {
    Angle a1(1), a2(2);
    AngleInterval i(a1, a2);
    CHECK(i.getA() == a1 && i.getB() == a2);
    CHECK(i != a1 && i != a2);
    CHECK(i.getSize() == a1);
    CHECK(i.getCenter() == Angle(1.5));
    checkProperties(i);
}

TEST_CASE(PointPointRelations) {
    AngleInterval u(Angle(0.2)), v(Angle(5.1));
    CHECK(u != v);
    checkRelations(u, u, CONTAINS | INTERSECTS | WITHIN);
    checkRelations(v, Angle(5.1), CONTAINS | INTERSECTS | WITHIN);
    checkRelations(u, v, DISJOINT);
    checkRelations(v, Angle(0.2), DISJOINT);
}

TEST_CASE(PointIntervalRelations) {
    Angle const in[3] = { Angle(1), Angle(3), Angle(2) };
    Angle const out[2] = { Angle(0), Angle(4) };
    AngleInterval i(in[0], in[1]);
    CHECK(i.getSize() > Angle(0));
    checkPoints(in, 3, out, 2, i);
    checkRelations(AngleInterval(Angle(1)),
                   Angle::nan(),
                   CONTAINS | DISJOINT);
    checkRelations(AngleInterval(Angle(1), Angle(5)),
                   Angle::nan(),
                   CONTAINS | DISJOINT);
}

TEST_CASE(IntervalIntervalRelations) {
    Angle a1(1), a2(2), a3(3), a4(4), a5(5);
    // Test disjoint intervals.
    checkDisjoint(AngleInterval(a1, a2), AngleInterval(a3, a4));
    // Test intervals that intersect only at endpoints.
    checkIntersects(AngleInterval(a1, a2), AngleInterval(a2, a3));
    // Test intervals with intersecting interiors.
    checkIntersects(AngleInterval(a2, a4), AngleInterval(a3, a5));
    // Test intervals that contain or are within another interval.
    checkContains(AngleInterval(a1, a5), AngleInterval(a1, a2));
    checkContains(AngleInterval(a1, a5), AngleInterval(a4, a5));
    checkContains(AngleInterval(a1, a5), AngleInterval(a2, a3));
}

TEST_CASE(PointExpansion) {
    Angle a1(1), a2(2), a3(3), a4(4), a5(5), a6(6);
    // Check expansions of  points.
    CHECK(AngleInterval(a1).expandedTo(a3).getSize() == a2);
    CHECK(AngleInterval(a1).expandedTo(AngleInterval(a3)) ==
          AngleInterval(a1).expandedTo(a3));
    CHECK(AngleInterval(a1).expandedTo(AngleInterval(a3)) ==
          AngleInterval(a3).expandedTo(a1));
    CHECK(AngleInterval(a1).expandedTo(AngleInterval(a2)) ==
          AngleInterval(a1, a2));
    CHECK(AngleInterval(a2).expandedTo(AngleInterval(a1)) ==
          AngleInterval(a1, a2));
    // Check expansions of points to intervals and vice versa.
    CHECK(AngleInterval(a1).expandedTo(AngleInterval(a3, a5)) ==
          AngleInterval(a3, a5).expandedTo(AngleInterval(a1)));
    CHECK(AngleInterval(a1).expandedTo(AngleInterval(a3, a5)) ==
          AngleInterval(a1, a5));
    CHECK(AngleInterval(a2)
            .expandedTo(AngleInterval(a3, a5))
            .isDisjointFrom(a6));
    CHECK(AngleInterval(a2)
            .expandedTo(AngleInterval(a3, a5))
            .isDisjointFrom(AngleInterval(a1)));
    CHECK(AngleInterval(a2)
            .expandedTo(AngleInterval(a3, a5)) ==
          AngleInterval(a3, a5)
            .expandedTo(AngleInterval(a2)));
    CHECK(AngleInterval(a3, a5).expandedTo(a4) == AngleInterval(a3, a5));
    CHECK(AngleInterval(a1).expandedTo(Angle::nan()) == AngleInterval(a1));
    CHECK(AngleInterval(a2, a3).expandedTo(Angle::nan()) ==
          AngleInterval(a2, a3));
}

TEST_CASE(IntervalExpansion) {
    Angle a1(1), a2(2), a3(3), a4(4), a5(5);
    CHECK(AngleInterval(a1, a2).expandedTo(AngleInterval(a2, a3)) ==
          AngleInterval(a1, a3));
    CHECK(AngleInterval(a2, a3).expandedTo(AngleInterval(a1, a2)) ==
          AngleInterval(a1, a3));
    CHECK(AngleInterval(a1, a2).expandedTo(AngleInterval(a4, a5)) ==
          AngleInterval(a1, a5));
    CHECK(AngleInterval(a4, a5).expandedTo(AngleInterval(a1, a2)) ==
          AngleInterval(a1, a5));
    CHECK(AngleInterval(a1, a3).expandedTo(AngleInterval(a2, a5)) ==
          AngleInterval(a1, a5));
    CHECK(AngleInterval(a1, a2).expandedTo(AngleInterval(a1, Angle::nan())) ==
          AngleInterval(a1, a2));
    CHECK(AngleInterval(a1, a2).expandedTo(AngleInterval(Angle::nan(), a1)) ==
          AngleInterval(a1, a2));
    CHECK(AngleInterval(a1, a2)
            .expandedTo(AngleInterval(Angle::nan(), Angle::nan())) ==
          AngleInterval(a1, a2));
}

TEST_CASE(PointContraction) {
    Angle a0(0), a1(1), a2(2), a3(3), a4(4);
    CHECK(AngleInterval(a1).clippedTo(AngleInterval(a1)) == AngleInterval(a1));
    CHECK(AngleInterval(a1).clippedTo(AngleInterval(a1)) == a1);
    CHECK(AngleInterval(a1).clippedTo(AngleInterval(a2)).isEmpty());
    CHECK(AngleInterval(a2).clippedTo(AngleInterval(a1)).isEmpty());
    CHECK(AngleInterval(a1).clippedTo(AngleInterval(a1, a2)) ==
          AngleInterval(a1));
    CHECK(AngleInterval(a1, a2).clippedTo(AngleInterval(a1)) ==
          AngleInterval(a1));
    CHECK(AngleInterval(a1, a2).clippedTo(AngleInterval(a2)) == a2);
    CHECK(AngleInterval(a2).clippedTo(AngleInterval(a1, a2)) ==
          AngleInterval(a2));
    CHECK(AngleInterval(a2, a3).clippedTo(a0).isEmpty());
    CHECK(AngleInterval(a2, a3).clippedTo(a4).isEmpty());
    CHECK(AngleInterval(a2).clippedTo(Angle::nan()).isEmpty());
    CHECK(AngleInterval(a2, a3).clippedTo(Angle::nan()).isEmpty());
}

TEST_CASE(IntervalContraction) {
    Angle a1(1), a2(2), a3(3), a4(4);
    CHECK(AngleInterval(a1, a2).clippedTo(AngleInterval(a2, a4)) == a2);
    CHECK(AngleInterval(a2, a4).clippedTo(AngleInterval(a1, a2)) == a2);
    CHECK(AngleInterval(a1, a2).clippedTo(AngleInterval(a3, a4)).isEmpty());
    CHECK(AngleInterval(a3, a4).clippedTo(AngleInterval(a1, a2)).isEmpty());
    CHECK(AngleInterval(a1, a3).clippedTo(AngleInterval(a2, a4)) ==
          AngleInterval(a2, a3));
    CHECK(AngleInterval(a2, a4).clippedTo(AngleInterval(a1, a3)) ==
          AngleInterval(a2, a3));
    CHECK(AngleInterval(a1, a2)
            .clippedTo(AngleInterval(a1, Angle::nan()))
            .isEmpty());
    CHECK(AngleInterval(a1, a2)
            .clippedTo(AngleInterval(Angle::nan(), a1))
            .isEmpty());
    CHECK(AngleInterval(a1, a2)
            .clippedTo(AngleInterval(Angle::nan(), Angle::nan()))
            .isEmpty());
}

TEST_CASE(Dilation) {
    Angle a0(0), a1(1), a2(2), a3(3), a5(5);
    CHECK(AngleInterval(a2).dilatedBy(a0) == a2);
    CHECK(AngleInterval(a2).dilatedBy(a1) == AngleInterval(a1, a3));
    CHECK(AngleInterval(a2, a3).dilatedBy(a2) == AngleInterval(a0, a5));
    CHECK(AngleInterval(a2, a3).dilatedBy(Angle::nan()) ==
          AngleInterval(a2, a3));
}

TEST_CASE(Erosion) {
    Angle a0(0), a1(1), a2(2), a3(3), a4(4), a5(5);
    CHECK(AngleInterval(a2).erodedBy(a0) == a2);
    CHECK(AngleInterval(a2).erodedBy(Angle(0.0001)).isEmpty());
    CHECK(AngleInterval(a2, a4).erodedBy(a1) == a3);
    CHECK(AngleInterval(a1, a5).erodedBy(a1) == AngleInterval(a2, a4));
    CHECK(AngleInterval(a2, a3).erodedBy(Angle::nan()) ==
          AngleInterval(a2, a3));
}
