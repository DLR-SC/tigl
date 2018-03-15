/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-03-015 Martin Siggel <Martin.Siggel@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// Users can wrap custom classes using this macro
// use e.g. WRAP_BOOST_OPTIONAL_CLASS(tigl::WingCSStructure)
%define WRAP_BOOST_OPTIONAL_CLASS(type)

%typemap(out, noblock=1) boost::optional<type> const &, boost::optional<type const> const &, boost::optional<type>&, boost::optional<type const>& {
  if (*$1) {
    $result = SWIG_NewPointerObj(SWIG_as_voidptr((type*)&$1->get()), $descriptor(type*), 0 | 0 );
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%typemap(out, noblock=1) boost::optional<type> {
  if ($1) {
    type* p_copy = new type(static_cast< const type& >($1.get()));
    $result = SWIG_NewPointerObj(SWIG_as_voidptr(p_copy), $descriptor(type*), SWIG_POINTER_OWN | 0 );
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%typemap(out, noblock=1) boost::optional<const type&>, boost::optional<type&> {
  if ($1) {
    $result = SWIG_NewPointerObj(SWIG_as_voidptr((type*)&($1.get())), $descriptor(type*), 0 | 0 );
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%enddef

%define WRAP_BOOST_OPTIONAL_BASIC_TYPE(type, PyFun_Convert)
%typemap(out, noblock=1) boost::optional<type>&, const boost::optional<type>& {
  if (*$1) {
    $result = PyFun_Convert($1->get());
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%typemap(out, noblock=1) boost::optional<type> {
  if ($1) {
    $result = PyFun_Convert($1.get());
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}
%enddef

// wrap some standard types
WRAP_BOOST_OPTIONAL_CLASS(std::string)

WRAP_BOOST_OPTIONAL_BASIC_TYPE(int, PyLong_FromLong)
WRAP_BOOST_OPTIONAL_BASIC_TYPE(unsigned int, PyLong_FromUnsignedLong)
WRAP_BOOST_OPTIONAL_BASIC_TYPE(double, PyFloat_FromDouble)
WRAP_BOOST_OPTIONAL_BASIC_TYPE(float, PyFloat_FromDouble)

