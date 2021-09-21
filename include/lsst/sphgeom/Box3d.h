/*
 * LSST Data Management System
 * See COPYRIGHT file at the top of the source tree.
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

#ifndef LSST_SPHGEOM_BOX3D_H_
#define LSST_SPHGEOM_BOX3D_H_

/// \file
/// \brief This file declares a class for representing
///        axis-aligned bounding boxes in ℝ³.

#include <iosfwd>

#include "Interval1d.h"
#include "Relationship.h"
#include "Vector3d.h"


namespace lsst {
namespace sphgeom {

/// `Box3d` represents a box in ℝ³. It is the Cartesian product of three
/// intervals of ℝ.
class Box3d {
public:
    // Factory functions
    static Box3d empty() {
        return Box3d();
    }

    static Box3d full() {
        return Box3d(Interval1d::full(),
                     Interval1d::full(),
                     Interval1d::full());
    }

    /// `aroundUnitSphere` returns a minimal Box3d containing the unit sphere.
    static Box3d aroundUnitSphere() {
        return Box3d(Interval1d(-1.0, 1.0),
                     Interval1d(-1.0, 1.0),
                     Interval1d(-1.0, 1.0));
    }

    /// This constructor creates an empty 3D box.
    Box3d() {}

    /// This constructor creates a box containing a single point.
    explicit Box3d(Vector3d const & v)
    {
        _intervals[0] = Interval1d(v.x());
        _intervals[1] = Interval1d(v.y());
        _intervals[2] = Interval1d(v.z());
        _enforceInvariants();
    }

    /// This constructor creates a box spanning the intervals
    /// [v1.x(), v2.x()], [v1.y(), v2.y()], and [v1.z(), v2.z()].
    Box3d(Vector3d const & v1, Vector3d const & v2)
    {
        _intervals[0] = Interval1d(v1.x(), v2.x());
        _intervals[1] = Interval1d(v1.y(), v2.y());
        _intervals[2] = Interval1d(v1.z(), v2.z());
        _enforceInvariants();
    }

    /// This constructor creates a box with center v, half-width w,
    /// half-height h, and half-depth d.
    Box3d(Vector3d const & v, double w, double h, double d)
    {
        _intervals[0] = Interval1d(v.x()).dilatedBy(w);
        _intervals[1] = Interval1d(v.y()).dilatedBy(h);
        _intervals[2] = Interval1d(v.z()).dilatedBy(d);
        _enforceInvariants();
    }

    /// This constructor creates a box spanning the given
    /// x, y, and z intervals.
    Box3d(Interval1d const & x,
          Interval1d const & y,
          Interval1d const & z)
    {
        _intervals[0] = Interval1d(x);
        _intervals[1] = Interval1d(y);
        _intervals[2] = Interval1d(z);
        _enforceInvariants();
    }

    /// Two 3D boxes are equal if they contain the same points.
    bool operator==(Box3d const & b) const {
        return _intervals[0] == b._intervals[0] &&
               _intervals[1] == b._intervals[1] &&
               _intervals[2] == b._intervals[2];
    }

    bool operator!=(Box3d const & b) const { return !(*this == b); }

    /// A box is equal to a point if it contains only that point.
    bool operator==(Vector3d const & v) const { return *this == Box3d(v); }

    bool operator!=(Vector3d const & v) const { return !(*this == v); }

    /// The function call operator returns the `i`-th interval of this box.
    Interval1d operator()(int i) const { return _intervals[i]; }

    Interval1d const & x() const { return _intervals[0]; }
    Interval1d const & y() const { return _intervals[1]; }
    Interval1d const & z() const { return _intervals[2]; }

    /// `isEmpty` returns true if this box does not contain any points.
    bool isEmpty() const {
        return x().isEmpty();
    }

    /// `isFull` returns true if this box contains all points in ℝ³.
    bool isFull() const {
        return x().isFull() && y().isFull() && z().isFull();
    }

    /// `getCenter` returns the center of this box. An arbitrary vector is
    /// returned for boxes that are empty or full.
    Vector3d getCenter() const {
        return Vector3d(x().getCenter(), y().getCenter(), z().getCenter());
    }

    /// `getWidth` returns the width (x-axis extent) of this box. It is
    /// negative or NaN for empty boxes, and infinite for full boxes.
    double getWidth() const { return x().getSize(); }

    /// `getHeight` returns the height (y-axis extent) of this box. It is
    /// negative or NaN for empty boxes, and infinite for full boxes.
    double getHeight() const { return y().getSize(); }

    /// `getDepth` returns the depth (z-axis extent) of this box. It is
    /// negative or NaN for empty boxes, and infinite for full boxes.
    double getDepth() const { return z().getSize(); }

    ///@{
    /// `contains` returns true iff the intersection of this box and b
    /// is equal to b.
    bool contains(Vector3d const & b) const {
        return x().contains(b.x()) &&
               y().contains(b.y()) &&
               z().contains(b.z());
    }

    bool contains(Box3d const & b) const {
        return x().contains(b.x()) &&
               y().contains(b.y()) &&
               z().contains(b.z());
    }

    bool contains(double x_, double y_, double z_) const {
        return x().contains(x_) && y().contains(y_) && z().contains(z_);
    }
    ///@}

    ///@{
    /// `isDisjointFrom` returns true iff the intersection of this box
    /// and b is empty.
    bool isDisjointFrom(Vector3d const & b) const { return !intersects(b); }

    bool isDisjointFrom(Box3d const & b) const { return !intersects(b); }
    ///@}

    ///@{
    /// `intersects` returns true iff the intersection of this box and b
    /// is non-empty.
    bool intersects(Vector3d const & b) const {
        return x().intersects(b.x()) &&
               y().intersects(b.y()) &&
               z().intersects(b.z());
    }

    bool intersects(Box3d const & b) const {
        return x().intersects(b.x()) &&
               y().intersects(b.y()) &&
               z().intersects(b.z());
    }
    ///@}

    ///@{
    /// `isWithin` returns true if the intersection of this box and b
    /// is this box.
    bool isWithin(Vector3d const & b) const {
        return x().isWithin(b.x()) &&
               y().isWithin(b.y()) &&
               z().isWithin(b.z());
    }

    bool isWithin(Box3d const & b) const {
        return x().isWithin(b.x()) &&
               y().isWithin(b.y()) &&
               z().isWithin(b.z());
    }
    ///@}

    ///@{
    /// `clipTo` sets this box to the intersection of this box and b.
    Box3d & clipTo(Vector3d const & b) {
        _intervals[0].clipTo(b.x());
        _intervals[1].clipTo(b.y());
        _intervals[2].clipTo(b.z());
        _enforceInvariants();
        return *this;
    }

    Box3d & clipTo(Box3d const & b) {
        _intervals[0].clipTo(b.x());
        _intervals[1].clipTo(b.y());
        _intervals[2].clipTo(b.z());
        _enforceInvariants();
        return *this;
    }
    ///@}

    ///@{
    /// `clippedTo` returns the intersection of this box and b.
    Box3d clippedTo(Vector3d const & b) const {
        return Box3d(*this).clipTo(b);
    }

    Box3d clippedTo(Box3d const & b) const {
        return Box3d(*this).clipTo(b);
    }
    ///@}

    ///@{
    /// `expandTo` minimally expands this box to contain b.
    Box3d & expandTo(Vector3d const & b) {
        _intervals[0].expandTo(b.x());
        _intervals[1].expandTo(b.y());
        _intervals[2].expandTo(b.z());
        return *this;
    }

    Box3d & expandTo(Box3d const & b) {
        _intervals[0].expandTo(b.x());
        _intervals[1].expandTo(b.y());
        _intervals[2].expandTo(b.z());
        return *this;
    }
    ///@}

    ///@{
    /// `expandedTo` returns the smallest box containing the union of
    /// this box and b.
    Box3d expandedTo(Vector3d const & b) const {
        return Box3d(*this).expandTo(b);
    }

    Box3d expandedTo(Box3d const & b) const {
        return Box3d(*this).expandTo(b);
    }
    ///@}

    /// `dilateBy` minimally expands or shrinks this Box to include or
    /// remove all points within distance |r| of its boundary.
    ///
    /// If this box is empty or full, or if r is zero, there is
    /// no effect. If r is positive, points are added, and if r is
    /// negative they are removed.
    Box3d & dilateBy(double r) { return dilateBy(r, r, r); }
    Box3d dilatedBy(double r) const { return Box3d(*this).dilateBy(r); }

    /// `dilateBy` morphologically dilates or erodes the x, y, and z intervals
    /// of this box by w, h, and d. If w is positive, the x interval is
    /// dilated by [-w,w]. If w is zero, the corresponding interval is not
    /// modified, and if it is negative, the longitude interval is eroded by
    /// [w,-w]. The action of h and d on the y and z intervals is analogous.
    ///
    /// If this box is empty or full, there is no effect.
    Box3d & dilateBy(double w, double h, double d) {
        _intervals[0].dilateBy(w);
        _intervals[1].dilateBy(h);
        _intervals[2].dilateBy(d);
        _enforceInvariants();
        return *this;
    }
    Box3d dilatedBy(double w, double h, double d) const {
        return Box3d(*this).dilateBy(w, h, d);
    }
    Box3d & erodeBy(double r) { return dilateBy(-r); }
    Box3d erodedBy(double r) const { return dilatedBy(-r); }
    Box3d & erodeBy(double w, double h, double d) {
        return dilateBy(-w, -h, -d);
    }
    Box3d erodedBy(double w, double h, double d) const {
        return dilatedBy(-w, -h, -d);
    }

    Relationship relate(Vector3d const & v) const { return relate(Box3d(v)); }

    Relationship relate(Box3d const & b) const {
        Relationship xr = x().relate(b.x());
        Relationship yr = y().relate(b.y());
        Relationship zr = z().relate(b.z());
        // If the box x, y, or z intervals are disjoint, then so are the
        // boxes. The other relationships must hold for all constituent
        // intervals in order to hold for the boxes.
        return ((xr & yr & zr) & (CONTAINS | WITHIN)) |
               ((xr | yr | zr) & DISJOINT);
    }


private:
    void _enforceInvariants() {
        // Make sure that all intervals are empty, or none are. This
        // simplifies the implementation of relate and dilateBy.
        if (x().isEmpty() || y().isEmpty() || z().isEmpty()) {
            _intervals[0] = Interval1d();
            _intervals[1] = Interval1d();
            _intervals[2] = Interval1d();
        }
    }

    Interval1d _intervals[3];
};

std::ostream & operator<<(std::ostream &, Box3d const &);

}} // namespace lsst::sphgeom

#endif // LSST_SPHGEOM_BOX3D_H_
