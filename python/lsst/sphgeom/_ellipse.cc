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

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/Matrix3d.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<Ellipse, std::unique_ptr<Ellipse>, Region> &cls) {
    cls.attr("TYPE_CODE") = py::int_(Ellipse::TYPE_CODE);

    cls.def_static("empty", &Ellipse::empty);
    cls.def_static("full", &Ellipse::full);

    cls.def(py::init<>());
    cls.def(py::init<Circle const &>(), "circle"_a);
    cls.def(py::init<UnitVector3d const &, Angle>(), "center"_a,
            "angle"_a = Angle(0.0));
    cls.def(py::init<UnitVector3d const &, UnitVector3d const &, Angle>(),
            "focus1"_a, "focus2"_a, "alpha"_a);
    cls.def(py::init<UnitVector3d const &, Angle, Angle, Angle>(), "center"_a,
            "alpha"_a, "beta"_a, "orientation"_a);
    cls.def(py::init<Ellipse const &>(), "ellipse"_a);

    cls.def("__eq__", &Ellipse::operator==, py::is_operator());
    cls.def("__ne__", &Ellipse::operator!=, py::is_operator());

    cls.def("isFull", &Ellipse::isFull);
    cls.def("isGreatCircle", &Ellipse::isGreatCircle);
    cls.def("isCircle", &Ellipse::isCircle);
    cls.def("getTransformMatrix", &Ellipse::getTransformMatrix);
    cls.def("getCenter", &Ellipse::getCenter);
    cls.def("getF1", &Ellipse::getF1);
    cls.def("getF2", &Ellipse::getF2);
    cls.def("getAlpha", &Ellipse::getAlpha);
    cls.def("getBeta", &Ellipse::getBeta);
    cls.def("getGamma", &Ellipse::getGamma);
    cls.def("complement", &Ellipse::complement);
    cls.def("complemented", &Ellipse::complemented);

    // Note that the Region interface has already been wrapped.

    cls.def("__str__", [](Ellipse const &self) {
        return py::str("Ellipse({!s}, {!s}, {!s})")
                .format(self.getF1(), self.getF2(), self.getAlpha());
    });
    cls.def("__repr__", [](Ellipse const &self) {
        return py::str("Ellipse({!r}, {!r}, {!r})")
                .format(self.getF1(), self.getF2(), self.getAlpha());
    });
    cls.def(py::pickle(&python::encode, &python::decode<Ellipse>));
}

}  // sphgeom
}  // lsst
