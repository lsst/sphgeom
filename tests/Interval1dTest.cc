/*
 * LSST Data Management System
 * Copyright 2016 AURA/LSST.
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
/// \brief This file contains tests for the Interval1d class.
///
/// These tests are brief because the Interval CRTP base class contains most
/// of the implementation and is already well covered by the AngleInterval
/// tests.

#include <cmath>

#include "lsst/sphgeom/Interval1d.h"

#include "test.h"
#include "relationshipTestUtils.h"


using namespace lsst::sphgeom;

void checkProperties(Interval1d const & i) {
    checkBasicProperties(i);
    // A non-empty interval should contain, be within, and intersect itself.
    checkRelationship(i, i, CONTAINS | WITHIN);
    // Taking the union with an identical or empty interval should be a no-op.
    CHECK(i.expandedTo(i) == i);
    CHECK(i.expandedTo(Interval1d()) == i);
    CHECK(Interval1d().expandedTo(i) == i);
    // Taking the intersection with an identical interval should be a no-op.
    CHECK(i.clippedTo(i) == i);
    // Intersecting with an empty interval should give the empty interval.
    CHECK(i.clippedTo(Interval1d()).isEmpty());
    CHECK(Interval1d().clippedTo(i).isEmpty());
    // Intersecting with a full interval should be a no-op.
    CHECK(i.clippedTo(Interval1d::full()) == i);
    CHECK(Interval1d::full().clippedTo(i) == i);
    // Taking the union with a full interval should give the full interval.
    CHECK(i.expandedTo(Interval1d::full()).isFull());
    CHECK(Interval1d::full().expandedTo(i) == Interval1d::full());
}

TEST_CASE(Stream) {
    Interval1d i = Interval1d(1, 2);
    std::stringstream ss;
    ss << i;
    CHECK(ss.str() == "[1, 2]");
}

TEST_CASE(EmptyInterval) {
    Interval1d emptyIntervals[5] = {
        Interval1d(),
        Interval1d(1, std::nan(nullptr)),
        Interval1d(std::nan(nullptr), 1),
        Interval1d(std::nan(nullptr), std::nan(nullptr)),
        Interval1d(2, 1)
    };
    for (int j = 0; j < 5; ++j) {
        Interval1d i = emptyIntervals[j];
        CHECK(i.isEmpty());
        CHECK(!i.isFull());
        CHECK(i == i);
        CHECK(i == std::nan(nullptr));
        // An empty interval should contain itself, be within itself,
        // and be disjoint from itself.
        checkRelationship(i, i, CONTAINS | WITHIN | DISJOINT);
        checkRelationship(i, std::nan(nullptr), CONTAINS | WITHIN | DISJOINT);
        for (int k = 0; k < j; ++k) {
            checkRelationship(i, emptyIntervals[k], CONTAINS | WITHIN | DISJOINT);
            checkRelationship(emptyIntervals[k], i, CONTAINS | WITHIN | DISJOINT);
        }
        checkRelationship(i, Interval1d(1), WITHIN | DISJOINT);
        checkRelationship(Interval1d(1), i, CONTAINS | DISJOINT);
        checkRelationship(i, 1, WITHIN | DISJOINT);
        // The union of the empty interval with itself should be empty.
        CHECK(i.expandedTo(i) == i);
        CHECK(i.expandedTo(1) == Interval1d(1));
        CHECK(i.expandedTo(Interval1d(1, 2)) == Interval1d(1, 2));
        // Intersecting with the empty interval should have no effect.
        CHECK(i.clippedTo(i) == i);
        // Morphological operations should have no effect on empty intervals.
        CHECK(i.dilatedBy(1) == i);
        CHECK(i.erodedBy(1) == i);
    }
}

TEST_CASE(FullInterval) {
    Interval1d i = Interval1d::full();
    checkProperties(i);
    // Morphological operations should have no effect on full intervals.
    CHECK(i.dilatedBy(1).isFull());
    CHECK(i.erodedBy(1).isFull());
    CHECK(i.dilatedBy(std::numeric_limits<double>::infinity()).isFull());
    CHECK(i.dilatedBy(-std::numeric_limits<double>::infinity()).isEmpty());
    CHECK(i.erodedBy(std::numeric_limits<double>::infinity()).isEmpty());
    CHECK(i.erodedBy(-std::numeric_limits<double>::infinity()).isFull());
}

TEST_CASE(Basic) {
    Interval1d i(0.5), j(1, 2);
    CHECK(i == 0.5);
    CHECK(i.getA() == i.getB());
    CHECK(j.getA() == 1 && j.getB() == 2);
    CHECK(i.getSize() == 0);
    CHECK(j.getSize() == 1);
    CHECK(i.getCenter() == i.getA());
    CHECK(j.getCenter() == 1.5);
    checkProperties(i);
    checkProperties(j);
}
