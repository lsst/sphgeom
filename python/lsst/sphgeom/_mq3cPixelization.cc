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

#include "lsst/sphgeom/Mq3cPixelization.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<Mq3cPixelization, Pixelization> &cls) {
    cls.attr("MAX_LEVEL") = py::int_(Mq3cPixelization::MAX_LEVEL);

    cls.def_static("level", &Mq3cPixelization::level);
    cls.def_static("quad", &Mq3cPixelization::quad);
    cls.def_static("neighborhood", &Mq3cPixelization::neighborhood);
    cls.def_static("asString", &Mq3cPixelization::asString);

    cls.def(py::init<int>(), "level"_a);
    cls.def(py::init<Mq3cPixelization const &>(), "mq3cPixelization"_a);

    cls.def("getLevel", &Mq3cPixelization::getLevel);

    cls.def("__eq__",
            [](Mq3cPixelization const &self, Mq3cPixelization const &other) {
                return self.getLevel() == other.getLevel();
            });
    cls.def("__ne__",
            [](Mq3cPixelization const &self, Mq3cPixelization const &other) {
                return self.getLevel() != other.getLevel();
            });
    cls.def("__repr__", [](Mq3cPixelization const &self) {
        return py::str("Mq3cPixelization({!s})").format(self.getLevel());
    });
    cls.def("__reduce__", [cls](Mq3cPixelization const &self) {
        return py::make_tuple(cls, py::make_tuple(self.getLevel()));
    });
}

}  // sphgeom
}  // lsst
