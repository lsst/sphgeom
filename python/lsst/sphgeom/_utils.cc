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
