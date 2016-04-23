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

#ifndef LSST_SPHGEOM_Q3C_H_
#define LSST_SPHGEOM_Q3C_H_

/// \file
/// \brief This file declares functions related to Q3C indexing.
///
/// Q3C is described by the following paper:
///
///     Koposov, S., Bartunov, O., Jul. 2006. Q3C, Quad Tree Cube –
///     The new sky-indexing concept for huge astronomical catalogues and
///     its realization for main astronomical queries (cone search and xmatch)
///     in Open Source Database PostgreSQL. In: Gabriel, C., Arviset, C.,
///     Ponz, D., Enrique, S. (Eds.), Astronomical Data Analysis Software
///     and Systems XV. Vol. 351 of Astronomical Society of the Pacific
///     Conference Series. p. 735.
///
/// available online at http://adsabs.harvard.edu/abs/2006ASPC..351..735K .
/// The authors provide an implementation at https://github.com/segasai/q3c.

#include <cstdint>


namespace lsst {
namespace sphgeom {

// Forward declarations
class UnitVector3d;

/// The maximum supported Q3C cube-face grid resolution is 2^30.
static constexpr uint32_t MAX_Q3C_RESOLUTION = 0x40000000;

/// Return the Q3C index of the unit vector v.
///
/// To find this index, v is first projected out to the cube
/// [-1,1]x[-1,1]x[-1,1] to obtain a face number (0-5) and coordinates
/// on that face, u and v, which both lie in [-1,1].
///
/// Each face of the cube is divided into a uniform n by n grid, where n need
/// not be a power of 2. Each grid cell is labeled according to either the
/// Morton curve or, if useHilbertCurve is true, the Hilbert curve over an
/// m by m grid, where m is the smallest power of 2 greater than or equal to n.
///
/// If the grid resolution n is not between 1 and MAX_Q3C_RESOLUTION
/// (inclusive), a std::invalid_argument is thrown. Grid coordinates in
/// [0, n) are obtained via a simple linear transform of the face coordinates,
/// unless reduceAreaVariations is true. In that case, face coordinates undergo
/// an additional quadratic transformation before being mapped to grid
/// coordinates. This reduces cell area variations from a factor of ~5.2
/// to a factor of ~1.55 at minimal computational cost.
///
/// Finally, the index of v is composed from the face number and the Morton or
/// Hilbert index of its grid coordinates. To obtain indexes compatible with the
/// Q3C PostgreSQL extension, this function should be called with
/// n = MAX_Q3C_RESOLUTION, useHilbertCurve = false, and
/// reduceAreaVariation = false.
uint64_t q3cIndex(UnitVector3d const & v,
	              uint32_t n,
	              bool useHilbertCurve,
	              bool reduceAreaVariation);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_Q3C_H_
