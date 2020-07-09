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

#include "lsst/sphgeom/RangeSet.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

namespace {

/// Convert a Python integer to a uint64_t.
uint64_t _uint64(py::handle const &obj) {
    try {
        return obj.cast<uint64_t>();
    } catch (py::cast_error const &) {
        throw py::value_error(
                "RangeSet elements and range beginning and "
                "end points must be non-negative integers "
                "less than 2**64");
    }
}

/// Make a RangeSet from an iterable. Each item must be an integer that fits
/// in a uint64_t, or a sequence of two such integers.
RangeSet makeRangeSet(py::iterable iterable) {
    RangeSet rs;
    for (py::handle item : iterable) {
        PyObject *o = item.ptr();
        if (PySequence_Check(o) && PySequence_Size(o) == 2) {
            uint64_t first = _uint64(py::reinterpret_steal<py::object>(
                    PySequence_GetItem(o, 0)));
            uint64_t last = _uint64(py::reinterpret_steal<py::object>(
                    PySequence_GetItem(o, 1)));
            rs.insert(first, last);
        } else {
            rs.insert(_uint64(item));
        }
    }
    return rs;
}

/// Make a python list of the ranges in the given RangeSet.
py::list ranges(RangeSet const &self) {
    py::list list;
    for (auto t : self) {
        list.append(py::make_tuple(py::int_(std::get<0>(t)),
                                   py::int_(std::get<1>(t))));
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
void defineClass(py::class_<RangeSet, std::shared_ptr<RangeSet>> &cls) {
    cls.def(py::init<>());
    cls.def(py::init<uint64_t>(), "integer"_a);
    cls.def(py::init([](uint64_t a, uint64_t b) {
                return new RangeSet(a, b);
            }),
            "first"_a, "last"_a);
    cls.def(py::init<RangeSet const &>(), "rangeSet"_a);
    cls.def(py::init(
            [](py::iterable iterable) {
                return new RangeSet(makeRangeSet(iterable));
            }),
            "iterable"_a);
    cls.def("__eq__", &RangeSet::operator==, py::is_operator());
    cls.def("__ne__", &RangeSet::operator!=, py::is_operator());

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
    cls.def("__invert__", &RangeSet::operator~, py::is_operator());
    cls.def("__and__", &RangeSet::operator&, py::is_operator());
    cls.def("__or__", &RangeSet::operator|, py::is_operator());
    cls.def("__sub__", &RangeSet::operator-, py::is_operator());
    cls.def("__xor__", &RangeSet::operator^, py::is_operator());
    cls.def("__iand__", &RangeSet::operator&=);
    cls.def("__ior__", &RangeSet::operator|=);
    cls.def("__isub__", &RangeSet::operator-=);
    cls.def("__ixor__", &RangeSet::operator^=);

    cls.def("__len__", &RangeSet::size);
    cls.def("__getitem__", [](RangeSet const &self, py::int_ i) {
        auto j = python::convertIndex(static_cast<ptrdiff_t>(self.size()), i);
        return py::cast(self.begin()[j]);
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
            "integer"_a, py::is_operator());
    cls.def("__contains__",
            (bool (RangeSet::*)(uint64_t, uint64_t) const) & RangeSet::contains,
            "first"_a, "last"_a, py::is_operator());
    cls.def("__contains__",
            (bool (RangeSet::*)(RangeSet const &) const) & RangeSet::contains,
            "rangeSet"_a, py::is_operator());

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
            [](RangeSet const &self) { return py::str(ranges(self)); });
    cls.def("__repr__", [](RangeSet const &self) {
        return py::str("RangeSet({!s})").format(ranges(self));
    });

    cls.def("__reduce__", [cls](RangeSet const &self) {
        return py::make_tuple(cls, py::make_tuple(ranges(self)));
    });
}

}  // sphgeom
}  // lsst
