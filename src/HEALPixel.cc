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
/// \brief This file contains the HEALPixel class implementation.

#include "lsst/sphgeom/HEALPixel.h"

#include <ostream>

namespace lsst {
namespace sphgeom {

namespace {

inline std::uint64_t parent(int level, HEALPixel const & p) {
    return p.nested() >> (2*(p.level() - level));
}

} // anonymous

bool HEALPixel::contains(HEALPixel const & p) const {
    if (level() < p.level()) {
        return nested() == parent(level(), p);
    } else if (level() > p.level()) {
        return false;
    } else {
        return nested() == p.nested();
    }
}

bool HEALPixel::isDisjointFrom(HEALPixel const & p) const {
    if (level() < p.level()) {
        return nested() != parent(level(), p);
    } else if (level() > p.level()) {
        return parent(p.level(), *this) != p.nested();
    } else {
        return nested() != p.nested();
    }
}

Relationship HEALPixel::relate(HEALPixel const & p) const {
    if (level() < p.level()) {
        return (nested() == parent(level(), p)) ? CONTAINS : DISJOINT;
    } else if (level() > p.level()) {
        return (parent(p.level(), *this) == p.nested()) ? WITHIN : DISJOINT;
    } else {
        return (nested() == p.nested()) ? (CONTAINS | WITHIN) : DISJOINT;
    }
}

std::ostream & operator<<(std::ostream & os, HEALPixel const & p) {
    return os << "{\"HEALPixel\": [level=" << p.level()
              << ", nested=" << p.nested() << "]";
}

}} // namespace lsst::sphgeom
