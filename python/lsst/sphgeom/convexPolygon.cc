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

#include "sphgeom.h"

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

namespace {
std::unique_ptr<ConvexPolygon> decode(py::bytes bytes) {
    uint8_t const *buffer = reinterpret_cast<uint8_t const *>(
            PYBIND11_BYTES_AS_STRING(bytes.ptr()));
    size_t n = static_cast<size_t>(PYBIND11_BYTES_SIZE(bytes.ptr()));
    return ConvexPolygon::decode(buffer, n);
}
}

template <>
void defineClass(py::class_<ConvexPolygon, std::shared_ptr<ConvexPolygon>,
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

    // Note that the Region interface has already been wrapped.

    // The lambda is necessary for now; returning the unique pointer
    // directly leads to incorrect results and crashes.
    cls.def_static("decode",
                   [](py::bytes bytes) { return decode(bytes).release(); },
                   "bytes"_a);

    cls.def("__repr__", [](ConvexPolygon const &self) {
        return py::str("ConvexPolygon({!r})").format(self.getVertices());
    });
    cls.def("__setstate__", [](ConvexPolygon &self, py::bytes bytes) {
        new (&self) ConvexPolygon(*decode(bytes));
    });
}

}  // sphgeom
}  // lsst
