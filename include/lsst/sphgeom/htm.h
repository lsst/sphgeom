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

#ifndef LSST_SPHGEOM_HTM_H_
#define LSST_SPHGEOM_HTM_H_

/// \file
/// \brief This file declares functions related to HTM indexing.
///
/// This software is based on work by A. Szalay, T. Budavari, G. Fekete at
/// The Johns Hopkins University, and Jim Gray, Microsoft Research. See the
/// following for more information:
///
/// - "Indexing the Sphere with the Hierarchical Triangular Mesh"
///   Szalay, Alexander S.; Gray, Jim; Fekete, George; Kunszt, Peter Z.;
///   Kukol, Peter; Thakar, Ani
///   2007, Arxiv e-prints
///
///   http://arxiv.org/abs/cs/0701164
///   http://adsabs.harvard.edu/abs/2007cs........1164S
///
/// - Budavári, Tamás; Szalay, Alexander S.; Fekete, György
///   "Searchable Sky Coverage of Astronomical Observations:
///   Footprints and Exposures"
///   Publications of the Astronomical Society of Pacific,
///   Volume 122, Issue 897, pp. 1375-1388 (2010).
///
///   http://adsabs.harvard.edu/abs/2010PASP..122.1375B
///
/// - http://voservices.net/spherical/

#include <cstdint>

#include "ConvexPolygon.h"


namespace lsst {
namespace sphgeom {

/// The maximum supported HTM subdivision level.
static constexpr int MAX_HTM_LEVEL = 24;

/// `htmLevel` returns the subdivision level of the given HTM index.
///
/// If i is not a valid HTM index, -1 is returned.
int htmLevel(uint64_t i);

/// `htmTrixel` returns the convex spherical polygon corresponding to the
/// triangular pixel (trixel) with the given index.
///
/// This polygon contains all points v with htmIndex(v, level) == i. However,
/// it may also contain points with index not equal to i. This is because
/// points that lie exactly on an edge (or vertex) of a polygon P are
/// considered to be inside P, but points on a shared trixel edge or vertex
/// are assigned to exactly one trixel.
///
/// If i is not a valid HTM index, a std::invalid_argument is thrown.
ConvexPolygon htmTrixel(uint64_t i);

/// `htmIndex` returns the HTM index of v.
///
/// If the subdivision level is not between 0 and MAX_HTM_LEVEL (inclusive),
/// a std::invalid_argument is thrown.
uint64_t htmIndex(UnitVector3d const & v, int level);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_HTM_H_
