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

#ifndef LSST_SPHGEOM_ANGLEINTERVAL_H_
#define LSST_SPHGEOM_ANGLEINTERVAL_H_

/// \file
/// \brief This file defines a class for representing angle intervals.

#include <iosfwd>
#include <limits>

#include "Angle.h"
#include "Interval.h"


namespace lsst {
namespace sphgeom {

/// `AngleInterval` represents closed intervals of arbitrary angles.
class AngleInterval : public Interval<AngleInterval, Angle> {
    typedef Interval<AngleInterval, Angle> Base;
public:
    // Factory functions
    static AngleInterval fromDegrees(double x, double y) {
        return AngleInterval(Angle::fromDegrees(x),
                             Angle::fromDegrees(y));
    }

    static AngleInterval fromRadians(double x, double y) {
        return AngleInterval(Angle::fromRadians(x),
                             Angle::fromRadians(y));
    }

    static AngleInterval empty() {
        return AngleInterval();
    }

    static AngleInterval full() {
        return AngleInterval(Angle(-std::numeric_limits<double>::infinity()),
                             Angle(std::numeric_limits<double>::infinity()));
    }

    // Delegation to base class constructors
    AngleInterval() : Base() {}

    explicit AngleInterval(Angle x) : Base(x) {}

    AngleInterval(Angle x, Angle y) : Base(x, y) {}

    AngleInterval(Base const & base) : Base(base) {}
};

std::ostream & operator<<(std::ostream &, AngleInterval const &);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_ANGLEINTERVAL_H_
