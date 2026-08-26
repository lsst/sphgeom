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
/// \brief This file contains the shortest round-trip double formatter.

#include "lsst/sphgeom/floatFormat.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#if SPHGEOM_HAVE_FP_TO_CHARS
#include <charconv>
#endif

namespace lsst {
namespace sphgeom {

namespace {

// The number of significant decimal digits needed to round-trip any double.
constexpr int MAX_SIGNIFICANT_DIGITS = 17;

// Buffer large enough for "-d.<16 digits>e-308" and a terminating NUL.
constexpr std::size_t SCRATCH_SIZE = 32;

/// `roundTrips` returns true when `value` written with `n` significant
/// decimal digits converts back to `value` exactly, and stores that
/// representation in `scratch`.
bool roundTrips(char (&scratch)[SCRATCH_SIZE], double value, int n) {
    std::snprintf(scratch, SCRATCH_SIZE, "%.*e", n - 1, value);
    return std::strtod(scratch, nullptr) == value;
}

/// `decimalDigits` returns the number of decimal digits in |e|.
int decimalDigits(int e) {
    int n = 1;
    for (unsigned u = static_cast<unsigned>(e < 0 ? -e : e); u >= 10; u /= 10) {
        ++n;
    }
    return n;
}

} // unnamed namespace

namespace detail {

void appendShortestDoubleFallback(std::string & out, double value) {
    if (std::isnan(value)) {
        out.append("nan");
        return;
    }
    if (std::isinf(value)) {
        out.append(value < 0.0 ? "-inf" : "inf");
        return;
    }
    // Find the smallest number of significant digits that round-trips. The
    // property is monotonic in the digit count: rounding to one more digit
    // can never move the result further from value, because the shorter
    // representation padded with a zero is one of the candidates. That makes
    // a binary search valid.
    char scratch[SCRATCH_SIZE];
    int lo = 1;
    int hi = MAX_SIGNIFICANT_DIGITS;
    while (lo < hi) {
        int const mid = lo + (hi - lo) / 2;
        if (roundTrips(scratch, value, mid)) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    roundTrips(scratch, value, lo);
    // Split the "[-]d[.ddd]e[+-]dd" scratch representation into its sign,
    // significand digits and decimal exponent. Because the digit count is
    // minimal the significand never has trailing zeros. Anything between the
    // significand digits is the radix character of the current locale and is
    // discarded; the output below always uses '.'.
    char const * p = scratch;
    bool const negative = (*p == '-');
    if (negative) {
        ++p;
    }
    char digits[MAX_SIGNIFICANT_DIGITS];
    int n = 0;
    for (; *p != 'e'; ++p) {
        if (*p >= '0' && *p <= '9') {
            digits[n++] = *p;
        }
    }
    int const exponent = std::atoi(p + 1);
    // Fixed notation writes exponent + 1 characters when the decimal point
    // falls at or past the last digit, one extra character for an embedded
    // point, and a leading "0." plus padding zeros for negative exponents.
    std::size_t fixedLength;
    if (exponent >= n - 1) {
        fixedLength = static_cast<std::size_t>(exponent + 1);
    } else if (exponent >= 0) {
        fixedLength = static_cast<std::size_t>(n + 1);
    } else {
        fixedLength = static_cast<std::size_t>(n - exponent + 1);
    }
    // Scientific notation writes the significand, an optional point, 'e', the
    // exponent sign and at least two exponent digits.
    int expDigits = decimalDigits(exponent);
    if (expDigits < 2) {
        expDigits = 2;
    }
    std::size_t const scientificLength =
        static_cast<std::size_t>(n + (n > 1 ? 1 : 0) + 2 + expDigits);

    if (negative) {
        out.push_back('-');
    }
    if (fixedLength <= scientificLength) {
        if (exponent >= n - 1) {
            // The significand is followed only by zeros, so the decimal
            // expansion of value has the same length as the minimal digits
            // padded with zeros - and it is the representation closest to
            // value, which is how ties on length are broken.
            std::snprintf(scratch, SCRATCH_SIZE, "%.0f", std::fabs(value));
            out.append(scratch);
        } else if (exponent >= 0) {
            out.append(digits, exponent + 1);
            out.push_back('.');
            out.append(digits + exponent + 1, n - exponent - 1);
        } else {
            out.append("0.");
            out.append(static_cast<std::size_t>(-exponent - 1), '0');
            out.append(digits, n);
        }
    } else {
        out.push_back(digits[0]);
        if (n > 1) {
            out.push_back('.');
            out.append(digits + 1, n - 1);
        }
        out.push_back('e');
        std::snprintf(scratch, SCRATCH_SIZE, "%+03d", exponent);
        out.append(scratch);
    }
}

} // namespace detail

void appendShortestDouble(std::string & out, double value) {
#if SPHGEOM_HAVE_FP_TO_CHARS
    char buf[MAX_SHORTEST_DOUBLE_CHARS];
    auto const r = std::to_chars(buf, buf + sizeof(buf), value);
    out.append(buf, r.ptr - buf);
#else
    detail::appendShortestDoubleFallback(out, value);
#endif
}

}} // namespace lsst::sphgeom
