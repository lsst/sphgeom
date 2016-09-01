/*
 * LSST Data Management System
 * Copyright 2016 AURA/LSST.
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

#ifndef LSST_SPHGEOM_HTMPIXELIZATION_H_
#define LSST_SPHGEOM_HTMPIXELIZATION_H_

/// \file
/// \brief This file declares a Pixelization subclass for the HTM
///        indexing scheme.

#include <cstdint>
#include <stdexcept>

#include "ConvexPolygon.h"
#include "Pixelization.h"


namespace lsst {
namespace sphgeom {

/// `HtmPixelization` provides [HTM indexing](\ref htm-overview) of points
/// and regions.
///
/// Instances of this class are immutable and very cheap to copy.
///
/// \warning Setting the `maxRanges` argument for envelope() or interior() to
/// a non-zero value below 6 can result in very poor region pixelizations
/// regardless of region size. For instance, if `maxRanges` is 1, a non-empty
/// circle centered on an axis will be approximated by a hemisphere or the
/// entire unit sphere, even as its radius tends to 0.
class HtmPixelization : public Pixelization {
public:
    /// `MAX_LEVEL` is the maximum supported HTM subdivision level.
    static constexpr int MAX_LEVEL = 24;

    /// `level` returns the subdivision level of the given HTM index.
    ///
    /// If i is not a valid HTM index, -1 is returned.
    static int level(uint64_t i);

    /// `triangle` returns the triangle corresponding to the given HTM index.
    ///
    /// If i is not a valid HTM index, a std::invalid_argument is thrown.
    static ConvexPolygon triangle(uint64_t i);

    /// `asString` converts the given HTM index to a human readable string.
    ///
    /// The first character in the return value is always 'N' or 'S',
    /// indicating whether the root triangle containing `i` is in the northern
    /// or southern hemisphere. The second character is the index of the root
    /// triangle within that hemisphere (a digit in [0-3]). Each subsequent
    /// character is a digit in [0-3] corresponding to a child trixel index,
    /// so that reading the string from left to right corresponds to descent
    /// of the HTM triangle-tree.
    ///
    /// If i is not a valid HTM index, a std::invalid_argument is thrown.
    static std::string asString(uint64_t i);

    /// This constructor creates an HTM pixelization of the sphere with
    /// the given subdivision level. If `level` ∉ [0, MAX_LEVEL],
    /// a std::invalid_argument is thrown.
    explicit HtmPixelization(int level);

    /// `getLevel` returns the subdivision level of this pixelization.
    int getLevel() const { return _level; }

    RangeSet universe() const override {
        return RangeSet(static_cast<uint64_t>(8) << 2 * _level,
                        static_cast<uint64_t>(16) << 2 * _level);
    }

    std::unique_ptr<Region> pixel(uint64_t i) const override {
        return std::unique_ptr<Region>(new ConvexPolygon(triangle(i)));
    }

    uint64_t index(UnitVector3d const &) const override;

    std::string toString(uint64_t i) const override { return asString(i); }

private:
    int _level;

    RangeSet _envelope(Region const &, size_t) const override;
    RangeSet _interior(Region const &, size_t) const override;
};

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_HTMPIXELIZATION_H_
