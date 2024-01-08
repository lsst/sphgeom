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
/// \brief This file contains tests for the NormalizedAngleInterval class.

#include <stdexcept>

#include "lsst/sphgeom/NormalizedAngleInterval.h"

#include "test.h"
#include "relationshipTestUtils.h"


using namespace lsst::sphgeom;

void checkProperties(NormalizedAngleInterval const & i) {
    checkBasicProperties(i);
    // A non-empty region should contain, be within, and intersect itself.
    checkRelationship(i, i, CONTAINS | WITHIN);
    // Taking the union with an identical or empty interval should be a no-op.
    CHECK(i.expandedTo(i) == i);
    CHECK(i.expandedTo(NormalizedAngleInterval()) == i);
    CHECK(NormalizedAngleInterval().expandedTo(i) == i);
    // Taking the intersection with an identical interval should be a no-op.
    CHECK(i.clippedTo(i) == i);
    // Intersecting with an empty interval should give the empty interval.
    CHECK(i.clippedTo(NormalizedAngleInterval()).isEmpty());
    CHECK(NormalizedAngleInterval().clippedTo(i).isEmpty());


    // The union with a full interval should result in a full interval.
    CHECK(i.expandedTo(NormalizedAngleInterval::full()).isFull());
    CHECK(NormalizedAngleInterval::full().expandedTo(i).isFull());
    // Intersecting with an identical or full interval should have no effect.
    CHECK(i.clippedTo(NormalizedAngleInterval::full()) == i);
    CHECK(NormalizedAngleInterval::full().clippedTo(i) == i);
    if (!i.isFull()) {
        // A full interval should contain and intersect a non-empty interval.
        checkContains(NormalizedAngleInterval::full(), i);
    }
}

TEST_CASE(Stream) {
    NormalizedAngleInterval i(Angle(1), Angle(2));
    std::stringstream ss;
    ss << i;
    CHECK(ss.str() == "[1, 2]");
}

TEST_CASE(Construction) {
    CHECK_THROW(NormalizedAngleInterval(Angle(7), Angle(1)),
                std::invalid_argument);
    CHECK_THROW(NormalizedAngleInterval(Angle(3), Angle(-1)),
                std::invalid_argument);
    CHECK(NormalizedAngleInterval(Angle(-10), Angle(10)).isFull());
    CHECK(NormalizedAngleInterval(Angle(10), Angle(20)).isFull());
    CHECK(NormalizedAngleInterval(Angle(6), Angle(7)).wraps());
    CHECK_THROW(NormalizedAngleInterval(Angle(-1), Angle(-20)),
                std::invalid_argument);
}

TEST_CASE(BasicEmptyInterval) {
    NormalizedAngleInterval i;
    CHECK(i.isEmpty());
    CHECK(!i.isFull());
    CHECK(i == i);
    CHECK(i == NormalizedAngleInterval::empty());
    CHECK(i == NormalizedAngle::nan());
    // An empty interval should contain itself, be within itself,
    // and be disjoint from itself.
    checkRelationship(i, i, CONTAINS | WITHIN | DISJOINT);
    checkRelationship(i, NormalizedAngle::nan(), CONTAINS | WITHIN | DISJOINT);
    checkRelationship(i, NormalizedAngle(1), DISJOINT | WITHIN);
    // The union with the empty/full interval should result in the
    // empty/full interval.
    CHECK(i.expandedTo(i) == i);
    CHECK(i.expandedTo(NormalizedAngleInterval::full()).isFull());
    // Intersecting with the empty/full interval should have no effect.
    CHECK(i.clippedTo(i) == i);
    CHECK(i.clippedTo(NormalizedAngleInterval::full()) == i);
    // Morphological operations should have no effect on empty intervals.
    CHECK(i.dilatedBy(Angle(PI)) == i);
    CHECK(i.erodedBy(Angle(PI)) == i);
    // Check properties of nearly empty intervals.
    NormalizedAngleInterval nearlyEmpty(Angle(2 * PI), Angle(1.0e-40));
    CHECK(!nearlyEmpty.isEmpty());
    CHECK(nearlyEmpty.getSize().asRadians() == 0.0);
    CHECK(NormalizedAngleInterval(Angle::nan(), Angle(1)).isEmpty());
    CHECK(NormalizedAngleInterval(Angle(1), Angle::nan()).isEmpty());

}

TEST_CASE(BasicFullInterval) {
    NormalizedAngleInterval i = NormalizedAngleInterval::full();
    CHECK(i.isFull());
    CHECK(!i.wraps());
    CHECK(i.getSize() == 2*Angle(PI));
    CHECK(i != NormalizedAngleInterval::empty());
    CHECK(!(i == NormalizedAngleInterval::empty()));
    checkProperties(i);
    // Morphological operations should have no effect on full intervals.
    CHECK(i.dilatedBy(Angle(PI)) == i);
    CHECK(i.erodedBy(Angle(PI)) == i);
    // Check properties of nearly full intervals.
    NormalizedAngleInterval nearlyFull(Angle(1.0e-40), Angle(2.0 * PI));
    CHECK(!nearlyFull.isFull());
    CHECK(nearlyFull.getSize().asRadians() == 2.0 * PI);
}

TEST_CASE(BasicPoint) {
    NormalizedAngle zero(0.0), a(0.1);
    NormalizedAngleInterval i(a);
    CHECK(i == a);
    CHECK(i != zero);
    CHECK(!i.isFull());
    CHECK(!i.wraps());
    CHECK(i.getA() == i.getB());
    CHECK(i.getSize() == zero);
    CHECK(i.getCenter() == i.getA());
    checkProperties(i);
}

TEST_CASE(BasicInterval) {
    NormalizedAngle a1(1.0), a2(2.0);
    NormalizedAngleInterval i(a1, a2);
    CHECK(i.getA() == a1 && i.getB() == a2);
    CHECK(i != a1 && i != a2);
    CHECK(!i.isFull());
    CHECK(!i.wraps());
    CHECK(i.getSize() == NormalizedAngle(1.0));
    CHECK(i.getCenter() == NormalizedAngle(1.5));
    checkProperties(i);
}

TEST_CASE(BasicWrappingInterval) {
    NormalizedAngle a2(2.0), a6(6.0), a1(1.0);
    NormalizedAngleInterval i1(a2, a1), i2(a6, a1);
    CHECK(i1.getA() == a2 && i1.getB() == a1);
    CHECK(i2.getA() == a6 && i1.getB() == a1);
    CHECK(!i1.isFull() && !i2.isFull());
    CHECK(i1.wraps() && i2.wraps());
    CHECK(i1 != i2);
    CHECK_CLOSE(i1.getSize().asRadians(), 2.0 * PI - 1.0, 2.0);
    CHECK_CLOSE(i1.getCenter().asRadians(), PI + 1.5, 1.0);
    CHECK_CLOSE(i2.getSize().asRadians(), 2.0 * PI - 5.0, 2.0);
    CHECK_CLOSE(i2.getCenter().asRadians(), 3.5 - PI, 1.0);
    checkProperties(i1);
    checkProperties(i2);
}

TEST_CASE(PointPointRelations) {
    NormalizedAngle a1(0.2), a2(5.1);
    NormalizedAngleInterval i1(a1), i2(a2);
    CHECK(i1 != i2);
    checkRelationship(i1, i1, CONTAINS | WITHIN);
    checkRelationship(i2, a2, CONTAINS | WITHIN);
    checkRelationship(i1, i2, DISJOINT);
    checkRelationship(i2, a1, DISJOINT);
}

TEST_CASE(PointIntervalRelations) {
    NormalizedAngle const in[3] = {
        NormalizedAngle(1),
        NormalizedAngle(3),
        NormalizedAngle(2)
    };
    NormalizedAngle const out[2] = {
        NormalizedAngle(0),
        NormalizedAngle(4)
    };
    NormalizedAngleInterval i(in[0], in[1]);
    CHECK(!i.wraps());
    CHECK(i.getSize() > Angle(0));
    checkPoints(in, 3, out, 2, i);
    checkRelationship(i, NormalizedAngle::nan(), CONTAINS | DISJOINT);
}

TEST_CASE(PointWrappingIntervalRelations) {
    NormalizedAngle const in[4] = {
        NormalizedAngle(3),
        NormalizedAngle(1),
        NormalizedAngle(0),
        NormalizedAngle(4)
    };
    NormalizedAngle const out[1] = { NormalizedAngle(2) };
    NormalizedAngleInterval i(in[0], in[1]);
    CHECK(i.wraps());
    CHECK(i.getSize() > Angle(0));
    checkPoints(in, 4, out, 1, i);
}

TEST_CASE(IntervalIntervalRelations) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5);
    // Test disjoint intervals.
    checkDisjoint(NormalizedAngleInterval(a1, a2),
                  NormalizedAngleInterval(a3, a4));
    // Test intervals that intersect only at endpoints.
    checkIntersects(NormalizedAngleInterval(a1, a2),
                    NormalizedAngleInterval(a2,a3));
    // Test intervals with intersecting interiors.
    checkIntersects(NormalizedAngleInterval(a2, a4),
                    NormalizedAngleInterval(a3, a5));
    // Test intervals that contain or are within another interval.
    checkContains(NormalizedAngleInterval(a1, a5),
                  NormalizedAngleInterval(a1, a2));
    checkContains(NormalizedAngleInterval(a1, a5),
                  NormalizedAngleInterval(a4, a5));
    checkContains(NormalizedAngleInterval(a1, a5),
                  NormalizedAngleInterval(a2, a3));
}

TEST_CASE(IntervalWrappingIntervalRelations) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5);
    // Test intervals that are disjoint.
    checkDisjoint(NormalizedAngleInterval(a2, a4),
                  NormalizedAngleInterval(a5, a1));
    // Test intervals that intersect only at one or two endpoints.
    checkIntersects(NormalizedAngleInterval(a1, a5),
                    NormalizedAngleInterval(a5, a1));
    checkIntersects(NormalizedAngleInterval(a3, a5),
                    NormalizedAngleInterval(a5, a1));
    checkIntersects(NormalizedAngleInterval(a1, a2),
                    NormalizedAngleInterval(a5, a1));
    // Test intervals that cross twice.
    checkIntersects(NormalizedAngleInterval(a1, a5),
                    NormalizedAngleInterval(a4, a2));
    // Test wrapping intervals that contain non-wrapping intervals.
    checkContains(NormalizedAngleInterval(a2, a1),
                  NormalizedAngleInterval(a2, a4));
    checkContains(NormalizedAngleInterval(a2, a1),
                  NormalizedAngleInterval(a3, a5));
    checkContains(NormalizedAngleInterval(a4, a2),
                  NormalizedAngleInterval(a1, a2));
    checkContains(NormalizedAngleInterval(a5, a3),
                  NormalizedAngleInterval(a1, a2));
}

TEST_CASE(WrappingWrappingIntervalRelations) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5);
    // Test intervals that intersect.
    checkIntersects(NormalizedAngleInterval(a4, a2),
                    NormalizedAngleInterval(a5, a3));
    // Test intervals that contain or are within another interval,
    // including intervals that share an endpoint.
    checkContains(NormalizedAngleInterval(a5, a3),
                  NormalizedAngleInterval(a5, a1));
    checkContains(NormalizedAngleInterval(a2, a1),
                  NormalizedAngleInterval(a5, a1));
    checkContains(NormalizedAngleInterval(a4, a2),
                  NormalizedAngleInterval(a5, a1));
}

TEST_CASE(PointExpansion) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5);
    // Check expansions to points.
    CHECK(NormalizedAngleInterval(a1).expandedTo(a3).getSize() == Angle(2));
    CHECK(!NormalizedAngleInterval(a1).expandedTo(a3).wraps());
    CHECK(NormalizedAngleInterval(a1)
            .expandedTo(NormalizedAngleInterval(a3)) ==
          NormalizedAngleInterval(a1)
            .expandedTo(a3));
    CHECK(NormalizedAngleInterval(a1)
            .expandedTo(NormalizedAngleInterval(a3)) ==
          NormalizedAngleInterval(a3)
            .expandedTo(NormalizedAngleInterval(a1)));
    CHECK(NormalizedAngleInterval(a1)
            .expandedTo(NormalizedAngleInterval(a5)) ==
          NormalizedAngleInterval(a5, a1));
    CHECK(NormalizedAngleInterval(a5)
            .expandedTo(NormalizedAngleInterval(a1)) ==
          NormalizedAngleInterval(a5, a1));
    // Check expansions to intervals (wrapping and non-wrapping).
    CHECK(!NormalizedAngleInterval(a1)
            .expandedTo(NormalizedAngleInterval(a3, a5))
            .wraps());
    CHECK(NormalizedAngleInterval(a1)
            .expandedTo(NormalizedAngleInterval(a3, a5)) ==
          NormalizedAngleInterval(a3, a5)
            .expandedTo(NormalizedAngleInterval(a1)));
    CHECK(NormalizedAngleInterval(a2)
            .expandedTo(NormalizedAngleInterval(a5, a1))
            .isDisjointFrom(a3));
    CHECK(NormalizedAngleInterval(a2)
            .expandedTo(NormalizedAngleInterval(a5, a1)) ==
          NormalizedAngleInterval(a5, a1)
            .expandedTo(NormalizedAngleInterval(a2)));
    CHECK(NormalizedAngleInterval::empty().expandedTo(a1) ==
          NormalizedAngleInterval(a1));
    CHECK(NormalizedAngleInterval(a1, a2).expandedTo(a2) ==
          NormalizedAngleInterval(a1, a2));
    CHECK(NormalizedAngleInterval(a2, a3).expandedTo(a1) ==
          NormalizedAngleInterval(a1, a3));
    CHECK(NormalizedAngleInterval(a2, a3).expandedTo(a4) ==
          NormalizedAngleInterval(a2, a4));
    CHECK(NormalizedAngleInterval(a1).expandedTo(NormalizedAngle::nan()) ==
          NormalizedAngleInterval(a1));
}

TEST_CASE(IntervalExpansion) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5), a6(6);
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a1, a2));
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a4, a5)) ==
          NormalizedAngleInterval(a4, a5)
            .expandedTo(NormalizedAngleInterval(a1, a2)));
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a4, a5)) ==
          NormalizedAngleInterval(a1, a5));
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a2, a3)) ==
          NormalizedAngleInterval(a2, a3)
            .expandedTo(NormalizedAngleInterval(a1, a2)));
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a2, a3)) ==
          NormalizedAngleInterval(a1, a3));
    CHECK(NormalizedAngleInterval(a1, a3)
            .expandedTo(NormalizedAngleInterval(a2, a5)) ==
          NormalizedAngleInterval(a2, a5)
            .expandedTo(NormalizedAngleInterval(a1, a3)));
    CHECK(NormalizedAngleInterval(a1, a3)
            .expandedTo(NormalizedAngleInterval(a2, a5)) ==
          NormalizedAngleInterval(a1, a5));
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a5, a6)) ==
          NormalizedAngleInterval(a5, a6)
            .expandedTo(NormalizedAngleInterval(a1, a2)));
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a5, a6)) ==
          NormalizedAngleInterval(a5, a2));
    CHECK(NormalizedAngleInterval(a1, a5)
            .expandedTo(NormalizedAngleInterval(a2, a3)) ==
          NormalizedAngleInterval(a1, a5));
    CHECK(NormalizedAngleInterval(a2, a3)
            .expandedTo(NormalizedAngleInterval(a1, a5)) ==
          NormalizedAngleInterval(a1, a5));
    CHECK(NormalizedAngleInterval(a1, a5)
            .expandedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a1, a5));
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a1, a5)) ==
          NormalizedAngleInterval(a1, a5));
}

TEST_CASE(WrappingIntervalExpansion) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5), a6(6);
    CHECK(NormalizedAngleInterval(a6, a1)
            .expandedTo(NormalizedAngleInterval(a6, a1)) ==
          NormalizedAngleInterval(a6, a1));
    CHECK(NormalizedAngleInterval(a1, a2)
            .expandedTo(NormalizedAngleInterval(a6, a1)) ==
          NormalizedAngleInterval(a6, a2));
    CHECK(NormalizedAngleInterval(a6, a1)
            .expandedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a6, a2));
    CHECK(NormalizedAngleInterval(a5, a6)
            .expandedTo(NormalizedAngleInterval(a6, a1)) ==
          NormalizedAngleInterval(a5, a1));
    CHECK(NormalizedAngleInterval(a6, a1)
            .expandedTo(NormalizedAngleInterval(a5, a6)) ==
          NormalizedAngleInterval(a5, a1));
    CHECK(NormalizedAngleInterval(a1, a6)
            .expandedTo(NormalizedAngleInterval(a6, a1))
            .isFull());
    CHECK(NormalizedAngleInterval(a6, a1)
            .expandedTo(NormalizedAngleInterval(a1, a6))
            .isFull());
    CHECK(NormalizedAngleInterval(a1, a6)
            .expandedTo(NormalizedAngleInterval(a5, a2))
            .isFull());
    CHECK(NormalizedAngleInterval(a5, a2)
            .expandedTo(NormalizedAngleInterval(a1, a6))
            .isFull());
    CHECK(NormalizedAngleInterval(a1, a3)
            .expandedTo(NormalizedAngleInterval(a5, a2)) ==
          NormalizedAngleInterval(a5, a3));
    CHECK(NormalizedAngleInterval(a5, a2)
            .expandedTo(NormalizedAngleInterval(a1, a3)) ==
          NormalizedAngleInterval(a5, a3));
    CHECK(NormalizedAngleInterval(a4, a6)
            .expandedTo(NormalizedAngleInterval(a5, a2)) ==
          NormalizedAngleInterval(a4, a2));
    CHECK(NormalizedAngleInterval(a5, a2)
            .expandedTo(NormalizedAngleInterval(a4, a6)) ==
          NormalizedAngleInterval(a4, a2));
    CHECK(NormalizedAngleInterval(a5, a2)
            .expandedTo(NormalizedAngleInterval::empty()) ==
          NormalizedAngleInterval(a5, a2));
}

TEST_CASE(PointContraction) {
    NormalizedAngle a0(0), a1(1), a2(2), a3(3), a4(4);
    CHECK(NormalizedAngleInterval(a1).clippedTo(NormalizedAngleInterval(a1)) ==
          NormalizedAngleInterval(a1));
    CHECK(NormalizedAngleInterval(a1).clippedTo(a1) ==
          NormalizedAngleInterval(a1));
    CHECK(NormalizedAngleInterval(a1)
            .clippedTo(NormalizedAngleInterval(a2))
            .isEmpty());
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a1)) ==
          NormalizedAngleInterval(a1));
    CHECK(NormalizedAngleInterval(a1, a2).clippedTo(a1) ==
          NormalizedAngleInterval(a1));
    CHECK(NormalizedAngleInterval(a1)
            .clippedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a1));
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a2)) ==
          NormalizedAngleInterval(a2));
    CHECK(NormalizedAngleInterval(a1, a2).clippedTo(a2) ==
          NormalizedAngleInterval(a2));
    CHECK(NormalizedAngleInterval(a2)
            .clippedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a2));
    CHECK(NormalizedAngleInterval(a1, a2).clippedTo(a0).isEmpty());
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a0))
            .isEmpty());
    CHECK(NormalizedAngleInterval(a1, a2).clippedTo(a4).isEmpty());
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a4))
            .isEmpty());
    CHECK(NormalizedAngleInterval(a1, a3).clippedTo(a2) ==
          NormalizedAngleInterval(a2));
    CHECK(NormalizedAngleInterval(a1, a3)
            .clippedTo(NormalizedAngleInterval(a2)) ==
          NormalizedAngleInterval(a2));
    CHECK(NormalizedAngleInterval(a1)
            .clippedTo(NormalizedAngleInterval::empty())
            .isEmpty());
}

TEST_CASE(IntervalContraction) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5), a6(6);
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a1, a2));
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a4, a5))
            .isEmpty());
    CHECK(NormalizedAngleInterval(a4, a5)
            .clippedTo(NormalizedAngleInterval(a1, a2))
            .isEmpty());
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a2, a3)) ==
          a2);
    CHECK(NormalizedAngleInterval(a2, a3)
            .clippedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a2));
    CHECK(NormalizedAngleInterval(a1, a3)
            .clippedTo(NormalizedAngleInterval(a2, a5)) ==
          NormalizedAngleInterval(a2, a3));
    CHECK(NormalizedAngleInterval(a2, a5)
            .clippedTo(NormalizedAngleInterval(a1, a3)) ==
          NormalizedAngleInterval(a2, a3));
    CHECK(NormalizedAngleInterval(a1, a6)
            .clippedTo(NormalizedAngleInterval::empty())
            .isEmpty());
}

TEST_CASE(WrappingIntervalContraction) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5), a6(6);
    CHECK(NormalizedAngleInterval(a5, a2)
            .clippedTo(NormalizedAngleInterval(a6, a1)) ==
          NormalizedAngleInterval(a6, a1));
    CHECK(NormalizedAngleInterval(a6, a1)
            .clippedTo(NormalizedAngleInterval(a5, a2)) ==
          NormalizedAngleInterval(a6, a1));
    CHECK(NormalizedAngleInterval(a5, a2)
            .clippedTo(NormalizedAngleInterval(a5, a3)) ==
          NormalizedAngleInterval(a5, a2));
    CHECK(NormalizedAngleInterval(a5, a3)
            .clippedTo(NormalizedAngleInterval(a5, a2)) ==
          NormalizedAngleInterval(a5, a2));
    CHECK(NormalizedAngleInterval(a5, a2)
            .clippedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a1, a2));
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a5, a2)) ==
          NormalizedAngleInterval(a1, a2));
    CHECK(NormalizedAngleInterval(a5, a3)
            .clippedTo(NormalizedAngleInterval(a1, a2)) ==
          NormalizedAngleInterval(a1, a2));
    CHECK(NormalizedAngleInterval(a1, a2)
            .clippedTo(NormalizedAngleInterval(a5, a3)) ==
          NormalizedAngleInterval(a1, a2));
    CHECK(NormalizedAngleInterval(a6, a1)
            .clippedTo(NormalizedAngleInterval(a1, a6)) ==
          NormalizedAngleInterval(a6, a1));
    CHECK(NormalizedAngleInterval(a1, a6)
            .clippedTo(NormalizedAngleInterval(a6, a1)) ==
          NormalizedAngleInterval(a6, a1));
    CHECK(NormalizedAngleInterval(a6, a1)
            .clippedTo(NormalizedAngleInterval(a2, a3))
            .isEmpty());
    CHECK(NormalizedAngleInterval(a2, a3)
            .clippedTo(NormalizedAngleInterval(a6, a1))
            .isEmpty());
    CHECK(NormalizedAngleInterval(a6, a1)
            .clippedTo(NormalizedAngleInterval::empty())
            .isEmpty());
}

TEST_CASE(Dilation) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5), a6(6);
    CHECK(NormalizedAngleInterval(a1).dilatedBy(Angle(0.0)) == a1);
    CHECK(NormalizedAngleInterval(a1).dilatedBy(Angle(PI)).isFull());
    CHECK(NormalizedAngleInterval::empty().dilatedBy(Angle(PI)).isEmpty());
    CHECK(NormalizedAngleInterval(a3).dilatedBy(a2) ==
          NormalizedAngleInterval(a1, a5));
    CHECK(NormalizedAngleInterval(a2, a3).dilatedBy(a1) ==
          NormalizedAngleInterval(a1, a4));
    CHECK(NormalizedAngleInterval(a4, a2).dilatedBy(a1).isFull());
    CHECK(NormalizedAngleInterval(a6, a1).dilatedBy(a1) ==
          NormalizedAngleInterval(a5, a2));
    CHECK(NormalizedAngleInterval(a1, a6).dilatedBy(a1).isFull());
    CHECK(NormalizedAngleInterval(a1, a6).dilatedBy(a2).isFull());
    CHECK(NormalizedAngleInterval(a1, a2).dilatedBy(a2) ==
          NormalizedAngleInterval(Angle(2*PI - 1), a4));
    CHECK(NormalizedAngleInterval(a5, a6).dilatedBy(a2) ==
          NormalizedAngleInterval(a3, Angle(8 - 2*PI)));
    NormalizedAngleInterval n(a2);
    n.dilateBy(a1);
    CHECK(n == NormalizedAngleInterval(a1, a3));
    CHECK(n.dilatedBy(Angle::nan()) == n);
    CHECK(n.dilatedBy(Angle(0)) == n);
}

TEST_CASE(Erosion) {
    NormalizedAngle a1(1), a2(2), a3(3), a4(4), a5(5), a6(6);
    CHECK(NormalizedAngleInterval(a1).erodedBy(Angle(PI)).isEmpty());
    CHECK(NormalizedAngleInterval(a1, a6).erodedBy(Angle(PI)).isEmpty());
    CHECK(NormalizedAngleInterval::full().erodedBy(Angle(PI)).isFull());
    CHECK(NormalizedAngleInterval(a2, a4).erodedBy(a1) == a3);
    CHECK(NormalizedAngleInterval(a2, a4).erodedBy(a2).isEmpty());
    CHECK(NormalizedAngleInterval(a4, a2).erodedBy(a1) ==
          NormalizedAngleInterval(a5, a1));
    CHECK(NormalizedAngleInterval(a5, a3).erodedBy(a2) ==
          NormalizedAngleInterval(Angle(7 - 2*PI), a1));
    CHECK(NormalizedAngleInterval(a3, a1).erodedBy(a2) ==
          NormalizedAngleInterval(a5, Angle(2*PI - 1)));
    CHECK(NormalizedAngleInterval(a6, a1).erodedBy(a3).isEmpty());
    CHECK(NormalizedAngleInterval(a2, a1).erodedBy(a3).isEmpty());
    NormalizedAngleInterval n(a1, a3);
    n.erodeBy(a1);
    CHECK(n == NormalizedAngleInterval(a2));
    CHECK(n.erodedBy(Angle::nan()) == n);
    CHECK(n.erodedBy(Angle(0)) == n);
}
