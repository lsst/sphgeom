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

#include "lsst/sphgeom/Vector3d.h"

#include <cstdio>
#include <ostream>

#include "lsst/sphgeom/Angle.h"
#include "lsst/sphgeom/UnitVector3d.h"


namespace lsst {
namespace sphgeom {

double Vector3d::normalize() {
    double maxabs;
    double d;
    double ax = std::fabs(_v[0]);
    double ay = std::fabs(_v[1]);
    double az = std::fabs(_v[2]);
    if (ax + ay + az == 0.0) {
        throw std::runtime_error("Cannot normalize zero vector");
    }
    if (ax < ay) {
        if (ay < az) {
            maxabs = az;
            _v[0] /= az;
            _v[1] /= az;
            _v[2] = std::copysign(1.0, _v[2]);
            d = _v[0] * _v[0] + _v[1] * _v[1];
        } else {
            maxabs = ay;
            _v[0] /= ay;
            _v[1] = std::copysign(1.0, _v[1]);
            _v[2] /= ay;
            d = _v[0] * _v[0] + _v[2] * _v[2];
        }
    } else {
        if (ax < az) {
            maxabs = az;
            _v[0] /= az;
            _v[1] /= az;
            _v[2] = std::copysign(1.0, _v[2]);
            d = _v[0] * _v[0] + _v[1] * _v[1];
        } else {
            maxabs = ax;
            _v[0] = std::copysign(1.0, _v[0]);
            _v[1] /= ax;
            _v[2] /= ax;
            d = _v[1] * _v[1] + _v[2] * _v[2];
        }
    }
    double norm = std::sqrt(1.0 + d);
    double s = 1.0 / norm;
    _v[0] *= s;
    _v[1] *= s;
    _v[2] *= s;
    return norm * maxabs;
}

Vector3d Vector3d::rotatedAround(UnitVector3d const & k, Angle a) const {
    // Use Rodrigues' rotation formula.
    Vector3d const & v = *this;
    double s = sin(a);
    double c = cos(a);
    return v * c + k.cross(v) * s + k * (k.dot(v) * (1.0 - c));
}

std::ostream & operator<<(std::ostream & os, Vector3d const & v) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "[%.17g, %.17g, %.17g]",
                  v.x(), v.y(), v.z());
    return os << buf;
}

}} // namespace lsst::sphgeom
