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

#ifndef LSST_SPHGEOM_CODEC_H_
#define LSST_SPHGEOM_CODEC_H_

// Optimized path requires little endian arch and support for unaligned loads.
#if defined(__x86_64__) or          \
    (defined(__aarch64__) and       \
     (defined(__LITTLE_ENDIAN__) or \
      (defined(__BYTE_ORDER__) and __BYTE_ORDER__ == 1234)))
#define OPTIMIZED_LITTLE_ENDIAN
#endif

#ifdef NO_OPTIMIZED_PATHS
#undef OPTIMIZED_LITTLE_ENDIAN
#endif

/// \file
/// \brief This file contains simple helper functions for encoding and
///        decoding primitive types to/from byte strings.

#include <vector>

namespace lsst {
namespace sphgeom {

/// `encodeDouble` appends an IEEE double in little-endian byte order
/// to the end of buffer.
inline void encodeDouble(double item, std::vector<uint8_t> & buffer) {
#ifdef OPTIMIZED_LITTLE_ENDIAN
    auto ptr = reinterpret_cast<uint8_t const *>(&item);
    buffer.insert(buffer.end(), ptr, ptr + 8);
#else
    union { uint64_t u; double d; };
    d = item;
    buffer.push_back(static_cast<uint8_t>(u));
    buffer.push_back(static_cast<uint8_t>(u >> 8));
    buffer.push_back(static_cast<uint8_t>(u >> 16));
    buffer.push_back(static_cast<uint8_t>(u >> 24));
    buffer.push_back(static_cast<uint8_t>(u >> 32));
    buffer.push_back(static_cast<uint8_t>(u >> 40));
    buffer.push_back(static_cast<uint8_t>(u >> 48));
    buffer.push_back(static_cast<uint8_t>(u >> 56));
#endif
}

/// `decodeDouble` extracts an IEEE double from the 8 byte little-endian byte
/// sequence in buffer.
inline double decodeDouble(uint8_t const * buffer) {
#ifdef OPTIMIZED_LITTLE_ENDIAN
    return *reinterpret_cast<double const *>(buffer);
#else
    union { uint64_t u; double d; };
    u = static_cast<uint64_t>(buffer[0]) +
        (static_cast<uint64_t>(buffer[1]) << 8) +
        (static_cast<uint64_t>(buffer[2]) << 16) +
        (static_cast<uint64_t>(buffer[3]) << 24) +
        (static_cast<uint64_t>(buffer[4]) << 32) +
        (static_cast<uint64_t>(buffer[5]) << 40) +
        (static_cast<uint64_t>(buffer[6]) << 48) +
        (static_cast<uint64_t>(buffer[7]) << 56);
    return d;
#endif
}

/// `encodeU64` appends an uint64 in little-endian byte order
/// to the end of buffer.
inline void encodeU64(std::uint64_t item, std::vector<uint8_t> & buffer) {
#ifdef OPTIMIZED_LITTLE_ENDIAN
    auto ptr = reinterpret_cast<uint8_t const *>(&item);
    buffer.insert(buffer.end(), ptr, ptr + 8);
#else
    union { uint64_t u; double d; };
    d = item;
    buffer.push_back(static_cast<uint8_t>(u));
    buffer.push_back(static_cast<uint8_t>(u >> 8));
    buffer.push_back(static_cast<uint8_t>(u >> 16));
    buffer.push_back(static_cast<uint8_t>(u >> 24));
    buffer.push_back(static_cast<uint8_t>(u >> 32));
    buffer.push_back(static_cast<uint8_t>(u >> 40));
    buffer.push_back(static_cast<uint8_t>(u >> 48));
    buffer.push_back(static_cast<uint8_t>(u >> 56));
#endif
}

/// `decodeU64` extracts an uint64 from the 8 byte little-endian byte
/// sequence in buffer.
inline std::uint64_t decodeU64(uint8_t const * buffer) {
#ifdef OPTIMIZED_LITTLE_ENDIAN
    return *reinterpret_cast<uint64_t const *>(buffer);
#else
    std::uint64_t u = static_cast<uint64_t>(buffer[0]) +
        (static_cast<uint64_t>(buffer[1]) << 8) +
        (static_cast<uint64_t>(buffer[2]) << 16) +
        (static_cast<uint64_t>(buffer[3]) << 24) +
        (static_cast<uint64_t>(buffer[4]) << 32) +
        (static_cast<uint64_t>(buffer[5]) << 40) +
        (static_cast<uint64_t>(buffer[6]) << 48) +
        (static_cast<uint64_t>(buffer[7]) << 56);
    return u;
#endif
}

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_CODEC_H_
