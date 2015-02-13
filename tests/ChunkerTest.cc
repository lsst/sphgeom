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
/// \brief This file contains tests for the Chunker class.

#include "Box.h"
#include "Chunker.h"

#include "Test.h"


using namespace lsst::sphgeom;

TEST_CASE(ChunksIntersecting1) {
    int32_t expectedChunkIds[21] = {
        6630, 6631, 6797, 6800, 6801, 6968, 6970, 6971,
        7138, 7140, 7141, 7308, 7310, 7311, 7478, 7480,
        7481, 7648, 7650, 7651, 7817
    };
    Box box = Box::fromDegrees(-0.1, -6, 4, 6);
    Chunker chunker(85, 14);
    CHECK(chunker.getNumStripes() == 85);
    CHECK(chunker.getNumSubStripesPerStripe() == 14);
    std::vector<int32_t> chunkIds = chunker.getChunksIntersecting(box);
    CHECK(chunkIds.size() == 21);
    for (size_t i = 0; i < chunkIds.size() && i < 21; ++i) {
        CHECK(chunkIds[i] == expectedChunkIds[i]);
    }
}
