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
#include "pybind11/stl.h"
#include "pybind11/numpy.h"

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<ConvexPolygon, std::unique_ptr<ConvexPolygon>,
                            Region> &cls) {
    cls.attr("TYPE_CODE") = py::int_(ConvexPolygon::TYPE_CODE);

    cls.def_static("convexHull", &ConvexPolygon::convexHull, "points"_a);

    cls.def(py::init<std::vector<UnitVector3d> const &>(), "points"_a);
    // Do not wrap the two unsafe (3 and 4 vertex) constructors
    cls.def(py::init<ConvexPolygon const &>(), "convexPolygon"_a);

    cls.def("__eq__", &ConvexPolygon::operator==, py::is_operator());
    cls.def("__ne__", &ConvexPolygon::operator!=, py::is_operator());

    cls.def("getVertices", &ConvexPolygon::getVertices);
    cls.def("getCentroid", &ConvexPolygon::getCentroid);

    // Note that much of the Region interface has already been wrapped. Here are bits that have not:
    // (include overloads from Region that would otherwise be shadowed).
    cls.def("contains", py::overload_cast<UnitVector3d const &>(&ConvexPolygon::contains, py::const_));
    cls.def("contains", py::overload_cast<Region const &>(&ConvexPolygon::contains, py::const_));
    cls.def("contains",
            py::vectorize((bool (ConvexPolygon::*)(double, double, double) const)&ConvexPolygon::contains),
            "x"_a, "y"_a, "z"_a);
    cls.def("contains",
            py::vectorize((bool (ConvexPolygon::*)(double, double) const)&ConvexPolygon::contains),
            "lon"_a, "lat"_a);
    cls.def("isDisjointFrom", &ConvexPolygon::isDisjointFrom);
    cls.def("intersects", &ConvexPolygon::intersects);
    cls.def("isWithin", &ConvexPolygon::isWithin);

    cls.def("__repr__", [](ConvexPolygon const &self) {
        return py::str("ConvexPolygon({!r})").format(self.getVertices());
    });
    cls.def(py::pickle(&python::encode, &python::decode<ConvexPolygon>));
}

}  // sphgeom
}  // lsst
