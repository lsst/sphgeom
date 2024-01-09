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

#ifndef LSST_SPHGEOM_LONLAT_H_
#define LSST_SPHGEOM_LONLAT_H_

/// \file
/// \brief This file contains a class representing spherical coordinates.

#include <iosfwd>
#include <stdexcept>

#include "NormalizedAngle.h"


namespace lsst{
namespace sphgeom {

class Vector3d;

/// `LonLat` represents a spherical coordinate (longitude/latitude angle) pair.
///
/// This class supports conversion from a `Vector3d`. All methods that accept a
/// `Vector3d` parameter shall convert from vector to longitude/latitude
/// coordinates according to the following conventions:
/// * (1, 0, 0) → (0°, 0°)
/// * (0, 1, 0) → (90°, 0°)
/// * (0, 0, 1) → (0°, +90°)
class LonLat {
public:
    static LonLat fromDegrees(double lon, double lat) {
        return LonLat(NormalizedAngle::fromDegrees(lon),
                      Angle::fromDegrees(lat));
    }

    static LonLat fromRadians(double lon, double lat) {
        return LonLat(NormalizedAngle::fromRadians(lon),
                      Angle::fromRadians(lat));
    }

    /// `latitudeOf` returns the latitude of the point on the unit sphere
    /// corresponding to the direction of v.
    static Angle latitudeOf(Vector3d const & v);

    /// `longitudeOf` returns the longitude of the point on the unit sphere
    /// corresponding to the direction of v.
    static NormalizedAngle longitudeOf(Vector3d const & v);

    /// This constructor creates the point with longitude and latitude angle
    /// equal to zero.
    LonLat() {}

    /// This constructor creates the point with the given longitude and
    /// latitude angles.
    LonLat(NormalizedAngle lon, Angle lat);

    /// This constructor creates the point on the unit sphere corresponding
    /// to the direction of v.
    LonLat(Vector3d const & v);

    bool operator==(LonLat const & p) const {
        return _lon == p._lon && _lat == p._lat;
    }

    bool operator!=(LonLat const & p) const {
        return _lon != p._lon || _lat != p._lat;
    }

    NormalizedAngle getLon() const { return _lon; }

    Angle getLat() const { return _lat; }

private:
    void _enforceInvariants();

    NormalizedAngle _lon;
    Angle _lat;
};

std::ostream & operator<<(std::ostream &, LonLat const &);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_LONLAT_H_
