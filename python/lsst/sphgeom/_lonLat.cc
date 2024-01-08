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
#include "lsst/sphgeom/Vector3d.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<LonLat, std::shared_ptr<LonLat>> &cls) {
    cls.def_static("fromDegrees", &LonLat::fromDegrees);
    cls.def_static("fromRadians", &LonLat::fromRadians);
    cls.def_static("latitudeOf", &LonLat::latitudeOf);
    cls.def_static("longitudeOf", &LonLat::longitudeOf);

    cls.def(py::init<>());
    cls.def(py::init<LonLat const &>());
    cls.def(py::init<NormalizedAngle, Angle>(), "lon"_a, "lat"_a);
    cls.def(py::init<Vector3d const &>(), "vector"_a);

    cls.def("__eq__", &LonLat::operator==, py::is_operator());
    cls.def("__nq__", &LonLat::operator!=, py::is_operator());

    cls.def("getLon", &LonLat::getLon);
    cls.def("getLat", &LonLat::getLat);

    cls.def("__len__", [](LonLat const &self) { return py::int_(2); });
    cls.def("__getitem__", [](LonLat const &self, py::object key) {
        auto t = py::make_tuple(self.getLon(), self.getLat());
        return t.attr("__getitem__")(key);
    });
    cls.def("__iter__", [](LonLat const &self) {
        auto t = py::make_tuple(self.getLon(), self.getLat());
        return t.attr("__iter__")();
    });

    cls.def("__str__", [](LonLat const &self) {
        return py::str("[{!s}, {!s}]")
                .format(self.getLon().asRadians(), self.getLat().asRadians());
    });
    cls.def("__repr__", [](LonLat const &self) {
        return py::str("LonLat.fromRadians({!r}, {!r})")
                .format(self.getLon().asRadians(), self.getLat().asRadians());
    });
    cls.def("__reduce__", [cls](LonLat const &self) {
        return py::make_tuple(cls,
                              py::make_tuple(self.getLon(), self.getLat()));
    });
}

}  // sphgeom
}  // lsst
