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

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
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
void defineClass(py::class_<Region, std::unique_ptr<Region>> &cls) {
    cls.def("clone", &Region::clone);
    cls.def("getBoundingBox", &Region::getBoundingBox);
    cls.def("getBoundingBox3d", &Region::getBoundingBox3d);
    cls.def("getBoundingCircle", &Region::getBoundingCircle);
    cls.def("contains", py::overload_cast<UnitVector3d const &>(&Region::contains, py::const_),
            "unitVector"_a);
    cls.def("contains", py::vectorize((bool (Region::*)(double, double, double) const)&Region::contains),
            "x"_a, "y"_a, "z"_a);
    cls.def("contains", py::vectorize((bool (Region::*)(double, double) const)&Region::contains),
            "lon"_a, "lat"_a);
    cls.def("__contains__", py::overload_cast<UnitVector3d const &>(&Region::contains, py::const_),
            py::is_operator());
    // The per-subclass relate() overloads are used to implement
    // double-dispatch in C++, and are not needed in Python.
    cls.def("relate",
            (Relationship(Region::*)(Region const &) const) & Region::relate,
            "region"_a);
    cls.def("encode", &python::encode);
    cls.def_static("decode", &python::decode<Region>, "bytes"_a);
}

}  // sphgeom
}  // lsst
