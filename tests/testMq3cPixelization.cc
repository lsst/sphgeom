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
/// \brief This file contains tests for modified-Q3C indexing.

#include <algorithm>

#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/Mq3cPixelization.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "test.h"

using namespace lsst::sphgeom;

TEST_CASE(InvalidLevel) {
    CHECK_THROW(Mq3cPixelization(-1), std::invalid_argument);
    CHECK_THROW((Mq3cPixelization(Mq3cPixelization::MAX_LEVEL + 1)),
                std::invalid_argument);
}


TEST_CASE(IndexPoint1) {
    // Level 1 hilbert lattice over all 6 cube faces, in order.
    UnitVector3d const points[] = {
        UnitVector3d(-0.5, -0.5, -1.0),
        UnitVector3d(-0.5,  0.5, -1.0),
        UnitVector3d( 0.5,  0.5, -1.0),
        UnitVector3d( 0.5, -0.5, -1.0),
        UnitVector3d( 1.0, -0.5, -0.5),
        UnitVector3d( 1.0, -0.5,  0.5),
        UnitVector3d( 1.0,  0.5,  0.5),
        UnitVector3d( 1.0,  0.5, -0.5),
        UnitVector3d( 0.5,  1.0, -0.5),
        UnitVector3d(-0.5,  1.0, -0.5),
        UnitVector3d(-0.5,  1.0,  0.5),
        UnitVector3d( 0.5,  1.0,  0.5),
        UnitVector3d( 0.5,  0.5,  1.0),
        UnitVector3d( 0.5, -0.5,  1.0),
        UnitVector3d(-0.5, -0.5,  1.0),
        UnitVector3d(-0.5,  0.5,  1.0),
        UnitVector3d(-1.0,  0.5,  0.5),
        UnitVector3d(-1.0,  0.5, -0.5),
        UnitVector3d(-1.0, -0.5, -0.5),
        UnitVector3d(-1.0, -0.5,  0.5),
        UnitVector3d(-0.5, -1.0,  0.5),
        UnitVector3d( 0.5, -1.0,  0.5),
        UnitVector3d( 0.5, -1.0, -0.5),
        UnitVector3d(-0.5, -1.0, -0.5)
    };
    auto pixelization = Mq3cPixelization(1);
    for (uint64_t i = 0; i < sizeof(points) / sizeof(UnitVector3d); ++i) {
        CHECK(pixelization.index(points[i]) == i + 10*4);
    }
}


TEST_CASE(IndexPoint3) {
    // Level 3 hilbert lattice
    double const lattice[64][2] = {
        {0, 0}, {0, 1}, {1, 1}, {1, 0},
        {2, 0}, {3, 0}, {3, 1}, {2, 1},
        {2, 2}, {3, 2}, {3, 3}, {2, 3},
        {1, 3}, {1, 2}, {0, 2}, {0, 3},
        {0, 4}, {1, 4}, {1, 5}, {0, 5},
        {0, 6}, {0, 7}, {1, 7}, {1, 6},
        {2, 6}, {2, 7}, {3, 7}, {3, 6},
        {3, 5}, {2, 5}, {2, 4}, {3, 4},
        {4, 4}, {5, 4}, {5, 5}, {4, 5},
        {4, 6}, {4, 7}, {5, 7}, {5, 6},
        {6, 6}, {6, 7}, {7, 7}, {7, 6},
        {7, 5}, {6, 5}, {6, 4}, {7, 4},
        {7, 3}, {7, 2}, {6, 2}, {6, 3},
        {5, 3}, {4, 3}, {4, 2}, {5, 2},
        {5, 1}, {4, 1}, {4, 0}, {5, 0},
        {6, 0}, {6, 1}, {7, 1}, {7, 0}
    };
    // For each cube face, construct (roughly) the center of each level 3
    // pixel, in hilbert order, and check that it has the expected index.
    auto pixelization = Mq3cPixelization(3);
    // Face 10: (u, v) = ( x,  y)
    for (unsigned int i = 0; i < 64; ++i) {
        double s = lattice[i][0];
        double t = lattice[i][1];
        UnitVector3d v(0.25 * s - 0.875, 0.25 * t - 0.875, -1.0);
        CHECK(pixelization.index(v) == i + 10*64);
    }
    // Face 11: (u, v) = ( y,  z)
    for (unsigned int i = 0; i < 64; ++i) {
        double s = lattice[i][0];
        double t = lattice[i][1];
        UnitVector3d v(1.0, 0.25 * s - 0.875, 0.25 * t - 0.875);
        CHECK(pixelization.index(v) == i + 11*64);
    }
    // Face 12: (u, v) = ( z, -x)
    for (unsigned int i = 0; i < 64; ++i) {
        double s = lattice[i][0];
        double t = lattice[i][1];
        UnitVector3d v(0.875 - 0.25 * t, 1.0, 0.25 * s - 0.875);
        CHECK(pixelization.index(v) == i + 12*64);
    }
    // Face 13: (u, v) = (-x, -y)
    for (unsigned int i = 0; i < 64; ++i) {
        double s = lattice[i][0];
        double t = lattice[i][1];
        UnitVector3d v(0.875 - 0.25 * s, 0.875 - 0.25 * t, 1.0);
        CHECK(pixelization.index(v) == i + 13*64);
    }
    // Face 14: (u, v) = (-y, -z)
    for (unsigned int i = 0; i < 64; ++i) {
        double s = lattice[i][0];
        double t = lattice[i][1];
        UnitVector3d v(-1.0, 0.875 - 0.25 * s, 0.875 - 0.25 * t);
        CHECK(pixelization.index(v) == i + 14*64);
    }
    // Face 15: (u, v) = (-z,  x)
    for (unsigned int i = 0; i < 64; ++i) {
        double s = lattice[i][0];
        double t = lattice[i][1];
        UnitVector3d v(0.25 * t - 0.875 , -1.0, 0.875 - 0.25 * s);
        CHECK(pixelization.index(v) == i + 15*64);
    }
}


TEST_CASE(Envelope) {
    auto pixelization = Mq3cPixelization(1);
    auto universe = pixelization.universe();
    for (uint64_t i = 10*4; i < 16*4; ++i) {
        UnitVector3d v = pixelization.quad(i).getCentroid();
        auto c = Circle(v, Angle::fromDegrees(0.1));
        RangeSet rs = pixelization.envelope(c);
        CHECK(rs == RangeSet(i));
        CHECK(rs.isWithin(universe));
    }
}


TEST_CASE(Interior) {
    auto pixelization = Mq3cPixelization(2);
    auto universe = pixelization.universe();
    for (uint64_t i = 10*16; i < 16*16; ++i) {
        auto p = pixelization.quad(i);
        auto c = p.getBoundingCircle();
        RangeSet rs = pixelization.interior(c);
        CHECK(rs == RangeSet(i));
        CHECK(rs.isWithin(universe));
        rs = pixelization.interior(p);
        CHECK(rs == RangeSet(i));
        CHECK(rs.isWithin(universe));
    }
}


TEST_CASE(Neighborhood) {
    for (int level = 0; level < 3; ++level) {
        auto pixelization = Mq3cPixelization(level);
        auto universe = pixelization.universe();
        for (uint64_t i = 10 << (2*level); i < 16 << 2*level; ++i) {
            ConvexPolygon q = pixelization.quad(i);
            RangeSet rs1 = pixelization.envelope(q);
            RangeSet rs2 = RangeSet(pixelization.neighborhood(i));
            CHECK(rs1 == rs2);
            CHECK(rs1.isWithin(universe));
            uint64_t n = rs1.cardinality();
            CHECK(n == 5 || n == 8 || n == 9);
        }
    }
}
