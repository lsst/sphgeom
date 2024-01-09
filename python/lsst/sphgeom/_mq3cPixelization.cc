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
