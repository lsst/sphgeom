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
/// \brief This file contains tests for space filling curve functions.

#include "lsst/sphgeom/curve.h"

#include "test.h"

using namespace lsst::sphgeom;

void checkMorton(uint32_t x, uint32_t y, uint64_t z) {
    CHECK(z == mortonIndex(x, y));
    uint32_t xi, yi;
    std::tie(xi, yi) = mortonIndexInverse(z);
    CHECK(x == xi);
    CHECK(y == yi);
}

void checkHilbert(uint32_t x, uint32_t y, uint32_t m, uint64_t h) {
    CHECK(h == hilbertIndex(x, y, m));
    uint32_t xi, yi;
    std::tie(xi, yi) = hilbertIndexInverse(h, m);
    CHECK(x == xi);
    CHECK(y == yi);
}

TEST_CASE(Log2) {
    for (uint32_t s = 0; s < 64; ++s) {
        uint32_t x32 = static_cast<uint32_t>(1) << s;
        uint64_t x64 = static_cast<uint64_t>(1) << s;
        if (s < 32) {
            CHECK(log2(x32) == s);
        }
        CHECK(log2(x64) == s);
    }
    CHECK(log2(static_cast<uint32_t>(0)) == 0);
    CHECK(log2(static_cast<uint64_t>(0)) == 0);
}

TEST_CASE(Morton) {
    checkMorton(0, 0, 0);
    checkMorton(1, 0, 1);
    checkMorton(0, 1, 2);
    checkMorton(1, 1, 3);
    checkMorton(0xffffffff, 0, UINT64_C(0x5555555555555555));
    checkMorton(0, 0xffffffff, UINT64_C(0xaaaaaaaaaaaaaaaa));
    checkMorton(0xffffffff, 0xffffffff, UINT64_C(0xffffffffffffffff));
    for (uint32_t xb = 0; xb < 32; ++xb) {
        for (uint32_t yb = 0; yb < 32; ++yb) {
            uint32_t x = static_cast<uint32_t>(1) << xb;
            uint32_t y = static_cast<uint32_t>(1) << yb;
            uint64_t z = (static_cast<uint64_t>(1) << (2 * xb)) +
                         (static_cast<uint64_t>(1) << (2 * yb + 1));
            checkMorton(x, y, z);
            checkMorton(~x, ~y, ~z);
        }
    }
}

TEST_CASE(Hilbert) {
    // Check order 1 Hilbert lattice
    checkHilbert(0, 0, 1, 0);
    checkHilbert(0, 1, 1, 1);
    checkHilbert(1, 1, 1, 2);
    checkHilbert(1, 0, 1, 3);
    // Check order 2 Hilbert lattice
    uint32_t const points2[16][2] = {
        {0, 0}, {1, 0}, {1, 1}, {0, 1},
        {0, 2}, {0, 3}, {1, 3}, {1, 2},
        {2, 2}, {2, 3}, {3, 3}, {3, 2},
        {3, 1}, {2, 1}, {2, 0}, {3, 0}
    };
    for (uint32_t i = 0; i < 16; ++i) {
        checkHilbert(points2[i][0], points2[i][1], 2, i);
    }
    // Check order 3 Hilbert lattice
    uint32_t const points3[64][2] = {
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
    for (uint32_t i = 0; i < 64; ++i) {
        checkHilbert(points3[i][0], points3[i][1], 3, i);
    }
}
