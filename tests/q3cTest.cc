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

#include "lsst/sphgeom/curve.h"
#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/q3c.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "test.h"

using namespace lsst::sphgeom;

void compareIndexes(uint64_t i0, uint64_t i1) {
    uint64_t const mask = UINT64_C(0xfffffffffffffff);
    uint32_t x0, y0;
    uint32_t x1, y1;
    // Compare face numbers for level 30 Q3C indexes i0 and i1.
    CHECK(i0 >> 60 == i1 >> 60);
    // Extract integer grid coordinates for level 30 Q3C indexes i0 and i1.
    std::tie(x0, y0) = mortonIndexInverse(i0 & mask);
    std::tie(x1, y1) = mortonIndexInverse(i1 & mask);
    // The difference between the expected and computed x and y values
    // must be at most 1. Note this code does not work for x1 or y1 = 0
    // or 2^30 - 1, so test points must be chosen accordingly.
    CHECK(x0 >= x1 - 1 && x0 <= x1 + 1);
    CHECK(y0 >= y1 - 1 && y0 <= y1 + 1);
}

TEST_CASE(InvalidResolution) {
    UnitVector3d v = UnitVector3d::X();
    CHECK_THROW(q3cIndex(v, 0, false, false), std::invalid_argument);
    CHECK_THROW(q3cIndex(v, MAX_Q3C_RESOLUTION + 1, false, false),
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
        LonLat::fromDegrees(20.0,  80.0),
        LonLat::fromDegrees(110.0, 80.0),
        LonLat::fromDegrees(200.0, 80.0),
        LonLat::fromDegrees(290.0, 80.0),
        LonLat::fromDegrees(20.0,  -80.0),
        LonLat::fromDegrees(110.0, -80.0),
        LonLat::fromDegrees(200.0, -80.0),
        LonLat::fromDegrees(290.0, -80.0),
    };
    // Expected Q3C indexes for the above, computed using the reference
    // PostgreSQL implementation, i.e. via:
    //
    // SELECT q3c_ang2ipix(0, 0);
    // ...
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
        UINT64_C(6433090441885948578),
    };
    // The PostgreSQL Q3C implementation uses a fixed grid resolution of 2^30.
    uint32_t const n = 0x40000000;
    // Check for close agreement with the PostgreSQL Q3C implementation.
    for (size_t i = 0; i < sizeof(points) / sizeof(LonLat); ++i) {
        UnitVector3d v(points[i]);
        compareIndexes(q3cIndex(v, n, false, false), indexes[i]);
        compareIndexes(q3cIndex(v, n - 1, false, false), indexes[i]);
    }
}
