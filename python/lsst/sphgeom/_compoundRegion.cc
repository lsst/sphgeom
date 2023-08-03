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

nb::str _repr(const char *class_name, CompoundRegion const &self) {
    std::string format = class_name;
    format += "(";
    nb::tuple operands(self.nOperands());
    for (unsigned i = 0; i != self.nOperands(); i++) {
        nb::object operand = nb::cast(self.getOperand(i), nb::rv_policy::reference);
        operands[i] = operand;
        if (i != 0) {
            format += ", ";
        }
        format += "{!r}";
    }
    format += ")";
    return nb::str(format).attr("format")(*operands);
}

template <typename _CompoundRegion>
std::unique_ptr<_CompoundRegion> _args_factory(const nb::args& args) {
    std::vector<std::unique_ptr<Region>> operands;
    for (auto&& item: args) {
        Region* region = item.cast<Region*>();
        operands.emplace_back(region->clone());
    }
    return std::make_unique<_CompoundRegion>(std::move(operands));
}

// Iterator for CompoundRegion.
class CompoundIterator {
public:
    CompoundIterator(CompoundRegion const& compound, size_t pos) : _compound(compound), _pos(pos) {}

    Region const& operator*() const { return _compound.getOperand(_pos); }

    CompoundIterator& operator++() {
        ++ _pos;
        return *this;
    }

    bool operator==(CompoundIterator const& other) const {
        return &_compound == &other._compound and _pos == other._pos;
    }

private:
    CompoundRegion const& _compound;
    size_t _pos;
};

}  // namespace

template <>
void defineClass(nb::class_<CompoundRegion, Region> &cls) {
    cls.def("nOperands", &CompoundRegion::nOperands);
    cls.def("__len__", &CompoundRegion::nOperands);
    cls.def(
        "__iter__",
        [](CompoundRegion const& region) {
            return nb::make_iterator(
                CompoundIterator(region, 0U), CompoundIterator(region, region.nOperands())
            );
        },
        nb::rv_policy::reference_internal  // Keeps region alive while iterator is in use.
    );
    cls.def(
        "cloneOperand",
        [](CompoundRegion const &self, std::ptrdiff_t n) {
            int nOperands = self.nOperands();
            return self.getOperand(python::convertIndex(nOperands, n)).clone();
        }
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
