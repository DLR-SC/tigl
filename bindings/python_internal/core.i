/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-11-23 Martin Siggel <Martin.Siggel@dlr.de>
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


%module core

%include common.i


%{
#include "CTiglMemoryPool.h"
#include "CTiglError.h"
%}

%feature("autodoc", "3");

// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

%include "CTiglMemoryPool.h"
%include "CTiglError.h"

%feature("director:except") {
    if ($error != NULL) {
        throw Swig::DirectorMethodException();
    }
}

%exception {
    try {
        $action
    }
    catch (tigl::CTiglError & err) {

        int code = err.getCode();
        PyObject* exc_class = PyExc_RuntimeError;

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

        PyErr_SetString(exc_class, const_cast<char*>(err.getError()));
        SWIG_fail;
    }
}
