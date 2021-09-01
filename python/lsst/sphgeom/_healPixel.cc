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

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/HEALPixel.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<HEALPixel, std::unique_ptr<HEALPixel>> &cls) {
    cls.def("clone", [](HEALPixel const &self) { return self.clone().release(); });
    cls.def("getBoundingBox", &HEALPixel::getBoundingBox);
    cls.def("getBoundingBox3d", &HEALPixel::getBoundingBox3d);
    cls.def("getBoundingCircle", &HEALPixel::getBoundingCircle);
    cls.def("relate", (Relationship(HEALPixel::*)(HEALPixel const &) const) & HEALPixel::relate,
            "region"_a);
}

}  // sphgeom
}  // lsst
