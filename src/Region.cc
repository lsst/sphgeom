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
/// \brief This file contains the Region class implementation.

#include <stdexcept>
#include <algorithm>

#include "lsst/sphgeom/Region.h"

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/CompoundRegion.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "base64.hpp"

namespace lsst {
namespace sphgeom {

bool Region::contains(double x, double y, double z) const {
    return contains(UnitVector3d(x, y, z));
}

bool Region::contains(double lon, double lat) const {
    return contains(UnitVector3d(LonLat::fromRadians(lon, lat)));
}

TriState
Region::overlaps(Region const& other) const {
    // Default implementation just uses `relate`, and it returns unknown state
    // more frequently, subclasses will want to implement better tests.
    auto r = this->relate(other);
    if ((r & DISJOINT).any()) {
        return TriState(false);
    } else if ((r & (CONTAINS | WITHIN)).any()) {
        return TriState(true);
    } else {
        return TriState();
    }
}

std::unique_ptr<Region> Region::decode(std::uint8_t const * buffer, size_t n) {
    if (buffer == nullptr || n == 0) {
        throw std::runtime_error("Byte-string is not an encoded Region");
    }
    std::uint8_t type = *buffer;
    if (type == Box::TYPE_CODE) {
        return Box::decode(buffer, n);
    } else if (type == Circle::TYPE_CODE) {
        return Circle::decode(buffer, n);
    } else if (type == ConvexPolygon::TYPE_CODE) {
        return ConvexPolygon::decode(buffer, n);
    } else if (type == Ellipse::TYPE_CODE) {
        return Ellipse::decode(buffer, n);
    } else if (type == UnionRegion::TYPE_CODE) {
        return UnionRegion::decode(buffer, n);
    } else if (type == IntersectionRegion::TYPE_CODE) {
        return IntersectionRegion::decode(buffer, n);
    }
    throw std::runtime_error("Byte-string is not an encoded Region");
}

std::unique_ptr<Region> Region::decodeBase64(std::string_view const & s) {
    if (s.empty()) {
        return std::unique_ptr<UnionRegion>(new UnionRegion({}));
    }
    auto region_begin = s.begin();
    auto region_end = std::find(s.begin(), s.end(), ':');
    if (region_end != s.end()) {
        std::vector<std::unique_ptr<Region>> union_args;
        while (region_end != s.end()) {
            auto bytes = base64::decode_into<std::vector<std::uint8_t>>(region_begin, region_end);
            union_args.push_back(decode(bytes));
            region_begin = region_end;
            ++region_begin;
            region_end = std::find(region_begin, s.end(), ':');
        }
        auto bytes = base64::decode_into<std::vector<std::uint8_t>>(region_begin, region_end);
        union_args.push_back(decode(bytes));
        return std::unique_ptr<UnionRegion>(new UnionRegion(std::move(union_args)));
    } else {
        auto bytes = base64::decode_into<std::vector<std::uint8_t>>(region_begin, region_end);
        return decode(bytes);
    }
}

TriState Region::decodeOverlapsBase64(std::string_view const & s) {
    TriState result(false);
    if (s.empty()) {
        // False makes the most sense as the limit of a logical OR of zero
        // terms (e.g. `any([])` in Python).
        return result;
    }
    auto begin = s.begin();
    while (result != true) {  // if result is known to be true, we're done.
        auto mid = std::find(begin, s.end(), '&');
        if (mid == s.end()) {
            throw std::runtime_error("No '&' found in encoded overlap expression term.");
        }
        auto a = Region::decode(base64::decode_into<std::vector<std::uint8_t>>(begin, mid));
        ++mid;
        auto end = std::find(mid, s.end(), '|');
        auto b = Region::decode(base64::decode_into<std::vector<std::uint8_t>>(mid, end));
        result = result | a->overlaps(*b);
        if (end == s.end()) {
            break;
        } else {
            begin = end;
            ++begin;
        }
    }
    return result;
}

std::vector<std::unique_ptr<Region>> Region::getRegions(Region const &region) {
    std::vector<std::unique_ptr<Region>> result;
    if (auto union_region = dynamic_cast<UnionRegion const *>(&region)) {
        for(unsigned i = 0; i < union_region->nOperands(); ++i) {
            result.emplace_back(union_region->getOperand(i).clone());
        }
    } else if(auto intersection_region = dynamic_cast<IntersectionRegion const *>(&region)) {
        for(unsigned i = 0; i < intersection_region->nOperands(); ++i) {
            result.emplace_back(intersection_region->getOperand(i).clone());
        }
    } else {
        result.emplace_back(region.clone());
    }
    return result;
}


}} // namespace lsst:sphgeom
