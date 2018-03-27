/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-11-20 Martin Siggel <Martin.Siggel@dlr.de>
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

%include std_string.i
%include std_vector.i

%include tigl_config.h

// swig cannot handle c++ 11 properly yet
#undef HAVE_CPP11

%include tigl_internal.h

#ifdef HAVE_STDSHARED_PTR
  %include std_shared_ptr.i
#else
  %include boost_shared_ptr.i
#endif

%include unique_pointer.i

%include boost_optional.i


#define Handle(ClassName)  Handle_##ClassName
#define TixiDocumentHandle int
#define TiglCPACSConfigurationHandle int

%{
// includes are required to for compilation
#include<Standard_ErrorHandler.hxx>
#include<Standard_Failure.hxx>
#include "tigl.h"
#include "CTiglError.h"

namespace
{
PyObject* tiglError_to_PyExc(const tigl::CTiglError& err) {
  PyObject* exc_class = PyExc_RuntimeError;

  int code = err.getCode();
  switch (code) {
  case TIGL_INDEX_ERROR:
    exc_class = PyExc_IndexError;
    break;
  case TIGL_UID_ERROR:
    exc_class = PyExc_KeyError;
    break;
  case TIGL_MATH_ERROR:
    exc_class = PyExc_ArithmeticError;
    break;
  case TIGL_NULL_POINTER:
    exc_class = PyExc_ValueError;
    break;
  default:
    exc_class = PyExc_RuntimeError;
  }
  return exc_class;
}
}

%}

%include doc.i
