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
