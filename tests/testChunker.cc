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

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Chunker.h"

#include "test.h"


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

TEST_CASE(AllSubChunks) {
     std::vector<int32_t> expectedSubChunkIds = {
         0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
         69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
         138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
         207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
         276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287,
         345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356,
         414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425,
         483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494,
         552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563,
         621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 632,
         690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701,
         759, 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770
    };
    Chunker chunker(85, 12);
    std::vector<int32_t> subChunkIds = chunker.getAllSubChunks(9630);
    CHECK(subChunkIds == expectedSubChunkIds);
}
