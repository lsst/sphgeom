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

#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/NormalizedAngle.h"
#include "lsst/sphgeom/Vector3d.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<NormalizedAngle> &cls) {
    // Provide the equivalent of the NormalizedAngle to Angle C++ cast
    // operator in Python
    py::implicitly_convertible<NormalizedAngle, Angle>();

    cls.def_static("nan", &NormalizedAngle::nan);
    cls.def_static("fromDegrees", &NormalizedAngle::fromDegrees);
    cls.def_static("fromRadians", &NormalizedAngle::fromRadians);
    cls.def_static("between", &NormalizedAngle::between, "a"_a, "b"_a);
    cls.def_static("center", &NormalizedAngle::center, "a"_a, "b"_a);

    cls.def(py::init<>());
    cls.def(py::init<NormalizedAngle const &>());
    cls.def(py::init<Angle const &>());
    cls.def(py::init<double>(), "radians"_a);
    cls.def(py::init<LonLat const &, LonLat const &>(), "a"_a, "b"_a);
    cls.def(py::init<Vector3d const &, Vector3d const &>(), "a"_a, "b"_a);

    cls.def("__eq__", &NormalizedAngle::operator==, py::is_operator());
    cls.def("__ne__", &NormalizedAngle::operator!=, py::is_operator());
    cls.def("__lt__", &NormalizedAngle::operator<, py::is_operator());
    cls.def("__gt__", &NormalizedAngle::operator>, py::is_operator());
    cls.def("__le__", &NormalizedAngle::operator<=, py::is_operator());
    cls.def("__ge__", &NormalizedAngle::operator>=, py::is_operator());

    cls.def("__neg__",
            (Angle(NormalizedAngle::*)() const) & NormalizedAngle::operator-);
    cls.def("__add__", &NormalizedAngle::operator+, py::is_operator());
    cls.def("__sub__",
            (Angle(NormalizedAngle::*)(Angle const &) const) &
                    NormalizedAngle::operator-,
            py::is_operator());
    cls.def("__mul__", &NormalizedAngle::operator*, py::is_operator());
    cls.def("__rmul__", &NormalizedAngle::operator*, py::is_operator());
    cls.def("__truediv__",
            (Angle(NormalizedAngle::*)(double) const) &
                    NormalizedAngle::operator/,
            py::is_operator());
    cls.def("__truediv__",
            (double (NormalizedAngle::*)(Angle const &) const) &
                    NormalizedAngle::operator/,
            py::is_operator());

    cls.def("asDegrees", &NormalizedAngle::asDegrees);
    cls.def("asRadians", &NormalizedAngle::asRadians);
    cls.def("isNan", &NormalizedAngle::isNan);
    cls.def("getAngleTo", &NormalizedAngle::getAngleTo);

    cls.def("__str__", [](NormalizedAngle const &self) {
        return py::str("{!s}").format(self.asRadians());
    });
    cls.def("__repr__", [](NormalizedAngle const &self) {
        return py::str("NormalizedAngle({!r})").format(self.asRadians());
    });

    cls.def("__reduce__", [cls](NormalizedAngle const &self) {
        return py::make_tuple(cls, py::make_tuple(self.asRadians()));
    });
}

}  // sphgeom
}  // lsst
