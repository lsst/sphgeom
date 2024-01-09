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
#include "pybind11/stl.h"

#include "lsst/sphgeom/python.h"
#include "lsst/sphgeom/python/utils.h"
#include "lsst/sphgeom/CompoundRegion.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

namespace {

py::str _repr(const char *format, CompoundRegion const &self) {
    py::object first = py::cast(self.getOperand(0), py::return_value_policy::reference);
    py::object second = py::cast(self.getOperand(1), py::return_value_policy::reference);
    return py::str(format).format(first, second);
}

}  // namespace

template <>
void defineClass(py::class_<CompoundRegion, std::unique_ptr<CompoundRegion>, Region> &cls) {
    cls.def(
        "cloneOperand",
        [](CompoundRegion const &self, std::ptrdiff_t n) {
            return self.getOperand(python::convertIndex(2, n)).clone();
        }
    );
}

template <>
void defineClass(py::class_<UnionRegion, std::unique_ptr<UnionRegion>, CompoundRegion> &cls) {
    cls.attr("TYPE_CODE") = py::int_(UnionRegion::TYPE_CODE);
    cls.def(py::init<Region const &, Region const &>());
    cls.def(py::pickle(&python::encode, &python::decode<UnionRegion>));
    cls.def("__repr__", [](CompoundRegion const &self) { return _repr("UnionRegion({!r}, {!r})", self); });
}

template <>
void defineClass(py::class_<IntersectionRegion, std::unique_ptr<IntersectionRegion>, CompoundRegion> &cls) {
    cls.attr("TYPE_CODE") = py::int_(IntersectionRegion::TYPE_CODE);
    cls.def(py::init<Region const &, Region const &>());
    cls.def(py::pickle(&python::encode, &python::decode<IntersectionRegion>));
    cls.def("__repr__", [](CompoundRegion const &self) { return _repr("IntersectionRegion({!r}, {!r})", self); });
}

}  // namespace sphgeom
}  // namespace lsst
