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

#ifndef LSST_SPHGEOM_ORIENTATION_H_
#define LSST_SPHGEOM_ORIENTATION_H_

/// \file
/// \brief This file declares functions for orienting points on the sphere.

#include "UnitVector3d.h"


namespace lsst {
namespace sphgeom {

/// `orientationExact` computes and returns the orientations of 3 vectors a, b
/// and c, which need not be normalized but are assumed to have finite
/// components. The return value is +1 if the vectors a, b, and c are in
/// counter-clockwise orientation, 0 if they are coplanar, colinear, or
/// identical, and -1 if they are in clockwise orientation. The implementation
/// uses arbitrary precision arithmetic to avoid floating point rounding error,
/// underflow and overflow.
int orientationExact(Vector3d const & a,
                     Vector3d const & b,
                     Vector3d const & c);

/// `orientation` computes and returns the orientations of 3 unit vectors
/// a, b and c. The return value is +1 if the vectors are in counter-clockwise
/// orientation, 0 if they are coplanar, colinear or identical, and -1 if
/// they are in clockwise orientation.
///
/// This is equivalent to computing the sign of the scalar triple product
/// a · (b x c), which is the sign of the determinant of the 3x3 matrix with
/// a, b and c as columns/rows.
///
/// The implementation proceeds by first computing a double precision
/// approximation, and then falling back to arbitrary precision arithmetic
/// when necessary. Consequently, the result is exact.
int orientation(UnitVector3d const & a,
                UnitVector3d const & b,
                UnitVector3d const & c);

/// `orientationX(b, c)` is equivalent to `orientation(UnitVector3d::X(), b, c)`.
int orientationX(UnitVector3d const & b, UnitVector3d const & c);

/// `orientationY(b, c)` is equivalent to `orientation(UnitVector3d::Y(), b, c)`.
int orientationY(UnitVector3d const & b, UnitVector3d const & c);

/// `orientationZ(b, c)` is equivalent to `orientation(UnitVector3d::Z(), b, c)`.
int orientationZ(UnitVector3d const & b, UnitVector3d const & c);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_ORIENTATION_H_
