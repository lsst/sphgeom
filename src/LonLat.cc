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
/// \brief This file contains the LonLat class implementation.

#include "lsst/sphgeom/LonLat.h"

#include <cmath>
#include <ostream>

#include "lsst/sphgeom/Vector3d.h"


namespace lsst {
namespace sphgeom {

Angle LonLat::latitudeOf(Vector3d const & v) {
    double d2 = v(0) * v(0) + v(1) * v(1);
    double lat = 0.0;
    if (v(2) != 0.0) {
        lat = std::atan2(v(2), sqrt(d2));
        if (std::fabs(lat) > 0.5 * PI) {
            lat = ::copysign(0.5 * PI, lat);
        }
    }
    return Angle(lat);
}

NormalizedAngle LonLat::longitudeOf(Vector3d const & v) {
    double d2 = v(0) * v(0) + v(1) * v(1);
    double lon = 0.0;
    if (d2 != 0.0) {
        lon = std::atan2(v(1), v(0));
        if (lon < 0.0) {
            lon += 2*PI;
        }
    }
    return NormalizedAngle(lon);
}


LonLat::LonLat(NormalizedAngle lon, Angle lat) : _lon(lon), _lat(lat) {
    if (std::fabs(_lat.asRadians()) > 0.5 * PI) {
        throw std::invalid_argument("invalid latitude angle");
    }
    _enforceInvariants();
}

LonLat::LonLat(Vector3d const & v) : _lon(longitudeOf(v)), _lat(latitudeOf(v)) {
    _enforceInvariants();
}

void LonLat::_enforceInvariants() {
    // Make sure that if one coordinate is NaN, the other is as well.
    if (_lon.isNan()) {
        _lat = Angle::nan();
    } else if (_lat.isNan()) {
        _lon = NormalizedAngle::nan();
    }
}

std::ostream & operator<<(std::ostream & os, LonLat const & p) {
    return os << '[' << p.getLon() << ", " << p.getLat() << ']';
}

}} // namespace lsst::sphgeom
