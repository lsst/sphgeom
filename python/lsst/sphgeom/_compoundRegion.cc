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
#include "pybind11/stl.h"

#include "lsst/sphgeom/python.h"
#include "lsst/sphgeom/python/utils.h"
#include "lsst/sphgeom/CompoundRegion.h"

namespace nb = nanobind;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

namespace {

nb::str _repr(const char *format, CompoundRegion const &self) {
    nb::object first = nb::cast(self.getOperand(0), nb::return_value_policy::reference);
    nb::object second = nb::cast(self.getOperand(1), nb::return_value_policy::reference);
    return nb::str(format).format(first, second);
}

}  // namespace

template <>
void defineClass(nb::class_<CompoundRegion, std::unique_ptr<CompoundRegion>, Region> &cls) {
    cls.def(
        "cloneOperand",
        [](CompoundRegion const &self, std::ptrdiff_t n) {
            return self.getOperand(python::convertIndex(2, n)).clone();
        }
    );
}

template <>
void defineClass(nb::class_<UnionRegion, std::unique_ptr<UnionRegion>, CompoundRegion> &cls) {
    cls.attr("TYPE_CODE") = nb::int_(UnionRegion::TYPE_CODE);
    cls.def(nb::init<Region const &, Region const &>());
    cls.def(nb::pickle(&python::encode, &python::decode<UnionRegion>));
    cls.def("__repr__", [](CompoundRegion const &self) { return _repr("UnionRegion({!r}, {!r})", self); });
}

template <>
void defineClass(nb::class_<IntersectionRegion, std::unique_ptr<IntersectionRegion>, CompoundRegion> &cls) {
    cls.attr("TYPE_CODE") = nb::int_(IntersectionRegion::TYPE_CODE);
    cls.def(nb::init<Region const &, Region const &>());
    cls.def(nb::pickle(&python::encode, &python::decode<IntersectionRegion>));
    cls.def("__repr__", [](CompoundRegion const &self) { return _repr("IntersectionRegion({!r}, {!r})", self); });
}

}  // namespace sphgeom
}  // namespace lsst
