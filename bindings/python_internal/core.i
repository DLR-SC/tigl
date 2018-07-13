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


%module (package="tigl3") core

%include common.i


%{
#include "CreateIfNotExists.h"
#include "CTiglMemoryPool.h"
#include "CTiglError.h"
#include "CCPACSStringVector.h"
#include "generated/CPACSUpdates.h"
#include "generated/CPACSHeader.h"
%}

%feature("autodoc", "3");

// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

%boost_optional(tigl::generated::CPACSUpdates)
%boost_optional(tigl::CCPACSStringVector)

%include "CreateIfNotExists.h"
%include "CTiglMemoryPool.h"
%include "generated/CPACSStringVectorBase.h"
%include "CCPACSStringVector.h"
%include "generated/CPACSUpdates.h"
%include "generated/CPACSHeader.h"

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
        PyErr_SetString(tiglError_to_PyExc(err), const_cast<char*>(err.what()));
        SWIG_fail;
    }
    catch(Standard_Failure & err) {
        PyErr_SetString(PyExc_RuntimeError, const_cast<char*>((std::string("OpenCASCADE Error: ") + err.GetMessageString()).c_str()));
        SWIG_fail;
    }
    catch(std::exception & err) {
        PyErr_SetString(PyExc_RuntimeError, const_cast<char*>(err.what()));
        SWIG_fail;
    }
    catch(...) {
        PyErr_SetString(PyExc_RuntimeError, const_cast<char*>("An unkown error occured!"));
        SWIG_fail;
    }
}
