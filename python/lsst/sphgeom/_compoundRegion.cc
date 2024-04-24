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
#include <nanobind/nanobind.h>

#include "lsst/sphgeom/python.h"
#include "lsst/sphgeom/python/utils.h"
#include "lsst/sphgeom/CompoundRegion.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace lsst {
namespace sphgeom {

namespace {

nb::str _repr(const char *format, CompoundRegion const &self) {
    nb::object first = nb::cast(self.getOperand(0), nb::rv_policy::reference);
    nb::object second = nb::cast(self.getOperand(1), nb::rv_policy::reference);
    return nb::str(format).format(first, second);
}

}  // namespace

template <>
void defineClass(nb::class_<CompoundRegion, Region> &cls) {
    cls.def(
        "cloneOperand",
        [](CompoundRegion const &self, std::ptrdiff_t n) {
            return self.getOperand(python::convertIndex(2, nanobind::int_(n))).clone();
        }// , nb::rv_policy::copy
    );
}

template <>
void defineClass(nb::class_<UnionRegion, CompoundRegion> &cls) {
    cls.attr("TYPE_CODE") = nb::int_(UnionRegion::TYPE_CODE);
    cls.def(nb::init<Region const &, Region const &>());
    //cls.def(nb::pickle(&python::encode, &python::decode<UnionRegion>));
    cls.def("__getstate__", &python::encode);
    cls.def("__setstate__", &python::decode<UnionRegion, nb::bytes>);
    cls.def("__repr__", [](CompoundRegion const &self) { return _repr("UnionRegion({!r}, {!r})", self); });
}

template <>
void defineClass(nb::class_<IntersectionRegion, CompoundRegion> &cls) {
    cls.attr("TYPE_CODE") = nb::int_(IntersectionRegion::TYPE_CODE);
    cls.def(nb::init<Region const &, Region const &>());
    cls.def("__getstate__", &python::encode);
    cls.def("__setstate__", &python::decode<IntersectionRegion, nb::bytes>);
    //cls.def(nb::pickle(&python::encode, &python::decode<IntersectionRegion>));
    cls.def("__repr__", [](CompoundRegion const &self) { return _repr("IntersectionRegion({!r}, {!r})", self); });
}

}  // namespace sphgeom
}  // namespace lsst
