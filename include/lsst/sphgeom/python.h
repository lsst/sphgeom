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

#ifndef PYTHON_LSST_SPHGEOM_SPHGEOM_H
#define PYTHON_LSST_SPHGEOM_SPHGEOM_H

#include <functional>
#include <vector>
#include <array>
#include <nanobind/ndarray.h>
#include <iostream>

namespace nanobind {
template<typename Class, typename ReturnType, typename... Args>
auto vectorize(ReturnType(Class::*func)(Args... args) const) {
    // Return a lambda capturing the ember function pointer
    std::tuple<Args...> tuple;
    constexpr auto N = std::tuple_size_v<decltype(tuple)>;
    static_assert(std::conjunction_v<std::is_scalar<Args>...>, "All arguments must be scalar types");

    return [func, tuple](Class &obj, const nanobind::ndarray<>  &a) -> ReturnType {
        std::cout << "Arguments: ";
        //for (int i = 0; i < N; i++) std::cout << a[i] << " ";
        std::cout << std::endl;
        std::array<double, N> ab;
        //std::copy_n(a.begin(), N, ab.begin());
        auto abc = reinterpret_cast<std::array<double, N> const *>(a.data());
        return std::apply(
                [func, &obj](Args... args) { return (obj.*func)(args...); }, *abc);
    };
};
}

namespace lsst {
namespace sphgeom {

template <typename NanoBindClass>
void defineClass(NanoBindClass &cls);

}  // sphgeom
}  // lsst

#endif  // PYTHON_LSST_SPHGEOM_SPHGEOM_H
