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
#include "nanobind/nanobind.h"

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Matrix3d.h"
#include "lsst/sphgeom/python/utils.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace lsst {
namespace sphgeom {

namespace {
Vector3d getRow(Matrix3d const &self, nb::int_ row) {
    return self.getRow(static_cast<int>(python::convertIndex(3, row)));
}
}

template <>
void defineClass(nb::class_<Matrix3d> &cls) {
    cls.def(nb::init<>());
    cls.def(nb::init<double, double, double, double, double, double, double,
                     double, double>(),
            "m00"_a, "m01"_a, "m02"_a, "m10"_a, "m11"_a, "m12"_a, "m20"_a,
            "m21"_a, "m22"_a);
    cls.def(nb::init<Vector3d const &>(), "diagonal"_a);
    cls.def(nb::init<double>(), "scale"_a);
    cls.def(nb::init<Matrix3d const &>(), "matrix"_a);

    cls.def("__eq__", &Matrix3d::operator==, nb::is_operator());
    cls.def("__ne__", &Matrix3d::operator!=, nb::is_operator());

    // Add bounds checking to getRow and getColumn
    cls.def("getRow", &getRow, "row"_a);
    cls.def("getColumn",
            [](Matrix3d const &self, nb::int_ col) {
                return self.getColumn(
                        static_cast<int>(python::convertIndex(3, col)));
            },
            "col"_a);

    cls.def("__len__", [](Matrix3d const &self) { return nb::int_(3); });
    cls.def("__getitem__", &getRow, nb::is_operator());
    cls.def("__getitem__",
            [](Matrix3d const &self, nb::tuple t) {
                if (t.size() > 2) {
                    throw nb::index_error("Too many indexes for Matrix3d");
                } else if (t.size() == 0) {
                    return nb::cast(self);
                } else if (t.size() == 1) {
                    auto t0 = nb::cast<nb::int_>(t[0]);
                    return nb::cast(getRow(self, t0));
                }
                auto t0 = nb::cast<nb::int_>(t[0]);
                auto t1 = nb::cast<nb::int_>(t[1]);
                return nb::cast(
                        self(python::convertIndex(3, t0),
                             python::convertIndex(3, t1)));
            },
            nb::is_operator());

    cls.def("inner", &Matrix3d::inner, "matrix"_a);
    cls.def("getSquaredNorm", &Matrix3d::getSquaredNorm);
    cls.def("getNorm", &Matrix3d::getNorm);

    cls.def("__mul__",
            (Vector3d(Matrix3d::*)(Vector3d const &) const) &
                    Matrix3d::operator*,
            "vector"_a, nb::is_operator());
    cls.def("__mul__",
            (Matrix3d(Matrix3d::*)(Matrix3d const &) const) &
                    Matrix3d::operator*,
            "matrix"_a, nb::is_operator());
    cls.def("__add__", &Matrix3d::operator+, nb::is_operator());
    cls.def("__sub__", &Matrix3d::operator-, nb::is_operator());

    cls.def("cwiseProduct", &Matrix3d::cwiseProduct);
    cls.def("transpose", &Matrix3d::transpose);
    cls.def("inverse", &Matrix3d::inverse);

    cls.def("__str__", [](Matrix3d const &self) {
        return nb::str("[[{!s}, {!s}, {!s}],\n"
                       " [{!s}, {!s}, {!s}],\n"
                       " [{!s}, {!s}, {!s}]]")
                .format(self(0, 0), self(0, 1), self(0, 2), self(1, 0),
                        self(1, 1), self(1, 2), self(2, 0), self(2, 1),
                        self(2, 2));
    });
    cls.def("__repr__", [](Matrix3d const &self) {
        return nb::str("Matrix3d({!r}, {!r}, {!r},\n"
                       "         {!r}, {!r}, {!r},\n"
                       "         {!r}, {!r}, {!r})")
                .format(self(0, 0), self(0, 1), self(0, 2), self(1, 0),
                        self(1, 1), self(1, 2), self(2, 0), self(2, 1),
                        self(2, 2));
    });
    cls.def("__reduce__", [cls](Matrix3d const &self) {
        auto args = nb::make_tuple(self(0, 0), self(0, 1), self(0, 2),
                                   self(1, 0), self(1, 1), self(1, 2),
                                   self(2, 0), self(2, 1), self(2, 2));
        return nb::make_tuple(cls, args);
    });
}

}  // sphgeom
}  // lsst
