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
/// \brief This file contains tests for the RangeSet class.

#include "lsst/sphgeom/RangeSet.h"

#include "test.h"


using namespace lsst::sphgeom;

TEST_CASE(DefaultConstructor) {
    // The default constructor should create an empty set.
    RangeSet s;
    CHECK(s.isValid());
    CHECK(s.empty());
    CHECK(s.size() == 0);
    CHECK(s.begin() == s.end());
    // The complement of an empty set should be full.
    s.complement();
    CHECK(s.isValid());
    CHECK(s.full());
    CHECK(s.size() == 1);
    CHECK(s.beginc() == s.endc());
}

TEST_CASE(RangeConstructor) {
    RangeSet s0(2);
    s0.insert(1);
    CHECK(s0.isValid());
    CHECK(!s0.empty());
    CHECK(s0.size() == 1);
    CHECK(s0.begin() != s0.end());
    CHECK(s0.contains(1) && s0.contains(2));
    RangeSet s1(1, 3);
    RangeSet s2(std::tuple<uint64_t, uint64_t>(1, 3));
    CHECK(s0 == s1);
    CHECK(s1 == s2);
    RangeSet s3(0, 0);
    CHECK(s3.isValid() && s3.full());
}

TEST_CASE(Iteration) {
    RangeSet s = {{0, 1}, {2, 3}, {4, 5}, {6, 7}};
    uint64_t u = 0;
    for (auto const & t: s) {
        CHECK(std::get<0>(t) == u);
        CHECK(std::get<1>(t) == u + 1);
        u += 2;
    }
    CHECK(u == 8);
}

TEST_CASE(SizeAndCardinality) {
    RangeSet s = {0, 2, 4, 6, 8};
    CHECK(s.size() == 5);
    CHECK(s.cardinality() == 5);
    s = {1, 2, 3, 4, 5};
    CHECK(s.size() == 1);
    CHECK(s.cardinality() == 5);
}

TEST_CASE(Stream) {
    RangeSet s = {{1, 2}, {3, 7}, {9, 0}};
    std::stringstream ss;
    ss << s;
    CHECK(ss.str() == "{\"RangeSet\": [[1, 2], [3, 7], [9, 0]]}");
}

TEST_CASE(Insert) {
    // Note: list initialization of RangeSets maps to a series of inserts calls.
    // Check wrapping ranges
    RangeSet s = {{3, 0}, {0, 2}};
    CHECK(s.isValid());
    CHECK(s == RangeSet(3, 2));
    CHECK(s == RangeSet(2, 3).complement());
    // Check shared end-points
    s = {{0, 2}, {4, 0}, {2, 4}};
    CHECK(s.isValid() && s.full());
    s = {{0, 2}, {2, 0}};
    CHECK(s.isValid() && s.full());
    s = {{3, 0}, {0, 3}};
    CHECK(s.isValid() && s.full());
    s = {{2, 3}, {3, 4}, {8, 9}, {4, 8}};
    CHECK(s.isValid() && s == RangeSet(2, 9));
    s = {{2, 3}, {1, 2}};
    CHECK(s.isValid() && s == RangeSet(1, 3));
    s = {{1, 2}, {3, 0}, {2, 3}};
    CHECK(s.isValid() && s == RangeSet(1,0));
    // Check overlapping ranges
    s = {{1, 3}, {5, 7}, {2, 6}};
    CHECK(s.isValid() && s == RangeSet(1, 7));
    s = {{2, 3}, {5, 7}, {1, 6}, {0, 2}};
    CHECK(s.isValid() && s == RangeSet(0, 7));
    s = {{1, 3}, {2, 0}};
    CHECK(s.isValid() && s == RangeSet(1, 0));
    // Check full-range insertion
    s = {1, 2, 3};
    s.insert(0, 0);
    CHECK(s.isValid() && s.full());
    // Check disjoint range insertions
    s = {{2, 3}, {4, 5}, {0, 1}};
    CHECK(s.isValid() && s == RangeSet({{0, 1}, {2, 3}, {4, 5}}));
    s = {{4, 5}, {8, 9}, {2, 3}, {6, 7}};
    CHECK(s.isValid() && s == RangeSet({{2, 3}, {4, 5}, {6, 7}, {8, 9}}));
}

TEST_CASE(Erase) {
    RangeSet s{{0, 0}};
    CHECK(s.isValid() && s.full());
    s.erase(0, 0);
    CHECK(s.isValid() && s.empty());
    s = {{0, 0}};
    s.erase(2, 3);
    CHECK(s == RangeSet({{0, 2}, {3, 0}}));
    s.erase(1, 4);
    CHECK(s == RangeSet({{0, 1}, {4, 0}}));
}

TEST_CASE(Intersection) {
    RangeSet empty;
    RangeSet s = empty.intersection(RangeSet{{0, 1}});
    CHECK(s.isValid() && s.empty());
    s = RangeSet{{1, 0}}.intersection(empty);
    CHECK(s.isValid() && s.empty());
    RangeSet a = {{0, 2}, {4, 6}, {8, 10}};
    RangeSet b = {{2, 4}, {6, 8}, {10, 12}};
    s = a.intersection(b);
    CHECK(s.isValid() && s.empty());
    a = {{0, 5}, {6, 7}, {8, 9}};
    b = {{0, 1}, {2, 3}, {4, 5}, {6, 10}};
    s = a;
    s &= b;
    CHECK(s.isValid() && s == RangeSet({0, 2, 4, 6, 8}));
    CHECK((a & a) == a);
}

TEST_CASE(Join) {
    RangeSet full(0, 0);
    RangeSet s = full.join(RangeSet{{0, 1}});
    CHECK(s.isValid() && s.full());
    s = RangeSet{{0, 1}}.join(full);
    CHECK(s.isValid() && s.full());
    RangeSet a = {{1, 3}, {5, 7}, {9, 11}};
    RangeSet b = {{0, 1}, {3, 5}, {7, 9}};
    s = a;
    s |= b;
    CHECK(s.isValid() && s == RangeSet({{0, 11}}));
    CHECK((a | a) == a);
}

TEST_CASE(Difference) {
    RangeSet empty = {};
    RangeSet full = {{0, 0}};
    RangeSet s = empty.difference(RangeSet(3));
    CHECK(s.isValid() && s.empty());
    s = RangeSet(3).difference(full);
    CHECK(s.isValid() && s.empty());
    RangeSet a = {{1, 3}, {5, 7}, {9, 12}};
    RangeSet b = {{2, 6}, {10, 11}};
    s = a - b;
    CHECK(s.isValid() && s == RangeSet({1, 6, 9, 11}));
    a -= a;
    CHECK(a.empty());
    s = full - RangeSet(2, 4);
    CHECK(s.isValid() && s == RangeSet(4, 2));
}

TEST_CASE(SymmetricDifference) {
    RangeSet empty = {};
    RangeSet full = {{0, 0}};
    RangeSet s = empty.symmetricDifference(empty);
    CHECK(s.isValid() && s.empty());
    s = full.symmetricDifference(full);
    CHECK(s.isValid() && s.empty());
    s = full.symmetricDifference(empty);
    CHECK(s.isValid() && s.full());
    RangeSet a = {{0, 3}, {9, 0}};
    RangeSet b = {{0, 4}, {8, 0}};
    s = a.symmetricDifference(b);
    CHECK(s.isValid() && s == RangeSet({{3, 4}, {8, 9}}));
    CHECK(s == b.symmetricDifference(a));
    s = a.symmetricDifference(RangeSet(4, 8));
    CHECK(s.isValid() && s == RangeSet({{0, 3}, {4, 8}, {9, 0}}));
    CHECK(s == RangeSet(4, 8).symmetricDifference(a));
    s = a.symmetricDifference(RangeSet(2, 10));
    CHECK(s.isValid() && s == RangeSet({{0, 2}, {3, 9}, {10, 0}}));
    CHECK(s == RangeSet(2, 10).symmetricDifference(a));
    a.complement();
    b = ~b;
    s = a;
    s ^= b;
    CHECK(s == RangeSet({{3, 4}, {8, 9}}));
    CHECK((s ^ s).empty());
}

TEST_CASE(IntersectsAndIsDisjointFrom) {
    RangeSet empty = {};
    RangeSet full = {{0, 0}};
    CHECK(!empty.intersects(empty));
    CHECK(!empty.intersects(full));
    CHECK(!empty.intersects(0, 0));
    CHECK(!empty.intersects(1));
    CHECK(full.intersects(full));
    CHECK(full.intersects(0, 0));
    CHECK(full.intersects(3, 1));
    CHECK(empty.isDisjointFrom(empty));
    CHECK(empty.isDisjointFrom(full));
    CHECK(empty.isDisjointFrom(0, 0));
    CHECK(empty.isDisjointFrom(1));
    CHECK(!full.isDisjointFrom(full));
    CHECK(!full.isDisjointFrom(0, 0));
    CHECK(!full.isDisjointFrom(1));
    CHECK(RangeSet(1).intersects(0, 0));
    CHECK(!RangeSet(1).isDisjointFrom(0, 0));
    CHECK(RangeSet(1, 4).intersects(2, 3));
    CHECK(!RangeSet(1, 4).isDisjointFrom(2, 3));
    CHECK(RangeSet(1, 4).intersects(3, 2));
    CHECK(!RangeSet(1, 4).isDisjointFrom(3, 2));
    CHECK(RangeSet(4, 1).intersects(3, 2));
    CHECK(!RangeSet(4, 1).isDisjointFrom(3, 2));
    CHECK(!RangeSet().intersects(0, 0));
    CHECK(RangeSet().isDisjointFrom(0, 0));
    CHECK(!RangeSet(1).intersects(2));
    CHECK(RangeSet(1).isDisjointFrom(2));
    CHECK(!RangeSet(1, 2).intersects(3, 4));
    CHECK(RangeSet(1, 2).isDisjointFrom(3, 4));
    CHECK(!RangeSet(4, 6).intersects(8, 2));
    CHECK(RangeSet(4, 6).isDisjointFrom(8, 2));
    CHECK(!RangeSet(8, 2).intersects(4, 6));
    CHECK(RangeSet(8, 2).isDisjointFrom(4, 6));
    RangeSet a = {{0, 1}, {5, 8}, {9, 0}};
    RangeSet b = {{1, 5}, {8, 9}};
    CHECK(!a.intersects(b));
    CHECK(a.isDisjointFrom(b));
    CHECK(a.intersects(~b));
    CHECK(!a.isDisjointFrom(~b));
    CHECK((~a).intersects(b));
    CHECK(!(~a).isDisjointFrom(b));
}

TEST_CASE(ContainsAndIsWithin) {
    RangeSet empty = {};
    RangeSet full = {{0, 0}};
    CHECK(empty.contains(empty));
    CHECK(empty.isWithin(empty));
    CHECK(!empty.contains(full));
    CHECK(empty.isWithin(full));
    CHECK(full.contains(1));
    CHECK(RangeSet(1, 4).contains(2, 3));
    CHECK(!RangeSet(2, 3).contains(1, 4));
    CHECK(RangeSet(2, 3).isWithin(1, 4));
    CHECK(!RangeSet(1, 4).contains(2, 5));
    CHECK(!RangeSet(1, 4).isWithin(2, 5));
    CHECK(RangeSet(4, 2).contains(5, 1));
    CHECK(RangeSet(5, 1).isWithin(4, 2));
    CHECK(RangeSet(5, 1).isWithin(RangeSet(4, 2)));
    RangeSet a = {{0, 1}, {5, 8}, {9, 0}};
    RangeSet b = {{1, 5}, {8, 9}};
    CHECK(a.contains(a));
    CHECK(b.isWithin(b));
    CHECK(!a.contains(b));
    CHECK(!b.contains(a));
    CHECK(!a.isWithin(b));
    CHECK(!b.isWithin(a));
    CHECK(a.contains(~b));
    CHECK(a.isWithin(~b));
    a = {{1, 3}, {7, 9}};
    b = {{0, 3}, {6, 8}};
    CHECK(!a.contains(b));
    CHECK(!b.contains(a));
    b.insert(8);
    CHECK(a.isWithin(b));
    CHECK(b.contains(a));
}

TEST_CASE(Simplify) {
    RangeSet empty;
    RangeSet full(0, 0);
    CHECK(empty.simplify(1).empty());
    CHECK(full.simplify(1).full());
    CHECK(RangeSet(1).simplify(64).full());
    RangeSet a = {{0, 1}, {5, 8}, {9, 0}};
    RangeSet b = {{1, 3}, {8, 10}, {11, 12}, {16, 0}};
    RangeSet s = a.simplified(2);
    CHECK(s.isValid() && s.full());
    s = b.simplified(2);
    CHECK(s.isValid() && s == RangeSet({{0, 4}, {8, 12}, {16, 0}}));
}

TEST_CASE(Scale) {
    RangeSet s = {{0, 1}, {5, 8}, {9, 0}};
    s.scale(10);
    CHECK(s.isValid() && s == RangeSet({{0, 10}, {50, 80}, {90, 0}}));
}
