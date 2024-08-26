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

#ifndef LSST_SPHGEOM_BIGINTEGER_H_
#define LSST_SPHGEOM_BIGINTEGER_H_

/// \file
/// \brief This file declares a class for arbitrary precision integers.

#include <cstring>
#include <stdexcept>
#include <cstdint>

namespace lsst {
namespace sphgeom {

/// `BigInteger` is an arbitrary precision signed integer class. It is
/// intended to be used in applications which need relatively small integers,
/// and only supports addition, subtraction and multiplication.
///
/// Internally, a BigInteger consists of a sign and an unsigned magnitude. The
/// magnitude is represented by an array of 32 bit digits, stored from least
/// to most significant. All non-zero integers have at least one digit, the
/// most significant of which is non-zero. Zero is defined to have no digits.
class BigInteger {
public:
    /// This constructor creates a zero-valued integer with the given digit
    /// array.
    BigInteger(std::uint32_t * digits, unsigned capacity) :
        _digits(digits),
        _capacity(capacity),
        _size(0),
        _sign(0)
    {}

    BigInteger & operator=(BigInteger const & b) {
        if (this != &b) {
            _checkCapacity(b._size);
            _sign = b._sign;
            _size = b._size;
            std::memcpy(_digits, b._digits, sizeof(std::uint32_t) * b._size);
        }
        return *this;
    }

    /// `getSign` returns -1, 0 or 1 if this integer is negative, zero or
    /// positive.
    int getSign() const { return _sign; }

    /// `getSize` returns the number of digits in the value of this integer.
    unsigned getSize() const { return _size; }

    /// `getCapacity` returns the number of digits in the underlying digit
    /// array.
    unsigned getCapacity() const { return _capacity; }

    /// `getDigits` returns the underlying digit array.
    std::uint32_t const * getDigits() const { return _digits; }

    /// `setToZero` sets this integer to zero.
    void setToZero() { _sign = 0; _size = 0; }

    /// `setTo` sets this integer to the given signed 64 bit integer value.
    void setTo(std::int64_t x) {
        if (x < 0) {
            setTo(static_cast<std::uint64_t>(-x));
            _sign = -1;
        } else {
            setTo(static_cast<std::uint64_t>(x));
        }
    }

    /// `setTo` sets this integer to the given unsigned 64 bit integer value.
    void setTo(std::uint64_t x) {
        _checkCapacity(2);
        _digits[0] = static_cast<std::uint32_t>(x);
        _digits[1] = static_cast<std::uint32_t>(x >> 32);
        _size = (_digits[1] == 0) ? (_digits[0] != 0) : 2;
        _sign = (_size != 0);
    }

    /// `negate` multiplies this integer by -1.
    void negate() { _sign = -_sign; }

    /// `add` adds b to this integer.
    BigInteger & add(BigInteger const & b);

    /// `subtract` subtracts b from this integer.
    BigInteger & subtract(BigInteger const & b);

    /// `multiplyPow2` multiplies this integer by 2ⁿ.
    BigInteger & multiplyPow2(unsigned n);

    /// `multiply` multiplies this integer by b.
    BigInteger & multiply(BigInteger const & b);

private:

    void _checkCapacity(unsigned n) const {
        if (_capacity < n) {
            throw std::runtime_error("BigInteger capacity is too small");
        }
    }

    std::uint32_t * _digits; // Unowned
    unsigned _capacity;
    unsigned _size;
    int _sign;
};

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_BIGINTEGER_H_
