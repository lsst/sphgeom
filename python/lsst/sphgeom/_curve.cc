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

#include "lsst/sphgeom/curve.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

void defineCurve(py::module &mod) {
    mod.def("log2", (uint8_t(*)(uint64_t)) & log2);
    mod.def("mortonIndex", (uint64_t(*)(uint32_t, uint32_t)) & mortonIndex,
            "x"_a, "y"_a);
    mod.def("mortonIndexInverse",
            (std::tuple<uint32_t, uint32_t>(*)(uint64_t)) & mortonIndexInverse,
            "z"_a);
    mod.def("mortonToHilbert", &mortonToHilbert, "z"_a, "m"_a);
    mod.def("hilbertToMorton", &hilbertToMorton, "h"_a, "m"_a);
    mod.def("hilbertIndex",
            (uint64_t(*)(uint32_t, uint32_t, int)) & hilbertIndex, "x"_a, "y"_a,
            "m"_a);
    mod.def("hilbertIndexInverse",
            (std::tuple<uint32_t, uint32_t>(*)(uint64_t, int)) &
                    hilbertIndexInverse,
            "h"_a, "m"_a);
}

}  // sphgeom
}  // lsst
