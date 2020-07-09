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

#include "lsst/sphgeom/AngleInterval.h"
#include "lsst/sphgeom/python/interval.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(
        py::class_<AngleInterval, std::shared_ptr<AngleInterval>> &cls) {
    python::defineInterval<decltype(cls), AngleInterval, Angle>(cls);

    cls.def_static("fromDegrees", &AngleInterval::fromDegrees, "x"_a, "y"_a);
    cls.def_static("fromRadians", &AngleInterval::fromRadians, "x"_a, "y"_a);
    cls.def_static("empty", &AngleInterval::empty);
    cls.def_static("full", &AngleInterval::full);

    cls.def(py::init<>());
    cls.def(py::init<Angle>(), "x"_a);
    cls.def(py::init<Angle, Angle>(), "x"_a, "y"_a);
    cls.def(py::init<AngleInterval const &>(), "interval"_a);

    cls.def("__str__", [](AngleInterval const &self) {
        return py::str("[{!s}, {!s}]")
                .format(self.getA().asRadians(), self.getB().asRadians());
    });
    cls.def("__repr__", [](AngleInterval const &self) {
        return py::str("AngleInterval.fromRadians({!r}, {!r})")
                .format(self.getA().asRadians(), self.getB().asRadians());
    });
}

}  // sphgeom
}  // lsst
