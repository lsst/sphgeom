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
/// \brief This file contains the Vector3d class implementation.

#include "lsst/sphgeom/Vector3d.h"

#if !defined(NO_SIMD) && defined(__x86_64__)
    #include <x86intrin.h>
#endif
#include <cstdio>
#include <ostream>

#include "lsst/sphgeom/Angle.h"
#include "lsst/sphgeom/UnitVector3d.h"


namespace lsst {
namespace sphgeom {

double Vector3d::normalize() {
    static constexpr uint8_t UNUSED = 255;
    // Given a 3 component vector (x, y, z), this LUT provides the indexes
    // of the components in order of smallest absolute value to largest.
    // The index into the LUT must be computed as:
    //
    //      ((|x| > |z|) << 2) +
    //      ((|x| > |y|) << 1) +
    //       (|y| > |z|)
    static uint8_t const COMPONENT[8][4] = {
        {0, 1, 2, UNUSED},
        {0, 2, 1, UNUSED},
        {1, 0, 2, UNUSED},
        {UNUSED, UNUSED, UNUSED, UNUSED},
        {UNUSED, UNUSED, UNUSED, UNUSED},
        {2, 0, 1, UNUSED},
        {1, 2, 0, UNUSED},
        {2, 1, 0, UNUSED}
    };
#if defined(NO_SIMD) || !defined(__x86_64__)
    double ax = std::fabs(_v[0]);
    double ay = std::fabs(_v[1]);
    double az = std::fabs(_v[2]);
    int index = ((ax > az) << 2) +
                ((ax > ay) << 1) +
                 (ay > az);
    double w = _v[COMPONENT[index][2]];
    if (w == 0.0) {
        throw std::runtime_error("Cannot normalize zero vector");
    }
    // Divide components by the absolute value of the largest
    // component to avoid overflow/underflow.
    double maxabs = std::fabs(w);
    double u = _v[COMPONENT[index][0]] / maxabs;
    double v = _v[COMPONENT[index][1]] / maxabs;
    w = std::copysign(1.0, w);
    double d = u * u + v * v;
    double norm = std::sqrt(1.0 + d);
    _v[COMPONENT[index][0]] = u / norm;
    _v[COMPONENT[index][1]] = v / norm;
    _v[COMPONENT[index][2]] = w / norm;
    return norm * maxabs;
#else
    static __m128d const m0m0 = _mm_set_pd(-0.0, -0.0);
    __m128d ayaz = _mm_andnot_pd(m0m0, _mm_loadu_pd(_v + 1));
    __m128d axax = _mm_andnot_pd(m0m0, _mm_set1_pd(_v[0]));
    __m128d az = _mm_unpackhi_pd(ayaz, _mm_setzero_pd());
    int index = (_mm_movemask_pd(_mm_cmpgt_pd(axax, ayaz)) << 1) |
                 _mm_movemask_pd(_mm_cmplt_sd(az, ayaz));
    // The lower double in uv contains the vector component
    // with the lowest absolute value. The higher double contains
    // the component with absolute value betweem the lowest and
    // highest absolute values.
    __m128d uv = _mm_set_pd(_v[COMPONENT[index][1]],
                            _v[COMPONENT[index][0]]);
    // ww contains two copies of the vector component with the
    // highest absolute value.
    __m128d ww = _mm_set1_pd(_v[COMPONENT[index][2]]);
    __m128d maxabs = _mm_andnot_pd(m0m0, ww);
    if (_mm_ucomieq_sd(ww, _mm_setzero_pd())) {
        throw std::runtime_error("Cannot normalize zero vector");
    }
    // Divide components by the absolute value of the largest
    // component to avoid overflow/underflow.
    uv = _mm_div_pd(uv, maxabs);
    ww = _mm_or_pd(_mm_and_pd(m0m0, ww), _mm_set1_pd(1.0));
    __m128d norm = _mm_mul_pd(uv, uv);
    norm = _mm_sqrt_sd(
        _mm_setzero_pd(),
        _mm_add_sd(
            _mm_set_sd(1.0),
            _mm_add_sd(norm, _mm_unpackhi_pd(norm, _mm_setzero_pd()))
        )
    );
    // Normalize components and store the results.
    ww = _mm_div_sd(ww, norm);
    uv = _mm_div_pd(uv, _mm_shuffle_pd(norm, norm, 0));
    _mm_store_sd(&_v[COMPONENT[index][0]], uv);
    _mm_storeh_pd(&_v[COMPONENT[index][1]], uv);
    _mm_store_sd(&_v[COMPONENT[index][2]], ww);
    return _mm_cvtsd_f64(_mm_mul_sd(norm, maxabs));
#endif
}

Vector3d Vector3d::rotatedAround(UnitVector3d const & k, Angle a) const {
    // Use Rodrigues' rotation formula.
    Vector3d const & v = *this;
    double s = sin(a);
    double c = cos(a);
    return v * c + k.cross(v) * s + k * (k.dot(v) * (1.0 - c));
}

std::ostream & operator<<(std::ostream & os, Vector3d const & v) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "[%.17g, %.17g, %.17g]",
                  v.x(), v.y(), v.z());
    return os << buf;
}

}} // namespace lsst::sphgeom
