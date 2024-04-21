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
#include <nanobind/nanobind.h>
#include <nanobind/stl/tuple.h>


#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/Region.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"
#include "lsst/sphgeom/python/utils.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(nb::class_<Circle, Region> &cls) {
    cls.attr("TYPE_CODE") = nb::int_(Circle::TYPE_CODE);

    cls.def_static("empty", &Circle::empty);
    cls.def_static("full", &Circle::full);
    cls.def_static("squaredChordLengthFor", &Circle::squaredChordLengthFor,
                   nb::arg("openingAngle"));
    cls.def_static("openingAngleFor", &Circle::openingAngleFor,
                   nb::arg("squaredChordLength"));

    cls.def(nb::init<>());
    cls.def(nb::init<UnitVector3d const &>(), nb::arg("center"));
    cls.def(nb::init<UnitVector3d const &, Angle>(), nb::arg("center"), nb::arg("angle"));
    cls.def(nb::init<UnitVector3d const &, double>(), nb::arg("center"),
            nb::arg("squaredChordLength"));
    cls.def(nb::init<Circle const &>(), nb::arg("circle"));

    cls.def("__eq__", &Circle::operator==, nb::is_operator());
    cls.def("__ne__", &Circle::operator!=, nb::is_operator());
    cls.def("__contains__",
            (bool (Circle::*)(Circle const &) const) & Circle::contains,
            nb::is_operator());
    // Rewrap this base class method since there are overloads in this subclass
    cls.def("__contains__",
            (bool (Circle::*)(UnitVector3d const &) const) & Circle::contains,
            nb::is_operator());

    cls.def("isEmpty", &Circle::isEmpty);
    cls.def("isFull", &Circle::isFull);
    cls.def("getCenter", &Circle::getCenter);
    cls.def("getSquaredChordLength", &Circle::getSquaredChordLength);
    cls.def("getOpeningAngle", &Circle::getOpeningAngle);
    cls.def("contains",
            (bool (Circle::*)(Circle const &) const) & Circle::contains);
    // Rewrap these base class methods since there are overloads in this subclass
    cls.def("contains",
            (bool (Circle::*)(UnitVector3d const &) const) & Circle::contains);
    cls.def("contains", nb::vectorize((bool (Circle::*)(double, double, double) const)&Circle::contains),
            nb::arg("x"), nb::arg("y"), nb::arg("z"));
    cls.def("contains", nb::vectorize((bool (Circle::*)(double, double) const)&Circle::contains),
            nb::arg("lon"), nb::arg("lat"));

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
            (Circle & (Circle::*)(UnitVector3d const &)) & Circle::expandTo, nb::rv_policy::reference);
    cls.def("expandTo",
            (Circle & (Circle::*)(Circle const &)) & Circle::expandTo, nb::rv_policy::reference);
    cls.def("expandedTo",
            (Circle(Circle::*)(UnitVector3d const &) const) &
                    Circle::expandedTo);
    cls.def("expandedTo",
            (Circle(Circle::*)(Circle const &) const) & Circle::expandedTo);
    cls.def("dilateBy", &Circle::dilateBy, nb::arg("radius"), nb::rv_policy::reference);
    cls.def("dilatedBy", &Circle::dilatedBy, nb::arg("radius"));
    cls.def("erodeBy", &Circle::erodeBy, nb::arg("radius"), nb::rv_policy::reference);
    cls.def("erodedBy", &Circle::erodedBy, nb::arg("radius"));
    cls.def("getArea", &Circle::getArea);
    cls.def("complement", &Circle::complement, nb::rv_policy::reference);
    cls.def("complemented", &Circle::complemented);

    // Note that the Region interface has already been wrapped.

    cls.def("__str__", [](Circle const &self) {
        return nb::str("Circle({!s}, {!s})")
                .format(self.getCenter(), self.getOpeningAngle());
    });
    cls.def("__repr__", [](Circle const &self) {
        return nb::str("Circle({!r}, {!r})")
                .format(self.getCenter(), self.getOpeningAngle());
    });
    cls.def("__getstate__", &python::encode);
    cls.def("__setstate__", &python::decode<Circle, nb::bytes>);
}

}  // sphgeom
}  // lsst
