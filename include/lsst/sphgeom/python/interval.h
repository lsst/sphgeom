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

#ifndef LSST_SPHGEOM_PYTHON_INTERVAL_H_
#define LSST_SPHGEOM_PYTHON_INTERVAL_H_

#include "pybind11/pybind11.h"

#include "relationship.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {
namespace python {
namespace {

/// Provide common methods for interval types via lambdas.
///
/// This avoids making the Interval CRTP base class implementation machinery
/// visible from Python. Furthermore, it can be used to wrap
/// NormalizedAngleInterval despite the latter not inheriting from Interval.
template <typename PyClass, typename Class, typename Scalar>
void defineInterval(PyClass& cls) {
    cls.def("__eq__", [](Class const& self,
                         Class const& other) { return self == other; },
            py::is_operator());
    cls.def("__eq__",
            [](Class const& self, Scalar other) { return self == other; },
            py::is_operator());
    cls.def("__ne__", [](Class const& self,
                         Class const& other) { return self != other; },
            py::is_operator());
    cls.def("__ne__",
            [](Class const& self, Scalar other) { return self != other; },
            py::is_operator());

    cls.def("getA", [](Class const& self) { return self.getA(); });
    cls.def("getB", [](Class const& self) { return self.getB(); });
    cls.def("isEmpty", [](Class const& self) { return self.isEmpty(); });
    cls.def("getCenter", [](Class const& self) { return self.getCenter(); });
    cls.def("getSize", [](Class const& self) { return self.getSize(); });

    cls.def("__contains__", [](Class const& self,
                               Scalar other) { return self.contains(other); },
            py::is_operator());
    cls.def("__contains__",
            [](Class const& self, Class const& other) {
                return self.contains(other);
            },
            py::is_operator());

    cls.def("contains", [](Class const& self, Scalar other) {
        return self.contains(other);
    });
    cls.def("contains", [](Class const& self, Class const& other) {
        return self.contains(other);
    });
    cls.def("isDisjointFrom", [](Class const& self, Scalar other) {
        return self.isDisjointFrom(other);
    });
    cls.def("isDisjointFrom", [](Class const& self, Class const& other) {
        return self.isDisjointFrom(other);
    });
    cls.def("intersects", [](Class const& self, Scalar other) {
        return self.intersects(other);
    });
    cls.def("intersects", [](Class const& self, Class const& other) {
        return self.intersects(other);
    });
    cls.def("isWithin", [](Class const& self, Scalar other) {
        return self.isWithin(other);
    });
    cls.def("isWithin", [](Class const& self, Class const& other) {
        return self.isWithin(other);
    });
    cls.def("relate", [](Class const& self, Scalar other) {
        return self.relate(other);
    });
    cls.def("relate", [](Class const& self, Class const& other) {
        return self.relate(other);
    });

    // Note that when a reference to *this is returned in C++, it will
    // have an existing wrapper object which is automatically returned
    // by pybind11 - no return value policy is needed. The explicit
    // reference return type for the corresponding lambdas seems to be
    // required to obtain this behavior.

    cls.def("clipTo", [](Class& self, Scalar other) -> Class & {
        self.clipTo(other);
        return self;
    });
    cls.def("clipTo", [](Class& self, Class const& other) -> Class & {
        self.clipTo(other);
        return self;
    });
    cls.def("clippedTo", [](Class const& self, Scalar other) {
        Class instance = self.clippedTo(other);
        return instance;
    });
    cls.def("clippedTo", [](Class const& self, Class const& other) {
        Class instance = self.clippedTo(other);
        return instance;
    });
    cls.def("expandTo", [](Class& self, Scalar other) -> Class & {
        self.expandTo(other);
        return self;
    });
    cls.def("expandTo", [](Class& self, Class const& other) -> Class & {
        self.expandTo(other);
        return self;
    });
    cls.def("expandedTo", [](Class const& self, Scalar other) {
        Class instance = self.expandedTo(other);
        return instance;
    });
    cls.def("expandedTo", [](Class const& self, Class const& other) {
        Class instance = self.expandedTo(other);
        return instance;
    });

    cls.def("dilateBy", [](Class& self, Scalar other) -> Class & {
        self.dilateBy(other);
        return self;
    });
    cls.def("dilatedBy", [](Class const& self, Scalar other) {
        Class instance = self.dilatedBy(other);
        return instance;
    });
    cls.def("erodeBy", [](Class& self, Scalar other) -> Class & {
        self.erodeBy(other);
        return self;
    });
    cls.def("erodedBy", [](Class const& self, Scalar other) {
        Class instance = self.erodedBy(other);
        return instance;
    });

    cls.def("__reduce__", [cls](Class const &self) {
        return py::make_tuple(cls, py::make_tuple(self.getA(), self.getB()));
    });
}

}  // unnamed
}  // python
}  // sphgeom
}  // lsst

#endif  // LSST_SPHGEOM_PYTHON_INTERVAL_H_
