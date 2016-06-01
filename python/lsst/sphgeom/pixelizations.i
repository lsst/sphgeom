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
    #include "lsst/sphgeom/curve.h"
    #include "lsst/sphgeom/Pixelization.h"
    #include "lsst/sphgeom/RangeSet.h"
    #include "lsst/sphgeom/HtmPixelization.h"
    #include "lsst/sphgeom/q3c.h"
%}

%{
    inline PyObject * _rangeBeginningOrEndToIntOrLong(uint64_t p, bool isEndPoint) {
        PyObject * o;
        if (p == 0 && isEndPoint) {
            // Map trailing bookend to 2**64
            PyObject * tmp = PyLong_FromUnsignedLongLong(UINT64_C(0x100000000));
            if (!tmp) {
                return nullptr;
            }
            o = PyNumber_Multiply(tmp, tmp);
            Py_DECREF(tmp);
        } else {
            if (p <= static_cast<unsigned long long>(PyInt_GetMax())) {
                o = PyInt_FromLong(static_cast<long>(p));
            } else {
                o = PyLong_FromUnsignedLongLong(p);
            }
        }
        return o;
    }

    inline PyObject * _rangesToList(std::vector<std::tuple<uint64_t, uint64_t> > const & input) {
        auto xdecref = [](PyObject * p) { Py_XDECREF(p); };
        std::unique_ptr<PyObject, decltype(xdecref)> result(PyList_New(input.size()), xdecref);
        if (!result) {
            return nullptr;
        }
        Py_ssize_t n = 0;
        for (auto const & t : input) {
            std::unique_ptr<PyObject, decltype(xdecref)> tup(PyTuple_New(2), xdecref);
            if (!tup) {
                return nullptr;
            }
            PyObject * o = _rangeBeginningOrEndToIntOrLong(std::get<0>(t), false);
            if (!o) {
                return nullptr;
            }
            PyTuple_SET_ITEM(tup.get(), 0, o);
            o = _rangeBeginningOrEndToIntOrLong(std::get<1>(t), true);
            if (!o) {
                return nullptr;
            }
            PyTuple_SET_ITEM(tup.get(), 1, o);
            PyList_SET_ITEM(result.get(), n, tup.release());
            ++n;
        }
        return result.release();
    }
%}

%typemap(out) std::vector<std::tuple<uint64_t, uint64_t> > {
    $result = _rangesToList($1);
}

%shared_ptr(lsst::sphgeom::Pixelization)
%shared_ptr(lsst::sphgeom::HtmPixelization)
%shared_ptr(lsst::sphgeom::RangeSet)

%copyctor lsst::sphgeom::RangeSet;

// TODO:
// - input and typecheck typemap for std::tuple<uint64_t, uint64_t>
// - construct RangeSet from std::vector<uint64_t>
// - construct RangeSet from std::vector<std::tuple<uint64_t, uint64_t> >

%typemap(out) lsst::sphgeom::RangeSet & {
    Py_INCREF(Py_None);
    $result = Py_None;
}

%extend lsst::sphgeom::RangeSet {
    %returnSelf(complement);
    %returnSelf(operator&=);
    %returnSelf(operator|=);
    %returnSelf(operator-=);
    %returnSelf(operator^=);
    %returnSelf(simplify);
}

%include "lsst/sphgeom/RangeSet.h"

// Instead of dealing with the complexities of exposing C++ iterators via
// SWIG, provide a way to get a copy of the ranges in a RangeSet as a Python
// list of tuples.
%extend lsst::sphgeom::RangeSet {
    std::vector<std::tuple<uint64_t, uint64_t> > ranges() const {
        return std::vector<std::tuple<uint64_t, uint64_t> >(self->begin(), self->end());
    }
}

%addStreamStr(RangeSet);

%include "lsst/sphgeom/Pixelization.h"
%include "lsst/sphgeom/curve.h"
%include "lsst/sphgeom/HtmPixelization.h"
%include "lsst/sphgeom/q3c.h"
