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

%module (package="tigl3") geometry

%include common.i

%shared_ptr(CNamedShape)


%{
#include<TopoDS.hxx>
%}

%include math_headers.i
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
#include "generated/CPACSPointX.h"
#include "tiglcommonfunctions.h"
#include "CTiglBSplineAlgorithms.h"
#include "CTiglBSplineApproxInterp.h"
#include "CTiglBSplineFit.h"
#include "CPointsToLinearBSpline.h"
#include "CTiglArcLengthReparameterization.h"
#include "CTiglMakeLoft.h"
#include "CTiglProjectOnLinearSpline.h"
#include "CTiglInterpolateCurveNetwork.h"
#include "CTiglPointsToBSplineInterpolation.h"
#include "CTiglCurvesToSurface.h"
%}


%feature("autodoc", "3");


// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

%template(CPointContainer) std::vector<gp_Pnt>;
%template(BSplineCurveList) std::vector<Handle_Geom_BSplineCurve>;
%template(CurveList) std::vector<Handle_Geom_Curve>;

%boost_optional(tigl::CCPACSPointAbsRel)
%boost_optional(tigl::CCPACSPoint)
%boost_optional(tigl::ECPACSTranslationType)
%boost_optional(tigl::generated::CPACSPointX)
%boost_optional(tigl::generated::CPACSPointXYZ)
%boost_optional(tigl::CCPACSPointListXYZ)

%include "CTiglPointsToBSplineInterpolation.h"
%include "CTiglInterpolateCurveNetwork.h"
%include "CTiglCurvesToSurface.h"
%include "tiglcommonfunctions.h"
%include "CTiglProjectOnLinearSpline.h"
%include "CTiglMakeLoft.h"
%include "CTiglArcLengthReparameterization.h"
%include "CPointsToLinearBSpline.h"
%include "CTiglBSplineApproxInterp.h"
%include "CTiglBSplineFit.h"
%include "CTiglBSplineAlgorithms.h"
%include "generated/CPACSPointX.h"
%include "ECPACSTranslationType.h"
%include "generated/CPACSPoint.h"
%include "generated/CPACSPointAbsRel.h"
%include "CCPACSPoint.h"
%include "CCPACSPointAbsRel.h"
%include "ECPACSTranslationType.h"
%include "CTiglTransformation.h"

// we want to replace Shape with another Shape function that returns by value
// as returning by ref can cause crashes in python
%extend CNamedShape {
    TopoDS_Shape Shape() const {
        return self->Shape();
    }
}
%ignore CNamedShape::Shape;
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

