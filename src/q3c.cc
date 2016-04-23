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

/// \file
/// \brief This file implements functions related to Q3C indexing.

#include "lsst/sphgeom/q3c.h"

#include <stdexcept>

#include "lsst/sphgeom/curve.h"
#include "lsst/sphgeom/UnitVector3d.h"


namespace lsst {
namespace sphgeom {

namespace {

    // If each cube face is overlaid with a uniform grid, then grid cell
    // area (on the sphere) varies by a factor of about 5.2. The largest
    // cells are in the center of the face, where the face is tangent to
    // the unit sphere, and the smallest are in the corners.
    //
    // The idea here, taken from the Google S2 library, is to apply a
    // separable transformation to face coordinates before converting
    // them to grid coordinates. This results in non-uniform edge spacing,
    // but edges remain geodesic.
    //
    // One way to do this is to notice that the central projection of
    // (z, x) = (cos θ, sin θ), where -π/4 ≤ θ ≤ π/4, onto the line z = 1
    // is (1, tan θ). So, applying f(x) = 4 arctan x / π to each face
    // coordinate will produce grid cell edges that subtend more uniform
    // angles. This reduces cell area variation down to a factor of about
    // 1.414.
    //
    // However, computing tangents and arctangents is relatively expensive.
    // Instead, we use f(x) = x(4 - |x|)/3, which is a decent quadratic
    // approximation to the above that cuts area variation down to a
    // factor of about 1.55. This function was arrived at by picking a
    // quadratic function f satisfying f(0) = 0 and f(1) = 1 close to the
    // best minimax rational approximation of degree 2 for 4 arctan x / π
    // on the interval [0, 1]:
    //
    // -0.00312224879 + (1.357648680 - 0.3514041823 x) x
    //
    // (computed with Maple using the Remez algorithm). The constraint
    // f(-x) = -f(x) (following the symmetry of arctan) is used to extend
    // the domain to [-1, 1]. Better approximations very likely exist,
    // but this one is already noticeably better than the quadratic
    // approximation used by the S2 library, which has an area variation
    // factor of about 2.1.

    inline std::tuple<uint32_t, uint32_t> faceToGrid(double x,
                                                     double y,
                                                     uint32_t n,
                                                     bool transform)
    {
        if (transform) {
            x = x * (1.3333333333333333 - 0.3333333333333333 * std::fabs(x));
            y = y * (1.3333333333333333 - 0.3333333333333333 * std::fabs(y));
        }
        double d = 0.5 * static_cast<double>(n);
        // Convert to grid coordinates and clamp values to [0, n)
        uint32_t gx = static_cast<uint32_t>(x * d + d);
        uint32_t gy = static_cast<uint32_t>(y * d + d);
        return std::make_tuple(gx - (gx == n), gy - (gy == n));
    }

    inline std::tuple<double, double> gridToFace(uint32_t x,
                                                 uint32_t y,
                                                 uint32_t n,
                                                 bool transform)
    {
        double d = 2.0 / static_cast<double>(n);
        double fx = static_cast<double>(x) * d - 1.0;
        double fy = static_cast<double>(y) * d - 1.0;
        if (transform) {
            double sx = sqrt(4.0 - 3.0 * std::fabs(fx));
            double sy = sqrt(4.0 - 3.0 * std::fabs(fy));
            fx = std::copysign(1.0, fx) * (2.0 - sx);
            fy = std::copysign(1.0, fy) * (2.0 - sy);
        }
        return std::make_tuple(fx, fy);
    }

} // unnamed namespace


uint64_t q3cIndex(UnitVector3d const & v,
                  uint32_t n,
                  bool useHilbertCurve,
                  bool reduceAreaVariation)
{
    if (n == 0 || n > MAX_Q3C_RESOLUTION) {
        throw std::invalid_argument("Q3C grid resolution not in [1, 2^30]");
    }
    // Compute the number of the cube face that v belongs to, and the
    // coordinates (fx, fy) of v on that face.
    uint64_t face;
    double fx;
    // c is the absolute value of the vector component for the
    // axis perpendicular to the desired face.
    double c;
    if (v.x() >= -v.y()) {
        if (v.x() > v.y()) {
            face = 1;
            fx = v.y();
            c = v.x();
        } else {
            face = 2;
            fx = -v.x();
            c = v.y();
        }
    } else {
        if (v.x() < v.y()) {
            face = 3;
            fx = -v.y();
            c = -v.x();
        } else {
            face = 4;
            fx = v.x();
            c = -v.y();
        }
    }
    // If the z-coordinate of v is too small or too large, then v
    // is not in an equatorial face.
    double fy = v.z();
    if (fy > c) {
        face = 0;
        fx = v.y();
        fy = -v.x();
        c = v.z();
    } else if (fy < -c) {
        face = 5;
        fx = v.y();
        fy = v.x();
        c = -v.z();
    }
    if (n == 1) {
        return face;
    }
    // Projecting v to the cube face computed above is a matter of scaling
    // by 1 / c; this maps fx and fy to numbers in the range [-1, 1].
    uint32_t gx, gy;
    std::tie(gx, gy) = faceToGrid(fx / c, fy / c, n, reduceAreaVariation);
    // Count the number of bits m (> 0) required to represent n - 1. The face
    // number (0-5) will be stored in bits 2m and above of the output.
    uint32_t m = log2(n - 1) + 1;
    // Map x, y to a Morton or Hilbert index and compose the result.
    uint64_t i = mortonIndex(gx, gy);
    if (useHilbertCurve) {
        i = mortonToHilbert(i, m);
    }
    return (face << (2 * m)) | i;
}

}} // namespace lsst::sphgeom
