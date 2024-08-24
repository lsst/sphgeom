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
/// \brief This file contains the HtmPixelization class implementation.

#include "lsst/sphgeom/HtmPixelization.h"

#include "lsst/sphgeom/curve.h"
#include "lsst/sphgeom/orientation.h"

#include "PixelFinder.h"


namespace lsst {
namespace sphgeom {

namespace {

// `rootVertex` returns the i-th (0-3) root vertex of HTM root triangle r (0-8).
UnitVector3d const & rootVertex(int r, int i) {
    static UnitVector3d const VERTICES[8][3] = {
        { UnitVector3d::X(), -UnitVector3d::Z(),  UnitVector3d::Y()},
        { UnitVector3d::Y(), -UnitVector3d::Z(), -UnitVector3d::X()},
        {-UnitVector3d::X(), -UnitVector3d::Z(), -UnitVector3d::Y()},
        {-UnitVector3d::Y(), -UnitVector3d::Z(),  UnitVector3d::X()},
        { UnitVector3d::X(),  UnitVector3d::Z(), -UnitVector3d::Y()},
        {-UnitVector3d::Y(),  UnitVector3d::Z(), -UnitVector3d::X()},
        {-UnitVector3d::X(),  UnitVector3d::Z(),  UnitVector3d::Y()},
        { UnitVector3d::Y(),  UnitVector3d::Z(),  UnitVector3d::X()}
    };
    return VERTICES[r][i];
}

// `HtmPixelFinder` locates trixels that intersect a region.
template <typename RegionType, bool InteriorOnly>
class HtmPixelFinder: public detail::PixelFinder<
    HtmPixelFinder<RegionType, InteriorOnly>, RegionType, InteriorOnly, 3>
{
    using Base = detail::PixelFinder<
        HtmPixelFinder<RegionType, InteriorOnly>, RegionType, InteriorOnly, 3>;
    using Base::visit;

public:
    HtmPixelFinder(RangeSet & ranges,
                   RegionType const & region,
                   int level,
                   size_t maxRanges):
        Base(ranges, region, level, maxRanges)
    {}

    void operator()() {
        UnitVector3d trixel[3];
        // Loop over HTM root triangles.
        for (std::uint64_t r = 0; r < 8; ++r) {
            for (int v = 0; v < 3; ++v) {
                trixel[v] = rootVertex(r, v);
            }
            visit(trixel, r + 8, 0);
        }
    }

    void subdivide(UnitVector3d const * trixel, std::uint64_t index, int level) {
        UnitVector3d mid[3] = {
            UnitVector3d(trixel[1] + trixel[2]),
            UnitVector3d(trixel[2] + trixel[0]),
            UnitVector3d(trixel[0] + trixel[1])
        };
        UnitVector3d child[3] = {trixel[0], mid[2], mid[1]};
        index *= 4;
        ++level;
        visit(child, index, level);
        child[0] = trixel[1];
        child[1] = mid[0];
        child[2] = mid[2];
        ++index;
        visit(child, index, level);
        child[0] = trixel[2];
        child[1] = mid[1];
        child[2] = mid[0];
        ++index;
        visit(child, index, level);
        ++index;
        visit(mid, index, level);
    }
};

} // unnamed namespace


int HtmPixelization::level(std::uint64_t i) {
    // An HTM index consists of 4 bits encoding the root triangle
    // number (8 - 15), followed by 2l bits, where each of the l bit pairs
    // encodes a child triangle number (0-3), and l is the desired level.
    int j = log2(i);
    // The level l is derivable from the index j of the MSB of i.
    // For i to be valid, j must be an odd integer > 1.
    if ((j & 1) == 0 || (j == 1)) {
        return -1;
    }
    return (j - 3) >> 1;
}

ConvexPolygon HtmPixelization::triangle(std::uint64_t i) {
    int l = level(i);
    if (l < 0 || l > MAX_LEVEL) {
        throw std::invalid_argument("Invalid HTM index");
    }
    l *= 2;
    std::uint64_t r = (i >> l) & 7;
    UnitVector3d v0 = rootVertex(r, 0);
    UnitVector3d v1 = rootVertex(r, 1);
    UnitVector3d v2 = rootVertex(r, 2);
    for (l -= 2; l >= 0; l -= 2) {
        int child = (i >> l) & 3;
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

std::string HtmPixelization::asString(std::uint64_t i) {
    char s[MAX_LEVEL + 2];
    int l = level(i);
    if (l < 0 || l > MAX_LEVEL) {
        throw std::invalid_argument("Invalid HTM index");
    }
    // Print in base-4, from least to most significant digit.
    char * p = s + (sizeof(s) - 1);
    for (; l >= 0; --l, --p, i >>= 2) {
        *p = '0' + (i & 3);
    }
    // The remaining bit corresponds to the hemisphere.
    *p = (i & 1) == 0 ? 'S' : 'N';
    return std::string(p, sizeof(s) - static_cast<size_t>(p - s));
}

HtmPixelization::HtmPixelization(int level) : _level(level) {
    if (level < 0 || level > MAX_LEVEL) {
        throw std::invalid_argument("Invalid HTM subdivision level");
    }
}

std::uint64_t HtmPixelization::index(UnitVector3d const & v) const {
    // Find the root triangle containing v.
    std::uint64_t r;
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
    UnitVector3d v0 = rootVertex(r, 0);
    UnitVector3d v1 = rootVertex(r, 1);
    UnitVector3d v2 = rootVertex(r, 2);
    std::uint64_t i = r + 8;
    for (int l = 0; l < _level; ++l) {
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

RangeSet HtmPixelization::_envelope(Region const & r, size_t maxRanges) const {
    return detail::findPixels<HtmPixelFinder, false>(r, maxRanges, _level);
}

RangeSet HtmPixelization::_interior(Region const & r, size_t maxRanges) const {
    return detail::findPixels<HtmPixelFinder, true>(r, maxRanges, _level);
}

}} // namespace lsst::sphgeom
