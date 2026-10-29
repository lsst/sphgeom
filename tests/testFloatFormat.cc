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
/// \brief This file contains tests for the shortest round-trip double
///        formatter.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>
#include <string>

#include "lsst/sphgeom/floatFormat.h"

#include "test.h"

#if SPHGEOM_HAVE_FP_TO_CHARS
#include <charconv>
#endif


using namespace lsst::sphgeom;

namespace {

std::string format(double v) {
    std::string s;
    appendShortestDouble(s, v);
    return s;
}

std::string formatFallback(double v) {
    std::string s;
    detail::appendShortestDoubleFallback(s, v);
    return s;
}

/// `minimalDigits` returns the fewest significant decimal digits that
/// round-trip to `v`, found by linear scan rather than by the binary search
/// the fallback uses. The scan starts from the 17 digits that always suffice
/// and shortens, because almost every double needs 16 or 17 of them.
int minimalDigits(double v) {
    char buf[32];
    for (int n = 17; n > 1; --n) {
        std::snprintf(buf, sizeof(buf), "%.*e", n - 2, v);
        if (std::strtod(buf, nullptr) != v) {
            return n;
        }
    }
    return 1;
}

/// `shortestLength` returns the length of the shortest string that round-trips
/// to the finite value `v`, computed independently of the formatter.
size_t shortestLength(double v) {
    int const n = minimalDigits(v);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*e", n - 1, v);
    int const exponent = std::atoi(std::strchr(buf, 'e') + 1);
    size_t fixed;
    if (exponent >= n - 1) {
        fixed = static_cast<size_t>(exponent + 1);
    } else if (exponent >= 0) {
        fixed = static_cast<size_t>(n + 1);
    } else {
        fixed = static_cast<size_t>(n - exponent + 1);
    }
    int expDigits = 2;
    for (int e = std::abs(exponent); e >= 100; e /= 10) {
        ++expDigits;
    }
    size_t const scientific = static_cast<size_t>(n + (n > 1 ? 1 : 0) + 2 + expDigits);
    size_t const length = fixed <= scientific ? fixed : scientific;
    return length + (std::signbit(v) ? 1 : 0);
}

/// `checkValue` verifies that both formatters write `v` as a shortest string
/// that converts back to `v` exactly.
void checkValue(double v) {
    for (std::string const & s : {format(v), formatFallback(v)}) {
        CHECK(std::strtod(s.c_str(), nullptr) == v);
        CHECK(s.size() == shortestLength(v));
        CHECK(s.size() <= MAX_SHORTEST_DOUBLE_CHARS);
    }
}

/// `randomFinite` returns a finite double drawn from a uniform distribution
/// over bit patterns, covering every magnitude including subnormals.
double randomFinite(std::mt19937_64 & gen) {
    for (;;) {
        std::uint64_t const u = gen();
        double v;
        std::memcpy(&v, &u, sizeof(v));
        if (std::isfinite(v)) {
            return v;
        }
    }
}

} // unnamed namespace


TEST_CASE(NonFinite) {
    double const inf = std::numeric_limits<double>::infinity();
    CHECK(format(inf) == "inf");
    CHECK(format(-inf) == "-inf");
    CHECK(format(std::numeric_limits<double>::quiet_NaN()) == "nan");
    CHECK(formatFallback(inf) == "inf");
    CHECK(formatFallback(-inf) == "-inf");
    CHECK(formatFallback(std::numeric_limits<double>::quiet_NaN()) == "nan");
}

TEST_CASE(KnownRepresentations) {
    struct { double value; char const * expected; } const cases[] = {
        {0.0, "0"},
        {-0.0, "-0"},
        {1.0, "1"},
        {-1.5, "-1.5"},
        {90.0, "90"},
        {359.999, "359.999"},
        {0.1, "0.1"},
        // Fixed and scientific notation are the same length here, and fixed
        // wins.
        {10000.0, "10000"},
        {0.001, "0.001"},
        {0.00012345, "0.00012345"},
        // Scientific notation is strictly shorter here.
        {100000.0, "1e+05"},
        {0.0001, "1e-04"},
        {1e15, "1e+15"},
        {5e-324, "5e-324"},
        {5.551115123125783e-17, "5.551115123125783e-17"},
        // Fixed notation is shorter, and prints the exact value rather than
        // the shortest significand padded with zeros.
        {1234567890123456.0, "1234567890123456"},
        {1.2003693070755666e19, "12003693070755665920"}
    };
    for (auto const & c : cases) {
        CHECK(format(c.value) == c.expected);
        CHECK(formatFallback(c.value) == c.expected);
    }
}

TEST_CASE(RoundTrip) {
    double const values[] = {
        0.0, -0.0, 1.0, -1.0, 0.5, 1e3, 1e4, 1e5, 1e-3, 1e-4, 1e-5, 1e16,
        1e17, 1e21, 1e308, 5e-324, 90.0, 180.0, 359.999,
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::denorm_min(),
        std::numeric_limits<double>::epsilon()
    };
    for (double v : values) {
        checkValue(v);
    }
    std::mt19937_64 gen(42);
    for (int i = 0; i < 50000; ++i) {
        checkValue(randomFinite(gen));
    }
    // Angles in degrees are what the STC-S writers actually format.
    std::uniform_real_distribution<double> angle(-360.0, 360.0);
    for (int i = 0; i < 50000; ++i) {
        checkValue(angle(gen));
    }
}

#if SPHGEOM_HAVE_FP_TO_CHARS
TEST_CASE(FallbackMatchesToChars) {
    // The fallback is what macOS builds with an old deployment target use, so
    // it must be indistinguishable from std::to_chars wherever the latter is
    // available - otherwise one release of the library would emit different
    // STC-S text on different platforms.
    auto check = [](double v) {
        char buf[MAX_SHORTEST_DOUBLE_CHARS];
        auto const r = std::to_chars(buf, buf + sizeof(buf), v);
        CHECK(formatFallback(v) == std::string(buf, r.ptr));
    };
    std::mt19937_64 gen(1234);
    for (int i = 0; i < 200000; ++i) {
        check(randomFinite(gen));
    }
    std::uniform_real_distribution<double> angle(-360.0, 360.0);
    for (int i = 0; i < 100000; ++i) {
        check(angle(gen));
    }
    // Powers of ten and their small multiples straddle every fixed versus
    // scientific notation boundary.
    for (int m = -30; m <= 30; ++m) {
        for (int k = 0; k <= 500; ++k) {
            check(k * std::pow(10.0, m));
        }
    }
}
#endif
