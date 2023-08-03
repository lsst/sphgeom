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
#include "nanobind/nanobind.h"

#include "lsst/sphgeom/python/relationship.h"

namespace nb = nanobind;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

void defineRelationship(nb::module &mod) {
    mod.attr("DISJOINT") = nb::cast(DISJOINT.to_ulong());
    mod.attr("INTERSECTS") = nb::cast(INTERSECTS.to_ulong());
    mod.attr("CONTAINS") = nb::cast(CONTAINS.to_ulong());
    mod.attr("WITHIN") = nb::cast(WITHIN.to_ulong());

    mod.def("invert", &invert, "relationship"_a);
}

}  // sphgeom
}  // lsst
