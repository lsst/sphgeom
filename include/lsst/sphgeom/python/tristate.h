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

#ifndef LSST_SPHGEOM_PYTHON_TRISTATE_H_
#define LSST_SPHGEOM_PYTHON_TRISTATE_H_

#include "pybind11/pybind11.h"

#include "../TriState.h"

namespace pybind11 {
namespace detail {

/// This struct is a partial specialization of type_caster for
/// for lsst::sphgeom::TriState.
///
/// It maps between TriState and Python bool or None, avoiding the need to
/// wrap the former. This header should be included by all wrappers for
/// functions that consume or return TriState instances.
template <>
struct type_caster<lsst::sphgeom::TriState> {
public:
    // Declare a local variable `value` of type lsst::sphgeom::TriState,
    // and describe the TriState type as an "bool | None" in pybind11-generated
    // docstrings.
    PYBIND11_TYPE_CASTER(lsst::sphgeom::TriState, _("bool | None"));

    // Convert a Python object to an lsst::sphgeom::TriState.
    bool load(handle src, bool) {
        if (src.is_none()) {
            value = lsst::sphgeom::TriState();
        } else {
            value = lsst::sphgeom::TriState(src.cast<bool>());
        }
        return true;
    }

    // Convert an lsst::sphgeom::TriState to a Python integer.
    static handle cast(lsst::sphgeom::TriState src, return_value_policy,
                       handle) {

        if (src == true) {
            Py_RETURN_TRUE;
        } else if (src == false) {
            Py_RETURN_FALSE;
        }
        Py_RETURN_NONE;
    }
};

}  // detail
}  // pybind11

#endif  // LSST_SPHGEOM_PYTHON_TRISTATE_H_
