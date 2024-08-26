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
/// \brief This file contains the ConvexPolygon class implementation.

#include "lsst/sphgeom/ConvexPolygon.h"

#include <ostream>
#include <stdexcept>

#include "lsst/sphgeom/codec.h"
#include "lsst/sphgeom/orientation.h"

#include "ConvexPolygonImpl.h"


namespace lsst {
namespace sphgeom {

namespace {

char const * const FOUND_ANTIPODAL_POINT =
    "The convex hull of the given point set is the "
    "entire unit sphere";

char const * const NOT_ENOUGH_POINTS =
    "The convex hull of a point set containing less than "
    "3 distinct, non-coplanar points is not a convex polygon";

// `findPlane` rearranges the entries of `points` such that the first two
// entries are distinct. If it is unable to do so, or if it encounters any
// antipodal points in the process, an exception is thrown. It returns an
// iterator to the first point in the input that was not consumed during
// the search.
std::vector<UnitVector3d>::iterator findPlane(
    std::vector<UnitVector3d> & points)
{
    if (points.empty()) {
        throw std::invalid_argument(NOT_ENOUGH_POINTS);
    }
    // Starting with the first point v0, find a distinct second point v1.
    UnitVector3d & v0 = points[0];
    UnitVector3d & v1 = points[1];
    std::vector<UnitVector3d>::iterator const end = points.end();
    std::vector<UnitVector3d>::iterator v = points.begin() + 1;
    for (; v != end; ++v) {
        if (v0 == -*v) {
            throw std::invalid_argument(FOUND_ANTIPODAL_POINT);
        }
        if (v0 != *v) {
            break;
        }
    }
    if (v == end) {
        throw std::invalid_argument(NOT_ENOUGH_POINTS);
    }
    v1 = *v;
    return ++v;
}

// `findTriangle` rearranges the entries of `points` such that the first
// three entries have counter-clockwise orientation. If it is unable to do so,
// or if it encounters any antipodal points in the process, an exception is
// thrown. It returns an iterator to the first point in the input that was not
// consumed during the search.
std::vector<UnitVector3d>::iterator findTriangle(
    std::vector<UnitVector3d> & points)
{
    std::vector<UnitVector3d>::iterator v = findPlane(points);
    std::vector<UnitVector3d>::iterator const end = points.end();
    UnitVector3d & v0 = points[0];
    UnitVector3d & v1 = points[1];
    // Note that robustCross() gives a non-zero result for distinct,
    // non-antipodal inputs, and that normalization never maps a non-zero
    // vector to the zero vector.
    UnitVector3d n(v0.robustCross(v1));
    for (; v != end; ++v) {
        int ccw = orientation(v0, v1, *v);
        if (ccw > 0) {
            // We found a counter-clockwise triangle.
            break;
        } else if (ccw < 0) {
            // We found a clockwise triangle. Swap the first two vertices to
            // flip its orientation.
            std::swap(v0, v1);
            break;
        }
        // v, v0 and v1 are coplanar.
        if (*v == v0 || *v == v1) {
            continue;
        }
        if (*v == -v0 || *v == -v1) {
            throw std::invalid_argument(FOUND_ANTIPODAL_POINT);
        }
        // At this point, v, v0 and v1 are distinct and non-antipodal.
        // If v is in the interior of the great circle segment (v0, v1),
        // discard v and continue.
        int v0v = orientation(n, v0, *v);
        int vv1 = orientation(n, *v, v1);
        if (v0v == vv1) {
            continue;
        }
        int v0v1 = orientation(n, v0, v1);
        // If v1 is in the interior of (v0, v), replace v1 with v and continue.
        if (v0v1 == -vv1) {
            v1 = *v; continue;
        }
        // If v0 is in the interior of (v, v1), replace v0 with v and continue.
        if (-v0v == v0v1) {
            v0 = *v; continue;
        }
        // Otherwise, (v0, v1) ∪ (v1, v) and (v, v0) ∪ (v0, v1) both span
        // more than π radians of the great circle defined by the v0 and v1,
        // so there is a pair of antipodal points in the corresponding great
        // circle segment.
        throw std::invalid_argument(FOUND_ANTIPODAL_POINT);
    }
    if (v == end) {
        throw std::invalid_argument(NOT_ENOUGH_POINTS);
    }
    points[2] = *v;
    return ++v;
}

void computeHull(std::vector<UnitVector3d> & points) {
    typedef std::vector<UnitVector3d>::iterator VertexIterator;
    VertexIterator hullEnd = points.begin() + 3;
    VertexIterator const end = points.end();
    // Start with a triangular hull.
    for (VertexIterator v = findTriangle(points); v != end; ++v) {
        // Compute the hull of the current hull and v.
        //
        // Notice that if v is in the current hull, v can be ignored. If
        // -v is in the current hull, then the hull of v and the current hull
        // is not a convex polygon.
        //
        // Otherwise, let i and j be the first and second end-points of an edge
        // in the current hull. We define the orientation of vertex j with
        // respect to vertex v as orientation(v, i, j). When neither v or -v
        // is in the current hull, there must be a sequence of consecutive
        // hull vertices that do not have counter-clockwise orientation with
        // respect to v. Insert v before the first vertex in this sequence
        // and remove all vertices in the sequence except the last to obtain
        // a new, larger convex hull.
        VertexIterator i = hullEnd - 1;
        VertexIterator j = points.begin();
        // toCCW is the vertex before the transition to counter-clockwise
        // orientation with respect to v.
        VertexIterator toCCW = hullEnd;
        // fromCCW is the vertex at which orientation with respect to v
        // changes from counter-clockwise to clockwise or coplanar. It may
        // equal toCCW.
        VertexIterator fromCCW = hullEnd;
        // Compute the orientation of the first point in the current hull
        // with respect to v.
        bool const firstCCW = orientation(*v, *i, *j) > 0;
        bool prevCCW = firstCCW;
        // Compute the orientation of points in the current hull with respect
        // to v, starting with the second point. Update toCCW / fromCCW when
        // we transition to / from counter-clockwise orientation.
        for (i = j, ++j; j != hullEnd; i = j, ++j) {
            if (orientation(*v, *i, *j) > 0) {
                if (!prevCCW) {
                    toCCW = i;
                    prevCCW = true;
                }
            } else if (prevCCW) {
                fromCCW = j;
                prevCCW = false;
            }
        }
        // Now that we know the orientation of the last point in the current
        // hull with respect to v, consider the first point in the current hull.
        if (firstCCW) {
            if (!prevCCW) {
                toCCW = i;
            }
        } else if (prevCCW) {
            fromCCW = points.begin();
        }
        // Handle the case where there is never a transition to / from
        // counter-clockwise orientation.
        if (toCCW == hullEnd) {
            // If all vertices are counter-clockwise with respect to v,
            // v is inside the current hull and can be ignored.
            if (firstCCW) {
                continue;
            }
            // Otherwise, no vertex is counter-clockwise with respect to v,
            // so that -v is inside the current hull.
            throw std::invalid_argument(FOUND_ANTIPODAL_POINT);
        }
        // Insert v into the current hull at fromCCW, and remove
        // all vertices between fromCCW and toCCW.
        if (toCCW < fromCCW) {
            if (toCCW != points.begin()) {
                fromCCW = std::copy(toCCW, fromCCW, points.begin());
            }
            *fromCCW++ = *v;
            hullEnd = fromCCW;
        } else if (toCCW > fromCCW) {
            *fromCCW++ = *v;
            if (toCCW != fromCCW) {
                hullEnd = std::copy(toCCW, hullEnd, fromCCW);
            }
        } else {
            if (fromCCW == points.begin()) {
                *hullEnd = *v;
            } else {
                UnitVector3d u = *v;
                std::copy_backward(fromCCW, hullEnd, hullEnd + 1);
                *fromCCW = u;
            }
            ++hullEnd;
        }
    }
    points.erase(hullEnd, end);
}

// TODO(smm): for all of this to be fully rigorous, we must prove that no two
// UnitVector3d objects u and v are exactly colinear unless u == v or u == -v.
// It's not clear that this is true. For example, (1, 0, 0) and (1 + ε, 0, 0)
// are colinear. This means that UnitVector3d should probably always normalize
// on construction. Currently, it does not normalize when created from a LonLat,
// and also contains some escape-hatches for performance. The normalize()
// function implementation may also need to be revisited.

// TODO(smm): This implementation is quadratic. It would be nice to implement
// a fast hull merging algorithm, which could then be used to implement Chan's
// algorithm.

} // unnamed namespace


ConvexPolygon::ConvexPolygon(std::vector<UnitVector3d> const & points) :
    _vertices(points)
{
    computeHull(_vertices);
}

bool ConvexPolygon::operator==(ConvexPolygon const & p) const {
    if (this == &p) {
        return true;
    }
    if (_vertices.size() != p._vertices.size()) {
        return false;
    }
    VertexIterator i = _vertices.begin();
    VertexIterator f = p._vertices.begin();
    VertexIterator const ep = p._vertices.end();
    // Find vertex f of p equal to the first vertex of this polygon.
    for (; f != ep; ++f) {
        if (*i == *f) {
            break;
        }
    }
    if (f == ep) {
        // No vertex of p is equal to the first vertex of this polygon.
        return false;
    }
    // Now, compare all vertices.
    ++i;
    for (VertexIterator j = f + 1; j != ep; ++i, ++j) {
        if (*i != *j) {
            return false;
        }
    }
    for (VertexIterator j = p._vertices.begin(); j != f; ++i, ++j) {
        if (*i != *j) {
            return false;
        }
    }
    return true;
}

UnitVector3d ConvexPolygon::getCentroid() const {
    return detail::centroid(_vertices.begin(), _vertices.end());
}

Circle ConvexPolygon::getBoundingCircle() const {
    return detail::boundingCircle(_vertices.begin(), _vertices.end());
}

Box ConvexPolygon::getBoundingBox() const {
    return detail::boundingBox(_vertices.begin(), _vertices.end());
}

Box3d ConvexPolygon::getBoundingBox3d() const {
    return detail::boundingBox3d(_vertices.begin(), _vertices.end());
}

bool ConvexPolygon::contains(UnitVector3d const & v) const {
    return detail::contains(_vertices.begin(), _vertices.end(), v);
}

bool ConvexPolygon::contains(Region const & r) const {
    return (relate(r) & CONTAINS) != 0;
}

bool ConvexPolygon::isDisjointFrom(Region const & r) const {
    return (relate(r) & DISJOINT) != 0;
}

bool ConvexPolygon::intersects(Region const & r) const {
    return !isDisjointFrom(r);
}

bool ConvexPolygon::isWithin(Region const & r) const {
    return (relate(r) & WITHIN) != 0;
}

Relationship ConvexPolygon::relate(Box const & b) const {
    return detail::relate(_vertices.begin(), _vertices.end(), b);
}

Relationship ConvexPolygon::relate(Circle const & c) const {
    return detail::relate(_vertices.begin(), _vertices.end(), c);
}

Relationship ConvexPolygon::relate(ConvexPolygon const & p) const {
    return detail::relate(_vertices.begin(), _vertices.end(), p);
}

Relationship ConvexPolygon::relate(Ellipse const & e) const {
    return detail::relate(_vertices.begin(), _vertices.end(), e);
}

std::vector<std::uint8_t> ConvexPolygon::encode() const {
    std::vector<std::uint8_t> buffer;
    std::uint8_t tc = TYPE_CODE;
    buffer.reserve(1 + 24 * _vertices.size());
    buffer.push_back(tc);
    for (UnitVector3d const & v: _vertices) {
        encodeDouble(v.x(), buffer);
        encodeDouble(v.y(), buffer);
        encodeDouble(v.z(), buffer);
    }
    return buffer;
}

std::unique_ptr<ConvexPolygon> ConvexPolygon::decode(std::uint8_t const * buffer,
                                                     size_t n)
{
    if (buffer == nullptr || *buffer != TYPE_CODE ||
        n < 1 + 24*3 || (n - 1) % 24 != 0) {
        throw std::runtime_error("Byte-string is not an encoded ConvexPolygon");
    }
    std::unique_ptr<ConvexPolygon> poly(new ConvexPolygon);
    ++buffer;
    size_t nv = (n - 1) / 24;
    poly->_vertices.reserve(nv);
    for (size_t i = 0; i < nv; ++i, buffer += 24) {
        poly->_vertices.push_back(UnitVector3d::fromNormalized(
            decodeDouble(buffer),
            decodeDouble(buffer + 8),
            decodeDouble(buffer + 16)
        ));
    }
    return poly;
}

std::ostream & operator<<(std::ostream & os, ConvexPolygon const & p) {
    typedef std::vector<UnitVector3d>::const_iterator VertexIterator;
    VertexIterator v = p.getVertices().begin();
    VertexIterator const end = p.getVertices().end();
    os << "{\"ConvexPolygon\": [" << *v;
    for (++v; v != end; ++v) { os << ", " << *v; }
    os << "]}";
    return os;
}

}} // namespace lsst::sphgeom
