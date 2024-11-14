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
#include "pybind11/numpy.h"

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/Region.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<Circle, std::unique_ptr<Circle>, Region> &cls) {
    cls.attr("TYPE_CODE") = py::int_(Circle::TYPE_CODE);

    cls.def_static("empty", &Circle::empty);
    cls.def_static("full", &Circle::full);
    cls.def_static("squaredChordLengthFor", &Circle::squaredChordLengthFor,
                   "openingAngle"_a);
    cls.def_static("openingAngleFor", &Circle::openingAngleFor,
                   "squaredChordLength"_a);

    cls.def(py::init<>());
    cls.def(py::init<UnitVector3d const &>(), "center"_a);
    cls.def(py::init<UnitVector3d const &, Angle>(), "center"_a, "angle"_a);
    cls.def(py::init<UnitVector3d const &, double>(), "center"_a,
            "squaredChordLength"_a);
    cls.def(py::init<Circle const &>(), "circle"_a);

    cls.def("__eq__", &Circle::operator==, py::is_operator());
    cls.def("__ne__", &Circle::operator!=, py::is_operator());
    cls.def("__contains__",
            (bool (Circle::*)(Circle const &) const) & Circle::contains,
            py::is_operator());
    // Rewrap this base class method since there are overloads in this subclass
    cls.def("__contains__",
            (bool (Circle::*)(UnitVector3d const &) const) & Circle::contains,
            py::is_operator());

    cls.def("isFull", &Circle::isFull);
    cls.def("getCenter", &Circle::getCenter);
    cls.def("getSquaredChordLength", &Circle::getSquaredChordLength);
    cls.def("getOpeningAngle", &Circle::getOpeningAngle);
    cls.def("contains",
            (bool (Circle::*)(Circle const &) const) & Circle::contains);
    // Rewrap these base class methods since there are overloads in this subclass
    cls.def("contains",
            (bool (Circle::*)(UnitVector3d const &) const) & Circle::contains);
    cls.def("contains", py::vectorize((bool (Circle::*)(double, double, double) const)&Circle::contains),
            "x"_a, "y"_a, "z"_a);
    cls.def("contains", py::vectorize((bool (Circle::*)(double, double) const)&Circle::contains),
            "lon"_a, "lat"_a);

    cls.def("isDisjointFrom",
            (bool (Circle::*)(UnitVector3d const &) const) &
                    Circle::isDisjointFrom);
    cls.def("isDisjointFrom",
            (bool (Circle::*)(Circle const &) const) & Circle::isDisjointFrom);
    cls.def("intersects",
            (bool (Circle::*)(UnitVector3d const &) const) &
                    Circle::intersects);
    cls.def("intersects",
            (bool (Circle::*)(Circle const &) const) & Circle::intersects);
    cls.def("isWithin",
            (bool (Circle::*)(UnitVector3d const &) const) & Circle::isWithin);
    cls.def("isWithin",
            (bool (Circle::*)(Circle const &) const) & Circle::isWithin);
    cls.def("clipTo",
            (Circle & (Circle::*)(UnitVector3d const &)) & Circle::clipTo);
    cls.def("clipTo", (Circle & (Circle::*)(Circle const &)) & Circle::clipTo);
    cls.def("clippedTo",
            (Circle(Circle::*)(UnitVector3d const &) const) &
                    Circle::clippedTo);
    cls.def("clippedTo",
            (Circle(Circle::*)(Circle const &) const) & Circle::clippedTo);
    cls.def("expandTo",
            (Circle & (Circle::*)(UnitVector3d const &)) & Circle::expandTo);
    cls.def("expandTo",
            (Circle & (Circle::*)(Circle const &)) & Circle::expandTo);
    cls.def("expandedTo",
            (Circle(Circle::*)(UnitVector3d const &) const) &
                    Circle::expandedTo);
    cls.def("expandedTo",
            (Circle(Circle::*)(Circle const &) const) & Circle::expandedTo);
    cls.def("dilateBy", &Circle::dilateBy, "radius"_a);
    cls.def("dilatedBy", &Circle::dilatedBy, "radius"_a);
    cls.def("erodeBy", &Circle::erodeBy, "radius"_a);
    cls.def("erodedBy", &Circle::erodedBy, "radius"_a);
    cls.def("getArea", &Circle::getArea);
    cls.def("complement", &Circle::complement);
    cls.def("complemented", &Circle::complemented);

    // Note that the Region interface has already been wrapped.

    cls.def("__str__", [](Circle const &self) {
        return py::str("Circle({!s}, {!s})")
                .format(self.getCenter(), self.getOpeningAngle());
    });
    cls.def("__repr__", [](Circle const &self) {
        return py::str("Circle({!r}, {!r})")
                .format(self.getCenter(), self.getOpeningAngle());
    });
    cls.def(py::pickle(&python::encode, &python::decode<Circle>));
}

}  // sphgeom
}  // lsst
