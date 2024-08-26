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

#ifndef LSST_SPHGEOM_CHUNKER_H_
#define LSST_SPHGEOM_CHUNKER_H_

/// \file
/// \brief This file declares a class for partitioning the sky into chunks
///        and sub-chunks.

#include <vector>
#include <cstdint>

#include "Angle.h"
#include "Box.h"


namespace lsst {
namespace sphgeom {

/// `SubChunks` represents a set of sub-chunks of a particular chunk.
///
/// TODO(smm): implement a more memory efficient representation than this.
struct SubChunks {
    std::int32_t chunkId;
    std::vector<std::int32_t> subChunkIds;

    SubChunks() : chunkId(-1) {}

    void swap(SubChunks & sc) {
        std::swap(chunkId, sc.chunkId);
        subChunkIds.swap(sc.subChunkIds);
    }
};


/// `Chunker` subdivides the unit sphere into longitude-latitude boxes.
///
/// The unit sphere is divided into latitude angle "stripes" of fixed
/// height H. For each stripe, a width W is computed such that any two points
/// in the stripe with longitude angles separated by at least W have angular
/// separation of at least H. The stripe is then broken into an integral number
/// of chunks of width at least W. The same procedure is used to obtain finer
/// subchunks - each stripe is broken into a configureable number of
/// equal-height "substripes", and each substripe is broken into equal-width
/// subchunks.
class Chunker {
public:
    Chunker(std::int32_t numStripes,
            std::int32_t numSubStripesPerStripe);

    bool operator==(Chunker const & c) const {
        return _numStripes == c._numStripes &&
               _numSubStripesPerStripe == c._numSubStripesPerStripe;
    }

    bool operator!=(Chunker const & c) const {
        return _numStripes != c._numStripes ||
               _numSubStripesPerStripe != c._numSubStripesPerStripe;
    }

    /// `getNumStripes` returns the number of fixed-height latitude intervals
    /// in the sky subdivision.
    std::int32_t getNumStripes() const {
        return _numStripes;
    }

    /// `getNumSubStripesPerStripe` returns the number of fixed-height latitude
    /// sub-intervals in each stripe.
    std::int32_t getNumSubStripesPerStripe() const {
        return _numSubStripesPerStripe;
    }

    /// `getChunksIntersecting` returns all the chunks that potentially
    /// intersect the given region.
    std::vector<std::int32_t> getChunksIntersecting(Region const & r) const;

    /// `getSubChunksIntersecting` returns all the sub-chunks that potentially
    /// intersect the given region.
    std::vector<SubChunks> getSubChunksIntersecting(Region const & r) const;

    /// `getAllChunks` returns the complete set of chunk IDs for the unit
    /// sphere.
    std::vector<std::int32_t> getAllChunks() const;

    /// `getAllSubChunks` returns the complete set of sub-chunk IDs
    /// for the given chunk.
    std::vector<std::int32_t> getAllSubChunks(std::int32_t chunkId) const;

    /// Return 'true' if the specified chunk number is valid
    bool valid(std::int32_t chunkId) const;

    Box getChunkBoundingBox(std::int32_t stripe, std::int32_t chunk) const;
    Box getSubChunkBoundingBox(std::int32_t subStripe, std::int32_t subChunk) const;

    /// Return the stripe for the specified chunkId
    std::int32_t getStripe(std::int32_t chunkId) const {
        return chunkId / (2 * _numStripes);
    }

    /// Return the chunk for the given chunkId and stripe
    std::int32_t getChunk(std::int32_t chunkId, std::int32_t stripe) const {
        return chunkId - stripe*2*_numStripes;
    }

private:
    struct Stripe {
        Angle chunkWidth;
        std::int32_t numChunksPerStripe;
        std::int32_t numSubChunksPerChunk;

        Stripe() :
            chunkWidth(0),
            numChunksPerStripe(0),
            numSubChunksPerChunk(0)
        {}
    };

    struct SubStripe {
        Angle subChunkWidth;
        std::int32_t numSubChunksPerChunk;

        SubStripe() : subChunkWidth(), numSubChunksPerChunk(0) {}
    };

    std::int32_t _getChunkId(std::int32_t stripe, std::int32_t chunk) const {
        return stripe * 2 * _numStripes + chunk;
    }

    std::int32_t _getSubChunkId(std::int32_t stripe, std::int32_t subStripe,
                           std::int32_t chunk, std::int32_t subChunk) const {
        std::int32_t y = subStripe - stripe * _numSubStripesPerStripe;
        std::int32_t x = subChunk -
                    chunk * _subStripes[subStripe].numSubChunksPerChunk;
        return y * _maxSubChunksPerSubStripeChunk + x;
    }

    void _getSubChunks(std::vector<SubChunks> & subChunks,
                       Region const & r,
                       NormalizedAngleInterval const & lon,
                       std::int32_t stripe,
                       std::int32_t chunk,
                       std::int32_t minSS,
                       std::int32_t maxSS) const;

    std::int32_t _numStripes;
    std::int32_t _numSubStripesPerStripe;
    std::int32_t _numSubStripes;
    std::int32_t _maxSubChunksPerSubStripeChunk;
    Angle _subStripeHeight;
    std::vector<Stripe> _stripes;
    std::vector<SubStripe> _subStripes;
};

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_CHUNKER_H_
