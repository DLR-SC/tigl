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

namespace tigl
{
    // define CreateIfNotExistsTag as a type which's values are compile-time constants (see implementation of boost::none)
    namespace internal
    {
        struct CreateIfNotExistsTagHelper{};
    }
    typedef int internal::CreateIfNotExistsTagHelper::*CreateIfNotExistsTag;

    // create constant CreateIfNotExists
    // TODO: we should import the original CreateIfNotExists.h if we can figure out the problems with the following line
    // const CreateIfNotExistsTag CreateIfNotExists = static_cast<CreateIfNotExistsTag>(0);
}
%include "CTiglMemoryPool.h"
%include "generated/CPACSStringVectorBase.h"
%include "CCPACSStringVector.h"
%include "generated/CPACSUpdates.h"
%include "generated/CPACSHeader.h"

%template(DoubleVector) std::vector<double>;

enum TiglReturnCode
{
    TIGL_SUCCESS             = 0,
    TIGL_ERROR               = 1,
    TIGL_NULL_POINTER        = 2,
    TIGL_NOT_FOUND           = 3,
    TIGL_XML_ERROR           = 4,
    TIGL_OPEN_FAILED         = 5,
    TIGL_CLOSE_FAILED        = 6,
    TIGL_INDEX_ERROR         = 7,
    TIGL_STRING_TRUNCATED    = 8,
    TIGL_WRONG_TIXI_VERSION  = 9,
    TIGL_UID_ERROR           = 10,
    TIGL_WRONG_CPACS_VERSION = 11,
    TIGL_UNINITIALIZED       = 12,
    TIGL_MATH_ERROR          = 13,
    TIGL_WRITE_FAILED        = 14
};

enum TiglLogLevel
{
    TILOG_SILENT   =0, /*!< No messages are printed. TiGL is completely silent, even in case of errors. */
    TILOG_ERROR    =1, /*!< Only error messages are printed. */
    TILOG_WARNING  =2, /*!< Only errors and warnings are printed on console. This is the default log level of TiGL. */
    TILOG_INFO     =3, /*!< In addition to TILOG_WANING, also informative messages are printed. */
    TILOG_DEBUG    =4, /*!< Also debug messages are printed. Enable this if you want to track down potential errors in TiGL. */
    TILOG_DEBUG1   =5, /*!< This level is only interesting for TiGL developers */
    TILOG_DEBUG2   =6, /*!< This level is only interesting for TiGL developers */
    TILOG_DEBUG3   =7, /*!< This level is only interesting for TiGL developers */
    TILOG_DEBUG4   =8  /*!< This level is only interesting for TiGL developers */
};

enum TiglBoolean
{
    TIGL_FALSE = 0,
    TIGL_TRUE  = 1
};

enum TiglSymmetryAxis
{
    TIGL_NO_SYMMETRY = 0,
    TIGL_X_Y_PLANE   = 1,
    TIGL_X_Z_PLANE   = 2,
    TIGL_Y_Z_PLANE   = 3
};

enum TiglAlgorithmCode
{
    TIGL_INTERPOLATE_LINEAR_WIRE  = 0,         /**< Use a linear interpolation between the points of a wire */
    TIGL_INTERPOLATE_BSPLINE_WIRE = 1,         /**< Use a BSpline interpolation between the points of a wire */
    TIGL_APPROXIMATE_BSPLINE_WIRE = 2          /**< Use a BSpline approximation for the points of a wire */
};

enum TiglStructureType
{
    UPPER_SHELL = 0,
    LOWER_SHELL = 1,
    INNER_STRUCTURE = 2
};

enum TiglContinuity
{
    C0 = 0,
    C1 = 1,
    C2 = 2
};

enum TiglImportExportFormat
{
    TIGL_IMPORTEXPORT_IGES = 0,            /**< Use IGES format for geometry import/export */
    TIGL_IMPORTEXPORT_STEP = 1,            /**< Use STEP format for geometry import/export */
    TIGL_IMPORTEXPORT_STL  = 2,            /**< Use STL format for geometry import/export */
    TIGL_IMPORTEXPORT_VTK  = 3             /**< Use VTK (XML/VTP) format for geometry import/export */
};

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

        PyErr_SetString(exc_class, const_cast<char*>(err.what()));
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
