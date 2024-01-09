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
