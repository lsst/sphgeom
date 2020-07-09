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

#include "lsst/sphgeom/Angle.h"
#include "lsst/sphgeom/UnitVector3d.h"
#include "lsst/sphgeom/utils.h"
#include "lsst/sphgeom/Vector3d.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

void defineUtils(py::module &mod) {
    mod.def("getMinSquaredChordLength", &getMinSquaredChordLength, "v"_a, "a"_a,
            "b"_a, "n"_a);
    mod.def("getMaxSquaredChordLength", &getMaxSquaredChordLength, "v"_a, "a"_a,
            "b"_a, "n"_a);
    mod.def("getMinAngleToCircle", &getMinAngleToCircle, "x"_a, "c"_a);
    mod.def("getMaxAngleToCircle", &getMaxAngleToCircle, "x"_a, "c"_a);
    mod.def("getWeightedCentroid", &getWeightedCentroid, "vector0"_a,
            "vector1"_a, "vector2"_a);
}

}  // sphgeom
}  // lsst
