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

#ifndef LSST_SPHGEOM_INTERVAL1D_H_
#define LSST_SPHGEOM_INTERVAL1D_H_

/// \file
/// \brief This file defines a class for representing intervals of ℝ.

#include <iosfwd>
#include <limits>

#include "Interval.h"


namespace lsst {
namespace sphgeom {

/// `Interval1d` represents closed intervals of ℝ. It can represent
/// both empty and full intervals, as well as single points.
class Interval1d : public Interval<Interval1d, double> {
    typedef Interval<Interval1d, double> Base;
public:

    static Interval1d empty() {
        return Interval1d();
    }

    static Interval1d full() {
        return Interval1d(-std::numeric_limits<double>::infinity(),
                          std::numeric_limits<double>::infinity());
    }

    // Delegation to base class constructors
    Interval1d() : Base() {}

    explicit Interval1d(double x) : Base(x) {}

    Interval1d(double x, double y) : Base(x, y) {}

    Interval1d(Base const & base) : Base(base) {}

    /// `isFull` returns true if this interval = ℝ.
    bool isFull() const {
        return getA() == -std::numeric_limits<double>::infinity() &&
               getB() == std::numeric_limits<double>::infinity();
    }
};

std::ostream & operator<<(std::ostream &, Interval1d const &);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_INTERVAL1D_H_
