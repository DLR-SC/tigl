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
  if (*$1 != boost::none) {
    $result = SWIG_NewPointerObj(SWIG_as_voidptr((type*)&$1->get()), $descriptor(type*), 0 | 0 );
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%typemap(out, noblock=1) boost::optional<type> {
  if (*(&$1) != boost::none) {
    type* p_copy = new type(static_cast< const type& >($1.get()));
    $result = SWIG_NewPointerObj(SWIG_as_voidptr(p_copy), $descriptor(type*), SWIG_POINTER_OWN | 0 );
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%typemap(out, noblock=1) boost::optional<const type&>, boost::optional<type&> {
  if (*(&$1) != boost::none) {
    $result = SWIG_NewPointerObj(SWIG_as_voidptr((type*)&($1.get())), $descriptor(type*), 0 | 0 );
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%typemap(in) const boost::optional<type>& (boost::optional<type> arg_optional_val, int res=0) {
  $1 = &arg_optional_val;
  if ($input != Py_None) {
    void* ptmp;
    res = SWIG_ConvertPtr($input, &ptmp, $descriptor(type*),  0 );
    if (!SWIG_IsOK(res)) {
      %argument_fail(res, type, $symname, $argnum); 
    }
    *$1 = *(reinterpret_cast< type * >(ptmp));
  }
}

%enddef

%typemap(in, noblock=1) boost::optional<std::string> {
  if ($input == Py_None) {
    $1 = boost::optional<type>();
  } else {
    $1 = boost::optional<type>(PyFun_ConvertToNative($input));
  }
}

%define WRAP_BOOST_OPTIONAL_BASIC_TYPE(type, PyFun_Convert, PyFun_ConvertToNative)
%typemap(out, noblock=1) boost::optional<type>&, const boost::optional<type>& {
  if (*$1 != boost::none) {
    $result = PyFun_Convert($1->get());
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%typemap(out, noblock=1) boost::optional<type>, boost::optional<const type&> {
  if (*(&$1) != boost::none) {
    $result = PyFun_Convert($1.get());
  } else {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}

%typemap(in, noblock=1) const boost::optional<type>& (boost::optional<type> arg_optional_val) {
  $1 = &arg_optional_val;
  if ($input != Py_None) {
    *$1 = PyFun_ConvertToNative($input);
  }
}

%typemap(in, noblock=1) boost::optional<type> {
  if ($input == Py_None) {
    $1 = boost::optional<type>();
  } else {
    $1 = boost::optional<type>(PyFun_ConvertToNative($input));
  }
}

%enddef

#define PYSTR_FROM_STD_STRING(stdstr) (PyString_FromString((stdstr).c_str()))

%fragment("boost_optional", "header") {

int as_std_string_val(PyObject *obj, std::string& result) {
  std::string *ptr = (std::string *)0;
  int res = SWIG_AsPtr_std_string(obj, &ptr);
  if (ptr) {
    result = *ptr;
    if (SWIG_IsNewObj(res)) {
      %delete(ptr);
      res = SWIG_DelNewMask(res);
    }
  }
  return res;
}

int as_boost_optional_string(PyObject *obj, boost::optional<std::string>& result) {
  int res = SWIG_OK;
  if (obj != Py_None) {
    std::string tmp;
    res = as_std_string_val(obj, tmp);
    result = boost::optional<std::string>(tmp);
  }
  else {
    result = boost::optional<std::string>();
  }
  return res;
}
} // end fragment

// wrap some standard types
WRAP_BOOST_OPTIONAL_BASIC_TYPE(std::string, PYSTR_FROM_STD_STRING, ___ )

%typemap(in, noblock=1, fragment="boost_optional") const boost::optional<std::string>& (boost::optional<std::string> arg_optional_val, int res_opt) {
  $1 = &arg_optional_val;
  res_opt = as_boost_optional_string($input, arg_optional_val);
  if (!SWIG_IsOK(res_opt)) {
    %argument_fail(res_opt, string, $symname, $argnum); 
  }
}

WRAP_BOOST_OPTIONAL_BASIC_TYPE(int, PyLong_FromLong, PyLong_AsLong)
WRAP_BOOST_OPTIONAL_BASIC_TYPE(unsigned int, PyLong_FromUnsignedLong, PyLong_AsUnsignedLong)
WRAP_BOOST_OPTIONAL_BASIC_TYPE(double, PyFloat_FromDouble, PyFloat_AsDouble)
WRAP_BOOST_OPTIONAL_BASIC_TYPE(float, PyFloat_FromDouble, PyFloat_AsDouble)

%define %boost_optional(TYPE)
WRAP_BOOST_OPTIONAL_CLASS(TYPE)
%enddef
