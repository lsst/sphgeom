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

/// \file
/// \brief This file contains the ConvexPolygon class implementation.

#include "lsst/sphgeom/ConvexPolygon.h"

#include <ostream>

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/Orientation.h"
#include "lsst/sphgeom/Utils.h"


namespace lsst {
namespace sphgeom {

namespace {

char const * const FOUND_ANTIPODAL_POINT =
    "The convex hull of the given point set is the "
    "entire unit sphere";

char const * const NOT_ENOUGH_POINTS =
    "The convex hull of a point set containing less than "
    "3 distinct, non-coplanar points is not a convex polygon";

struct Vector3dLessThan {
    bool operator()(Vector3d const & v0, Vector3d const & v1) const {
        if (v0.x() == v1.x()) {
            if (v0.y() == v1.y()) {
                return v0.z() < v1.z();
            }
            return v0.y() < v1.y();
        }
        return v0.x() < v1.x();
    }
};

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
                std::copy_backward(fromCCW, hullEnd, hullEnd + 1);
                *fromCCW = *v;
            }
            ++hullEnd;
        }
    }
    points.erase(hullEnd, end);
    // Since the points in the hull are distinct, there is a unique minimum
    // point - rotate the points vector to make it the first one. This allows
    // operator== for ConvexPolygon to be implemented simply by comparing
    // vertices.
    VertexIterator minVertex = points.begin();
    Vector3dLessThan lessThan;
    for (VertexIterator v = minVertex + 1, e = points.end(); v != e; ++v) {
        if (lessThan(*v, *minVertex)) {
            minVertex = v;
        }
    }
    std::rotate(points.begin(), minVertex, points.end());
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
    VertexIterator j = p._vertices.begin();
    VertexIterator const end = _vertices.end();
    for (; i != end; ++i, ++j) {
        if (*i != *j) {
            return false;
        }
    }
    return true;
}

UnitVector3d ConvexPolygon::getCentroid() const {
    // The center of mass is obtained via trivial generalization of
    // the formula for spherical triangles from:
    //
    // The centroid and inertia tensor for a spherical triangle
    // John E. Brock
    // 1974, Naval Postgraduate School, Monterey Calif.
    Vector3d cm;
    VertexIterator const end = _vertices.end();
    VertexIterator i = end - 1;
    VertexIterator j = _vertices.begin();
    for (; j != end; i = j, ++j) {
        Vector3d v = (*i).robustCross(*j);
        double s = 0.5 * v.normalize();
        double c = (*i).dot(*j);
        double a = (s == 0.0 && c == 0.0) ? 0.0 : std::atan2(s, c);
        cm += v * a;
    }
    return UnitVector3d(cm);
}

Circle ConvexPolygon::getBoundingCircle() const {
    UnitVector3d c = getCentroid();
    // Compute the maximum squared chord length between the centroid and
    // all vertices.
    VertexIterator const end = _vertices.end();
    VertexIterator i = _vertices.begin();
    double cl2 = 0.0;
    for (; i != end; ++i) {
        cl2 = std::max(cl2, (*i - c).getSquaredNorm());
    }
    // Add double the maximum squared-chord-length error, so that the
    // bounding circle we return also reliably CONTAINS this polygon.
    return Circle(c, cl2 + 2.0 * MAX_SQUARED_CHORD_LENGTH_ERROR);
}

Box ConvexPolygon::getBoundingBox() const {
    Angle const eps(5.0e-10); // ~ 0.1 milli-arcseconds
    Box bbox;
    VertexIterator const end = _vertices.end();
    VertexIterator i = end - 1;
    VertexIterator j = _vertices.begin();
    bool haveCW = false;
    bool haveCCW = false;
    // Compute the bounding box for each vertex. When converting a Vector3d
    // to a LonLat, the relative error on the longitude is about 4*2^-53,
    // and the relative error on the latitude is about twice that (assuming
    // std::atan2 and std::sqrt accurate to within 1 ulp). We convert each
    // vertex to a conservative bounding box for its spherical coordinates,
    // and compute a bounding box for the union of all these boxes.
    //
    // Furthermore, the latitude range of an edge can be greater than the
    // latitude range of its endpoints - this occurs when the minimum or
    // maximum latitude point on the great circle defined by the edge vertices
    // lies in the edge interior.
    for (; j != end; i = j, ++j) {
        LonLat p(*j);
        bbox.expandTo(Box(p, eps, eps));
        if (!haveCW || !haveCCW) {
            int o = orientationZ(*i, *j);
            haveCCW = haveCCW || (o > 0);
            haveCW = haveCW || (o < 0);
        }
        // Compute the plane normal for edge i, j.
        Vector3d n = (*i).robustCross(*j);
        // Compute a vector v with positive z component that lies on both the
        // edge plane and on the plane defined by the z axis and the edge plane
        // normal. This is the direction of maximum latitude for the great
        // circle containing the edge, and -v is the direction of minimum
        // latitude.
        //
        // TODO(smm): Do a proper error analysis.
        Vector3d v(-n.x() * n.z(),
                   -n.y() * n.z(),
                   n.x() * n.x() + n.y() * n.y());
        if (v != Vector3d()) {
            // The plane defined by the z axis and n has normal
            // (-n.y(), n.x(), 0.0). Compute the dot product of this plane
            // normal with vertices i and j.
            double zni = i->y() * n.x() - i->x() * n.y();
            double znj = j->y() * n.x() - j->x() * n.y();
            // Check if v or -v is in the edge interior.
            if (zni > 0.0 && znj < 0.0) {
                bbox = Box(bbox.getLon(), bbox.getLat().expandedTo(
                    LonLat::latitudeOf(v) + eps));
            } else if (zni < 0.0 && znj > 0.0) {
                bbox = Box(bbox.getLon(), bbox.getLat().expandedTo(
                    LonLat::latitudeOf(-v) - eps));
            }
        }
    }
    // If this polygon contains a pole, its bounding box must contain all
    // longitudes.
    if (!haveCW) {
        Box northPole(Box::allLongitudes(), AngleInterval(Angle(0.5 * PI)));
        bbox.expandTo(northPole);
    } else if (!haveCCW) {
        Box southPole(Box::allLongitudes(), AngleInterval(Angle(-0.5 * PI)));
        bbox.expandTo(southPole);
    }
    return bbox;
}

Box3d ConvexPolygon::getBoundingBox3d() const {
    static double const maxError = 1.0e-14;
    VertexIterator const end = _vertices.end();
    // Compute the extrema of all vertex coordinates.
    VertexIterator j = _vertices.begin();
    double emin[3] = { j->x(), j->y(), j->z() };
    double emax[3] = { j->x(), j->y(), j->z() };
    for (++j; j != end; ++j) {
        for (int i = 0; i < 3; ++i) {
            double v = j->operator()(i);
            emin[i] = std::min(emin[i], v);
            emax[i] = std::max(emax[i], v);
        }
    }
    // Compute the extrema of all edges.
    //
    // It can be shown that the great circle with unit normal vector
    // n = (n₀, n₁, n₂) has extrema in x at:
    //
    //   (∓√(1 - n₀²), ±n₁n₀/√(1 - n₀²), ±n₂n₀/√(1 - n₀²))
    //
    // in y at:
    //
    //   (±n₀n₁/√(1 - n₁²), ∓√(1 - n₁²), ±n₂n₁/√(1 - n₁²))
    //
    // and in z at
    //
    //   (±n₀n₂/√(1 - n₂²), ±n₁n₂/√(1 - n₂²), ∓√(1 - n₂²))
    //
    // Compute these vectors for each edge, determine whether they lie in
    // the edge, and update the extrema if so. Rounding errors in these
    // computations are compensated for by expanding the bounding box
    // prior to returning it.
    j = end - 1;
    VertexIterator k = _vertices.begin();
    for (; k != end; j = k, ++k) {
        UnitVector3d n(j->robustCross(*k));
        for (int i = 0; i < 3; ++i) {
            double ni = n(i);
            double d = std::fabs(1.0 - ni * ni);
            if (d > 0.0) {
                Vector3d e(i == 0 ? -d : n.x() * ni,
                           i == 1 ? -d : n.y() * ni,
                           i == 2 ? -d : n.z() * ni);
                // If e or -e lies in the lune defined by the half great
                // circle passing through n and a and the half great circle
                // passing through n and b, the edge contains an extremum.
                Vector3d v = e.cross(n);
                double vdj = v.dot(*j);
                double vdk = v.dot(*k);
                if (vdj >= 0.0 && vdk <= 0.0) {
                    emin[i] = std::min(emin[i], -std::sqrt(d));
                }
                if (vdj <= 0.0 && vdk >= 0.0) {
                    emax[i] = std::max(emax[i], std::sqrt(d));
                }
            }
        }
    }
    // Check whether which of the standard basis vectors and their antipodes
    // are inside this polygon.
    bool a[3] = { true, true, true };
    bool b[3] = { true, true, true };
    j = end - 1;
    k = _vertices.begin();
    for (; k != end; j = k, ++k) {
        // Test the standard basis vectors against the plane defined by
        // vertices (j, k). Note that orientation(-x, *j, *k) =
        // -orientation(x, *j, *k).
        int ox = orientationX(*j, *k);
        a[0] = a[0] && (ox <= 0);
        b[0] = b[0] && (ox >= 0);
        int oy = orientationY(*j, *k);
        a[1] = a[1] && (oy <= 0);
        b[1] = b[1] && (oy >= 0);
        int oz = orientationZ(*j, *k);
        a[2] = a[2] && (oz <= 0);
        b[2] = b[2] && (oz >= 0);
    }
    // At this point, b[i] is true iff the standard basis vector eᵢ
    // is inside all the half spaces defined by the polygon edges.
    // Similarly, a[i] is true iff -eᵢ is inside the same half spaces.
    for (int i = 0; i < 3; ++i) {
        emin[i] = a[i] ? -1.0 : std::max(-1.0, emin[i] - maxError);
        emax[i] = b[i] ? 1.0 : std::min(1.0, emax[i] + maxError);
    }
    return Box3d(Interval1d(emin[0], emax[0]),
                 Interval1d(emin[1], emax[1]),
                 Interval1d(emin[2], emax[2]));
}

bool ConvexPolygon::contains(UnitVector3d const & v) const {
    VertexIterator const end = _vertices.end();
    VertexIterator i = end - 1;
    VertexIterator j = _vertices.begin();
    for (; j != end; i = j, ++j) {
        if (orientation(v, *i, *j) < 0) {
            return false;
        }
    }
    return true;
}

Relationship ConvexPolygon::relate(Box const & b) const {
    // TODO(smm): be more accurate when computing box relations.
    return getBoundingBox().relate(b) & (DISJOINT | WITHIN);
}

Relationship ConvexPolygon::relate(Circle const & c) const {
    if (c.isEmpty()) {
        return CONTAINS | DISJOINT;
    }
    if (c.isFull()) {
        return WITHIN;
    }
    // Determine whether or not the circle and polygon boundaries intersect.
    // If the polygon vertices are not all inside or all outside of c, then the
    // boundaries cross.
    bool inside = false;
    VertexIterator const end = _vertices.end();
    for (VertexIterator v = _vertices.begin(); v != end; ++v) {
        double d = (*v - c.getCenter()).getSquaredNorm();
        if (std::fabs(d - c.getSquaredChordLength()) <
            MAX_SQUARED_CHORD_LENGTH_ERROR) {
            // A polygon vertex is close to the circle boundary.
            return INTERSECTS;
        }
        bool b = d < c.getSquaredChordLength();
        if (v == _vertices.begin()) {
            inside = b;
        } else if (inside != b) {
            // There are box vertices both inside and outside of c.
            return INTERSECTS;
        }
    }
    if (inside) {
        // All polygon vertices are inside c. Look for points in the polygon
        // edge interiors that are outside c.
        for (VertexIterator a = end - 1, b = _vertices.begin();
             b != end; a = b, ++b) {
            Vector3d n = a->robustCross(*b);
            double d = getMaxSquaredChordLength(c.getCenter(), *a, *b, n);
            if (d > c.getSquaredChordLength() -
                    MAX_SQUARED_CHORD_LENGTH_ERROR) {
                return INTERSECTS;
            }
        }
        // The polygon boundary is conclusively inside c. It may still be the
        // case that the circle punches a hole in the polygon. We check that
        // the polygon does not contain the complement of c by testing whether
        // or not it contains the anti-center of c.
        if (contains(-c.getCenter())) {
            return INTERSECTS;
        }
        return WITHIN;
    }
    // All polygon vertices are outside c. Look for points in the polygon edge
    // interiors that are inside c.
    for (VertexIterator a = end - 1, b = _vertices.begin();
         b != end; a = b, ++b) {
        Vector3d n = a->robustCross(*b);
        double d = getMinSquaredChordLength(c.getCenter(), *a, *b, n);
        if (d < c.getSquaredChordLength() + MAX_SQUARED_CHORD_LENGTH_ERROR) {
            return INTERSECTS;
        }
    }
    // The polygon boundary is conclusively outside of c. If the polygon
    // contains the circle center, then the polygon contains c. Otherwise, the
    // polygon and circle are disjoint.
    if (contains(c.getCenter())) {
        return CONTAINS;
    }
    return DISJOINT;
}

Relationship ConvexPolygon::relate(ConvexPolygon const & p) const {
    // TODO(smm): Make this more performant. Instead of the current quadratic
    // implementation, it should be possible to determine whether the boundaries
    // intersect by adapting the following method to the sphere:
    //
    // A new linear algorithm for intersecting convex polygons
    // Computer Graphics and Image Processing, Volume 19, Issue 1, May 1982, Page 92
    // Joseph O'Rourke, Chi-Bin Chien, Thomas Olson, David Naddor
    //
    // http://www.sciencedirect.com/science/article/pii/0146664X82900235
    VertexIterator const e = _vertices.end();
    VertexIterator const ep = p._vertices.end();
    size_t n = 0; // number of vertices of this polygon in p
    size_t m = 0; // number of vertices of p in this polygon
    for (VertexIterator a = _vertices.begin(); a != e; ++a) {
        n += p.contains(*a);
    }
    for (VertexIterator c = p._vertices.begin(); c != ep; ++c) {
        m += contains(*c);
    }
    if (n == _vertices.size()) {
        if (m == p._vertices.size()) {
            return CONTAINS | WITHIN;
        }
        return WITHIN;
    } else if (m == p._vertices.size()) {
        return CONTAINS;
    }
    if (n > 0 || m > 0) {
        // There is at least one point common to this polygon and p.
        return INTERSECTS;
    }
    // Consider all possible edge pairs and look for a crossing.
    for (VertexIterator a = e - 1, b = _vertices.begin();
         b != e; a = b, ++b) {
        for (VertexIterator c = ep - 1, d = p._vertices.begin();
             d != ep; c = d, ++d) {
            int acd = orientation(*a, *c, *d);
            int bcd = orientation(*b, *c, *d);
            if (acd == -bcd && acd != 0) {
                int cab = orientation(*c, *a, *b);
                int dab = orientation(*d, *a, *b);
                if (cab == -dab && cab != 0) {
                    // Found a non-degenerate edge crossing
                    return INTERSECTS;
                }
            }
        }
    }
    return DISJOINT;
}

Relationship ConvexPolygon::relate(Ellipse const & e) const {
    return relate(e.getBoundingCircle()) & (CONTAINS | DISJOINT);
}

std::ostream & operator<<(std::ostream & os, ConvexPolygon const & p) {
    typedef std::vector<UnitVector3d>::const_iterator VertexIterator;
    os << "ConvexPolygon(\n"
          "    ";
    VertexIterator v = p.getVertices().begin();
    VertexIterator const end = p.getVertices().end();
    for (; v != end; ++v) {
        if (v != p.getVertices().begin()) {
            os << ",\n"
                  "    ";
        }
        os << *v;
    }
    os << "\n"
          ")";
    return os;
}

}} // namespace lsst::sphgeom
