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
#include "pybind11/numpy.h"

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/AngleInterval.h"
#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/NormalizedAngleInterval.h"
#include "lsst/sphgeom/Region.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<Box, std::unique_ptr<Box>, Region> &cls) {
    cls.attr("TYPE_CODE") = py::int_(Box::TYPE_CODE);

    cls.def_static("fromDegrees", &Box::fromDegrees, "lon1"_a, "lat1"_a,
                   "lon2"_a, "lat2"_a);
    cls.def_static("fromRadians", &Box::fromRadians, "lon1"_a, "lat1"_a,
                   "lon2"_a, "lat2"_a);
    cls.def_static("empty", &Box::empty);
    cls.def_static("full", &Box::full);
    cls.def_static("halfWidthForCircle", &Box::halfWidthForCircle, "radius"_a,
                   "lat"_a);
    cls.def_static("allLongitudes", &Box::allLongitudes);
    cls.def_static("allLatitudes", &Box::allLatitudes);

    cls.def(py::init<>());
    cls.def(py::init<LonLat const &>(), "point"_a);
    cls.def(py::init<LonLat const &, LonLat const &>(), "point1"_a, "point2"_a);
    cls.def(py::init<LonLat const &, Angle, Angle>(), "center"_a, "width"_a,
            "height"_a);
    cls.def(py::init<NormalizedAngleInterval const &, AngleInterval const &>(),
            "lon"_a, "lat"_a);
    cls.def(py::init<Box const &>(), "box"_a);

    cls.def("__eq__", (bool (Box::*)(Box const &) const) & Box::operator==,
            py::is_operator());
    cls.def("__eq__", (bool (Box::*)(LonLat const &) const) & Box::operator==,
            py::is_operator());
    cls.def("__ne__", (bool (Box::*)(Box const &) const) & Box::operator!=,
            py::is_operator());
    cls.def("__ne__", (bool (Box::*)(LonLat const &) const) & Box::operator!=,
            py::is_operator());
    cls.def("__contains__",
            (bool (Box::*)(LonLat const &) const) & Box::contains,
            py::is_operator());
    cls.def("__contains__", (bool (Box::*)(Box const &) const) & Box::contains,
            py::is_operator());
    // Rewrap this base class method since there are overloads in this subclass
    cls.def("__contains__",
            (bool (Box::*)(UnitVector3d const &) const) & Box::contains,
            py::is_operator());

    cls.def("getLon", &Box::getLon);
    cls.def("getLat", &Box::getLat);
    cls.def("isEmpty", &Box::isEmpty);
    cls.def("isFull", &Box::isFull);
    cls.def("getCenter", &Box::getCenter);
    cls.def("getWidth", &Box::getWidth);
    cls.def("getHeight", &Box::getHeight);
    cls.def("contains", (bool (Box::*)(LonLat const &) const) & Box::contains);
    cls.def("contains", (bool (Box::*)(Box const &) const) & Box::contains);
    // Rewrap these base class methods since there are overloads in this subclass
    cls.def("contains",
            (bool (Box::*)(UnitVector3d const &) const) & Box::contains);
    cls.def("contains", py::vectorize((bool (Box::*)(double, double, double) const)&Box::contains),
            "x"_a, "y"_a, "z"_a);
    cls.def("contains", py::vectorize((bool (Box::*)(double, double) const)&Box::contains),
            "lon"_a, "lat"_a);
    cls.def("isDisjointFrom",
            (bool (Box::*)(LonLat const &) const) & Box::isDisjointFrom);
    cls.def("isDisjointFrom",
            (bool (Box::*)(Box const &) const) & Box::isDisjointFrom);
    cls.def("intersects",
            (bool (Box::*)(LonLat const &) const) & Box::intersects);
    cls.def("intersects", (bool (Box::*)(Box const &) const) & Box::intersects);
    cls.def("isWithin", (bool (Box::*)(LonLat const &) const) & Box::isWithin);
    cls.def("isWithin", (bool (Box::*)(Box const &) const) & Box::isWithin);
    cls.def("clipTo", (Box & (Box::*)(LonLat const &)) & Box::clipTo);
    cls.def("clipTo", (Box & (Box::*)(Box const &)) & Box::clipTo);
    cls.def("clippedTo", (Box(Box::*)(LonLat const &) const) & Box::clippedTo);
    cls.def("clippedTo", (Box(Box::*)(Box const &) const) & Box::clippedTo);
    cls.def("expandTo", (Box & (Box::*)(LonLat const &)) & Box::expandTo);
    cls.def("expandTo", (Box & (Box::*)(Box const &)) & Box::expandTo);
    cls.def("expandedTo",
            (Box(Box::*)(LonLat const &) const) & Box::expandedTo);
    cls.def("expandedTo", (Box(Box::*)(Box const &) const) & Box::expandedTo);
    cls.def("dilateBy", (Box & (Box::*)(Angle)) & Box::dilateBy, "angle"_a);
    cls.def("dilateBy", (Box & (Box::*)(Angle, Angle)) & Box::dilateBy,
            "width"_a, "height"_a);
    cls.def("dilatedBy", (Box(Box::*)(Angle) const) & Box::dilatedBy,
            "angle"_a);
    cls.def("dilatedBy", (Box(Box::*)(Angle, Angle) const) & Box::dilatedBy,
            "width"_a, "height"_a);
    cls.def("erodeBy", (Box & (Box::*)(Angle)) & Box::erodeBy, "angle"_a);
    cls.def("erodeBy", (Box & (Box::*)(Angle, Angle)) & Box::erodeBy, "width"_a,
            "height"_a);
    cls.def("erodedBy", (Box(Box::*)(Angle) const) & Box::erodedBy, "angle"_a);
    cls.def("erodedBy", (Box(Box::*)(Angle, Angle) const) & Box::erodedBy,
            "width"_a, "height"_a);
    cls.def("getArea", &Box::getArea);
    cls.def("relate",
            (Relationship(Box::*)(LonLat const &) const) & Box::relate,
            "point"_a);
    // Rewrap this base class method since there are overloads in this subclass
    cls.def("relate",
            (Relationship(Box::*)(Region const &) const) & Box::relate,
            "region"_a);

    // Note that the Region interface has already been wrapped.

    cls.def("__str__", [](Box const &self) {
        return py::str("Box({!s}, {!s})").format(self.getLon(), self.getLat());
    });
    cls.def("__repr__", [](Box const &self) {
        return py::str("Box({!r}, {!r})").format(self.getLon(), self.getLat());
    });
    cls.def(py::pickle(&python::encode, &python::decode<Box>));
}

}  // sphgeom
}  // lsst
