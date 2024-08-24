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

#ifndef LSST_SPHGEOM_MQ3CPIXELIZATION_H_
#define LSST_SPHGEOM_MQ3CPIXELIZATION_H_

/// \file
/// \brief This file declares a Pixelization subclass for the modified Q3C
///        indexing scheme.

#include <cstdint>
#include <vector>

#include "ConvexPolygon.h"
#include "Pixelization.h"


namespace lsst {
namespace sphgeom {

/// `Mq3cPixelization` provides [modified Q3C indexing](\ref q3c-modified)
/// of points and regions.
///
/// Instances of this class are immutable and very cheap to copy.
///
/// \warning Setting the `maxRanges` argument for envelope() or interior()
/// to a non-zero value below 4 can result in very poor region pixelizations
/// regardless of region size. For instance, if `maxRanges` is 1, a non-empty
/// circle centered on an axis will be approximated by the indexes for an
/// entire cube face, even as the circle radius tends to 0.
class Mq3cPixelization : public Pixelization {
public:
    /// The maximum supported cube-face grid resolution is 2^30 by 2^30.
    static constexpr int MAX_LEVEL = 30;

    /// `level` returns the subdivision level of the given modified Q3C index.
    ///
    /// If i is not a valid modified Q3C index, -1 is returned.
    static int level(std::uint64_t i);

    /// `quad` returns the quadrilateral corresponding to the modified Q3C
    /// pixel with index `i`.
    ///
    /// If `i` is not a valid modified Q3C index, a std::invalid_argument
    /// is thrown.
    static ConvexPolygon quad(std::uint64_t i);

    /// `neighborhood` returns the indexes of all pixels that share a vertex
    /// with pixel `i` (including `i` itself). A Q3C pixel has 8 - k adjacent
    /// pixels, where k is the number of vertices that are also root pixel
    /// vertices (0, 1, or 4).
    ///
    /// If `i` is not a valid modified Q3C index, a std::invalid_argument
    /// is thrown.
    static std::vector<std::uint64_t> neighborhood(std::uint64_t i);

    /// `toString` converts the given modified-Q3C index to a human readable
    /// string.
    ///
    /// The first two characters in the return value are always '+X', '+Y',
    /// '+Z', '-X', '-Y', or '-Z'. They give the normal vector of the cube
    /// face F containing `i`. Each subsequent character is a digit in [0-3]
    /// corresponding to a child pixel index, so that reading the string
    /// from left to right corresponds to descent of the quad-tree overlaid
    /// on F.
    ///
    /// If i is not a valid modified-Q3C index, a std::invalid_argument is
    /// thrown.
    static std::string asString(std::uint64_t i);

    /// This constructor creates a modified Q3C pixelization of the sphere
    /// with the given subdivision level. If `level` ∉ [0, MAX_LEVEL],
    /// a std::invalid_argument is thrown.
    explicit Mq3cPixelization(int level);

    /// `getLevel` returns the subdivision level of this pixelization.
    int getLevel() const { return _level; }

    RangeSet universe() const override {
        return RangeSet(static_cast<std::uint64_t>(10) << 2 * _level,
                        static_cast<std::uint64_t>(16) << 2 * _level);
    }

    std::unique_ptr<Region> pixel(std::uint64_t i) const override;

    std::uint64_t index(UnitVector3d const & v) const override;

    std::string toString(std::uint64_t i) const override { return asString(i); }

private:
    int _level;

    RangeSet _envelope(Region const & r, size_t maxRanges) const override;
    RangeSet _interior(Region const & r, size_t maxRanges) const override;
};

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_MQ3CPIXELIZATION_H_
