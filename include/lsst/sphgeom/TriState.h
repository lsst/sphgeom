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

#ifndef LSST_SPHGEOM_TRISTATE_H_
#define LSST_SPHGEOM_TRISTATE_H_

/// \file
/// \brief This file declares a class for representing tri-state values.

#include <iostream>
#include <stdexcept>


namespace lsst {
namespace sphgeom {

/// `TriState` represents a boolean value with additional `unknown` state.
/// Instances of this class can be compared to booleans `true` and `false`,
/// when the state is unknown, comparisons will return `false`.
class TriState {
public:
    /// Construct value in unknown state.
    TriState() {}

    /// Construct value in a known state.
    explicit TriState(bool value) : _known(true), _value(value) {}

    TriState& operator=(TriState const & other) = default;

    /// @brief Compare this tri-state value with other tri-state value.
    /// @param other Tri-state value to compare to.
    /// @return True is returned when both states are unknown or when both
    /// states are known and values are equal, false returned otherwise.
    bool operator==(TriState const & other) const {
        if (not _known) {
            return not other._known;
        } else {
            return other._known && _value == other._value;
        }
    }

    bool operator!=(TriState const & other) const {
        return not this->operator==(other);
    }

    /// @brief Compare this tri-state value with a boolean.
    /// @param value Boolean value to compare to.
    /// @return True if the state is known to be equal to the given boolean.
    bool operator==(bool value) const {
        return _known && _value == value;
    }

    bool operator!=(bool value) const {
        return not this->operator==(value);
    }

    /// @brief Compute the logical OR of two TriState values.
    /// @param other Other TriState value.
    /// @return True if either operand is known to be true, false if both
    ///         operands are known to be false, and unknown otherwise.
    TriState operator|(TriState const & other) const {
        if (*this == true || other == true) {
            return TriState(true);
        } else if (*this == false && other == false) {
            return TriState(false);
        } else {
            return TriState();
        }
    }

    /// @brief Compute the logical AND of two TriState values.
    /// @param other Other TriState value.
    /// @return False if either operand is known to be false, true if both
    ///         operands are known to be true, and unknown otherwise.
    TriState operator&(TriState const & other) const {
        if (*this == false || other == false) {
            return TriState(false);
        } else if (*this == true && other == true) {
            return TriState(true);
        } else {
            return TriState();
        }
    }

    /// @brief Compute the logical NOT of a TriState value.
    /// @return True if the current state is known to be false, false if the
    ///         current state is known to be true, and unknown otherwise.
    TriState operator~() const {
        if (known()) {
            return TriState(!_value);
        }
        return TriState();
    }

    /// @brief Check whether the state is known.
    /// @return True is returned when state is known.
    bool known() const { return _known; }

private:
    bool _known = false;
    bool _value = false;
};

inline
std::ostream & operator<<(std::ostream & stream, TriState const & value) {
    const char* str = "unknown";
    if (value == true) {
        str = "true";
    } else if (value == false) {
        str = "false";
    }
    return stream << str;
}

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_TRISTATE_H_
