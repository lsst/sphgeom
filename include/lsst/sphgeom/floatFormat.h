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

#ifndef LSST_SPHGEOM_FLOATFORMAT_H_
#define LSST_SPHGEOM_FLOATFORMAT_H_

/// \file
/// \brief This file declares a routine for formatting doubles using the
///        shortest decimal string that round-trips.

#include <cstddef>
#include <string>

#if defined(__APPLE__)
#include <Availability.h>
#endif

/// `SPHGEOM_HAVE_FP_TO_CHARS` is 1 when the floating point `std::to_chars`
/// overloads can be used, and 0 otherwise.
///
/// libc++ implements those overloads in the dylib rather than in the header,
/// and annotates them as introduced in macOS 13.3, so they cannot be called
/// when the deployment target is older than that.  Python extensions inherit
/// their deployment target from the interpreter they are built against, which
/// for the PyPI wheel builds is much older than 13.3.  130300 is the value
/// `__MAC_OS_X_VERSION_MIN_REQUIRED` takes for macOS 13.3; it is spelled out
/// because the `__MAC_13_3` constant is missing from older SDKs.
#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED < 130300
#define SPHGEOM_HAVE_FP_TO_CHARS 0
#else
#define SPHGEOM_HAVE_FP_TO_CHARS 1
#endif

namespace lsst {
namespace sphgeom {

/// The maximum number of characters `appendShortestDouble` can append.
constexpr std::size_t MAX_SHORTEST_DOUBLE_CHARS = 24;

namespace detail {

/// `appendShortestDoubleFallback` is the implementation of
/// `appendShortestDouble` used where `SPHGEOM_HAVE_FP_TO_CHARS` is 0.
///
/// It is compiled unconditionally so that the unit tests exercise it, and
/// check it against `std::to_chars`, on every platform.  Both it and the
/// `SPHGEOM_HAVE_FP_TO_CHARS` switch can be deleted once no supported build
/// targets a macOS older than 13.3.
void appendShortestDoubleFallback(std::string & out, double value);

} // namespace detail

/// `appendShortestDouble` appends `value` to `out` using the shortest
/// decimal string that `std::strtod` converts back to `value` exactly.
///
/// The significand carries the fewest digits that round-trip, and the value
/// is written in whichever of fixed and scientific notation is shorter, with
/// ties resolved in favour of fixed notation.  Non-finite values are written
/// as `nan`, `inf` or `-inf`.  At most `MAX_SHORTEST_DOUBLE_CHARS` characters
/// are appended.
void appendShortestDouble(std::string & out, double value);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_FLOATFORMAT_H_
