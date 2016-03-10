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
/// \brief This file contains the Matrix3d class implementation.

#include "lsst/sphgeom/Matrix3d.h"

#include <cstdio>
#include <ostream>


namespace lsst {
namespace sphgeom {

std::ostream & operator<<(std::ostream & os, Matrix3d const & m) {
    return os << '[' << m.getRow(0) << ", " << m.getRow(1) << ", " << m.getRow(2) << ']';
}

}} // namespace lsst::sphgeom
