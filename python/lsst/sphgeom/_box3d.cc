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
#include "pybind11/numpy.h"

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/python/relationship.h"
#include "lsst/sphgeom/python/utils.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

template <>
void defineClass(py::class_<Box3d, std::shared_ptr<Box3d>> &cls) {
    cls.def_static("empty", &Box3d::empty);
    cls.def_static("full", &Box3d::full);
    cls.def_static("aroundUnitSphere", &Box3d::aroundUnitSphere);

    cls.def(py::init<>());
    cls.def(py::init<Vector3d const &>(), "vector"_a);
    cls.def(py::init<Vector3d const &, Vector3d const &>(), "vector1"_a,
            "vector2"_a);
    cls.def(py::init<Vector3d const &, double, double, double>(), "center"_a,
            "halfWidth"_a, "halfHeight"_a, "halfDepth"_a);
    cls.def(py::init<Interval1d const &, Interval1d const &,
                     Interval1d const &>(),
            "x"_a, "y"_a, "z"_a);
    cls.def(py::init<Box3d const &>(), "box3d"_a);

    cls.def("__eq__",
            (bool (Box3d::*)(Box3d const &) const) & Box3d::operator==,
            py::is_operator());
    cls.def("__eq__",
            (bool (Box3d::*)(Vector3d const &) const) & Box3d::operator==,
            py::is_operator());
    cls.def("__ne__",
            (bool (Box3d::*)(Box3d const &) const) & Box3d::operator!=,
            py::is_operator());
    cls.def("__ne__",
            (bool (Box3d::*)(Vector3d const &) const) & Box3d::operator!=,
            py::is_operator());
    cls.def("__contains__",
            (bool (Box3d::*)(Vector3d const &) const) & Box3d::contains,
            py::is_operator());
    cls.def("__contains__",
            (bool (Box3d::*)(Box3d const &) const) & Box3d::contains,
            py::is_operator());
    cls.def("__len__", [](Box3d const &self) { return py::int_(3); });
    cls.def("__getitem__", [](Box3d const &self, py::int_ row) {
        return self(static_cast<int>(python::convertIndex(3, row)));
    });

    cls.def("x", &Box3d::x);
    cls.def("y", &Box3d::y);
    cls.def("z", &Box3d::z);
    cls.def("isEmpty", &Box3d::isEmpty);
    cls.def("isFull", &Box3d::isFull);
    cls.def("getCenter", &Box3d::getCenter);
    cls.def("getWidth", &Box3d::getWidth);
    cls.def("getHeight", &Box3d::getHeight);
    cls.def("getDepth", &Box3d::getDepth);

    cls.def("contains",
            (bool (Box3d::*)(Vector3d const &) const) & Box3d::contains);
    cls.def("contains",
            (bool (Box3d::*)(Box3d const &) const) & Box3d::contains);
    cls.def("contains", py::vectorize((bool (Box3d::*)(double, double, double) const)&Box3d::contains),
            "x"_a, "y"_a, "z"_a);
    cls.def("isDisjointFrom",
            (bool (Box3d::*)(Vector3d const &) const) & Box3d::isDisjointFrom);
    cls.def("isDisjointFrom",
            (bool (Box3d::*)(Box3d const &) const) & Box3d::isDisjointFrom);
    cls.def("intersects",
            (bool (Box3d::*)(Vector3d const &) const) & Box3d::intersects);
    cls.def("intersects",
            (bool (Box3d::*)(Box3d const &) const) & Box3d::intersects);
    cls.def("isWithin",
            (bool (Box3d::*)(Vector3d const &) const) & Box3d::isWithin);
    cls.def("isWithin",
            (bool (Box3d::*)(Box3d const &) const) & Box3d::isWithin);

    cls.def("clipTo", (Box3d & (Box3d::*)(Vector3d const &)) & Box3d::clipTo);
    cls.def("clipTo", (Box3d & (Box3d::*)(Box3d const &)) & Box3d::clipTo);
    cls.def("clippedTo",
            (Box3d(Box3d::*)(Vector3d const &) const) & Box3d::clippedTo);
    cls.def("clippedTo",
            (Box3d(Box3d::*)(Box3d const &) const) & Box3d::clippedTo);
    cls.def("expandTo",
            (Box3d & (Box3d::*)(Vector3d const &)) & Box3d::expandTo);
    cls.def("expandTo", (Box3d & (Box3d::*)(Box3d const &)) & Box3d::expandTo);
    cls.def("expandedTo",
            (Box3d(Box3d::*)(Vector3d const &) const) & Box3d::expandedTo);
    cls.def("expandedTo",
            (Box3d(Box3d::*)(Box3d const &) const) & Box3d::expandedTo);

    cls.def("dilateBy", (Box3d & (Box3d::*)(double)) & Box3d::dilateBy,
            "radius"_a);
    cls.def("dilateBy",
            (Box3d & (Box3d::*)(double, double, double)) & Box3d::dilateBy,
            "width"_a, "height"_a, "depth"_a);
    cls.def("dilatedBy", (Box3d(Box3d::*)(double) const) & Box3d::dilatedBy,
            "radius"_a);
    cls.def("dilatedBy",
            (Box3d(Box3d::*)(double, double, double) const) & Box3d::dilatedBy,
            "width"_a, "height"_a, "depth"_a);
    cls.def("erodeBy", (Box3d & (Box3d::*)(double)) & Box3d::erodeBy,
            "radius"_a);
    cls.def("erodeBy",
            (Box3d & (Box3d::*)(double, double, double)) & Box3d::erodeBy,
            "width"_a, "height"_a, "depth"_a);
    cls.def("erodedBy", (Box3d(Box3d::*)(double) const) & Box3d::erodedBy,
            "radius"_a);
    cls.def("erodedBy",
            (Box3d(Box3d::*)(double, double, double) const) & Box3d::erodedBy,
            "width"_a, "height"_a, "depth"_a);

    cls.def("relate",
            (Relationship(Box3d::*)(Vector3d const &) const) & Box3d::relate);
    cls.def("relate",
            (Relationship(Box3d::*)(Box3d const &) const) & Box3d::relate);

    cls.def("__str__", [](Box3d const &self) {
        return py::str("[{!s},\n"
                       " {!s},\n"
                       " {!s}]")
                .format(self.x(), self.y(), self.z());
    });
    cls.def("__repr__", [](Box3d const &self) {
        return py::str("Box3d({!r},\n"
                       "      {!r},\n"
                       "      {!r})")
                .format(self.x(), self.y(), self.z());
    });
    cls.def("__reduce__", [cls](Box3d const &self) {
        return py::make_tuple(cls,
                              py::make_tuple(self.x(), self.y(), self.z()));
    });
}

}  // sphgeom
}  // lsst
