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
/// \brief This file contains the HTM indexing implementation.

#include "lsst/sphgeom/htm.h"

#include <stdexcept>

#include "lsst/sphgeom/curve.h"
#include "lsst/sphgeom/orientation.h"


namespace lsst {
namespace sphgeom {

namespace {
    // Store raw coordinates rather than UnitVector3d objects so that library
    // clients cannot run into the static initialization order fiasco.
    alignas(64) double const ROOT_VERTEX[8][3][3] = {
        {{ 1.0,  0.0, 0.0}, {0.0, 0.0, -1.0}, { 0.0,  1.0, 0.0}},
        {{ 0.0,  1.0, 0.0}, {0.0, 0.0, -1.0}, {-1.0,  0.0, 0.0}},
        {{-1.0,  0.0, 0.0}, {0.0, 0.0, -1.0}, { 0.0, -1.0, 0.0}},
        {{ 0.0, -1.0, 0.0}, {0.0, 0.0, -1.0}, { 1.0,  0.0, 0.0}},
        {{ 1.0,  0.0, 0.0}, {0.0, 0.0,  1.0}, { 0.0, -1.0, 0.0}},
        {{ 0.0, -1.0, 0.0}, {0.0, 0.0,  1.0}, {-1.0,  0.0, 0.0}},
        {{-1.0,  0.0, 0.0}, {0.0, 0.0,  1.0}, { 0.0,  1.0, 0.0}},
        {{ 0.0,  1.0, 0.0}, {0.0, 0.0,  1.0}, { 1.0,  0.0, 0.0}}
    };
}

int htmLevel(uint64_t i) {
    // An HTM index consists of 4 bits encoding the root triangle
    // number (8 - 15), followed by 2l bits, where each of the l bit pairs
    // encodes a child triangle number (0-3), and l is the desired level.
    int j = log2(i);
    // The level l is trivially derivable from the index j of the MSB of i.
    // For i to be valid, j must be an odd integer > 1.
    if ((j & 1) == 0 || (j == 1)) {
        return -1;
    }
    return (j - 3) >> 1;
}

ConvexPolygon htmTrixel(uint64_t i) {
    int level = htmLevel(i);
    if (level < 0 || level > MAX_HTM_LEVEL) {
        throw std::invalid_argument("Invalid HTM index");
    }
    level = level << 1;
    uint64_t r = (i >> level) & 7;
    UnitVector3d v0 = UnitVector3d::fromNormalized(
        ROOT_VERTEX[r][0][0], ROOT_VERTEX[r][0][1], ROOT_VERTEX[r][0][2]);
    UnitVector3d v1 = UnitVector3d::fromNormalized(
        ROOT_VERTEX[r][1][0], ROOT_VERTEX[r][1][1], ROOT_VERTEX[r][1][2]);
    UnitVector3d v2 = UnitVector3d::fromNormalized(
        ROOT_VERTEX[r][2][0], ROOT_VERTEX[r][2][1], ROOT_VERTEX[r][2][2]);
    for (level -= 2; level >= 0; level -= 2) {
        int child = (i >> level) & 3;
        UnitVector3d m12 = UnitVector3d(v1 + v2);
        UnitVector3d m20 = UnitVector3d(v2 + v0);
        UnitVector3d m01 = UnitVector3d(v0 + v1);
        switch (child) {
            case 0: v1 = m01; v2 = m20; break;
            case 1: v0 = v1; v1 = m12; v2 = m01; break;
            case 2: v0 = v2; v1 = m20; v2 = m12; break;
            case 3: v0 = m12; v1 = m20; v2 = m01; break;
        }
    }
    return ConvexPolygon(v0, v1, v2);
}

uint64_t htmIndex(UnitVector3d const & v, int level) {
    if (level < 0 || level > MAX_HTM_LEVEL) {
        std::invalid_argument("Invalid HTM subdivision level");
    }
    // Find the root triangle containing v.
    uint64_t r;
    if (v.z() < 0.0) {
        // v is in the southern hemisphere (root triangle 0, 1, 2, or 3).
        if (v.y() > 0.0) {
            r = (v.x() > 0.0) ? 0 : 1;
        } else if (v.y() == 0.0) {
            r = (v.x() >= 0.0) ? 0 : 2;
        } else {
            r = (v.x() < 0.0) ? 2 : 3;
        }
    } else {
        // v is in the northern hemisphere (root triangle 4, 5, 6, or 7).
        if (v.y() > 0.0) {
            r = (v.x() > 0.0) ? 7 : 6;
        } else if (v.y() == 0.0) {
            r = (v.x() >= 0.0) ? 7 : 5;
        } else {
            r = (v.x() < 0.0) ? 5 : 4;
        }
    }
    UnitVector3d v0 = UnitVector3d::fromNormalized(
        ROOT_VERTEX[r][0][0], ROOT_VERTEX[r][0][1], ROOT_VERTEX[r][0][2]);
    UnitVector3d v1 = UnitVector3d::fromNormalized(
        ROOT_VERTEX[r][1][0], ROOT_VERTEX[r][1][1], ROOT_VERTEX[r][1][2]);
    UnitVector3d v2 = UnitVector3d::fromNormalized(
        ROOT_VERTEX[r][2][0], ROOT_VERTEX[r][2][1], ROOT_VERTEX[r][2][2]);
    uint64_t i = r + 8;
    for (int l = 0; l < level; ++l) {
        UnitVector3d m01 = UnitVector3d(v0 + v1);
        UnitVector3d m20 = UnitVector3d(v2 + v0);
        i <<= 2;
        if (orientation(v, m01, m20) >= 0) {
            v1 = m01; v2 = m20;
            continue;
        }
        UnitVector3d m12 = UnitVector3d(v1 + v2);
        if (orientation(v, m12, m01) >= 0) {
            v0 = v1; v1 = m12; v2 = m01;
            i += 1;
        } else if (orientation(v, m20, m12) >= 0) {
            v0 = v2; v1 = m20; v2 = m12;
            i += 2;
        } else {
            v0 = m12; v1 = m20; v2 = m01;
            i += 3;
        }
    }
    return i;
}

}} // namespace lsst::sphgeom
