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

#include "lsst/sphgeom/Interval1d.h"
#include "lsst/sphgeom/python/interval.h"

namespace nb = nanobind;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(nb::class_<Interval1d> &cls) {
    python::defineInterval<decltype(cls), Interval1d, double>(cls);

    cls.def_static("empty", &Interval1d::empty);
    cls.def_static("full", &Interval1d::full);

    cls.def(nb::init<>());
    cls.def(nb::init<double>(), "x"_a);
    cls.def(nb::init<double, double>(), "x"_a, "y"_a);
    cls.def(nb::init<Interval1d const &>(), "interval"_a);

    cls.def("isFull", &Interval1d::isFull);

    cls.def("__str__", [](Interval1d const &self) {
        return nb::str("[{!s}, {!s}]").format(self.getA(), self.getB());
    });
    cls.def("__repr__", [](Interval1d const &self) {
        return nb::str("Interval1d({!r}, {!r})")
                .format(self.getA(), self.getB());
    });
}

}  // sphgeom
}  // lsst
