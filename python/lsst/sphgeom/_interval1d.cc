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

#include "lsst/sphgeom/Interval1d.h"
#include "lsst/sphgeom/python/interval.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<Interval1d, std::shared_ptr<Interval1d>> &cls) {
    python::defineInterval<decltype(cls), Interval1d, double>(cls);

    cls.def_static("empty", &Interval1d::empty);
    cls.def_static("full", &Interval1d::full);

    cls.def(py::init<>());
    cls.def(py::init<double>(), "x"_a);
    cls.def(py::init<double, double>(), "x"_a, "y"_a);
    cls.def(py::init<Interval1d const &>(), "interval"_a);

    cls.def("isFull", &Interval1d::isFull);

    cls.def("__str__", [](Interval1d const &self) {
        return py::str("[{!s}, {!s}]").format(self.getA(), self.getB());
    });
    cls.def("__repr__", [](Interval1d const &self) {
        return py::str("Interval1d({!r}, {!r})")
                .format(self.getA(), self.getB());
    });
}

}  // sphgeom
}  // lsst
