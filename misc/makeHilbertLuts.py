# Copyright (C) 2015 Pierre de Buyl and contributors
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   a. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#   b. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#   c. Neither the name of the <organization> nor the
#      names of its contributors may be used to endorse or promote products
#      derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" &
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Generates the lookup tables used by the C++ Hilbert index
functions in `lsst/sphgeom/curve.h`.
"""

import math

import numpy as np

N = 2


def bin_str(i):
    """Return a string representation of i with N bits."""
    out = ""
    for j in range(N - 1, -1, -1):
        if (i >> j) & 1 == 1:
            out += "1"
        else:
            out += "0"
    return out


def rotate_right(x, d):
    """Rotate x by d bits to the right."""
    d = d % N
    out = x >> d
    for i in range(d):
        bit = (x & 2**i) >> i
        out |= bit << (N + i - d)
    return out


def rotate_left(x, d):
    """Rotate x by d bits to the left."""
    d = d % N
    out = x << d
    out = out & (2**N - 1)
    for i in range(d):
        bit = (x & 2 ** (N - 1 - d + 1 + i)) >> (N - 1 - d + 1 + i)
        out |= bit << i
    return out


def bit_component(x, i):
    """Return i-th bit of x."""
    return (x & 2**i) >> i


def gc(i):
    """Return the Gray code index of i."""
    return i ^ (i >> 1)


def e(i):
    """Return the entry point of hypercube i."""
    if i == 0:
        return 0
    else:
        return gc(2 * int(math.floor((i - 1) // 2)))


def f(i):
    """Return the exit point of hypercube i."""
    return e(2**N - 1 - i) ^ 2 ** (N - 1)


def i_to_p(i):
    """Extract the 3d position from a 3-bit integer."""
    return [bit_component(i, j) for j in (0, 1, 2)]


def inverse_gc(g):
    """Invert the gray code."""
    i = g
    j = 1
    while j < N:
        i = i ^ (g >> j)
        j = j + 1
    return i


def g(i):
    """Return the direction between subcube i and the next one."""
    return int(np.log2(gc(i) ^ gc(i + 1)))


def d(i):
    """Return the direction of the arrow within a subcube."""
    if i == 0:
        return 0
    elif (i % 2) == 0:
        return g(i - 1) % N
    else:
        return g(i) % N


def T(e, d, b):
    """Transform b."""
    out = b ^ e
    return rotate_right(out, d + 1)


def T_inv(e, d, b):
    """Inverse transform b."""
    return T(rotate_right(e, d + 1), N - d - 2, b)


def TR_algo2(p, M):
    """Return the Hilbert index of point p."""
    # h will contain the Hilbert index
    h = 0
    # ve and vd contain the entry point and dimension of the current subcube
    ve = 0
    vd = 0
    for i in range(M - 1, -1, -1):
        # the cell label is constructed in two steps
        # 1. extract the relevant bits from p
        ll = [bit_component(px, i) for px in p]
        # 2. construct a integer whose bits are given by l
        ll = sum([lx * 2**j for j, lx in enumerate(ll)])
        # transform l into the current subcube
        ll = T(ve, vd, ll)
        # obtain the gray code ordering from the label l
        w = inverse_gc(ll)
        # compose (see [TR] lemma 2.13) the transform of ve and vd
        # with the data of the subcube
        ve = ve ^ (rotate_left(e(w), vd + 1))
        vd = (vd + d(w) + 1) % N
        # move the index to more significant bits and add current value
        h = (h << N) | w
    return h


def TR_algo3(h, M):
    """Return the coordinates for the Hilbert index h."""
    ve = 0
    vd = 0
    p = [0] * N
    for i in range(M - 1, -1, -1):
        w = [bit_component(h, i * N + ii) for ii in range(N)]
        w = sum([wx * 2**j for j, wx in enumerate(w)])
        ll = gc(w)
        ll = T_inv(ve, vd, ll)
        for j in range(N):
            p[j] += bit_component(ll, j) << i
        ve = ve ^ rotate_left(e(w), vd + 1)
        vd = (vd + d(w) + 1) % N
    return p


def deinterleave(z, M):
    """De-interleave."""
    x = 0
    y = 0
    for i in range(M):
        x = x | (2**i * bit_component(z, 2 * i))
        y = y | (2**i * bit_component(z, 2 * i + 1))
    return x, y


def make_TR_algo2_lut(M):
    """Return a LUT for the Hilbert index of point p."""
    h = 0
    lut = []
    for ie in (0, 3):
        for id in (0, 1):
            for z in range(2 ** (2 * M)):
                p = deinterleave(z, M)
                h = 0
                ve = ie
                vd = id
                for i in range(M - 1, -1, -1):
                    # the cell label is constructed in two steps
                    # 1. extract the relevant bits from p
                    ll = [bit_component(px, i) for px in p]
                    # 2. construct a integer whose bits are given by l
                    ll = sum([lx * 2**j for j, lx in enumerate(ll)])
                    # transform l into the current subcube
                    ll = T(ve, vd, ll)
                    # obtain the gray code ordering from the label l
                    w = inverse_gc(ll)
                    # compose (see [TR] lemma 2.13) the transform of ve and vd
                    # with the data of the subcube
                    ve = ve ^ (rotate_left(e(w), vd + 1))
                    vd = (vd + d(w) + 1) % N
                    # move the index to more significant bits and add
                    # current value
                    h = (h << N) | w
                lut.append(h | (vd << 2 * M) | ((ve % 2) << (2 * M + 1)))
    return lut


def make_TR_algo3_lut(M):
    """Return a LUT for the point corresponding to the Hilbert index h."""
    h = 0
    lut = []
    for ie in (0, 3):
        for id in (0, 1):
            for h in range(2 ** (2 * M)):
                z = 0
                ve = ie
                vd = id
                for i in range(M - 1, -1, -1):
                    w = [bit_component(h, i * N + ii) for ii in range(N)]
                    w = sum([wx * 2**j for j, wx in enumerate(w)])
                    ll = gc(w)
                    ll = T_inv(ve, vd, ll)
                    ve = ve ^ rotate_left(e(w), vd + 1)
                    vd = (vd + d(w) + 1) % N
                    z = (z << N) | ll
                lut.append(z | (vd << 2 * M) | ((ve % 2) << (2 * M + 1)))
    return lut


# Print out the lookup tables for the C++ 2-D hilbert curve implementation.
algo2_lut = make_TR_algo2_lut(3)
algo3_lut = make_TR_algo3_lut(3)
print("alignas(64) static uint8_t const HILBERT_LUT_3[256] = {", end="")
for i, x in enumerate(algo2_lut):
    if i % 8 == 0:
        print("\n   ", end="")
    print(" 0x%02x" % x, end="," if i != len(algo2_lut) - 1 else "")
print("\n}\n")
print("alignas(64) static uint8_t const HILBERT_INVERSE_LUT_3[256] = {", end="")
for i, x in enumerate(algo3_lut):
    if i % 8 == 0:
        print("\n   ", end="")
    print(" 0x%02x" % x, end="," if i != len(algo3_lut) - 1 else "")
print("\n}\n")
