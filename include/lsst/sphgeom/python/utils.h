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

#ifndef LSST_SPHGEOM_PYTHON_UTILS_H_
#define LSST_SPHGEOM_PYTHON_UTILS_H_

#include "pybind11/pybind11.h"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <cstdint>

#include "lsst/sphgeom/Region.h"

namespace lsst {
namespace sphgeom {
namespace python {

/// Convert a Python index `i` over a sequence with length `len` to a
/// non-negative (C++ style) index, and perform a bounds-check.
inline ptrdiff_t convertIndex(ptrdiff_t len, pybind11::int_ i) {
    auto j = static_cast<ptrdiff_t>(i);
    if ((j == -1 && PyErr_Occurred()) || j < -len || j >= len) {
        PyErr_Clear();
        throw pybind11::index_error(
                pybind11::str("Index {} not in range({}, {})")
                        .format(i, -len, len));
    }
    return (j < 0) ? j + len : j;
}


/// Encode a Region as a pybind11 bytes object
inline pybind11::bytes encode(Region const &self) {
    std::vector<std::uint8_t> bytes = self.encode();
    return pybind11::bytes(reinterpret_cast<char const *>(bytes.data()),
                     bytes.size());
}

/// Decode a Region from a pybind11 bytes object.
template <typename R>
std::unique_ptr<R> decode(pybind11::bytes bytes) {
    std::uint8_t const *buffer = reinterpret_cast<std::uint8_t const *>(
            PYBIND11_BYTES_AS_STRING(bytes.ptr()));
    size_t n = static_cast<size_t>(PYBIND11_BYTES_SIZE(bytes.ptr()));
    return R::decode(buffer, n);
}

/// Create a vector of Region (or Region-subclass) pointers by copying the
/// regions from a sized Python iterable (e.g. S == py::tuple).
///
/// Note that the pybind11 built-in STL conversions don't work, because they
/// use unique_ptr - we can't transfer ownership out of Python, and those
/// converters don't know about our clone methods.
template <typename S>
inline std::vector<std::unique_ptr<Region>> convert_region_sequence(S const & seq) {
    std::vector<std::unique_ptr<Region>> result;
    result.reserve(seq.size());
    for (pybind11::handle py_region : seq) {
        result.push_back(py_region.cast<Region const &>().clone());
    }
    return result;
}

}  // python
}  // sphgeom
}  // lsst

#endif  // LSST_SPHGEOM_PYTHON_UTILS_H_
