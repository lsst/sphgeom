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

%{
    #include "lsst/sphgeom/Interval.h"
    #include "lsst/sphgeom/AngleInterval.h"
    #include "lsst/sphgeom/Interval1d.h"
    #include "lsst/sphgeom/NormalizedAngleInterval.h"
%}

%include "lsst/sphgeom/Interval.h"

%define %intervalBaseFor(DERIVED, SCALAR)
    %typemap(out) lsst::sphgeom::Interval<lsst::sphgeom::DERIVED, SCALAR> & {
        Py_INCREF(Py_None);
        $result = Py_None;
    }
    
    %extend lsst::sphgeom::Interval<lsst::sphgeom::DERIVED, SCALAR> {
        %returnSelf(clipTo);
        %returnSelf(expandTo);
        %returnSelf(dilateBy);
        %returnSelf(erodeBy);
    }

    %template(DERIVED ## Base) lsst::sphgeom::Interval<lsst::sphgeom::DERIVED, SCALAR>;
%enddef

%intervalBaseFor(Interval1d, double); 
%intervalBaseFor(AngleInterval, lsst::sphgeom::Angle); 

%copyctor lsst::sphgeom::Interval1d;
%copyctor lsst::sphgeom::AngleInterval;

%include "lsst/sphgeom/Interval1d.h"
%include "lsst/sphgeom/AngleInterval.h"


%addStreamStr(Interval1d);
%extend lsst::sphgeom::Interval1d {
    %pythoncode %{
        def __reduce__(self):
            return (Interval1d, (self.getA(), self.getB()))

        def __repr__(self):
            return "Interval1d({!r}, {!r})".format(self.getA(), self.getB())
    %}
}

%addStreamStr(AngleInterval);
%extend lsst::sphgeom::AngleInterval {
    %pythoncode %{
        def __reduce__(self):
            return (AngleInterval.fromRadians,
                    (self.getA().asRadians(), self.getB().asRadians()))

        def __repr__(self):
            return "AngleInterval.fromRadians({!r}, {!r})".format(
                self.getA().asRadians(), self.getB().asRadians())
    %}
}


%safeChainingFor(NormalizedAngleInterval);

%include "lsst/sphgeom/NormalizedAngleInterval.h"

%addStreamStr(NormalizedAngleInterval);
%extend lsst::sphgeom::NormalizedAngleInterval {
    %pythoncode %{
        def __reduce__(self):
            return (NormalizedAngleInterval.fromRadians,
                    (self.getA().asRadians(), self.getB().asRadians()))

        def __repr__(self):
            return "NormalizedAngleInterval.fromRadians({!r}, {!r})".format(
                self.getA().asRadians(), self.getB().asRadians())
    %}
}
