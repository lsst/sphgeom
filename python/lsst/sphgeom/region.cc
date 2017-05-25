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
#include "lsst/sphgeom/Box3d.h"
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
py::bytes encode(Region const &self) {
    std::vector<uint8_t> bytes = self.encode();
    return py::bytes(reinterpret_cast<char const *>(bytes.data()),
                     bytes.size());
}
}

template <>
void defineClass(py::class_<Region, std::shared_ptr<Region>> &cls) {
    cls.def("clone", [](Region const &self) { return self.clone().release(); });
    cls.def("getBoundingBox", &Region::getBoundingBox);
    cls.def("getBoundingBox3d", &Region::getBoundingBox3d);
    cls.def("getBoundingCircle", &Region::getBoundingCircle);
    cls.def("contains", &Region::contains, "unitVector"_a);
    cls.def("__contains__", &Region::contains, "unitVector"_a,
            py::is_operator());
    // The per-subclass relate() overloads are used to implement
    // double-dispatch in C++, and are not needed in Python.
    cls.def("relate",
            (Relationship(Region::*)(Region const &) const) & Region::relate,
            "region"_a);
    cls.def("encode", &encode);

    cls.def("__getstate__", &encode);

    cls.def_static(
            "decode",
            [](py::bytes bytes) {
                uint8_t const *buffer = reinterpret_cast<uint8_t const *>(
                        PYBIND11_BYTES_AS_STRING(bytes.ptr()));
                size_t n =
                        static_cast<size_t>(PYBIND11_BYTES_SIZE(bytes.ptr()));
                return Region::decode(buffer, n).release();
            },
            "bytes"_a);
}

}  // sphgeom
}  // lsst
