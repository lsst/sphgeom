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

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Q3cPixelization.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<Q3cPixelization, Pixelization> &cls) {
    cls.attr("MAX_LEVEL") = py::int_(Q3cPixelization::MAX_LEVEL);

    cls.def(py::init<int>(), "level"_a);
    cls.def(py::init<Q3cPixelization const &>(), "q3cPixelization"_a);

    cls.def("getLevel", &Q3cPixelization::getLevel);
    cls.def("quad", &Q3cPixelization::quad);
    cls.def("neighborhood", &Q3cPixelization::neighborhood);

    cls.def("__eq__",
            [](Q3cPixelization const &self, Q3cPixelization const &other) {
                return self.getLevel() == other.getLevel();
            });
    cls.def("__ne__",
            [](Q3cPixelization const &self, Q3cPixelization const &other) {
                return self.getLevel() != other.getLevel();
            });
    cls.def("__repr__", [](Q3cPixelization const &self) {
        return py::str("Q3cPixelization({!s})").format(self.getLevel());
    });
    cls.def("__reduce__", [cls](Q3cPixelization const &self) {
        return py::make_tuple(cls, py::make_tuple(self.getLevel()));
    });
}

}  // sphgeom
}  // lsst
