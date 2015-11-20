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

%module geometry

%include common.i

%shared_ptr(CNamedShape)


%{
// includes are required to for compilation
#include<Standard_ErrorHandler.hxx>
#include<Standard_Failure.hxx>
#include<TopoDS.hxx>
#include <math_Vector.hxx>
#include <math_Matrix.hxx>
#include <Handle_Geom_BSplineCurve.hxx>
%}

// Import python-occ definitions to
// have a compatible interface
%import Math.i
%import Geom.i
%import TopoDS.i
%import tmath.i

%{
// All includes that are required for the wrapper compilation
#include "tiglmathfunctions.h"
#include "CNamedShape.h"
#include "PNamedShape.h"
#include "CTiglPoint.h"
#include "CFunctionToBspline.h"
#include "CCPACSTransformation.h"
#include "CCSTCurveBuilder.h"
#include "ECPACSTranslationType.h"
#include "CTiglTransformation.h"
%}


%feature("autodoc", "3");


// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

%template(DoubleVector) std::vector<double>;

%include "ECPACSTranslationType.h"
%include "CTiglTransformation.h"
%include "CNamedShape.h"
%include "CSharedPtr.h"
%include "PNamedShape.h"
%include "CTiglPoint.h"
%include "CFunctionToBspline.h"
%include "CCPACSTransformation.h"
%include "CCSTCurveBuilder.h"

%inline %{

PNamedShape getshape() {
    return PNamedShape(new CNamedShape(TopoDS_Shape(), "bla"));
}

%}
