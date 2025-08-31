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

#include "lsst/sphgeom/Q3cPixelization.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(nb::class_<Q3cPixelization, Pixelization> &cls) {
    cls.attr("MAX_LEVEL") = nb::int_(Q3cPixelization::MAX_LEVEL);

    cls.def(nb::init<int>(), "level"_a);
    cls.def(nb::init<Q3cPixelization const &>(), "q3cPixelization"_a);

    cls.def("getLevel", &Q3cPixelization::getLevel);
    cls.def("quad", &Q3cPixelization::quad);
    cls.def("neighborhood", &Q3cPixelization::neighborhood);

    cls.def("__eq__",
            [](Q3cPixelization const &self, Q3cPixelization const &other) {
                return self.getLevel() == other.getLevel();
            });
    cls.def("__ne__",
            [](Q3cPixelization const &self, Q3cPixelization const &other) {
                return self.getLevel() != other.getLevel();
            });
    cls.def("__repr__", [](Q3cPixelization const &self) {
        return nb::str("Q3cPixelization({!s})").format(self.getLevel());
    });
    cls.def("__reduce__", [cls](Q3cPixelization const &self) {
        return nb::make_tuple(cls, nb::make_tuple(self.getLevel()));
    });
}

}  // sphgeom
}  // lsst
