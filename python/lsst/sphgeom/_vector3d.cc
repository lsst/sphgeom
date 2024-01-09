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

#include "lsst/sphgeom/Angle.h"
#include "lsst/sphgeom/UnitVector3d.h"
#include "lsst/sphgeom/Vector3d.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<Vector3d, std::shared_ptr<Vector3d>> &cls) {
    cls.def(py::init<>());
    cls.def(py::init<double, double, double>(), "x"_a, "y"_a, "z"_a);
    cls.def(py::init<Vector3d const &>(), "vector"_a);
    // Construct a Vector3d from a UnitVector3d, enabling implicit
    // conversion from UnitVector3d to Vector3d in python via
    // py::implicitly_convertible
    cls.def(py::init([](UnitVector3d const &u) {
        return new Vector3d(u.x(), u.y(), u.z());
    }));

    cls.def("__eq__", &Vector3d::operator==, py::is_operator());
    cls.def("__ne__", &Vector3d::operator!=, py::is_operator());
    cls.def("__neg__", (Vector3d(Vector3d::*)() const) & Vector3d::operator-);
    cls.def("__add__", &Vector3d::operator+, py::is_operator());
    cls.def("__sub__",
            (Vector3d(Vector3d::*)(Vector3d const &) const) &
                    Vector3d::operator-,
            py::is_operator());
    cls.def("__mul__", &Vector3d::operator*, py::is_operator());
    cls.def("__truediv__", &Vector3d::operator/, py::is_operator());

    cls.def("__iadd__", &Vector3d::operator+=);
    cls.def("__isub__", &Vector3d::operator-=);
    cls.def("__imul__", &Vector3d::operator*=);
    cls.def("__itruediv__", &Vector3d::operator/=);

    cls.def("x", &Vector3d::x);
    cls.def("y", &Vector3d::y);
    cls.def("z", &Vector3d::z);
    cls.def("dot", &Vector3d::dot);
    cls.def("getSquaredNorm", &Vector3d::getSquaredNorm);
    cls.def("getNorm", &Vector3d::getNorm);
    cls.def("isZero", &Vector3d::isZero);
    cls.def("normalize", &Vector3d::normalize);
    cls.def("isNormalized", &Vector3d::isNormalized);
    cls.def("cross", &Vector3d::cross);
    cls.def("cwiseProduct", &Vector3d::cwiseProduct);
    cls.def("rotatedAround", &Vector3d::rotatedAround, "axis"_a, "angle"_a);

    cls.def("__len__", [](Vector3d const &self) { return py::int_(3); });
    cls.def("__getitem__", [](Vector3d const &self, py::int_ i) {
        return self(python::convertIndex(3, i));
    });

    cls.def("__str__", [](Vector3d const &self) {
        return py::str("[{!s}, {!s}, {!s}]")
                .format(self.x(), self.y(), self.z());
    });
    cls.def("__repr__", [](Vector3d const &self) {
        return py::str("Vector3d({!r}, {!r}, {!r})")
                .format(self.x(), self.y(), self.z());
    });

    cls.def("__reduce__", [cls](Vector3d const &self) {
        return py::make_tuple(cls,
                              py::make_tuple(self.x(), self.y(), self.z()));
    });
}

}  // sphgeom
}  // lsst
