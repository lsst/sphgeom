/*
 * LSST Data Management System
 * See COPYRIGHT file at the top of the source tree.
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

#ifndef LSST_SPHGEOM_PYTHON_UTILS_H_
#define LSST_SPHGEOM_PYTHON_UTILS_H_

#include "pybind11/pybind11.h"

#include <limits>
#include <sstream>
#include <stdexcept>

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
    std::vector<uint8_t> bytes = self.encode();
    return pybind11::bytes(reinterpret_cast<char const *>(bytes.data()),
                     bytes.size());
}

}  // python
}  // sphgeom
}  // lsst

#endif  // LSST_SPHGEOM_PYTHON_UTILS_H_
