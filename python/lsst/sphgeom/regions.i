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
    // For Python 2/3 compatibility
    #include "bytesobject.h"

    #include "lsst/sphgeom/Region.h"
    #include "lsst/sphgeom/Box.h"
    #include "lsst/sphgeom/Box3d.h"
    #include "lsst/sphgeom/Circle.h"
    #include "lsst/sphgeom/Ellipse.h"
    #include "lsst/sphgeom/ConvexPolygon.h"
%}


// This typemap maps a C++ bytestring to a Python bytestring.
%typemap(out) std::vector<uint8_t>, std::vector<uint8_t> const & {
    $result = PyBytes_FromStringAndSize(
        reinterpret_cast<char const *>($1.data()),
        static_cast<Py_ssize_t>($1.size())
    );
}

// This typemaps converts a unique Region pointer to a shared one.
%typemap(out) std::unique_ptr<lsst::sphgeom::Region> {
    $result = SWIG_NewPointerObj(
        SWIG_as_voidptr(new std::shared_ptr<lsst::sphgeom::Region>($1.release())),
        SWIGTYPE_p_std__shared_ptrT_lsst__sphgeom__Region_t,
        SWIG_POINTER_OWN
    );
}

// This typemap unpacks a Python bytes or bytearray object
// into a pair of arguments (for Region::decode).
%typemap(in) (uint8_t const * buffer, size_t n) {
    if (PyBytes_Check($input)) {
        $1 = reinterpret_cast<uint8_t *>(PyBytes_AsString($input));
        $2 = static_cast<size_t>(PyBytes_Size($input));
    } else if (PyByteArray_Check($input)) {
        $1 = reinterpret_cast<uint8_t *>(PyByteArray_AsString($input));
        $2 = static_cast<size_t>(PyByteArray_Size($input));
    } else {
        PyErr_SetString(PyExc_TypeError, "expected a bytearray or byte string");
        return nullptr;
    }
}


%shared_ptr(lsst::sphgeom::Box3d);
%ignore operator<<(std::ostream & s, lsst::sphgeom::Box3d const & a);


// Even though this package does not use shared pointers, other packages may
// wish to, especially for the Region types. To enable this, wrap Region and
// its subclasses using %shared_ptr.

%shared_ptr(lsst::sphgeom::Region)

%extend lsst::sphgeom::Region {
    %ignore decode(std::vector<uint8_t> const &);
}

%define %prepareRegion(TYPE)
    %shared_ptr(lsst::sphgeom::TYPE);

    %ignore operator<<(std::ostream & s, lsst::sphgeom::TYPE const & a);

    %typemap(out) std::unique_ptr<lsst::sphgeom::TYPE> {
        $result = SWIG_NewPointerObj(
            SWIG_as_voidptr(new std::shared_ptr<lsst::sphgeom::TYPE>($1.release())),
            SWIGTYPE_p_std__shared_ptrT_lsst__sphgeom__ ## TYPE ## _t,
            SWIG_POINTER_OWN
        );
    }

    %extend lsst::sphgeom::TYPE {
        %ignore decode(std::vector<uint8_t> const &);
    }
%enddef


%prepareRegion(Box);
%prepareRegion(Circle);
%prepareRegion(Ellipse);
%prepareRegion(ConvexPolygon);

%safeChainingFor(Box);
%safeChainingFor(Box3d);
%safeChainingFor(Circle);

%extend lsst::sphgeom::Circle {
    %returnSelf(complement);
}

%extend lsst::sphgeom::Ellipse {
    // TODO: wrap Matrix3d and expose this method.
    %ignore getTransformMatrix;
}

%include "lsst/sphgeom/Region.h"
%include "lsst/sphgeom/Box.h"
%include "lsst/sphgeom/Box3d.h"
%include "lsst/sphgeom/Circle.h"
%include "lsst/sphgeom/Ellipse.h"
%include "lsst/sphgeom/ConvexPolygon.h"

%addStreamStr(Box3d);

%define %finishRegion(TYPE)
    %addStreamStr(TYPE);
    %extend lsst::sphgeom::TYPE {
        static std::shared_ptr<TYPE> cast(std::shared_ptr<Region> p) {
            return std::dynamic_pointer_cast<lsst::sphgeom::TYPE>(p);
        }

        %pythoncode %{
            def __reduce__(self):
                return (self.decode, (self.encode(),))
        %}
    }
%enddef

%finishRegion(Box);
%finishRegion(Circle);
%finishRegion(Ellipse);
%finishRegion(ConvexPolygon);

%extend lsst::sphgeom::Box {
    %pythoncode %{
        def __repr__(self):
            return "{}.fromRadians({!r}, {!r}, {!r}, {!r})".format(
                self.__class__.__name__,
                self.getLon().getA(),
                self.getLat().getA(),
                self.getLon().getB(),
                self.getLat().getB()
            )
    %}
}

%extend lsst::sphgeom::Box3d {
    %pythoncode %{
        def __repr__(self):
            return "{}({!r}, {!r}, {!r})".format(
                self.__class__.__name__, self.x(), self.y(), self.z())
    %}
}

%extend lsst::sphgeom::Circle {
    %pythoncode %{
        def __repr__(self):
            return "{}({!r}, {!r})".format(
                self.__class__.__name__, self.getCenter(), self.getSquaredChordLength())
    %}
}

%extend lsst::sphgeom::ConvexPolygon {
    %pythoncode %{
        def __repr__(self):
            return "{}({!r})".format(self.__class__.__name__, self.getVertices())
    %}
}
