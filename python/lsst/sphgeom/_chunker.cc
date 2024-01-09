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
#include "pybind11/stl.h"

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Chunker.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace sphgeom {

namespace {
py::str toString(Chunker const &self) {
    return py::str("Chunker({!s}, {!s})")
            .format(self.getNumStripes(), self.getNumSubStripesPerStripe());
}
}

template <>
void defineClass(py::class_<Chunker, std::shared_ptr<Chunker>> &cls) {
    cls.def(py::init<int32_t, int32_t>(), "numStripes"_a,
            "numSubStripesPerStripe"_a);

    cls.def("__eq__", &Chunker::operator==, py::is_operator());
    cls.def("__ne__", &Chunker::operator!=, py::is_operator());

    cls.def_property_readonly("numStripes", &Chunker::getNumStripes);
    cls.def_property_readonly("numSubStripesPerStripe",
                              &Chunker::getNumSubStripesPerStripe);

    cls.def("getChunksIntersecting", &Chunker::getChunksIntersecting,
            "region"_a);
    cls.def("getSubChunksIntersecting",
            [](Chunker const &self, Region const &region) {
                py::list results;
                for (auto const &sc : self.getSubChunksIntersecting(region)) {
                    results.append(py::make_tuple(sc.chunkId, sc.subChunkIds));
                }
                return results;
            },
            "region"_a);
    cls.def("getAllChunks", &Chunker::getAllChunks);
    cls.def("getAllSubChunks", &Chunker::getAllSubChunks, "chunkId"_a);

    cls.def("getChunkBoundingBox", &Chunker::getChunkBoundingBox, "stripe"_a, "chunk"_a);
    cls.def("getSubChunkBoundingBox", &Chunker::getSubChunkBoundingBox, "subStripe"_a, "subChunk"_a);

    cls.def("getStripe", &Chunker::getStripe, "chunkId"_a);
    cls.def("getChunk", &Chunker::getChunk, "chunkId"_a, "stripe"_a);


    cls.def("__str__", &toString);
    cls.def("__repr__", &toString);

    cls.def("__reduce__", [cls](Chunker const &self) {
        return py::make_tuple(cls,
                              py::make_tuple(self.getNumStripes(),
                                             self.getNumSubStripesPerStripe()));
    });
}

}  // sphgeom
}  // lsst
