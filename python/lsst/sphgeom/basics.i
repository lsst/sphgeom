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
    #include "lsst/sphgeom/Angle.h"
    #include "lsst/sphgeom/NormalizedAngle.h"
    #include "lsst/sphgeom/LonLat.h"
    #include "lsst/sphgeom/UnitVector3d.h"
    #include "lsst/sphgeom/Vector3d.h"
%}

%ignore operator<<(std::ostream & s, lsst::sphgeom::Angle const & a);
%ignore operator<<(std::ostream & s, lsst::sphgeom::NormalizedAngle const & a);
%ignore operator<<(std::ostream & s, lsst::sphgeom::LonLat const & p);
%ignore operator<<(std::ostream & s, lsst::sphgeom::Vector3d const & v);
%ignore operator<<(std::ostream & s, lsst::sphgeom::UnitVector3d const & v);

%mimicCast(Angle, NormalizedAngle);

%extend lsst::sphgeom::Vector3d {
    // Don't expose the unsafe parts of the C++ API
    %ignore getData;
    %ignore operator ();
}

%extend lsst::sphgeom::UnitVector3d {
    // Don't expose the unsafe parts of the C++ API
    %ignore getData;
    %ignore operator ();
    %ignore fromNormalized;
}

%mimicCast(Vector3d, UnitVector3d);

%copyctor lsst::sphgeom::Vector3d;


%include "lsst/sphgeom/Angle.h"
%include "lsst/sphgeom/Vector3d.h"
%include "lsst/sphgeom/NormalizedAngle.h"
%include "lsst/sphgeom/LonLat.h"
%include "lsst/sphgeom/UnitVector3d.h"


%define %addSpecialsToAngleType(TYPE)
%extend lsst::sphgeom::TYPE {
    %pythoncode %{
        def __reduce__(self):
            return (TYPE, (self.asRadians(),))

        def __rmul__(self, *args):
            return self.__mul__(*args)

        def __str__(self):
            return repr(self.asRadians())

        def __repr__(self):
            return "{}({!r})".format(self.__class__.__name__, self.asRadians())

        __truediv__ = __div__
    %}
}
%enddef

%addSpecialsToAngleType(Angle);
%addSpecialsToAngleType(NormalizedAngle);

%addStreamStr(LonLat);

%extend lsst::sphgeom::LonLat {
    %pythoncode %{
        def __reduce__(self):
            return (LonLat.fromRadians,
                    (self.getLon().asRadians(), self.getLat().asRadians()))

        def __repr__(self):
            return "LonLat.fromRadians({!r}, {!r})".format(
                self.getLon().asRadians(), self.getLat().asRadians())
    %}
}

%define %addSpecialsToVectorType(TYPE)
%addStreamStr(TYPE);
%extend lsst::sphgeom::TYPE {
    %pythoncode %{
        def __len__(self):
            return 3

        def __getitem__(self, key):
            return (self.x(), self.y(), self.z()).__getitem__(key)

        def __iter__(self):
            yield self.x()
            yield self.y()
            yield self.z()

        def __reduce__(self):
            return (TYPE, (self.x(), self.y(), self.z()))

        def __repr__(self):
            return "{}({!r}, {!r}, {!r})".format(
                self.__class__.__name__, self.x(), self.y(), self.z())

        __truediv__ = __div__
    %}
}
%enddef

%addSpecialsToVectorType(Vector3d);
%addSpecialsToVectorType(UnitVector3d);

%{
    inline PyObject * _convertToList(std::vector<lsst::sphgeom::UnitVector3d> const & input) {
        std::unique_ptr<PyObject> result(PyList_New(input.size()));
        if (!result) {
            return nullptr;
        }
        Py_ssize_t n = 0;
        for (auto i = input.begin(); i != input.end(); ++i, ++n) {
            PyObject * v = SWIG_NewPointerObj(
                (new lsst::sphgeom::UnitVector3d(*i)),
                SWIGTYPE_p_lsst__sphgeom__UnitVector3d,
                SWIG_POINTER_OWN | 0
            );
            if (v == nullptr) {
                return 0;
            }
            PyList_SET_ITEM(result.get(), n, v);
        }
        return result.release();
    }
%}

%typemap(out) std::vector<lsst::sphgeom::UnitVector3d> {
    $result = _convertToList($1);
}
%typemap(out) std::vector<lsst::sphgeom::UnitVector3d> const & {
    $result = _convertToList(*$1);
}

%typemap(in) std::vector<lsst::sphgeom::UnitVector3d> const & (std::vector<lsst::sphgeom::UnitVector3d> v) {
    {
        PyObject * iterator = PyObject_GetIter($input);
        if (iterator == nullptr) {
            PyErr_SetString(PyExc_TypeError,
                            "expected an iterable over lsst.sphgeom.UnitVector3d objects");
            return nullptr;
        }
        PyObject * item = nullptr;
        for (item = PyIter_Next(iterator); item != nullptr; item = PyIter_Next(iterator)) {
            void * ptr = nullptr;
            int res = SWIG_ConvertPtr(item, &ptr, $descriptor(lsst::sphgeom::UnitVector3d *), 0);
            if (!SWIG_IsOK(res) || ptr == nullptr) {
                PyErr_SetString(PyExc_TypeError,
                                "expected an iterable over lsst.sphgeom.UnitVector3d objects");
                return nullptr;
            }
            v.emplace_back(*reinterpret_cast<lsst::sphgeom::UnitVector3d *>(ptr));
            Py_DECREF(item);
        }
        Py_DECREF(iterator);
        $1 = &v;
    }
}
