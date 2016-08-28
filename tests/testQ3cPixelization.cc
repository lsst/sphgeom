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
/// \brief This file contains tests for Q3C indexing.

#include <algorithm>

#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/Q3cPixelization.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "test.h"

using namespace lsst::sphgeom;

void compareIndexes(Q3cPixelization const & p, uint64_t i0, uint64_t i1) {
    // Check that the neighborhood of each index contains the other index.
    // This ensures that the indexes are either the same, or correspond to
    // to adjacent pixels.
    std::vector<uint64_t> neighborhood = p.neighborhood(i0);
    auto n = std::find(neighborhood.begin(), neighborhood.end(), i1);
    CHECK(n != neighborhood.end());
    neighborhood = p.neighborhood(i1);
    n = std::find(neighborhood.begin(), neighborhood.end(), i0);
    CHECK(n != neighborhood.end());
}

TEST_CASE(InvalidLevel) {
    CHECK_THROW(Q3cPixelization(-1), std::invalid_argument);
    CHECK_THROW((Q3cPixelization(Q3cPixelization::MAX_LEVEL + 1)),
                std::invalid_argument);
}

TEST_CASE(IndexPoint) {
    // A collection of test points distributed over all 6 Q3C cube faces.
    LonLat const points[] = {
        LonLat::fromDegrees(  0.0,   0.0),
        LonLat::fromDegrees( 90.0,   0.0),
        LonLat::fromDegrees(180.0,   0.0),
        LonLat::fromDegrees(270.0,   0.0),
        LonLat::fromDegrees(340.0,  20.0),
        LonLat::fromDegrees(340.0, -20.0),
        LonLat::fromDegrees( 20.0,  20.0),
        LonLat::fromDegrees( 20.0, -20.0),
        LonLat::fromDegrees( 70.0,  20.0),
        LonLat::fromDegrees( 70.0, -20.0),
        LonLat::fromDegrees(110.0,  20.0),
        LonLat::fromDegrees(110.0, -20.0),
        LonLat::fromDegrees(160.0,  20.0),
        LonLat::fromDegrees(160.0, -20.0),
        LonLat::fromDegrees(200.0,  20.0),
        LonLat::fromDegrees(200.0, -20.0),
        LonLat::fromDegrees(250.0,  20.0),
        LonLat::fromDegrees(250.0, -20.0),
        LonLat::fromDegrees(290.0,  20.0),
        LonLat::fromDegrees(290.0, -20.0),
        LonLat::fromDegrees( 20.0,  80.0),
        LonLat::fromDegrees(110.0,  80.0),
        LonLat::fromDegrees(200.0,  80.0),
        LonLat::fromDegrees(290.0,  80.0),
        LonLat::fromDegrees( 20.0, -80.0),
        LonLat::fromDegrees(110.0, -80.0),
        LonLat::fromDegrees(200.0, -80.0),
        LonLat::fromDegrees(290.0, -80.0)
    };
    // Expected Q3C indexes for the above, computed using the reference
    // PostgreSQL implementation, i.e. via:
    //
    // SELECT q3c_ang2ipix(0, 0);
    // ...
    //
    // The PostgreSQL Q3C implementation uses a fixed grid resolution of 2^30.
    uint64_t const indexes[] = {
        UINT64_C(2017612633061982208),
        UINT64_C(3170534137668829184),
        UINT64_C(4323455642275676160),
        UINT64_C(5476377146882523136),
        UINT64_C(1851042551661831114),
        UINT64_C(1376575149377390944),
        UINT64_C(2082189364443149983),
        UINT64_C(1607721962158709813),
        UINT64_C(3003964056268678090),
        UINT64_C(2529496653984237920),
        UINT64_C(3235110869049996959),
        UINT64_C(2760643466765556789),
        UINT64_C(4156885560875525066),
        UINT64_C(3682418158591084896),
        UINT64_C(4388032373656843935),
        UINT64_C(3913564971372403765),
        UINT64_C(5309807065482372042),
        UINT64_C(4835339663197931872),
        UINT64_C(5540953878263690911),
        UINT64_C(5066486475979250741),
        UINT64_C(471152125043369297),
        UINT64_C(870242532897312759),
        UINT64_C(681769379563477678),
        UINT64_C(282678971709534216),
        UINT64_C(6639278876168802299),
        UINT64_C(6249046108789368157),
        UINT64_C(6042857674506514436),
        UINT64_C(6433090441885948578)
    };
    for (int level = 30; level >= 0; --level) {
        Q3cPixelization p(level);
        // Check for close agreement with the PostgreSQL Q3C implementation.
        for (size_t i = 0; i < sizeof(points) / sizeof(LonLat); ++i) {
            compareIndexes(p, p.index(UnitVector3d(points[i])),
                           indexes[i] >> (60 - 2*level));
        }
    }
}


TEST_CASE(Envelope) {
    auto pixelization = Q3cPixelization(1);
    for (uint64_t i = 0; i < 4*6; ++i) {
        UnitVector3d v = pixelization.quad(i).getCentroid();
        auto c = Circle(v, Angle::fromDegrees(0.1));
        RangeSet rs = pixelization.envelope(c);
        CHECK(rs == RangeSet(i));
    }
}


TEST_CASE(Interior) {
    auto pixelization = Q3cPixelization(2);
    for (uint64_t i = 0; i < 4*4*6; ++i) {
        auto p = pixelization.quad(i);
        auto c = p.getBoundingCircle();
        RangeSet rs = pixelization.interior(c);
        CHECK(rs == RangeSet(i));
        rs = pixelization.interior(p);
        CHECK(rs == RangeSet(i));
    }
}


TEST_CASE(Neighborhood) {
    for (int level = 0; level < 3; ++level) {
        Q3cPixelization p(level);
        for (uint64_t i = 0; i < (6 << 2*level); ++i) {
            ConvexPolygon q = p.quad(i);
            RangeSet rs1 = p.envelope(q);
            RangeSet rs2 = RangeSet(p.neighborhood(i));
            CHECK(rs1 == rs2);
        }
    }
}
