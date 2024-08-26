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
/// \brief This file contains the RangeSet implementation.

#include "lsst/sphgeom/RangeSet.h"

#include <algorithm>
#include <ostream>


namespace lsst {
namespace sphgeom {

namespace {

// `roundUpToEven` returns the smallest multiple of 2
// greater than or equal to i.
inline ptrdiff_t roundUpToEven(ptrdiff_t i) { return i + (i & 1); }


// `RangeIter` is a stride-2 iterator over std::uint64_t values
// in an underlying array.
struct RangeIter {

    // For std::iterator_traits
    using difference_type = ptrdiff_t;
    using value_type = std::uint64_t;
    using pointer = std::uint64_t *;
    using reference = std::uint64_t &;
    using iterator_category = std::random_access_iterator_tag;

    RangeIter() = default;
    explicit RangeIter(std::uint64_t * p) : ptr{p} {}

    friend void swap(RangeIter & a, RangeIter & b) { std::swap(a.ptr, b.ptr); }

    // Arithmetic
    RangeIter & operator++() { ptr += 2; return *this; }
    RangeIter & operator--() { ptr -= 2; return *this; }

    RangeIter operator++(int) { RangeIter i(*this); ptr += 2; return i; }
    RangeIter operator--(int) { RangeIter i(*this); ptr -= 2; return i; }

    RangeIter operator+(ptrdiff_t n) const { return RangeIter(ptr + 2 * n); }
    RangeIter operator-(ptrdiff_t n) const { return RangeIter(ptr + 2 * n); }

    RangeIter & operator+=(ptrdiff_t n) { ptr += 2 * n; return *this; }
    RangeIter & operator-=(ptrdiff_t n) { ptr -= 2 * n; return *this; }

    friend RangeIter operator+(ptrdiff_t n, RangeIter const & i) { return i + n; }

    ptrdiff_t operator-(RangeIter const & i) const { return (ptr - i.ptr) / 2; }

    // Comparison
    bool operator==(RangeIter const & i) const { return ptr == i.ptr; }
    bool operator!=(RangeIter const & i) const { return ptr != i.ptr; }
    bool operator<(RangeIter const & i) const { return ptr < i.ptr; }
    bool operator>(RangeIter const & i) const { return ptr > i.ptr; }
    bool operator<=(RangeIter const & i) const { return ptr <= i.ptr; }
    bool operator>=(RangeIter const & i) const { return ptr >= i.ptr; }

    // Dereferencing
    std::uint64_t & operator*() const { return *ptr; }
    std::uint64_t * operator->() const { return ptr; }
    std::uint64_t & operator[](ptrdiff_t n) const { return ptr[2 * n]; }

    std::uint64_t * ptr = nullptr;
};

} // unnamed namespace


RangeSet::RangeSet(std::initializer_list<std::uint64_t> list) :
    RangeSet(list.begin(), list.end())
{}

RangeSet::RangeSet(std::initializer_list<std::pair<std::uint64_t, std::uint64_t>> list) {
    for (auto t: list) {
        insert(std::get<0>(t), std::get<1>(t));
    }
}

void RangeSet::insert(std::uint64_t first, std::uint64_t last) {
    if (first == last) {
        fill();
    } else {
        // Ensure that there is enough space for 2 new values in _ranges.
        // Afterwards, none of the possible modifications of _ranges will throw,
        // so the strong exception safety guarantee is provided.
        _ranges.reserve(_ranges.size() + 2);
        if (first <= last - 1) {
            _insert(first, last);
        } else {
            _insert(0, last);
            _insert(first, 0);
        }
    }
}

void RangeSet::erase(std::uint64_t first, std::uint64_t last) {
    // To erase [first, last), insert it into the complement of this set,
    // then complement the result. The complements are performed in the
    // constructor and destructor of a local object, so that the second
    // complement is executed even if insert throws.
    struct Complementor {
        RangeSet & set;
        Complementor(RangeSet & s) : set(s) { set.complement(); }
        ~Complementor() { set.complement(); }
    };
    Complementor c(*this);
    insert(first, last);
}

RangeSet RangeSet::intersection(RangeSet const & s) const {
    RangeSet result;
    if (this == &s) {
        result = s;
    } else {
        result._intersect(_begin(), _end(), s._begin(), s._end());
    }
    return result;
}

RangeSet RangeSet::join(RangeSet const & s) const {
    RangeSet result;
    if (this == &s) {
        result = s;
    } else {
        // A ∪ B = ¬(¬A ∩ ¬B)
        result._intersect(_beginc(), _endc(), s._beginc(), s._endc());
        result.complement();
    }
    return result;
}

RangeSet RangeSet::difference(RangeSet const & s) const {
    RangeSet result;
    if (this != &s) {
        // A ∖ B = A ∩ ¬B
        result._intersect(_begin(), _end(), s._beginc(), s._endc());
    }
    return result;
}

RangeSet RangeSet::symmetricDifference(RangeSet const & s) const {
    RangeSet result;
    if (this != &s) {
        if (empty()) {
            result = s;
        } else if (s.empty()) {
            result = *this;
        } else {
            // Sweep through the beginning and end points of the ranges from
            // sets A and B in ascending order.
            //
            // Passing through the beginning of a range toggles the
            // corresponding state (astate or bstate) from 0 to 1, and passing
            // through the end toggles it from 1 to 0. The XOR of astate and
            // bstate determines whether or not the current position of the
            // sweep is inside the symmetric set difference of A and B.
            //
            // Merging the sorted lists of ranges from each set is complicated
            // by trailing zero bookends. These compare less than or equal to
            // all other values, even though logically they are greater than
            // them all.
            //
            // To handle this, leading zero bookends are dealt with outside of
            // the main loop. Then, 1 is subtracted from all other values prior
            // to comparison, yielding the proper ordering (since subtracting
            // 1 from a trailing zero bookend results in 2^64 - 1).
            std::uint64_t const * a = _begin();
            std::uint64_t const * aend = _end();
            std::uint64_t const * b = s._begin();
            std::uint64_t const * bend = s._end();
            int astate = (*a == 0);
            int bstate = (*b == 0);
            a += astate;
            b += bstate;
            // state is 0 if the next value to output is the beginning
            // of a range, and 1 otherwise.
            int state = astate ^ bstate;
            // Start with a vector containing just the leading bookend.
            result._ranges = {0};
            // If 0 is contained in exactly one of the two sets, it is
            // in their symmetric difference.
            result._offset = (state == 0);
            // Merge lists until one or both are exhausted.
            while (a != aend && b != bend) {
                std::uint64_t av = *a - 1;
                std::uint64_t bv = *b - 1;
                // The pointer(s) yielding the minimal value will be
                // incremented.
                bool ainc = (av <= bv);
                bool binc = (bv <= av);
                std::uint64_t minval = ainc ? av : bv;
                astate ^= ainc;
                bstate ^= binc;
                // Output the minimum value if the output state changes.
                if (state != (astate ^ bstate)) {
                    result._ranges.push_back(minval + 1);
                    state ^= 1;
                }
                a += ainc;
                b += binc;
            }
            // The sweep has exhausted at least one list. Each remaining
            // beginning and end point in the other list will toggle the
            // output state, so they can simply be appended to the result.
            result._ranges.insert(result._ranges.end(), a, aend);
            result._ranges.insert(result._ranges.end(), b, bend);
            // Append a trailing bookend if necessary.
            if ((aend[-1] == 0) == (bend[-1] == 0)) {
                result._ranges.push_back(0);
            }
        }
    }
    return result;
}

bool RangeSet::intersects(std::uint64_t first, std::uint64_t last) const {
    if (empty()) {
        return false;
    }
    if (first == last) {
        return true;
    }
    if (first <= last - 1) {
        std::uint64_t r[2] = {first, last};
        return _intersectsOne(r, _begin(), _end());
    }
    std::uint64_t r[4] = {0, last, first, 0};
    return _intersectsOne(r, _begin(), _end()) ||
           _intersectsOne(r + 2, _begin(), _end());
}

bool RangeSet::intersects(RangeSet const & s) const {
    if (empty() || s.empty()) {
        return false;
    }
    return _intersects(_begin(), _end(), s._begin(), s._end());
}

bool RangeSet::contains(std::uint64_t first, std::uint64_t last) const {
    if (full()) {
        return true;
    }
    if (first == last) {
        return false;
    }
    if (first <= last - 1) {
        std::uint64_t r[2] = {first, last};
        return !_intersectsOne(r, _beginc(), _endc());
    }
    std::uint64_t r[4] = {0, last, first, 0};
    return !_intersectsOne(r, _beginc(), _endc()) &&
           !_intersectsOne(r + 2, _beginc(), _endc());
}

bool RangeSet::contains(RangeSet const & s) const {
    if (s.empty() || full()) {
        return true;
    }
    return !_intersects(_beginc(), _endc(), s._begin(), s._end());
}

bool RangeSet::isWithin(std::uint64_t first, std::uint64_t last) const {
    if (empty() || first == last) {
        return true;
    }
    if (last <= first - 1) {
        std::uint64_t r[2] = {last, first};
        return !_intersectsOne(r, _begin(), _end());
    }
    std::uint64_t r[4] = {0, first, last, 0};
    return !_intersectsOne(r, _begin(), _end()) &&
           !_intersectsOne(r + 2, _begin(), _end());
}

std::uint64_t RangeSet::cardinality() const {
    std::uint64_t sz = 0;
    for (auto r = _begin(), e = _end(); r != e; r += 2) {
        sz += r[1] - r[0];
    }
    return sz;
}

RangeSet & RangeSet::simplify(std::uint32_t n) {
    if (empty() || n == 0) {
        return *this;
    } else if (n >= 64) {
        fill();
        return *this;
    }
    // Compute m, the integer with n LSBs set to 1. Then, (x & ~m) is x
    // rounded down to the nearest multiple of 2^n, and (x + m) & ~m is
    // x rounded up to the nearest multiple of 2^n.
    std::uint64_t const m = (static_cast<std::uint64_t>(1) << n) - 1;
    std::uint64_t * r = const_cast<std::uint64_t *>(_begin());
    std::uint64_t * rend = const_cast<std::uint64_t *>(_end());
    std::uint64_t * out = r;
    // Expand the first range.
    std::uint64_t first = r[0] & ~m;
    std::uint64_t last = (r[1] + m) & ~m;
    if (r[0] != 0 && first == 0) {
        // The expanded first range now contains the leading bookend.
        _offset = false;
        --out;
    }
    out[0] = first;
    out[1] = last;
    // Expand the remaining ranges.
    for (r += 2; last != 0 && r != rend; r += 2) {
        std::uint64_t u = r[0] & ~m;
        std::uint64_t v = (r[1] + m) & ~m;
        if (u > last) {
            out += 2;
            out[0] = u;
        }
        out[1] = v;
        last = v;
    }
    out += 2;
    if (last != 0) {
        // Append a trailing bookend if necessary.
        *out = 0;
        ++out;
    }
    // Erase everything after the location of the new trailing bookend.
    _ranges.erase(_ranges.begin() + (out  - _ranges.data()), _ranges.end());
    return *this;
}

RangeSet & RangeSet::scale(std::uint64_t i) {
    if (empty() || i == 1) {
        return *this;
    } else if (i == 0) {
        clear();
        return *this;
    }
    std::uint64_t overflowThreshold = static_cast<std::uint64_t>(-1) / i;
    auto r = _ranges.begin();
    auto rend = _ranges.end();
    for (; r < rend; ++r) {
        std::uint64_t value = *r;
        if (value > overflowThreshold) {
            *r = 0;
            ++r;
            break;
        }
        *r = value * i;
    }
    _ranges.erase(r, rend);
    return *this;
}

bool RangeSet::isValid() const {
    // Bookends are mandatory.
    if (_ranges.size() < 2) {
        return false;
    }
    if (_ranges.front() != 0 || _ranges.back() != 0) {
        return false;
    }
    // Values except the last one must be monotonically increasing.
    for (auto i = _ranges.begin() + 1, e = _ranges.end() - 1; i != e; ++i) {
        if (i[0] <= i[-1]) {
            return false;
        }
    }
    return true;
}

void RangeSet::_insert(std::uint64_t first, std::uint64_t last) {
    // First, check if this set is empty, or if [first, last) extends
    // or comes after the last range in this set.
    //
    // It is assumed that first <= last - 1; that is, first and last
    // do not correspond to a full range, or one that wraps.
    std::uint64_t * r = const_cast<std::uint64_t *>(_begin());
    std::uint64_t * rend = const_cast<std::uint64_t *>(_end());
    if (r == rend) {
        // This set is empty.
        std::uint64_t array[4] = {0, first, last, 0};
        _ranges.assign(array + (first == 0), array + (4 - (last == 0)));
        _offset = (first != 0);
    } else if (first >= rend[-2]) {
        if (rend[-1] != 0) {
            if (first <= rend[-1]) {
                // [first, last) extends the last range in this set.
                rend[-1] = std::max(last - 1, rend[-1] - 1) + 1;
                if (last == 0) {
                    _ranges.pop_back();
                }
            } else {
                // [first, last) follows the last range in this set.
                rend[0] = first;
                _ranges.push_back(last);
                if (last != 0) {
                    _ranges.push_back(0);
                }
            }
        }
    } else {
        // Find a, the first range with end point a[1] >= first, and b,
        // the first range with beginning point b[0] > last.
        std::uint64_t * a;
        std::uint64_t * b;
        if (first == 0) {
            a = r;
        } else {
            // Subtract one from values before comparisons so that the
            // trailing zero bookend is ordered properly.
            a = std::lower_bound(RangeIter(r + 1), RangeIter(rend + 1), first,
                                 [](std::uint64_t u, std::uint64_t v) {
                                     return u - 1 < v - 1;
                                 }).ptr - 1;
        }
        if (last == 0) {
            b = rend;
        } else {
            b = std::upper_bound(RangeIter(a), RangeIter(rend), last).ptr;
        }
        // Perform the insert. Inserts involving a leading or trailing bookend
        // are special cased.
        if (first == 0) {
            _offset = false;
            if (b == r) {
                _ranges.insert(_ranges.begin() + (r - _ranges.data()), last);
            } else {
                --b;
                *b = std::max(last - 1, *b - 1) + 1;
                _ranges.erase(_ranges.begin() + 1,
                              _ranges.begin() + (b - _ranges.data()));
            }
        } else if (last == 0) {
            *a = std::min(first, *a);
            ++a;
            _ranges.erase(_ranges.begin() + (a - _ranges.data()),
                          _ranges.end() - 1);
        } else {
            if (a == b) {
                _ranges.insert(_ranges.begin() + (a - _ranges.data()),
                               {first, last});
            } else {
                --b;
                *b = std::max(last - 1, *b - 1) + 1;
                *a = std::min(first, *a);
                ++a;
                _ranges.erase(_ranges.begin() + (a - _ranges.data()),
                              _ranges.begin() + (b - _ranges.data()));
            }
        }
    }
}

/// `_intersectOne` stores the intersection of the single range pointed
/// to by `a` and the ranges pointed to by `b` in `v`.
void RangeSet::_intersectOne(std::vector<std::uint64_t> & v,
                             std::uint64_t const * a,
                             std::uint64_t const * b,
                             std::uint64_t const * bend)
{
    // The range B = [b[0], bend[-1]) contains all the ranges
    // [b[0], b[1]), ... , [bend[-2], bend[-1]). If [a[0], a[1])
    // does not intersect B, it does not intersect any of them.
    //
    // Note that one is subtracted from range end-points prior to
    // comparison - otherwise, trailing zero bookends would not be
    // ordered properly with respect to other values.
    if (a[0] > bend[-1] - 1 || a[1] - 1 < b[0]) {
        return;
    }
    if (b + 2 == bend) {
        // Output the intersection of the ranges pointed to by a and b.
        std::uint64_t u = std::max(a[0], b[0]);
        if (u != 0) {
            v.push_back(u);
        }
        v.push_back(std::min(a[1] - 1, b[1] - 1) + 1);
    } else if (a[0] <= b[0] && a[1] - 1 >= bend[-1] - 1) {
        // [a[0], a[1]) contains [b[0], bend[-1]), so it contains
        // [b[0], b[1]), [b[2], b[3]), ... , [bend[-2], bend[-1]).
        v.insert(v.end(), b + (b[0] == 0), bend);
    } else {
        // Divide and conquer - split the list of ranges pointed
        // to by b in half and recurse.
        std::uint64_t const * bmid = b + roundUpToEven((bend - b) >> 1);
        _intersectOne(v, a, b, bmid);
        _intersectOne(v, a, bmid, bend);
    }
}

/// `_intersect` stores the intersection of the ranges pointed to by `a`
/// and the ranges pointed to by `b` in `v`.
void RangeSet::_intersect(std::vector<std::uint64_t> & v,
                          std::uint64_t const * a,
                          std::uint64_t const * aend,
                          std::uint64_t const * b,
                          std::uint64_t const * bend)
{
    // The range A = [a[0], aend[-1]) contains all the ranges
    // [a[0], a[1]), ... , [aend[-2], aend[-1]), and similarly,
    // B = [b[0], bend[-1]) contains all the ranges pointed to by b.
    // A and B must intersect if any of the ranges pointed to by
    // a and b do, so if A and B are disjoint there is nothing to do.
    //
    // Note that one is subtracted from range end-points prior to
    // comparison - otherwise, trailing zero bookends would not be
    // ordered properly with respect to other values.
    if (a + 2 == aend) {
        _intersectOne(v, a, b, bend);
    } else if (b + 2 == bend) {
        _intersectOne(v, b, a, aend);
    } else if (a[0] <= bend[-1] - 1 && aend[-1] - 1 >= b[0]) {
        // Divide and conquer - split the lists of ranges pointed to by a and b
        // in half and recurse. This is a depth first dual-tree traversal of
        // implicit balanced binary trees, where a sorted list of ranges forms
        // the bottom level of a tree, and level N - 1 is obtained from level N
        // by joining adjacent ranges.
        //
        // Note that the order of the recursive calls matters -
        // output must be generated in sorted order.
        std::uint64_t const * amid = a + roundUpToEven((aend - a) >> 1);
        std::uint64_t const * bmid = b + roundUpToEven((bend - b) >> 1);
        _intersect(v, a, amid, b, bmid);
        _intersect(v, a, amid, bmid, bend);
        _intersect(v, amid, aend, b, bmid);
        _intersect(v, amid, aend, bmid, bend);
    }
}

void RangeSet::_intersect(std::uint64_t const * a,
                          std::uint64_t const * aend,
                          std::uint64_t const * b,
                          std::uint64_t const * bend)
{
    if (a == aend || b == bend) {
        clear();
    } else {
        // Start with a vector containing just the leading bookend.
        _ranges = {0};
        // If both a and b contain 0, their intersection contains 0,
        // and _offset must be 0 (false).
        _offset = ((*a != 0) || (*b != 0));
        // Compute the intersection and append a trailing bookend if necessary.
        _intersect(_ranges, a, aend, b, bend);
        if ((aend[-1] != 0) || (bend[-1] != 0)) {
            _ranges.push_back(0);
        }
    }
}

/// `_intersectsOne` checks if the single range pointed to by `a` intersects
/// any of the ranges pointed to by `b`.
bool RangeSet::_intersectsOne(std::uint64_t const * a,
                              std::uint64_t const * b,
                              std::uint64_t const * bend)
{
    // See the comments in _intersectOne for an explanation.
    if (a[0] > bend[-1] - 1 || a[1] - 1 < b[0]) {
        return false;
    }
    if (b + 2 == bend || a[0] <= b[0] || a[1] - 1 >= bend[-1] - 1) {
        return true;
    }
    std::uint64_t const * bmid = b + roundUpToEven((bend - b) >> 1);
    return _intersectsOne(a, b, bmid) || _intersectsOne(a, bmid, bend);
}

/// `_intersects` checks if any of the ranges pointed to by `a` intersect
/// any of the ranges pointed to by `b`.
bool RangeSet::_intersects(std::uint64_t const * a,
                           std::uint64_t const * aend,
                           std::uint64_t const * b,
                           std::uint64_t const * bend)
{
    // See the comments in _intersect for an explanation.
    if (a + 2 == aend) {
        return _intersectsOne(a, b, bend);
    }
    if (b + 2 == bend) {
        return _intersectsOne(b, a, aend);
    }
    if (a[0] > bend[-1] - 1 || aend[-1] - 1 < b[0]) {
        return false;
    }
    std::uint64_t const * amid = a + roundUpToEven((aend - a) >> 1);
    std::uint64_t const * bmid = b + roundUpToEven((bend - b) >> 1);
    return _intersects(a, amid, b, bmid) ||
           _intersects(a, amid, bmid, bend) ||
           _intersects(amid, aend, b, bmid) ||
           _intersects(amid, aend, bmid, bend);
}

std::ostream & operator<<(std::ostream & os, RangeSet const & s) {
    os << "{\"RangeSet\": [";
    bool first = true;
    for (auto const & t: s) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << '[' << std::get<0>(t) << ", " << std::get<1>(t) << ']';
    }
    os << "]}";
    return os;
}

}} // namespace lsst::sphgeom
