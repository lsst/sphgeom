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
#include <nanobind/stl/vector.h>
#include <nanobind/stl/unique_ptr.h>
#include <nanobind/ndarray.h>

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/Region.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "lsst/sphgeom/python/relationship.h"
#include "lsst/sphgeom/python/utils.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(nb::class_<Region> &cls) {
    cls.def("clone", &Region::clone);
    cls.def("getBoundingBox", &Region::getBoundingBox);
    cls.def("getBoundingBox3d", &Region::getBoundingBox3d);
    cls.def("getBoundingCircle", &Region::getBoundingCircle);
    cls.def("contains", nb::overload_cast<UnitVector3d const &>(&Region::contains, nb::const_),
            "unitVector"_a);
    // vectorize
    cls.def("contains",
            nb::vectorize((bool (Region::*)(double, double, double) const)&Region::contains),
            nb::arg("x"), nb::arg("y"), nb::arg("z"));
    // vectorize
    cls.def("contains", nb::vectorize((bool (Region::*)(double, double) const)&Region::contains) ,
            nb::arg("lon"), nb::arg("lat"));
    cls.def("__contains__", nb::overload_cast<UnitVector3d const &>(&Region::contains, nb::const_),
            nb::is_operator());
    // The per-subclass relate() overloads are used to implement
    // double-dispatch in C++, and are not needed in Python.
    cls.def("relate",
            (Relationship(Region::*)(Region const &) const) & Region::relate,
            nb::arg("region"));
    cls.def("encode", &python::encode);
    cls.def_static("decode", &python::decode<Region>, nb::arg("bytes"));
}

}  // sphgeom
}  // lsst
