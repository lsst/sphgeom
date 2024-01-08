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
#include "pybind11/pybind11.h"

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Angle.h"
#include "lsst/sphgeom/AngleInterval.h"
#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Chunker.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/CompoundRegion.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/HtmPixelization.h"
#include "lsst/sphgeom/Interval1d.h"
#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/Matrix3d.h"
#include "lsst/sphgeom/Mq3cPixelization.h"
#include "lsst/sphgeom/NormalizedAngle.h"
#include "lsst/sphgeom/NormalizedAngleInterval.h"
#include "lsst/sphgeom/Pixelization.h"
#include "lsst/sphgeom/Q3cPixelization.h"
#include "lsst/sphgeom/RangeSet.h"
#include "lsst/sphgeom/Region.h"
#include "lsst/sphgeom/UnitVector3d.h"
#include "lsst/sphgeom/Vector3d.h"

namespace py = pybind11;

namespace lsst {
namespace sphgeom {

void defineCurve(py::module&);
void defineOrientation(py::module&);
void defineRelationship(py::module&);
void defineUtils(py::module&);

namespace {

PYBIND11_MODULE(_sphgeom, mod) {
    // Create all Python class instances up front, then define them.
    //
    // This results in docstrings containing only Python type names, even
    // when there are circular dependencies in C++.

    py::class_<Angle> angle(mod, "Angle");
    py::class_<NormalizedAngle> normalizedAngle(mod, "NormalizedAngle");
    py::class_<LonLat, std::shared_ptr<LonLat>> lonLat(mod, "LonLat");
    py::class_<Vector3d, std::shared_ptr<Vector3d>> vector3d(mod, "Vector3d");
    py::class_<UnitVector3d, std::shared_ptr<UnitVector3d>> unitVector3d(
            mod, "UnitVector3d");
    py::class_<Matrix3d, std::shared_ptr<Matrix3d>> matrix3d(mod, "Matrix3d");

    py::class_<AngleInterval, std::shared_ptr<AngleInterval>> angleInterval(
            mod, "AngleInterval");
    py::class_<NormalizedAngleInterval,
               std::shared_ptr<NormalizedAngleInterval>>
            normalizedAngleInterval(mod, "NormalizedAngleInterval");
    py::class_<Interval1d, std::shared_ptr<Interval1d>> interval1d(
            mod, "Interval1d");

    py::class_<Box3d, std::shared_ptr<Box3d>> box3d(mod, "Box3d");

    py::class_<Region, std::unique_ptr<Region>> region(mod, "Region");
    py::class_<Box, std::unique_ptr<Box>, Region> box(mod, "Box");
    py::class_<Circle, std::unique_ptr<Circle>, Region> circle(mod, "Circle");
    py::class_<ConvexPolygon, std::unique_ptr<ConvexPolygon>, Region>
            convexPolygon(mod, "ConvexPolygon");
    py::class_<Ellipse, std::unique_ptr<Ellipse>, Region> ellipse(mod,
                                                                  "Ellipse");
    py::class_<CompoundRegion, std::unique_ptr<CompoundRegion>, Region> compoundRegion(mod, "CompoundRegion");
    py::class_<UnionRegion, std::unique_ptr<UnionRegion>, CompoundRegion> unionRegion(mod, "UnionRegion");
    py::class_<IntersectionRegion, std::unique_ptr<IntersectionRegion>, CompoundRegion>
            intersectionRegion(mod, "IntersectionRegion");

    py::class_<RangeSet, std::shared_ptr<RangeSet>> rangeSet(mod, "RangeSet");

    py::class_<Pixelization> pixelization(mod, "Pixelization");
    py::class_<HtmPixelization, Pixelization> htmPixelization(
            mod, "HtmPixelization");
    py::class_<Mq3cPixelization, Pixelization> mq3cPixelization(
            mod, "Mq3cPixelization");
    py::class_<Q3cPixelization, Pixelization> q3cPixelization(
            mod, "Q3cPixelization");

    py::class_<Chunker, std::shared_ptr<Chunker>> chunker(mod, "Chunker");

    defineClass(angle);
    defineClass(normalizedAngle);
    defineClass(lonLat);
    defineClass(vector3d);
    defineClass(unitVector3d);
    defineClass(matrix3d);

    defineClass(angleInterval);
    defineClass(normalizedAngleInterval);
    defineClass(interval1d);

    defineClass(box3d);

    defineClass(region);
    defineClass(box);
    defineClass(circle);
    defineClass(convexPolygon);
    defineClass(ellipse);
    defineClass(compoundRegion);
    defineClass(unionRegion);
    defineClass(intersectionRegion);

    defineClass(rangeSet);

    defineClass(pixelization);
    defineClass(htmPixelization);
    defineClass(mq3cPixelization);
    defineClass(q3cPixelization);

    defineClass(chunker);

    // Define C++ functions.

    defineCurve(mod);
    defineOrientation(mod);
    defineRelationship(mod);
    defineUtils(mod);
}

}  // <anonymous>
}  // sphgeom
}  // lsst
