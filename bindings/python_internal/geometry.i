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

%module (package="tigl") geometry

%include common.i

%shared_ptr(CNamedShape)


%{
#include<TopoDS.hxx>
#include <math_Vector.hxx>
#include <math_Matrix.hxx>
#include <Handle_Geom_BSplineCurve.hxx>
%}

// Import python-occ definitions to
// have a compatible interface
%import math.i
%import Geom.i
%import TopoDS.i
%import tmath.i
%import core.i

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
#include "ITiglGeometricComponent.h"
#include "CTiglError.h"
#include "CTiglShapeCache.h"
#include "CTiglPolyData.h"
#include "CTiglSymetricSplineBuilder.h"
#include "CWireToCurve.h"
#include "ListPNamedShape.h"
#include "CCPACSPoint.h"
#include "ECPACSTranslationType.h"
#include "CTiglAbstractGeometricComponent.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSPointListXYZ.h"
#include "CCPACSPointListRelXYZ.h"
#include "generated/CPACSPointXYZ.h"
%}


%feature("autodoc", "3");


// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

%template(CPointContainer) std::vector<gp_Pnt>;

%include "ECPACSTranslationType.h"
%include "generated/CPACSPoint.h"
%include "generated/CPACSPointAbsRel.h"
%include "CCPACSPoint.h"
%include "CCPACSPointAbsRel.h"
%include "ECPACSTranslationType.h"
%include "CTiglTransformation.h"
%include "CNamedShape.h"
%include "CSharedPtr.h"
%include "PNamedShape.h"
%include "CTiglPoint.h"
%include "generated/CPACSPointXYZ.h"
%include "generated/CPACSPointListXYZVector.h"
%include "generated/CPACSPointListRelXYZVector.h"
%include "CCPACSPointListRelXYZ.h"
%include "CCPACSPointListXYZ.h"
%include "CFunctionToBspline.h"
%include "generated/CPACSTransformation.h"
%include "CCPACSTransformation.h"
%include "CCSTCurveBuilder.h"
%include "ITiglGeometricComponent.h"
%include "CTiglAbstractGeometricComponent.h"
%include "CTiglAbstractSegment.h"
%include "CTiglShapeCache.h"
%include "CTiglPolyData.h"
%include "CTiglSymetricSplineBuilder.h"
%include "CWireToCurve.h"
%include "ListPNamedShape.h"

