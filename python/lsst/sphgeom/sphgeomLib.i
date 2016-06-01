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

%define sphgeomLib_DOCSTRING
"
Python interface to lsst::sphgeom
"
%enddef

%feature("autodoc", "1");
%module(package="lsst.sphgeom", docstring=sphgeomLib_DOCSTRING) sphgeomLib

%{
    #include <memory>
    #include <sstream>
    #include <vector>

    #include "lsst/sphgeom/Relationship.h"

    using lsst::sphgeom::Relationship;

    // SwigValueWrapper is busted for std::unique_ptr.
    template <typename T>
    class SwigValueWrapper<std::unique_ptr<T>> {
        std::unique_ptr<T> ptr;
    public:
        SwigValueWrapper() : ptr() {}
        SwigValueWrapper & operator=(std::unique_ptr<T> && r) {
            ptr = std::move(r);
            return *this;
        }
        T * release() { return ptr.release(); }
    };
%}


%pythonnondynamic;
%naturalvar;


%include "cpointer.i"
%include "exception.i"
%include "stdint.i"
%include "std_string.i"
%include "std_shared_ptr.i"
%include "carrays.i"
%include "typemaps.i"


%exception {
    try {
        $action
    }
    SWIG_CATCH_STDEXCEPT
}

// Wrapping the stream output operator is useless.
%ignore lsst::sphgeom::operator<<;

// Relationship is an alias for std::bitset<3>. SWIG does nothing good
// with bitsets, so they are mapped to integers. Note that because there is
// no Relationship.h %include, SWIG doesn't know anything about the type -
// not even its namespace. To get this typemap to match, it must be written
// exactly as Relationship appears in the C++ source, that is,
// without any namespace qualification.
%typemap(out) Relationship {
    $result = PyInt_FromLong(static_cast<long>(($1).to_ulong()));
}

// Module level constants mirroring the constexprs in Relationship.h
%pythoncode %{
# Bit-mask indicating that two Regions intersect, but neither
# contains the other.
INTERSECTS = 0
# Bit-mask indicating that two Regions are disjoint.
DISJOINT = 1
# Bit-mask indicating that a Region A contains another region B.
CONTAINS = 2
# Bit-mask indicating that a Region A is within another region B.
WITHIN = 4
%}

%include "macros.i"
%include "basics.i"
%include "intervals.i"
%include "regions.i"
%include "pixelizations.i"
