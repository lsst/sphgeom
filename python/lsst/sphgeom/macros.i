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


// This macro adds a definition of __str__ based on the stream output operator.
%define %addStreamStr(TYPE)
    %extend lsst::sphgeom::TYPE {
        std::string __str__() const {
            std::ostringstream oss;
            oss << (*self);
            return oss.str();
        }
    }
%enddef


// This macro provides a safe equivalent to returning *this.
%define %returnSelf(FUNC)
    %feature("pythonappend") FUNC %{ val = self %}
%enddef

%define %safeChainingFor(TYPE)
    %typemap(out) lsst::sphgeom::TYPE & {
        Py_INCREF(Py_None);
        $result = Py_None;
    }

    %extend lsst::sphgeom::TYPE {
        %returnSelf(clipTo);
        %returnSelf(expandTo);
        %returnSelf(dilateBy);
        %returnSelf(erodeBy);
    }
%enddef



// Given a class REFINED which has a non-explicit cast operator producing a
// GENERAL const &, this macro sets up SWIG input and typecheck typemaps
// to make Python behave similarly to C++.

%define %mimicCast(GENERAL, REFINED)

%extend lsst::sphgeom::REFINED {
    %rename(as ## GENERAL) operator GENERAL const &;
}

// To mimic the implicit casts from REFINED to GENERAL in C++, check
// for REFINED everytime we see an GENERAL input argument.
%typemap(in) lsst::sphgeom::GENERAL const & {
    {
        void * arg = nullptr;
        int res = SWIG_ConvertPtr($input, &arg, $descriptor(lsst::sphgeom::GENERAL *), 0);
        if (!SWIG_IsOK(res)) {
            res = SWIG_ConvertPtr($input, &arg, $descriptor(lsst::sphgeom::REFINED *), 0);
            if (!SWIG_IsOK(res)) {
                PyErr_SetString(PyExc_TypeError, "expected a lsst::sphgeom::" #GENERAL
                                " or a lsst::sphgeom::" #REFINED);
                return nullptr;
            }
            if (arg == nullptr) {
                PyErr_SetString(PyExc_ValueError, "invalid null reference of type "
                                "lsst::sphgeom::" #REFINED);
                return nullptr;
            }
            $1 = const_cast<lsst::sphgeom::GENERAL *>(
                &static_cast<lsst::sphgeom::GENERAL const &>(
                    *reinterpret_cast<lsst::sphgeom::REFINED *>(arg)
                )
            );
        } else if (arg == nullptr) {
            PyErr_SetString(PyExc_ValueError, "invalid null reference of type "
                            "lsst::sphgeom::" #GENERAL);
            return nullptr;
        } else {
            $1 = reinterpret_cast<lsst::sphgeom::GENERAL *>(arg);
        }
    }
}

%typemap(in) lsst::sphgeom::GENERAL {
    {
        void * arg = nullptr;
        int res = SWIG_ConvertPtr($input, &arg, $descriptor(lsst::sphgeom::GENERAL *), 0);
        if (!SWIG_IsOK(res)) {
            res = SWIG_ConvertPtr($input, &arg, $descriptor(lsst::sphgeom::REFINED *), 0);
            if (!SWIG_IsOK(res)) {
                PyErr_SetString(PyExc_TypeError, "expected a lsst::sphgeom::" #GENERAL
                                " or a lsst::sphgeom::" #REFINED);
                return nullptr;
            }
            if (arg == nullptr) {
                PyErr_SetString(PyExc_ValueError, "invalid null reference of type "
                                "lsst::sphgeom::" #REFINED);
                return nullptr;
            }
            $1 = static_cast<lsst::sphgeom::GENERAL const &>(
                *reinterpret_cast<lsst::sphgeom::REFINED *>(arg)
            );
        } else if (arg == nullptr) {
            PyErr_SetString(PyExc_ValueError, "invalid null reference of type "
                            "lsst::sphgeom::" #GENERAL);
            return nullptr;
        } else {
            $1 = *reinterpret_cast<lsst::sphgeom::GENERAL *>(arg);
        }
    }
}

// The new input typemaps above require a corresponding typecheck typemap.
//
// The typecheck typemap for REFINED is also redefined so that it has lower precedence
// than the one for GENERAL. This way, if there is an overload taking a REFINED
// and another taking a GENERAL, REFINED arguments get dispatched to the correct one.
%typecheck(10000) lsst::sphgeom::REFINED {
    {
        void *ptr;
        if (!SWIG_IsOK(SWIG_ConvertPtr($input, &ptr, $descriptor(lsst::sphgeom::REFINED *), 0))) {
            $1 = 0;
            PyErr_Clear();
        } else {
            $1 = 1;
        }
    }
}

%typecheck(10001) lsst::sphgeom::GENERAL {
    {
        void *ptr;
        if (!SWIG_IsOK(SWIG_ConvertPtr($input, &ptr, $descriptor(lsst::sphgeom::GENERAL *), 0)) &&
            !SWIG_IsOK(SWIG_ConvertPtr($input, &ptr, $descriptor(lsst::sphgeom::REFINED *), 0))) {
            $1 = 0;
            PyErr_Clear();
        } else {
            $1 = 1;
        }
    }
}

%typecheck(SWIG_TYPECHECK_POINTER) lsst::sphgeom::GENERAL const & {
    {
        void *ptr;
        if (!SWIG_IsOK(SWIG_ConvertPtr($input, &ptr, $descriptor(lsst::sphgeom::GENERAL *), 0)) &&
            !SWIG_IsOK(SWIG_ConvertPtr($input, &ptr, $descriptor(lsst::sphgeom::REFINED *), 0))) {
            $1 = 0;
            PyErr_Clear();
        } else {
            $1 = 1;
        }
    }
}

%enddef
