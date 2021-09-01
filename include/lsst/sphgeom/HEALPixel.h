/*
 * LSST Data Management System
 * Copyright 2014-2015 AURA/LSST.
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

#ifndef LSST_SPHGEOM_HEALPIXEL_H_
#define LSST_SPHGEOM_HEALPIXEL_H_

/// \file
/// \brief This file declares classes for implementing HEALPix Pixelizations,
///        including a still-abstract Region base class for its pixels.

#include <iosfwd>

#include "Region.h"
#include "UnitVector3d.h"


namespace lsst {
namespace sphgeom {

/// Implementation-agnostic state for all `HEALPixel` implementations.
///
/// Code that uses `HEALPixel` but does not provide an implementation of it
/// can ignore this class.
struct HEALPixelState {
    uint8_t level;
    uint64_t nested;
    UnitVector3d vertices[4];
};

/// A `HEALPixel` is a cell in the Hierarchical Equal Area Pixelization
/// (HEALPix) scheme.   HEALPix is not implemented in the sphgeom package
/// directly (and multiple downstream implementations may coexist), but this
/// interface is included because all `Region` classes must be known to
/// each other in order to participate in its double-dispatch topological
/// operators.
class HEALPixel : public Region {
public:
    static constexpr uint8_t TYPE_CODE = 'h';

    /// Return the NESTED-system ID for this pixel.
    uint64_t nested() const { return _state.nested; }

    /// Return the level (i.e. depth) of this pixel.
    int level() const { return _state.level; }

    bool operator==(HEALPixel const & p) const {
        return nested() == p.nested() && level() == p.level();
    }
    bool operator!=(HEALPixel const & p) const { return !(*this == p); }

    bool contains(UnitVector3d const & x) const override;

    /// `contains` returns true if the intersection of this HEALPixel and x
    /// is equal to x.
    bool contains(HEALPixel const & x) const;

    ///@{
    /// `isDisjointFrom` returns true if the intersection of this HEALPixel and
    /// x is empty.
    bool isDisjointFrom(UnitVector3d const & x) const { return !contains(x); }
    bool isDisjointFrom(HEALPixel const & x) const;
    ///@}

    ///@{
    /// `intersects` returns true if the intersection of this HEALPixel and x
    /// is non-empty.
    bool intersects(UnitVector3d const & x) const { return contains(x); }
    bool intersects(HEALPixel const & x) const { return !isDisjointFrom(x); }
    ///@}

    ///@{
    /// `isWithin` returns true if the intersection of this HEALPixel and x
    /// is this HEALPixel.
    bool isWithin(UnitVector3d const &) const { return false; }
    bool isWithin(HEALPixel const & x) const { return x.contains(*this); }
    ///@}

    /// `getArea` returns the area of this HEALPix in steradians.
    virtual double getArea() const = 0;

    // Region interface is left to subclasses, except for HEALPixel-HEALPixel
    // relations and the dispatch method for generic Region.
    using Region::relate;
    Relationship relate(Region const &) const override;
    Relationship relate(HEALPixel const &) const override;

    // Encode is implemented to save level and NESTED ID.
    std::vector<uint8_t> encode() const;

    /// Function pointer type for registerDecodeFunction.
    typedef std::unique_ptr<HEALPixel>(*DecodeFunction)(HEALPixelState const & state);

    /// Register a function pointer to call when a encoded HEALPixel is
    /// encountered by `decode`.
    ///
    /// New registrations override previous ones.
    static void registerDecodeFunction(DecodeFunction func);

    /// Deserialize a byte string produced by `encode` into the
    /// implementation-agnostic state struct.
    static HEALPixelState decodeState(uint8_t const * buffer, size_t n);

    ///@{
    /// `decode` deserializes a HEALPixel from a byte string produced by
    /// encode.
    ///
    /// This first calls `decodeState`, then delegates the rest to the callback
    /// registered with `registerDecodeFunction`.
    static std::unique_ptr<HEALPixel> decode(std::vector<uint8_t> const & s) {
        return decode(s.data(), s.size());
    }
    static std::unique_ptr<HEALPixel> decode(uint8_t const * buffer, size_t n);
    ///@}

protected:
    static constexpr size_t ENCODED_SIZE = 106;

    /// Construct a HEALPixel for the given level and NESTED ID.
    explicit HEALPixel(HEALPixelState const & state)
        : _state(state)
    {}

    HEALPixelState const _state;
};

std::ostream & operator<<(std::ostream &, HEALPixel const &);


}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_HEALPIXEL_H_
