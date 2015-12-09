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
/// \brief This file contains tests for the Matrix3d class.

#include "lsst/sphgeom/Matrix3d.h"

#include "Test.h"


using namespace lsst::sphgeom;

struct Fixture {
    Matrix3d M1;
    Matrix3d M2;
    Fixture();
};

Fixture::Fixture() {
    M1 = Matrix3d(1, 2, 3,
                  4, 5, 6,
                  7, 8, 9);
    M2 = Matrix3d(10, 11, 12,
                  13, 14, 15,
                  16, 17, 18);
}

FIXTURE_TEST_CASE(Stream, Fixture) {
    Matrix3d M(Vector3d(-1, 10, 100));
    std::stringstream ss;
    ss << M;
    CHECK(ss.str() == "Matrix3d(-1,  0,   0,\n"
                      "          0, 10,   0,\n"
                      "          0,  0, 100)");
    ss.str(std::string());
    M1.print(ss, 4);
    CHECK(ss.str() == "    Matrix3d(1, 2, 3,\n"
                      "             4, 5, 6,\n"
                      "             7, 8, 9)");
}

TEST_CASE(Comparison) {
    double e[3][3] = {{0.0, 0.0, 0.0},
                      {0.0, 0.0, 0.0},
                      {0.0, 0.0, 0.0}};
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            e[r][c] = 1.0;
            Matrix3d M1(e[0][0], e[0][1], e[0][2],
                        e[1][0], e[1][1], e[1][2],
                        e[2][0], e[2][1], e[2][2]);
            Matrix3d M2(-e[0][0], -e[0][1], -e[0][2],
                        -e[1][0], -e[1][1], -e[1][2],
                        -e[2][0], -e[2][1], -e[2][2]);
            CHECK(M1 == M1);
            CHECK(M1 != M2);
            CHECK(!(M1 == M2));
            CHECK(!(M2 != M2));
        }
    }
}

FIXTURE_TEST_CASE(ComponentAccess, Fixture) {
    Matrix3d Z;
    for (int c = 0; c < 3; ++c) {
        CHECK(Z.getColumn(c).isZero());
        for (int r = 0; r < 3; ++r) {
            CHECK(Z(r, c) == 0);
        }
    }
    double e[3][3];
    for (int c = 0; c < 3; ++c) {
        for (int r = 0; r < 3; ++r) {
            e[r][c] = r*3 + c + 1;
            CHECK(M1(r, c) == r*3 + c + 1);
        }
    }
    Matrix3d T1(e[0][0], e[0][1], e[0][2],
                e[1][0], e[1][1], e[1][2],
                e[2][0], e[2][1], e[2][2]);
    CHECK(M1 == T1);
    CHECK(M1 != Z);
}

FIXTURE_TEST_CASE(Identity, Fixture) {
    Matrix3d I(1);
    for (int c = 0; c < 3; ++c) {
        for (int r = 0; r < 3; ++r) {
            if (r == c) {
                CHECK(I(r,c) == 1);
            } else {
                CHECK(I(r,c) == 0);
            }
        }
    }
    CHECK(M1 * I == M1);
    CHECK(I * M1 == M1);
}

FIXTURE_TEST_CASE(Diagonal, Fixture) {
    Matrix3d D(Vector3d(0, 1, 2));
    Matrix3d S(2);
    for (int c = 0; c < 3; ++c) {
        for (int r = 0; r < 3; ++r) {
            if (r == c) {
                CHECK(D(r,c) == r);
            } else {
                CHECK(D(r,c) == 0);
            }
        }
    }
    CHECK(M1 * S == M1 + M1);
    CHECK(S * M1 == M1 * S);
}

FIXTURE_TEST_CASE(CwiseProduct, Fixture) {
    Matrix3d R( 10,  22,  36,
                52,  70,  90,
               112, 136, 162);
    CHECK(M1.cwiseProduct(M2) == R);
}

FIXTURE_TEST_CASE(Inner, Fixture) {
    Matrix3d I(1);
    CHECK(M1.inner(I) == I.inner(M1));
    CHECK(M1.inner(I) == 15);
}

FIXTURE_TEST_CASE(Norm, Fixture) {
    Matrix3d I(1);
    CHECK(I.getSquaredNorm() == 3);
    CHECK_CLOSE(I.getNorm(), std::sqrt(3), 1);
    CHECK(M1.getSquaredNorm() == (9*(9 + 1)*(2*9 + 1))/6);
    CHECK(M1.getNorm() == std::sqrt((9*(9 + 1)*(2*9 + 1))/6));
}

FIXTURE_TEST_CASE(Sum, Fixture) {
    Matrix3d R(11, 13, 15,
               17, 19, 21,
               23, 25, 27);
    CHECK(M1 + M2 == R);
}

FIXTURE_TEST_CASE(Difference, Fixture) {
    Matrix3d R(9, 9, 9,
               9, 9, 9,
               9, 9, 9);
    CHECK(M2 - M1 == R);
    CHECK((M1 + M2) - M1 == M2);
}

FIXTURE_TEST_CASE(Product, Fixture) {
    Matrix3d I(1);
    Vector3d v(1, 2, 3);
    CHECK(I * v == v);
    Matrix3d M(1, -1, 0,
               1,  1, 0,
               0,  0, 1);
    Matrix3d N( 1, 1, 0,
               -1, 1, 0,
                0, 0, 1);
    CHECK(N * (M * v) == Vector3d(2, 4, 3));
    Matrix3d R12( 84,  90,  96,
                 201, 216, 231,
                 318, 342, 366);
    Matrix3d R21(138, 171, 204,
                 174, 216, 258,
                 210, 261, 312);
    CHECK(M1 * M2 == R12);
    CHECK(M2 * M1 == R21);
}

FIXTURE_TEST_CASE(Transpose, Fixture) {
    Matrix3d R(1, 4, 7,
               2, 5, 8,
               3, 6, 9);
    CHECK(M1.transpose() == R);
    CHECK(M1.transpose().transpose() == M1);
}

TEST_CASE(Inverse) {
    Matrix3d I(1);
    Matrix3d N( 4,  4,  4,
               -1,  1,  0,
                1, -1, -1);
    Matrix3d M = N.inverse();
    Matrix3d R(0.125,  0, 0.5,
               0.125,  1, 0.5,
                   0, -1,  -1);
    CHECK(M == R);
    CHECK(N * M == I);
    CHECK(M * N == I);
}
