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

#ifndef LSST_SPHGEOM_CONSTANTS_H_
#define LSST_SPHGEOM_CONSTANTS_H_

/// \file
/// \brief This file contains common constants.


namespace lsst {
namespace sphgeom {

// Note: given a compiler that does correctly rounded decimal to
// binary floating point conversions, PI = 0x1.921fb54442d18p1 in
// IEEE double precision format. This is less than π.
constexpr double PI = 3.1415926535897932384626433832795;
constexpr double ONE_OVER_PI = 0.318309886183790671537767526745;
constexpr double RAD_PER_DEG = 0.0174532925199432957692369076849;
constexpr double DEG_PER_RAD = 57.2957795130823208767981548141;

// The maximum error of std::asin in IEEE double precision arithmetic,
// assuming 1 ulp of error in its argument, is about
// π/2 - arcsin (1 - 2⁻⁵³), or a little less than 1.5e-8 radians
// (3.1 milliarcsec).
constexpr double MAX_ASIN_ERROR = 1.5e-8;

// The computation of squared chord length between two unit vectors
// involves 8 elementary operations on numbers with magnitude ≤ 4. Its
// maximum error can be shown to be < 2.5e-15.
constexpr double MAX_SQUARED_CHORD_LENGTH_ERROR = 2.5e-15;

// The largest value ε such that 1 + ε rounds to 1 in IEEE double
// precision, assuming round-to-nearest-ties-to-even rounding.
constexpr double EPSILON = 1.1102230246251565e-16;

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_CONSTANTS_H_
