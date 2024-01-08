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

#ifndef LSST_SPHGEOM_CONVEXPOLYGONIMPL_H_
#define LSST_SPHGEOM_CONVEXPOLYGONIMPL_H_

/// \file
/// \brief This file contains the meat of the ConvexPolygon implementation.
///
/// These functions are parametrized by vertex iterator type, making
/// it possible to call them given only a fixed size vertex array. The
/// functions which compute indexes of HTM triangles and Q3C quads intersecting
/// a spherical region use them to avoid the cost of creating ConvexPolygon
/// objects for each triangle/quad.

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/Ellipse.h"
#include "lsst/sphgeom/orientation.h"
#include "lsst/sphgeom/utils.h"


namespace lsst {
namespace sphgeom {
namespace detail {

template <typename VertexIterator>
UnitVector3d centroid(VertexIterator const begin, VertexIterator const end) {
    // The center of mass is obtained via trivial generalization of
    // the formula for spherical triangles from:
    //
    // The centroid and inertia tensor for a spherical triangle
    // John E. Brock
    // 1974, Naval Postgraduate School, Monterey Calif.
    Vector3d cm;
    VertexIterator i = std::prev(end);
    VertexIterator j = begin;
    for (; j != end; i = j, ++j) {
        Vector3d v = (*i).robustCross(*j);
        double s = 0.5 * v.normalize();
        double c = (*i).dot(*j);
        double a = (s == 0.0 && c == 0.0) ? 0.0 : std::atan2(s, c);
        cm += v * a;
    }
    return UnitVector3d(cm);
}

template <typename VertexIterator>
Circle boundingCircle(VertexIterator const begin, VertexIterator const end) {
    UnitVector3d c = centroid(begin, end);
    // Compute the maximum squared chord length between the centroid and
    // all vertices.
    VertexIterator i = begin;
    double cl2 = 0.0;
    for (; i != end; ++i) {
        cl2 = std::max(cl2, (*i - c).getSquaredNorm());
    }
    // Add double the maximum squared-chord-length error, so that the
    // bounding circle we return also reliably CONTAINS this polygon.
    return Circle(c, cl2 + 2.0 * MAX_SQUARED_CHORD_LENGTH_ERROR);
}


template <typename VertexIterator>
Box boundingBox(VertexIterator const begin, VertexIterator const end) {
    Angle const eps(5.0e-10); // ~ 0.1 milli-arcseconds
    Box bbox;
    VertexIterator i = std::prev(end);
    VertexIterator j = begin;
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

template <typename VertexIterator>
Box3d boundingBox3d(VertexIterator const begin, VertexIterator const end) {
    static double const maxError = 1.0e-14;
    // Compute the extrema of all vertex coordinates.
    VertexIterator j = begin;
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
    j = std::prev(end);
    VertexIterator k = begin;
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
    // Check whether the standard basis vectors and their antipodes
    // are inside this polygon.
    bool a[3] = { true, true, true };
    bool b[3] = { true, true, true };
    j = std::prev(end);
    k = begin;
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

template <typename VertexIterator>
bool contains(VertexIterator const begin,
              VertexIterator const end,
              UnitVector3d const & v)
{
    VertexIterator i = std::prev(end);
    VertexIterator j = begin;
    for (; j != end; i = j, ++j) {
        if (orientation(v, *i, *j) < 0) {
            return false;
        }
    }
    return true;
}

template <typename VertexIterator>
Relationship relate(VertexIterator const begin,
                    VertexIterator const end,
                    Box const & b)
{
    // TODO(smm): be more accurate when computing box relations.
    return boundingBox(begin, end).relate(b) & (DISJOINT | WITHIN);
}

template <typename VertexIterator>
Relationship relate(VertexIterator const begin,
                    VertexIterator const end,
                    Circle const & c)
{
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
    for (VertexIterator v = begin; v != end; ++v) {
        double d = (*v - c.getCenter()).getSquaredNorm();
        if (std::fabs(d - c.getSquaredChordLength()) <
            MAX_SQUARED_CHORD_LENGTH_ERROR) {
            // A polygon vertex is close to the circle boundary.
            return INTERSECTS;
        }
        bool b = d < c.getSquaredChordLength();
        if (v == begin) {
            inside = b;
        } else if (inside != b) {
            // There are box vertices both inside and outside of c.
            return INTERSECTS;
        }
    }
    if (inside) {
        // All polygon vertices are inside c. Look for points in the polygon
        // edge interiors that are outside c.
        for (VertexIterator a = std::prev(end), b = begin; b != end; a = b, ++b) {
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
        if (contains(begin, end, -c.getCenter())) {
            return INTERSECTS;
        }
        return WITHIN;
    }
    // All polygon vertices are outside c. Look for points in the polygon edge
    // interiors that are inside c.
    for (VertexIterator a = std::prev(end), b = begin; b != end; a = b, ++b) {
        Vector3d n = a->robustCross(*b);
        double d = getMinSquaredChordLength(c.getCenter(), *a, *b, n);
        if (d < c.getSquaredChordLength() + MAX_SQUARED_CHORD_LENGTH_ERROR) {
            return INTERSECTS;
        }
    }
    // The polygon boundary is conclusively outside of c. If the polygon
    // contains the circle center, then the polygon contains c. Otherwise, the
    // polygon and circle are disjoint.
    if (contains(begin, end, c.getCenter())) {
        return CONTAINS;
    }
    return DISJOINT;
}

template <typename VertexIterator1,
          typename VertexIterator2>
Relationship relate(VertexIterator1 const begin1,
                    VertexIterator1 const end1,
                    VertexIterator2 const begin2,
                    VertexIterator2 const end2)
{
    // TODO(smm): Make this more performant. Instead of the current quadratic
    // implementation, it should be possible to determine whether the boundaries
    // intersect by adapting the following method to the sphere:
    //
    // A new linear algorithm for intersecting convex polygons
    // Computer Graphics and Image Processing, Volume 19, Issue 1, May 1982, Page 92
    // Joseph O'Rourke, Chi-Bin Chien, Thomas Olson, David Naddor
    //
    // http://www.sciencedirect.com/science/article/pii/0146664X82900235
    bool all1 = true;
    bool any1 = false;
    bool all2 = true;
    bool any2 = false;
    for (VertexIterator1 i = begin1; i != end1; ++i) {
        bool b = contains(begin2, end2, *i);
        all1 = b && all1;
        any1 = b || any1;
    }
    for (VertexIterator2 j = begin2; j != end2; ++j) {
        bool b = contains(begin1, end1, *j);
        all2 = b && all2;
        any2 = b || any2;
    }
    if (all1 || all2) {
        // All vertices of one or both polygons are inside the other
        return (all1 ? WITHIN : INTERSECTS) | (all2 ? CONTAINS : INTERSECTS);
    }
    if (any1 || any2) {
        // The polygons have at least one point in common.
        return INTERSECTS;
    }
    // No vertex of either polygon is inside the other. Consider all
    // possible edge pairs and look for a crossing.
    for (VertexIterator1 a = std::prev(end1), b = begin1;
         b != end1; a = b, ++b) {
        for (VertexIterator2 c = std::prev(end2), d = begin2;
             d != end2; c = d, ++d) {
            int acd = orientation(*a, *c, *d);
            int bdc = orientation(*b, *d, *c);
            if (acd == bdc && acd != 0) {
                int cba = orientation(*c, *b, *a);
                int dab = orientation(*d, *a, *b);
                if (cba == dab && cba == acd) {
                    // Found a non-degenerate edge crossing
                    return INTERSECTS;
                }
            }
        }
    }
    return DISJOINT;
}

template <typename VertexIterator>
Relationship relate(VertexIterator const begin,
                    VertexIterator const end,
                    ConvexPolygon const & p)
{
    return relate(begin, end, p.getVertices().begin(), p.getVertices().end());
}

template <typename VertexIterator>
Relationship relate(VertexIterator const begin,
                    VertexIterator const end,
                    Ellipse const & e)
{
    return relate(begin, end, e.getBoundingCircle()) & (CONTAINS | DISJOINT);
}

}}} // namespace lsst::sphgeom::detail

#endif // LSST_SPHGEOM_CONVEXPOLYGONIMPL_H_
