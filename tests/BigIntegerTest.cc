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
/// \brief This file contains tests for the Angle class.

#include "BigInteger.h"

#include "Test.h"


using namespace lsst::sphgeom;

TEST_CASE(Construction) {
    uint32_t buffer[8];
    BigInteger b(buffer, sizeof(buffer) / sizeof(uint32_t));
    CHECK(b.getSign() == 0);
    CHECK(b.getSize() == 0);
    CHECK(b.getDigits() == buffer);
    CHECK(b.getCapacity() == sizeof(buffer) / sizeof(uint32_t));
}

TEST_CASE(Basic) {
    uint32_t buffer1[8];
    uint32_t buffer2[1];
    BigInteger b1(buffer1, sizeof(buffer1) / sizeof(uint32_t));
    BigInteger b2(buffer2, sizeof(buffer2) / sizeof(uint32_t));
    uint64_t const i = (static_cast<uint64_t>(2) << 32) + 1;
    b1.setTo(i);
    CHECK(b1.getSign() == 1);
    CHECK(b1.getSize() == 2);
    CHECK(b1.getDigits()[0] == 1);
    CHECK(b1.getDigits()[1] == 2);
    b1.setToZero();
    CHECK(b1.getSign() == 0);
    CHECK(b1.getSize() == 0);
    CHECK_THROW(b2.setTo(i), std::runtime_error);
}

TEST_CASE(AddSubtract1) {
    uint32_t buffer1[8], buffer2[8], buffer3[8];
    BigInteger b1(buffer1, sizeof(buffer1) / sizeof(uint32_t));
    BigInteger b2(buffer2, sizeof(buffer2) / sizeof(uint32_t));
    BigInteger b3(buffer3, sizeof(buffer3) / sizeof(uint32_t));
    uint64_t const a = 0xffffffff;
    uint64_t const i = (a << 32);
    b1.setTo(a); // b1 = 2^32 - 1
    b2.setTo(i); // b2 = 2^64 - 2^32
    b1.add(b2); // b1 = 2^64 - 1
    b2 = b1;
    b1.add(b2); // b1 = 2^65 - 2
    CHECK(b1.getSign() == 1 && b1.getSize() == 3);
    CHECK(b1.getDigits()[0] == a - 1);
    CHECK(b1.getDigits()[1] == a);
    CHECK(b1.getDigits()[2] == 1);
    b2.setTo(a);
    b2.negate(); // b2 = 1 - 2^32
    b3 = b2;
    b2.add(b1); // b2 = 2^65 - 2^32 - 1
    b1.add(b3); // b1 = 2^65 - 2^32 - 1
    CHECK(b2.getSign() == 1 && b2.getSize() == 3);
    CHECK(b2.getDigits()[0] == a);
    CHECK(b2.getDigits()[1] == a - 1);
    CHECK(b2.getDigits()[2] == 1);
    CHECK(b1.getSign() == 1 && b1.getSize() == 3);
    CHECK(b1.getDigits()[0] == a);
    CHECK(b1.getDigits()[1] == a - 1);
    CHECK(b1.getDigits()[2] == 1);
    b3 = b1;
    b3.subtract(b1); // b3 = 0
    CHECK(b3.getSign() == 0);
    CHECK(b3.getSize() == 0);
}

TEST_CASE(AddSubtract2) {
    uint32_t buffer1[8], buffer2[8], buffer3[8];
    BigInteger b1(buffer1, sizeof(buffer1) / sizeof(uint32_t));
    BigInteger b2(buffer2, sizeof(buffer2) / sizeof(uint32_t));
    BigInteger b3(buffer3, sizeof(buffer3) / sizeof(uint32_t));
    uint64_t const a = 0xffffffff;
    b1.setTo(a);
    b1.add(b1); // b1 = 2^33 - 2
    b2.setTo(static_cast<uint64_t>(2));
    b1.add(b2); // b1 = 2^33
    CHECK(b1.getSign() == 1 && b1.getSize() == 2);
    CHECK(b1.getDigits()[0] == 0);
    CHECK(b1.getDigits()[1] == 2);
    b2.setTo(a);
    b2.add(b2); // b2 = 2^33 - 2
    b1.subtract(b2); // b1 = 2
    CHECK(b1.getSign() == 1 && b1.getSize() == 1);
    CHECK(b1.getDigits()[0] == 2);
}

TEST_CASE(AddSubtract3) {
    uint32_t buffer1[8], buffer2[8], buffer3[8];
    BigInteger b1(buffer1, sizeof(buffer1) / sizeof(uint32_t));
    BigInteger b2(buffer2, sizeof(buffer2) / sizeof(uint32_t));
    BigInteger b3(buffer3, sizeof(buffer3) / sizeof(uint32_t));
    b1.setTo(static_cast<uint64_t>(1));
    b2.setToZero();
    b1.add(b2);
    CHECK(b1.getSign() == 1 && b1.getSize() == 1 && b1.getDigits()[0] == 1);
    b2.add(b1);
    CHECK(b2.getSign() == 1 && b2.getSize() == 1 && b2.getDigits()[0] == 1);
    b2.add(b2);
    b2.negate();
    b1.add(b2);
    CHECK(b1.getSign() == -1 && b1.getSize() == 1 && b1.getDigits()[0] == 1);
    b1.subtract(b1);
    CHECK(b1.getSign() == 0 && b1.getSize() == 0);
}

TEST_CASE(MultiplyPow2) {
    uint32_t buffer[8];
    BigInteger b(buffer, sizeof(buffer) / sizeof(uint32_t));
    b.setTo(static_cast<uint64_t>(1));
    // Shift by 0
    b.multiplyPow2(0);
    CHECK(b.getSign() == 1 && b.getSize() == 1);
    CHECK(b.getDigits()[0] == 1);
    // Shift by an exact multiple of the digit size
    b.multiplyPow2(32);
    CHECK(b.getSign() == 1 && b.getSize() == 2);
    CHECK(b.getDigits()[0] == 0);
    CHECK(b.getDigits()[1] == 1);
    // Shift by less than the digit size
    b.multiplyPow2(31);
    CHECK(b.getSign() == 1 && b.getSize() == 2);
    CHECK(b.getDigits()[0] == 0);
    CHECK(b.getDigits()[1] == (static_cast<uint32_t>(1) << 31));
    b.multiplyPow2(1);
    CHECK(b.getSign() == 1 && b.getSize() == 3);
    CHECK(b.getDigits()[0] == 0);
    CHECK(b.getDigits()[1] == 0);
    CHECK(b.getDigits()[2] == 1);
    // Shift by more than the digit size
    b.multiplyPow2(33);
    CHECK(b.getSign() == 1 && b.getSize() == 4);
    CHECK(b.getDigits()[0] == 0);
    CHECK(b.getDigits()[1] == 0);
    CHECK(b.getDigits()[2] == 0);
    CHECK(b.getDigits()[3] == 2);
    b.setToZero();
    b.multiplyPow2(1234567);
    CHECK(b.getSign() == 0 && b.getSize() == 0);
}

TEST_CASE(Multiply1) {
    uint32_t buffer[8];
    BigInteger b(buffer, sizeof(buffer) / sizeof(uint32_t));
    uint64_t const a = 0xffffffff;
    b.setTo(a);
    b.multiply(b);
    CHECK(b.getSize() == 2);
    CHECK(b.getDigits()[0] == 1);
    CHECK(b.getDigits()[1] == a - 1);
}

TEST_CASE(Multiply2) {
    uint32_t buffer1[8], buffer2[8];
    BigInteger b1(buffer1, sizeof(buffer1) / sizeof(uint32_t));
    BigInteger b2(buffer2, sizeof(buffer2) / sizeof(uint32_t));
    uint64_t const a = 0xffffffff;
    uint64_t const i = (a << 32) + a;
    b1.setTo(i);
    b1.multiplyPow2(32);
    b2.setTo(a);
    b1.add(b2);
    b2.setTo(i);
    b1.multiply(b2);
    CHECK(b1.getSize() == 5);
    CHECK(b1.getDigits()[0] == 1);
    CHECK(b1.getDigits()[1] == 0);
    CHECK(b1.getDigits()[2] == a);
    CHECK(b1.getDigits()[3] == a - 1);
    CHECK(b1.getDigits()[4] == a);
}

TEST_CASE(Multiply3) {
    uint32_t buffer1[8], buffer2[8];
    BigInteger b1(buffer1, sizeof(buffer1) / sizeof(uint32_t));
    BigInteger b2(buffer2, sizeof(buffer2) / sizeof(uint32_t));
    uint64_t const one = 1;
    b1.setTo(one);
    b2.setToZero();
    b1.multiply(b2);
    CHECK(b1.getSign() == 0 && b1.getSize() == 0);
    b1.setTo(one);
    b2.setTo(one);
    b2.multiplyPow2(32);
    b1.multiply(b2);
    CHECK(b1.getSign() == 1 && b1.getSize() == 2);
    CHECK(b1.getDigits()[0] == 0);
    CHECK(b1.getDigits()[1] == 1);
}
