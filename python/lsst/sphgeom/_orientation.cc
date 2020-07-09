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
#include "pybind11/pybind11.h"

#include "lsst/sphgeom/orientation.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

void defineOrientation(py::module &mod) {
    mod.def("orientationExact", &orientationExact, "a"_a, "b"_a, "c"_a);
    mod.def("orientation", &orientation, "a"_a, "b"_a, "c"_a);
    mod.def("orientationX", &orientationX, "b"_a, "c"_a);
    mod.def("orientationY", &orientationY, "b"_a, "c"_a);
    mod.def("orientationZ", &orientationZ, "b"_a, "c"_a);
}

}  // sphgeom
}  // lsst
