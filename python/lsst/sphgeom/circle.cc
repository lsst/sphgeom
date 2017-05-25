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

#include "sphgeom.h"

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/Region.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

namespace {
std::unique_ptr<Circle> decode(py::bytes bytes) {
    uint8_t const *buffer = reinterpret_cast<uint8_t const *>(
            PYBIND11_BYTES_AS_STRING(bytes.ptr()));
    size_t n = static_cast<size_t>(PYBIND11_BYTES_SIZE(bytes.ptr()));
    return Circle::decode(buffer, n);
}
}

template <>
void defineClass(py::class_<Circle, std::shared_ptr<Circle>, Region> &cls) {
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

    cls.def("isEmpty", &Circle::isEmpty);
    cls.def("isFull", &Circle::isFull);
    cls.def("getCenter", &Circle::getCenter);
    cls.def("getSquaredChordLength", &Circle::getSquaredChordLength);
    cls.def("getOpeningAngle", &Circle::getOpeningAngle);
    cls.def("contains",
            (bool (Circle::*)(Circle const &) const) & Circle::contains);
    // Rewrap this base class method since there are overloads in this subclass
    cls.def("contains",
            (bool (Circle::*)(UnitVector3d const &) const) & Circle::contains);

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

    // The lambda is necessary for now; returning the unique pointer
    // directly leads to incorrect results and crashes.
    cls.def_static("decode",
                   [](py::bytes bytes) { return decode(bytes).release(); },
                   "bytes"_a);

    cls.def("__str__", [](Circle const &self) {
        return py::str("Circle({!s}, {!s})")
                .format(self.getCenter(), self.getOpeningAngle());
    });
    cls.def("__repr__", [](Circle const &self) {
        return py::str("Circle({!r}, {!r})")
                .format(self.getCenter(), self.getOpeningAngle());
    });
    cls.def("__setstate__", [](Circle &self, py::bytes bytes) {
        new (&self) Circle(*decode(bytes));
    });
}

}  // sphgeom
}  // lsst
