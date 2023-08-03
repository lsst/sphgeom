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
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "lsst/sphgeom/python.h"

#include "lsst/sphgeom/Chunker.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace lsst {
namespace sphgeom {

namespace {
nb::str toString(Chunker const &self) {
    return nb::str("Chunker({!s}, {!s})")
            .format(self.getNumStripes(), self.getNumSubStripesPerStripe());
}
}

template <>
void defineClass(nb::class_<Chunker> &cls) {
    cls.def(nb::init<int32_t, int32_t>(), nb::arg("numStripes"),
            nb::arg("numSubStripesPerStripe"));

    cls.def("__eq__", &Chunker::operator==, nb::is_operator());
    cls.def("__ne__", &Chunker::operator!=, nb::is_operator());

    cls.def_prop_ro("numStripes", &Chunker::getNumStripes);
    cls.def_prop_ro("numSubStripesPerStripe",
                              &Chunker::getNumSubStripesPerStripe);

    cls.def("getChunksIntersecting", &Chunker::getChunksIntersecting,
            nb::arg("region"));
    cls.def("getSubChunksIntersecting",
            [](Chunker const &self, Region const &region) {
                nb::list results;
                for (auto const &sc : self.getSubChunksIntersecting(region)) {
                    results.append(nb::make_tuple(sc.chunkId, sc.subChunkIds));
                }
                return results;
            },
            nb::arg("region"));
    cls.def("getAllChunks", &Chunker::getAllChunks);
    cls.def("getAllSubChunks", &Chunker::getAllSubChunks, nb::arg("chunkId"));

    cls.def("getChunkBoundingBox", &Chunker::getChunkBoundingBox, nb::arg("stripe"), nb::arg("chunk"));
    cls.def("getSubChunkBoundingBox", &Chunker::getSubChunkBoundingBox, nb::arg("subStripe"), nb::arg("subChunk"));

    cls.def("getStripe", &Chunker::getStripe, nb::arg("chunkId"));
    cls.def("getChunk", &Chunker::getChunk, nb::arg("chunkId"), nb::arg("stripe"));


    cls.def("__str__", &toString);
    cls.def("__repr__", &toString);

    cls.def("__reduce__", [cls](Chunker const &self) {
        return nb::make_tuple(cls,
                              nb::make_tuple(self.getNumStripes(),
                                             self.getNumSubStripesPerStripe()));
    });
}

}  // sphgeom
}  // lsst
