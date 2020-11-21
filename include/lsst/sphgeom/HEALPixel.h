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
/// \brief This file declares an abstract base class for implementing
///        HEALPix Pixelizations, including a concrete Region class for its
///        pixels.

#include <iosfwd>

#include "Region.h"
#include "UnitVector3d.h"


namespace lsst {
namespace sphgeom {

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
    uint64_t nested() const { return _nested; }

    /// Return the level (i.e. depth) of this pixel.
    int level() const { return _level; }

    bool operator==(HEALPixel const & p) const {
        return nested() == p.nested() && level() == p.level();
    }
    bool operator!=(HEALPixel const & p) const { return !(*this == p); }

    using Region::contains;

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

    virtual Relationship relate(UnitVector3d const & v) const = 0;

    // Region interface is left to subclasses, except for HEALPixel-HEALPixel
    // relations.
    using Region::relate;
    Relationship relate(HEALPixel const &) const override;

protected:

    /// Construct a HEALPixel for the given level and NESTED ID.
    explicit HEALPixel(int level, uint64_t nested)
        : _level(level), _nested(nested)
    {}

private:
    int _level;
    std::uint64_t _nested;
};

std::ostream & operator<<(std::ostream &, HEALPixel const &);


}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_HEALPIXEL_H_
