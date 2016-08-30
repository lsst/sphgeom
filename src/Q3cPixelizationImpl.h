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

#ifndef LSST_SPHGEOM_Q3CPIXELIZATIONIMPL_H_
#define LSST_SPHGEOM_Q3CPIXELIZATIONIMPL_H_

/// \file
/// \brief This file contains functions used by Q3C pixelization
///        implementations.

#include <cstdint>
#if defined(NO_SIMD) || !defined(__x86_64__)
    #include <tuple>
#else
    #include <x86intrin.h>
#endif

#include "lsst/sphgeom/UnitVector3d.h"


namespace lsst {
namespace sphgeom {
namespace {

// LUT that provides the maximum grid coordinate value M in terms of
// the subdivision level L; M = 2^L - 1.
double const ST_MAX[31] = {
    0.0,
    1.0,
    3.0,
    7.0,
    15.0,
    31.0,
    63.0,
    127.0,
    255.0,
    511.0,
    1023.0,
    2047.0,
    4095.0,
    8191.0,
    16383.0,
    32767.0,
    65535.0,
    131071.0,
    262143.0,
    524287.0,
    1048575.0,
    2097151.0,
    4194303.0,
    8388607.0,
    16777215.0,
    33554431.0,
    67108863.0,
    134217727.0,
    268435455.0,
    536870911.0,
    1073741823.0
};

// LUT that provides the face to grid coordinate scaling factor F in terms of
// the subdivision level L; F = 2^(L - 1).
double const GRID_SCALE[31] = {
    0.5,
    1.0,
    2.0,
    4.0,
    8.0,
    16.0,
    32.0,
    64.0,
    128.0,
    256.0,
    512.0,
    1024.0,
    2048.0,
    4096.0,
    8192.0,
    16384.0,
    32768.0,
    65536.0,
    131072.0,
    262144.0,
    524288.0,
    1048576.0,
    2097152.0,
    4194304.0,
    8388608.0,
    16777216.0,
    33554432.0,
    67108864.0,
    134217728.0,
    268435456.0,
    536870912.0
};

// LUT that provides the grid to face coordinate scaling factor F in terms of
// the subdivision level L; F = 2^(1 - L).
double const FACE_SCALE[31] = {
    2.0,
    1.0,
    0.5,
    0.25,
    0.125,
    0.0625,
    0.03125,
    0.015625,
    0.0078125,
    0.00390625,
    0.001953125,
    0.0009765625,
    0.00048828125,
    0.000244140625,
    0.0001220703125,
    6.103515625e-5,
    3.0517578125e-5,
    1.52587890625e-5,
    7.62939453125e-6,
    3.814697265625e-6,
    1.9073486328125e-6,
    9.5367431640625e-7,
    4.76837158203125e-7,
    2.384185791015625e-7,
    1.1920928955078125e-7,
    5.9604644775390625e-8,
    2.98023223876953125e-8,
    1.490116119384765625e-8,
    7.450580596923828125e-9,
    3.7252902984619140625e-9,
    1.86264514923095703125e-9,
};

// The functions below use a number of additional lookup tables
// for performance.
//
// The first LUT, faceNumbers, is used to find the cube face that a unit
// vector p belongs to. In particular, component comparisons are used to
// build an index into the LUT. That index is computed as follows:
//
//      int index = ((p.x() >  p.y()) << 5) +
//                  ((p.x() > -p.y()) << 4) +
//                  ((p.x() >  p.z()) << 3) +
//                  ((p.x() > -p.z()) << 2) +
//                  ((p.y() >  p.z()) << 1) +
//                   (p.y() > -p.z());
//
// and the Q3C (or modified-Q3C) face number is just:
//
//      int face = faceNumbers[index];
//
// Compare this to something like:
//
//      int face;
//      double a;
//      if (p.x() > -p.y()) {
//          if (p.x() > p.y()) {
//              face = 1;
//              a = p.x();
//          } else {
//              face = 2;
//              a = p.y();
//          }
//      } else {
//          if (p.x() > p.y()) {
//              face = 4;
//              a = -p.y();
//          } else {
//              face = 3;
//              a = -p.x();
//          }
//      }
//      if (p.z() > a) {
//          face = 0;
//      } else if (p.z() < -a) {
//          face = 5;
//      }
//
// In the latter case, the comparisons performed depend on the data, and the
// branches involved will be hard to predict for random input. The LUT-based
// approach allows comparisons to be performed in parallel without speculation
// by introducing some redundant computation. It replaces control dependencies
// (branching) with data dependencies.
//
// The second LUT, faceComponents, maps from a face number F (0-5) to a
// 4-element array A that contains the following quantities for a unit vector
// V belonging to F:
//
//  A[0]: index of the component of V corresponding to the face u coordinate
//  A[1]: index of the component of V corresponding to the face v coordinate
//  A[2]: index of the component of V with maximum absolute value
//  A[3]: unused (padding)
//
// Finally, the third LUT, faceConstants, maps from a face number F (0-5) to
// an array of doubles A that contains the following quantities for a unit
// vector V belonging to F:
//
//  A[0]: scaling factor (±1) that, when multiplied by the component of V with
//        index faceComponents[face][0], yields the u coordinate of V
//  A[1]: scaling factor (±1) that, when multiplied by the component of V with
//        index faceComponents[face][1], yields the v coordinate of V
//  A[2]: value (±1) that has the same sign as the component of V with
//        maximum absolute value
//  A[3]: unused (padding)

#if defined(NO_SIMD) || !defined(__x86_64__)

    int faceNumber(UnitVector3d const & v, uint8_t const (&faceNumbers)[64]) {
        int index = ((v.x() >  v.y()) << 5) +
                    ((v.x() > -v.y()) << 4) +
                    ((v.x() >  v.z()) << 3) +
                    ((v.x() > -v.z()) << 2) +
                    ((v.y() >  v.z()) << 1) +
                     (v.y() > -v.z());
        return faceNumbers[index];
    }

    UnitVector3d faceToSphere(int face,
                              double u,
                              double v,
                              uint8_t const (&faceComponents)[6][4],
                              double const (&faceConstants)[6][4])
    {
        double p[3];
        double d = u * u + v * v;
        double n = std::sqrt(1.0 + d);
        p[faceComponents[face][0]] = (u * faceConstants[face][0]) / n;
        p[faceComponents[face][1]] = (v * faceConstants[face][1]) / n;
        p[faceComponents[face][2]] = faceConstants[face][2] / n;
        return UnitVector3d::fromNormalized(p[0], p[1], p[2]);
    }

    std::tuple<int32_t, int32_t> faceToGrid(int level, double u, double v) {
        double const gridScale = GRID_SCALE[level];
        double const stMax = ST_MAX[level];
        double s = u * gridScale + gridScale;
        double t = v * gridScale + gridScale;
        s = std::max(std::min(s, stMax), 0.0);
        t = std::max(std::min(t, stMax), 0.0);
        return std::make_tuple(static_cast<int32_t>(s),
                               static_cast<int32_t>(t));
    }

    std::tuple<double, double> gridToFace(int level, int32_t s, int32_t t) {
        double const faceScale = FACE_SCALE[level];
        return std::make_tuple(static_cast<double>(s) * faceScale - 1.0,
                               static_cast<double>(t) * faceScale - 1.0);
    }

    std::tuple<double, double> atanApprox(double u, double v) {
        return std::make_tuple(
            u * (1.3333333333333333 - 0.3333333333333333 * std::fabs(u)),
            v * (1.3333333333333333 - 0.3333333333333333 * std::fabs(v))
        );
    }

    std::tuple<double, double> atanApproxInverse(double u, double v) {
        return std::make_tuple(
            std::copysign(2.0 - std::sqrt(4.0 - 3.0 * std::fabs(u)), u),
            std::copysign(2.0 - std::sqrt(4.0 - 3.0 * std::fabs(v)), v)
        );
    }

#else

    int faceNumber(UnitVector3d const & v, uint8_t const (&faceNumbers)[64]) {
        __m128d const m00 = _mm_set_pd(0.0, -0.0);
        __m128d xx = _mm_set1_pd(v.x());
        __m128d yy = _mm_set1_pd(v.y());
        __m128d zz = _mm_set1_pd(v.z());
        __m128d myy = _mm_xor_pd(yy, m00);
        __m128d mzz = _mm_xor_pd(zz, m00);
        int index = (_mm_movemask_pd(_mm_cmpgt_pd(xx, myy)) << 4) +
                    (_mm_movemask_pd(_mm_cmpgt_pd(xx, mzz)) << 2) +
                     _mm_movemask_pd(_mm_cmpgt_pd(yy, mzz));
        return faceNumbers[index];
    }

    UnitVector3d faceToSphere(int face,
                              __m128d uv,
                              uint8_t const (&faceComponents)[6][4],
                              double const (&faceConstants)[6][4])
    {
        double p[3];
        __m128d norm = _mm_mul_pd(uv, uv);
        norm = _mm_sqrt_sd(
            _mm_setzero_pd(),
            _mm_add_sd(
                _mm_set_sd(1.0),
                _mm_add_sd(norm, _mm_unpackhi_pd(norm, _mm_setzero_pd()))
            )
        );
        __m128d w = _mm_set_sd(faceConstants[face][2]);
        w = _mm_div_sd(w, norm);
        uv = _mm_mul_pd(
            _mm_div_pd(uv, _mm_shuffle_pd(norm, norm, 0)),
            _mm_load_pd(&faceConstants[face][0])
        );
        _mm_store_sd(&p[faceComponents[face][0]], uv);
        _mm_storeh_pd(&p[faceComponents[face][1]], uv);
        _mm_store_sd(&p[faceComponents[face][2]], w);
        return UnitVector3d::fromNormalized(p[0], p[1], p[2]);
    }

    __m128i faceToGrid(int level, __m128d uv) {
        __m128d const gridScale = _mm_set1_pd(GRID_SCALE[level]);
        __m128d const stMax = _mm_set1_pd(ST_MAX[level]);
        __m128d st = _mm_add_pd(_mm_mul_pd(uv, gridScale), gridScale);
        st = _mm_min_pd(_mm_max_pd(st, _mm_setzero_pd()), stMax);
        return _mm_shuffle_epi32(_mm_cvttpd_epi32(st), 0x98);
    }

    __m128d gridToFace(int level, __m128i st) {
        __m128d const faceScale = _mm_set1_pd(FACE_SCALE[level]);
        __m128d xy = _mm_cvtepi32_pd(_mm_shuffle_epi32(st, 8));
        return _mm_sub_pd(_mm_mul_pd(xy, faceScale), _mm_set1_pd(1.0));
    }

    __m128d atanApprox(__m128d uv) {
        __m128d abs_uv = _mm_andnot_pd(_mm_set_pd(-0.0, -0.0), uv);
        return _mm_mul_pd(
            uv,
            _mm_sub_pd(
                _mm_set1_pd(1.3333333333333333),
                _mm_mul_pd(_mm_set1_pd(0.3333333333333333), abs_uv)
            )
        );
    }

    __m128d atanApproxInverse(__m128d uv) {
        __m128d signbits = _mm_and_pd(uv, _mm_set_pd(-0.0, -0.0));
        __m128d abs_uv = _mm_andnot_pd(_mm_set_pd(-0.0, -0.0), uv);
        __m128d tmp = _mm_sub_pd(_mm_set1_pd(4.0),
                                 _mm_mul_pd(_mm_set1_pd(3.0), abs_uv));
        uv = _mm_sub_pd(_mm_set1_pd(2.0), _mm_sqrt_pd(tmp));
        return _mm_or_pd(signbits, uv);
    }

#endif

} // unnamed namespace
}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_Q3CPIXELIZATIONIMPL_H_
