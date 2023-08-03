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
#include <nanobind/stl/list.h>
#include <nanobind/stl/tuple.h>

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/RangeSet.h"
#include "lsst/sphgeom/python/utils.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace lsst {
namespace sphgeom {

namespace {

/// Convert a Python integer to a uint64_t.
uint64_t _uint64(nb::handle const &obj) {
    try {
        return nb::cast<uint64_t>(obj);
    } catch (nb::cast_error const &) {
        throw nb::value_error(
                "RangeSet elements and range beginning and "
                "end points must be non-negative integers "
                "less than 2**64");
    }
}

/// Make a RangeSet from an iterable. Each item must be an integer that fits
/// in a uint64_t, or a sequence of two such integers.
RangeSet makeRangeSet(nb::iterable iterable) {
    RangeSet rs;
    for (nb::handle item : iterable) {
        PyObject *o = item.ptr();
        if (PySequence_Check(o) && PySequence_Size(o) == 2) {
            uint64_t first = _uint64(nb::steal<nb::object>(
                    PySequence_GetItem(o, 0)));
            uint64_t last = _uint64(nb::steal<nb::object>(
                    PySequence_GetItem(o, 1)));
            rs.insert(first, last);
        } else {
            rs.insert(_uint64(item));
        }
    }
    return rs;
}

/// Make a python list of the ranges in the given RangeSet.
nb::list ranges(RangeSet const &self) {
    nb::list list;
    for (auto t : self) {
        list.append(nb::make_tuple(nb::int_(std::get<0>(t)),
                                   nb::int_(std::get<1>(t))));
    }
    return list;
}

// TODO: In C++, the end-point of a range containing 2**64 - 1 is 0, because
// unsigned integer arithmetic is modular, and 2**64 does not fit in a
// uint64_t. In Python, it would perhaps be nicer to map between C++
// range end-point values of 0 and the Python integer 2**64. Since this is
// somewhat involved, it is left as future work.

}  // <anonymous>

template <>
void defineClass(nb::class_<RangeSet> &cls) {
    cls.def(nb::init<>());
    cls.def(nb::init<uint64_t>(), "integer"_a);
    cls.def("__init__", [](RangeSet *t, uint64_t a, uint64_t b) {
                new (t) RangeSet(a, b);
            }, nb::arg("first"), nb::arg("last"));
    cls.def(nb::init<RangeSet const &>(), "rangeSet"_a);
    cls.def("__init__",
            [](RangeSet *t, nb::iterable iterable) {
                new (t) RangeSet(makeRangeSet(iterable));
            }), nb::arg("iterable");
    cls.def("__eq__", &RangeSet::operator==, nb::is_operator());
    cls.def("__ne__", &RangeSet::operator!=, nb::is_operator());

    cls.def("insert", (void (RangeSet::*)(uint64_t)) & RangeSet::insert,
            "integer"_a);
    cls.def("insert",
            (void (RangeSet::*)(uint64_t, uint64_t)) & RangeSet::insert,
            "first"_a, "last"_a);
    cls.def("erase", (void (RangeSet::*)(uint64_t)) & RangeSet::erase,
            "integer"_a);
    cls.def("erase", (void (RangeSet::*)(uint64_t, uint64_t)) & RangeSet::erase,
            "first"_a, "last"_a);

    cls.def("complement", &RangeSet::complement);
    cls.def("complemented", &RangeSet::complemented);
    cls.def("intersection", &RangeSet::intersection, "rangeSet"_a);
    // In C++, the set union function is named join because union is a keyword.
    // Python does not suffer from the same restriction.
    cls.def("union", &RangeSet::join, "rangeSet"_a);
    cls.def("difference", &RangeSet::difference, "rangeSet"_a);
    cls.def("symmetricDifference", &RangeSet::symmetricDifference,
            "rangeSet"_a);
    cls.def("__invert__", &RangeSet::operator~, nb::is_operator());
    cls.def("__and__", &RangeSet::operator&, nb::is_operator());
    cls.def("__or__", &RangeSet::operator|, nb::is_operator());
    cls.def("__sub__", &RangeSet::operator-, nb::is_operator());
    cls.def("__xor__", &RangeSet::operator^, nb::is_operator());
    cls.def("__iand__", &RangeSet::operator&=);
    cls.def("__ior__", &RangeSet::operator|=);
    cls.def("__isub__", &RangeSet::operator-=);
    cls.def("__ixor__", &RangeSet::operator^=);

    cls.def("__len__", &RangeSet::size);
    cls.def("__getitem__", [](RangeSet const &self, nb::int_ i) {
        auto j = python::convertIndex(static_cast<ptrdiff_t>(self.size()), i);
        return nb::cast(self.begin()[j]);
    });

    cls.def("intersects",
            (bool (RangeSet::*)(uint64_t) const) & RangeSet::intersects,
            "integer"_a);
    cls.def("intersects",
            (bool (RangeSet::*)(uint64_t, uint64_t) const) &
                    RangeSet::intersects,
            "first"_a, "last"_a);
    cls.def("intersects",
            (bool (RangeSet::*)(RangeSet const &) const) & RangeSet::intersects,
            "rangeSet"_a);

    cls.def("contains",
            (bool (RangeSet::*)(uint64_t) const) & RangeSet::contains,
            "integer"_a);
    cls.def("contains",
            (bool (RangeSet::*)(uint64_t, uint64_t) const) & RangeSet::contains,
            "first"_a, "last"_a);
    cls.def("contains",
            (bool (RangeSet::*)(RangeSet const &) const) & RangeSet::contains,
            "rangeSet"_a);
    cls.def("__contains__",
            (bool (RangeSet::*)(uint64_t) const) & RangeSet::contains,
            "integer"_a, nb::is_operator());
    cls.def("__contains__",
            (bool (RangeSet::*)(uint64_t, uint64_t) const) & RangeSet::contains,
            "first"_a, "last"_a, nb::is_operator());
    cls.def("__contains__",
            (bool (RangeSet::*)(RangeSet const &) const) & RangeSet::contains,
            "rangeSet"_a, nb::is_operator());

    cls.def("isWithin",
            (bool (RangeSet::*)(uint64_t) const) & RangeSet::isWithin,
            "integer"_a);
    cls.def("isWithin",
            (bool (RangeSet::*)(uint64_t, uint64_t) const) & RangeSet::isWithin,
            "first"_a, "last"_a);
    cls.def("isWithin",
            (bool (RangeSet::*)(RangeSet const &) const) & RangeSet::isWithin,
            "rangeSet"_a);

    cls.def("isDisjointFrom",
            (bool (RangeSet::*)(uint64_t) const) & RangeSet::isDisjointFrom,
            "integer"_a);
    cls.def("isDisjointFrom",
            (bool (RangeSet::*)(uint64_t, uint64_t) const) &
                    RangeSet::isDisjointFrom,
            "first"_a, "last"_a);
    cls.def("isDisjointFrom",
            (bool (RangeSet::*)(RangeSet const &) const) &
                    RangeSet::isDisjointFrom,
            "rangeSet"_a);

    cls.def("simplify", &RangeSet::simplify, "n"_a);
    cls.def("simplified", &RangeSet::simplified, "n"_a);
    cls.def("scale", &RangeSet::scale, "factor"_a);
    cls.def("scaled", &RangeSet::scaled, "factor"_a);
    cls.def("fill", &RangeSet::fill);
    cls.def("clear", &RangeSet::clear);
    cls.def("empty", &RangeSet::empty);
    cls.def("full", &RangeSet::full);
    cls.def("size", &RangeSet::size);
    cls.def("cardinality", &RangeSet::cardinality);
    // max_size() and swap() are omitted. The former is a C++ container
    // requirement, and the latter doesn't seem relevant to Python.
    cls.def("isValid", &RangeSet::cardinality);
    cls.def("ranges", &ranges);

    cls.def("__str__",
            [](RangeSet const &self) { return nb::str(ranges(self)); });
    cls.def("__repr__", [](RangeSet const &self) {
        return nb::str("RangeSet({!s})").format(ranges(self));
    });

    cls.def("__reduce__", [cls](RangeSet const &self) {
        return nb::make_tuple(cls, nb::make_tuple(ranges(self)));
    });
}

}  // sphgeom
}  // lsst
