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

#include <memory>

#include "lsst/sphgeom/Angle.h"
#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/UnitVector3d.h"
#include "lsst/sphgeom/Vector3d.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {
namespace {

PYBIND11_MODULE(unitVector3d, mod) {
    py::module::import("lsst.sphgeom.vector3d");

    py::class_<UnitVector3d, std::shared_ptr<UnitVector3d>> cls(mod,
                                                                "UnitVector3d");
    // Provide the equivalent of the UnitVector3d to Vector3d C++ cast
    // operator in Python
    py::implicitly_convertible<UnitVector3d, Vector3d>();

    cls.def_static(
            "orthogonalTo",
            (UnitVector3d(*)(Vector3d const &)) & UnitVector3d::orthogonalTo,
            "vector"_a);
    cls.def_static("orthogonalTo",
                   (UnitVector3d(*)(Vector3d const &, Vector3d const &)) &
                           UnitVector3d::orthogonalTo,
                   "vector1"_a, "vector2"_a);
    cls.def_static("orthogonalTo",
                   (UnitVector3d(*)(NormalizedAngle const &)) &
                           UnitVector3d::orthogonalTo,
                   "meridian"_a);
    cls.def_static("northFrom", &UnitVector3d::northFrom, "vector"_a);
    cls.def_static("X", &UnitVector3d::X);
    cls.def_static("Y", &UnitVector3d::Y);
    cls.def_static("Z", &UnitVector3d::Z);
    // The fromNormalized static factory functions are not exposed to
    // Python, as they are easy to misuse and intended only for performance
    // critical code (i.e. not Python).

    cls.def(py::init<>());
    cls.def(py::init<UnitVector3d const &>(), "unitVector"_a);
    cls.def(py::init<Vector3d const &>(), "vector"_a);
    cls.def(py::init<double, double, double>(), "x"_a, "y"_a, "z"_a);
    cls.def(py::init<LonLat const &>(), "lonLat"_a);
    cls.def(py::init<Angle, Angle>(), "lon"_a, "lat"_a);

    cls.def("__eq__", &UnitVector3d::operator==, py::is_operator());
    cls.def("__ne__", &UnitVector3d::operator!=, py::is_operator());
    cls.def("__neg__",
            (UnitVector3d(UnitVector3d::*)() const) & UnitVector3d::operator-);
    cls.def("__add__", &UnitVector3d::operator+, py::is_operator());
    cls.def("__sub__",
            (Vector3d(UnitVector3d::*)(Vector3d const &) const) &
                    UnitVector3d::operator-,
            py::is_operator());
    cls.def("__mul__", &UnitVector3d::operator*, py::is_operator());
    cls.def("__truediv__", &UnitVector3d::operator/, py::is_operator());

    cls.def("x", &UnitVector3d::x);
    cls.def("y", &UnitVector3d::y);
    cls.def("z", &UnitVector3d::z);
    cls.def("x", &UnitVector3d::dot);
    cls.def("dot", &UnitVector3d::dot);
    cls.def("cross", &UnitVector3d::cross);
    cls.def("robustCross", &UnitVector3d::robustCross);
    cls.def("cwiseProduct", &UnitVector3d::cwiseProduct);
    cls.def("rotatedAround", &UnitVector3d::rotatedAround, "axis"_a, "angle"_a);

    cls.def("__len__", [](UnitVector3d const &self) { return py::int_(3); });
    cls.def("__getitem__", [](UnitVector3d const &self, py::int_ i) {
        return self(python::convertIndex(3, i));
    });

    cls.def("__str__", [](UnitVector3d const &self) {
        return py::str("[{!s}, {!s}, {!s}]")
                .format(self.x(), self.y(), self.z());
    });
    cls.def("__repr__", [](UnitVector3d const &self) {
        return py::str("UnitVector3d({!r}, {!r}, {!r})")
                .format(self.x(), self.y(), self.z());
    });

    // Do not implement __reduce__ for pickling. Why? Given:
    //
    //    u = UnitVector3d(x, y, z)
    //    v = UnitVector3d(u.x(), u.y(), u.z())
    //
    // u may not be identical to v, since the constructor normalizes its input
    // components. Furthermore, UnitVector3d::fromNormalized is not visible to
    // Python, and even if it were, pybind11 is currently incapable of returning
    // a picklable reference to it.
    cls.def("__getstate__", [](UnitVector3d const &self) {
        return py::make_tuple(self.x(), self.y(), self.z());
    });
    cls.def("__setstate__", [](UnitVector3d &self, py::tuple t) {
        if (t.size() != 3) {
            throw std::runtime_error("Invalid state!");
        }
        new (&self) UnitVector3d(UnitVector3d::fromNormalized(
                t[0].cast<double>(), t[1].cast<double>(), t[2].cast<double>()));
    });
}

}  // <anonymous>
}  // sphgeom
}  // lsst
