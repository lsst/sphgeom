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
/// \brief This file contains the Vector3d class implementation.

#include "lsst/sphgeom/UnitVector3d.h"

#include <ostream>


namespace lsst {
namespace sphgeom {

UnitVector3d UnitVector3d::orthogonalTo(Vector3d const & v) {
    if (std::fabs(v.x()) > std::fabs(v.y())) {
        return UnitVector3d(-v.z(), 0.0, v.x());
    }
    return UnitVector3d(0.0, v.z(), -v.y());
}

UnitVector3d UnitVector3d::orthogonalTo(Vector3d const & v1,
                                        Vector3d const & v2)
{
    Vector3d n = (v2 + v1).cross(v2 - v1);
    if (n.isZero()) {
        return orthogonalTo(v1);
    }
    return UnitVector3d(n);
}

UnitVector3d UnitVector3d::northFrom(Vector3d const & v) {
    Vector3d n(-v.x() * v.z(),
               -v.y() * v.z(),
               v.x() * v.x() + v.y() * v.y());
    if (n.isZero()) {
        UnitVector3d u;
        u._v = Vector3d(-::copysign(1.0, v.z()), 0.0, 0.0);
        return u;
    }
    return UnitVector3d(n);
}

UnitVector3d::UnitVector3d(Angle lon, Angle lat) {
    double sinLon = sin(lon);
    double cosLon = cos(lon);
    double sinLat = sin(lat);
    double cosLat = cos(lat);
    _v = Vector3d(cosLon * cosLat,
                  sinLon * cosLat,
                  sinLat);
}

std::ostream & operator<<(std::ostream & os, UnitVector3d const & v) {
    return os << static_cast<Vector3d const &>(v);
}

}} // namespace lsst::sphgeom
