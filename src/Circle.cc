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

/// \file
/// \brief This file contains the Circle class implementation.

#include "lsst/sphgeom/Circle.h"

#include <ostream>
#include <stdexcept>

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/ConvexPolygon.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/codec.h"


namespace lsst {
namespace sphgeom {

double Circle::squaredChordLengthFor(Angle a) {
    if (a.asRadians() < 0.0) {
        return -1.0;
    }
    if (a.asRadians() >= PI) {
        return 4.0;
    }
    double s = sin(0.5 * a);
    return 4.0 * s * s;
}

Angle Circle::openingAngleFor(double squaredChordLength) {
    // Note: the maximum error in the opening angle (and circle bounding box
    // width) computations is ~ 2 * MAX_ASIN_ERROR.
    if (squaredChordLength < 0.0) {
        return Angle(-1.0);
    }
    if (squaredChordLength >= 4.0) {
        return Angle(PI);
    }
    return Angle(2.0 * std::asin(0.5 * std::sqrt(squaredChordLength)));
}

bool Circle::contains(Circle const & x) const {
    if (isFull() || x.isEmpty()) {
        return true;
    }
    if (isEmpty() || x.isFull()) {
        return false;
    }
    if (*this == x) {
        return true;
    }
    NormalizedAngle cc(_center, x._center);
    return _openingAngle >
           cc + x._openingAngle + 4.0 * Angle(MAX_ASIN_ERROR);
}

bool Circle::isDisjointFrom(Circle const & x) const {
    if (isEmpty() || x.isEmpty()) {
        return true;
    }
    if (isFull() || x.isFull()) {
        return false;
    }
    NormalizedAngle cc(_center, x._center);
    return cc > _openingAngle + x._openingAngle +
                4.0 * Angle(MAX_ASIN_ERROR);
}

Circle & Circle::clipTo(UnitVector3d const & x) {
    *this = contains(x) ? Circle(x) : empty();
    return *this;
}

Circle & Circle::clipTo(Circle const & x) {
    if (isEmpty() || x.isFull()) {
        return *this;
    }
    if (isFull() || x.isEmpty()) {
        *this = x;
        return *this;
    }
    Angle a = _openingAngle;
    Angle b = x._openingAngle;
    NormalizedAngle cc(_center, x._center);
    if (cc > a + b + 4.0 * Angle(MAX_ASIN_ERROR)) {
        // This circle is disjoint from x.
        *this = empty();
        return *this;
    }
    // The circles (nearly) intersect, or one contains the other.
    // For now, take the easy route and just use the smaller of
    // the two circles as a bound on their intersection.
    //
    // TODO(smm): Compute the minimal bounding circle.
    if (b < a) {
        *this = x;
    }
    return *this;
}

Circle & Circle::expandTo(UnitVector3d const & x) {
    // For any circle c and unit vector x, c.expandTo(x).contains(x)
    // should return true.
    if (isEmpty()) {
        *this = Circle(x);
    } else if (!contains(x)) {
        // Compute the normal vector for the plane defined by _center and x.
        UnitVector3d n = UnitVector3d::orthogonalTo(_center, x);
        // The minimal bounding circle (MBC) includes unit vectors on the plane
        // with normal n that span from _center.rotatedAround(n, -_openingAngle)
        // to x. The MBC center is the midpoint of this interval.
        NormalizedAngle cx(_center, x);
        Angle o = 0.5 * (cx + _openingAngle);
        Angle r = 0.5 * (cx - _openingAngle);
        // Rotate _center by angle r around n to obtain the MBC center. This is
        // done using Rodriques' formula, simplified by taking advantage of the
        // orthogonality of _center and n.
        _center = UnitVector3d(_center * cos(r) + n.cross(_center) * sin(r));
        _squaredChordLength = squaredChordLengthFor(o + Angle(MAX_ASIN_ERROR));
        _openingAngle = o + Angle(MAX_ASIN_ERROR);
    }
    return *this;
}

Circle & Circle::expandTo(Circle const & x) {
    if (isEmpty() || x.isFull()) {
        *this = x;
        return *this;
    }
    if (x.isEmpty() || isFull()) {
        return *this;
    }
    NormalizedAngle cc(_center, x._center);
    if (cc + x._openingAngle + 4.0 * Angle(MAX_ASIN_ERROR) <= _openingAngle) {
        // This circle contains x.
        return *this;
    }
    if (cc + _openingAngle + 4.0 * Angle(MAX_ASIN_ERROR) <= x._openingAngle) {
        // x contains this circle.
        *this = x;
        return *this;
    }
    // The circles intersect or are disjoint.
    Angle o = 0.5 * (cc + _openingAngle + x._openingAngle);
    if (o + 2.0 * Angle(MAX_ASIN_ERROR) >= Angle(PI)) {
        *this = full();
        return *this;
    }
    // Compute the normal vector for the plane defined by the circle centers.
    UnitVector3d n = UnitVector3d::orthogonalTo(_center, x._center);
    // The minimal bounding circle (MBC) includes unit vectors on the plane
    // with normal n that span from _center.rotatedAround(n, -_openingAngle)
    // to x._center.rotatedAround(n, x._openingAngle). The MBC center is the
    // midpoint of this interval.
    Angle r = o - _openingAngle;
    // Rotate _center by angle r around n to obtain the MBC center. This is
    // done using Rodriques' formula, simplified by taking advantage of the
    // orthogonality of _center and n.
    _center = UnitVector3d(_center * cos(r) + n.cross(_center) * sin(r));
    _squaredChordLength = squaredChordLengthFor(o + Angle(MAX_ASIN_ERROR));
    _openingAngle = o + Angle(MAX_ASIN_ERROR);
    return *this;
}

Circle & Circle::dilateBy(Angle r) {
    if (!isEmpty() && !isFull() &&
        (r.asRadians() > 0.0 || r.asRadians() < 0.0)) {
        Angle o = _openingAngle + r;
        _squaredChordLength = squaredChordLengthFor(o);
        _openingAngle = o;
    }
    return *this;
}

Circle & Circle::complement() {
    if (isEmpty()) {
        // The complement of an empty circle is a full circle.
        _squaredChordLength = 4.0;
        _openingAngle = Angle(PI);
    } else if (isFull()) {
        // The complement of a full circle is an empty circle.
        _squaredChordLength = -1.0;
        _openingAngle = Angle(-1.0);
    } else {
        _center = -_center;
        _squaredChordLength = 4.0 - _squaredChordLength;
        _openingAngle = Angle(PI) - _openingAngle;
    }
    return *this;
}

Box Circle::getBoundingBox() const {
    LonLat c(_center);
    Angle h = _openingAngle + 2.0 * Angle(MAX_ASIN_ERROR);
    NormalizedAngle w(Box::halfWidthForCircle(h, c.getLat()) +
                      Angle(MAX_ASIN_ERROR));
    return Box(c, w, h);
}

Box3d Circle::getBoundingBox3d() const {
    static double const MAX_BOUNDARY_ERROR = 6.2e-16; // > 5.5ε, where ε = 2^-53
    if (isEmpty()) {
        return Box3d();
    }
    if (isFull()) {
        return Box3d::aroundUnitSphere();
    }
    // Given circle center c and standard basis vector eᵢ, to check whether
    // ±eᵢ is inside the circle we need to check that (c ∓ eᵢ)·(c ∓ eᵢ) ≤ s.
    // Since c·c = 1, eᵢ·eᵢ = 1 (c and eᵢ are unit vectors) this is the
    // same as checking that 2 ∓ 2c·eᵢ ≤ s, or 2 ∓ 2cᵢ ≤ s, where cᵢ is
    // the i-th component of c.
    //
    // Besides any standard basis vectors inside the circle, the bounding box
    // must also include the circle boundary. To find the extent of this
    // boundary along a particular axis, note that we can write the i-th
    // component of the circle center vector as the sine of a latitude angle
    // (treating the i-th standard basis vector as "north"). So given a circle
    // opening angle θ, the desired extent is
    //
    //     [min(sin(asin(cᵢ) ± θ)), max(sin(asin(cᵢ) ± θ))]
    //
    // which can be simplified using the usual trigonometric identities to
    // arrive at the code below.
    Interval1d e[3];
    double s = sin(_openingAngle);
    double c = cos(_openingAngle);
    for (int i = 0; i < 3; ++i) {
        double ci = _center(i);
        double di = std::sqrt(1.0 - ci * ci);
        double bmin = 1.0, bmax = -1.0;
        if (2.0 - 2.0 * ci <= _squaredChordLength) {
            bmax = 1.0;
        }
        if (2.0 + 2.0 * ci <= _squaredChordLength) {
            bmin = -1.0;
        }
        double b0 = ci * c + di * s;
        bmax = std::max(bmax, b0 + MAX_BOUNDARY_ERROR);
        bmin = std::min(bmin, b0 - MAX_BOUNDARY_ERROR);
        double b1 = ci * c - di * s;
        bmax = std::max(bmax, b1 + MAX_BOUNDARY_ERROR);
        bmin = std::min(bmin, b1 - MAX_BOUNDARY_ERROR);
        e[i] = Interval1d(std::max(-1.0, bmin), std::min(1.0, bmax));
    }
    return Box3d(e[0], e[1], e[2]);
}

Relationship Circle::relate(UnitVector3d const & v) const {
    if (contains(v)) {
        return CONTAINS;
    } else if (isEmpty()) {
        return DISJOINT | WITHIN;
    }
    return DISJOINT;
}

Relationship Circle::relate(Box const & b) const {
    // Box-Circle relations are implemented by Box.
    return invert(b.relate(*this));
}

Relationship Circle::relate(Circle const & c) const {
    if (isEmpty()) {
        if (c.isEmpty()) {
            return CONTAINS | DISJOINT | WITHIN;
        }
        return DISJOINT | WITHIN;
    } else if (c.isEmpty()) {
        return CONTAINS | DISJOINT;
    }
    // Neither circle is empty.
    if (isFull()) {
        if (c.isFull()) {
            return CONTAINS | WITHIN;
        }
        return CONTAINS;
    } else if (c.isFull()) {
        return WITHIN;
    }
    // Special case equality, which can be missed by logic below due to
    // round-off error.
    if (*this == c) {
        return CONTAINS | WITHIN;
    }
    // Neither circle is full.
    NormalizedAngle cc(_center, c._center);
    if (cc > _openingAngle + c._openingAngle + 4.0 * Angle(MAX_ASIN_ERROR)) {
        return DISJOINT;
    }
    if (cc + c._openingAngle + 4.0 * Angle(MAX_ASIN_ERROR) <= _openingAngle) {
        return CONTAINS;
    } else if (cc + _openingAngle + 4.0 * Angle(MAX_ASIN_ERROR) <=
               c._openingAngle) {
        return WITHIN;
    }
    return INTERSECTS;
}

Relationship Circle::relate(ConvexPolygon const & p) const {
    // ConvexPolygon-Circle relations are implemented by ConvexPolygon.
    return invert(p.relate(*this));
}

Relationship Circle::relate(Ellipse const & e) const {
    // Ellipse-Circle relations are implemented by Ellipse.
    return invert(e.relate(*this));
}

TriState Circle::overlaps(Box const & b) const {
    // Box-Circle relations are implemented by Box.
    return b.overlaps(*this);
}

TriState Circle::overlaps(Circle const & c) const {
    return TriState(not this->isDisjointFrom(c));
}

TriState Circle::overlaps(ConvexPolygon const & p) const {
    // ConvexPolygon-Circle relations are implemented by ConvexPolygon.
    return p.overlaps(*this);
}

TriState Circle::overlaps(Ellipse const & e) const {
    // Ellipse-Circle relations are implemented by Ellipse.
    return e.overlaps(*this);
}

std::vector<std::uint8_t> Circle::encode() const {
    std::vector<std::uint8_t> buffer;
    std::uint8_t tc = TYPE_CODE;
    buffer.reserve(ENCODED_SIZE);
    buffer.push_back(tc);
    encodeDouble(_center.x(), buffer);
    encodeDouble(_center.y(), buffer);
    encodeDouble(_center.z(), buffer);
    encodeDouble(_squaredChordLength, buffer);
    encodeDouble(_openingAngle.asRadians(), buffer);
    return buffer;
}

std::unique_ptr<Circle> Circle::decode(std::uint8_t const * buffer, size_t n) {
    if (buffer == nullptr || n != ENCODED_SIZE || *buffer != TYPE_CODE) {
        throw std::runtime_error("Byte-string is not an encoded Circle");
    }
    std::unique_ptr<Circle> circle(new Circle);
    ++buffer;
    double x = decodeDouble(buffer); buffer += 8;
    double y = decodeDouble(buffer); buffer += 8;
    double z = decodeDouble(buffer); buffer += 8;
    double squaredChordLength = decodeDouble(buffer); buffer += 8;
    double openingAngle = decodeDouble(buffer); buffer += 8;
    circle->_center = UnitVector3d::fromNormalized(x, y, z);
    circle->_squaredChordLength = squaredChordLength;
    circle->_openingAngle = Angle(openingAngle);
    return circle;
}

std::ostream & operator<<(std::ostream & os, Circle const & c) {
    char tail[32];
    std::snprintf(tail, sizeof(tail), ", %.17g]}", c.getSquaredChordLength());
    return os << "{\"Circle\": [" << c.getCenter() << tail;
}

}} // namespace lsst::sphgeom
