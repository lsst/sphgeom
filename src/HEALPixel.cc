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
/// \brief This file contains the HEALPixel class implementation.

#include "lsst/sphgeom/HEALPixel.h"

#include <mutex>
#include <ostream>

#include "lsst/sphgeom/codec.h"

namespace lsst {
namespace sphgeom {

namespace {

inline std::uint64_t parent(int level, HEALPixel const & p) {
    return p.nested() >> (2*(p.level() - level));
}

} // anonymous

bool HEALPixel::contains(HEALPixel const & p) const {
    if (level() < p.level()) {
        return nested() == parent(level(), p);
    } else if (level() > p.level()) {
        return false;
    } else {
        return nested() == p.nested();
    }
}

bool HEALPixel::isDisjointFrom(HEALPixel const & p) const {
    if (level() < p.level()) {
        return nested() != parent(level(), p);
    } else if (level() > p.level()) {
        return parent(p.level(), *this) != p.nested();
    } else {
        return nested() != p.nested();
    }
}

Relationship HEALPixel::relate(Region const & r) const {
    return invert(r.relate(*this));
}

Relationship HEALPixel::relate(HEALPixel const & p) const {
    if (level() < p.level()) {
        return (nested() == parent(level(), p)) ? CONTAINS : DISJOINT;
    } else if (level() > p.level()) {
        return (parent(p.level(), *this) == p.nested()) ? WITHIN : DISJOINT;
    } else {
        return (nested() == p.nested()) ? (CONTAINS | WITHIN) : DISJOINT;
    }
}

static std::mutex registeredDecoderMutex;
static HEALPixel::DecodeFunction registeredDecoder = nullptr;

void HEALPixel::registerDecodeFunction(DecodeFunction func) {
    std::lock_guard<std::mutex> guard(registeredDecoderMutex);
    registeredDecoder = func;
}

std::vector<uint8_t> HEALPixel::encode() const {
    std::vector<uint8_t> buffer;
    uint8_t tc = TYPE_CODE;
    buffer.reserve(ENCODED_SIZE);
    buffer.push_back(tc);
    buffer.push_back(static_cast<uint8_t>(_state.level));
    encodeU64(_state.nested, buffer);
    for (UnitVector3d const & v: _state.vertices) {
        encodeDouble(v.x(), buffer);
        encodeDouble(v.y(), buffer);
        encodeDouble(v.z(), buffer);
    }
    return buffer;
}

HEALPixelState HEALPixel::decodeState(uint8_t const * buffer, size_t n) {
    if (buffer == nullptr || n != ENCODED_SIZE || *buffer != TYPE_CODE) {
        throw std::runtime_error("Byte-string is not an encoded HEALPixel");
    }
    ++buffer;
    HEALPixelState state;
    state.level = *buffer; ++buffer;
    state.nested = decodeU64(buffer); buffer += 8;
    for (size_t i = 0; i < 4; ++i, buffer += 24) {
        state.vertices[i] = UnitVector3d::fromNormalized(
            decodeDouble(buffer),
            decodeDouble(buffer + 8),
            decodeDouble(buffer + 16)
        );
    }
    return state;
}

std::unique_ptr<HEALPixel> HEALPixel::decode(uint8_t const * buffer, size_t n) {
    auto state = decodeState(buffer, n);
    std::lock_guard<std::mutex> guard(registeredDecoderMutex);
    if (registeredDecoder == nullptr) {
        throw std::runtime_error("No decode callback registered for HEALPixel regions.");
    }
    return registeredDecoder(state);
}

    bool HEALPixel::contains(const UnitVector3d &v) const {
        return false;
    }

    std::ostream & operator<<(std::ostream & os, HEALPixel const & p) {
    return os << "{\"HEALPixel\": [level=" << p.level()
              << ", nested=" << p.nested() << "]";
}

}} // namespace lsst::sphgeom
