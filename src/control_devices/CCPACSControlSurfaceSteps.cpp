/*
* Copyright (C) 2025 German Aerospace Center (DLR/SC)
*
* Created: 2025 Ole Albers <ole.albers@dlr.de>
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

#include "CCPACSControlSurfaceSteps.h"
#include "generated/CPACSControlSurfacePath.h"
#include "CCPACSControlSurfaces.h"

#include "tiglmathfunctions.h"


namespace tigl
{

CCPACSControlSurfaceSteps::CCPACSControlSurfaceSteps(generated::CPACSControlSurfacePath* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaceSteps(parent, uidMgr)
{
}

gp_Trsf CCPACSControlSurfaceSteps::GetTransformation(double controlParameter, gp_Pnt innerHingeOld , gp_Pnt outerHingeOld) const
{
    // this block of code calculates all needed values to rotate and move the controlSurfaceDevice according
    // to the given controlParal by using a linearInterpolation.
    std::vector<double> controlParams, innerXTrans, outerXTrans, innerYTrans, innerZTrans, outerZTrans, rotations;

    for (const auto& step : GetSteps()) {
        controlParams.push_back(step->GetControlParameter());
        CTiglPoint innerHingeTrans(0, 0, 0);
        if (step->GetInnerHingeTranslation().is_initialized()) {
            innerHingeTrans = step->GetInnerHingeTranslation()->AsPoint();
        }
        innerXTrans.push_back(innerHingeTrans.x);
        innerYTrans.push_back(innerHingeTrans.y);
        innerZTrans.push_back(innerHingeTrans.z);

        CTiglPoint outerHingeTrans(0, 0, 0);
        if (step->GetOuterHingeTranslation().is_initialized()) {
            outerHingeTrans.x = step->GetOuterHingeTranslation()->GetX();
            outerHingeTrans.z = step->GetOuterHingeTranslation()->GetZ();
        }

        outerXTrans.push_back(outerHingeTrans.x);
        outerZTrans.push_back(outerHingeTrans.z);
        rotations.push_back(step->GetHingeLineRotation().value_or(0.));
    }

    double rotation          = Interpolate(controlParams, rotations, controlParameter);
    double innerTranslationX = Interpolate(controlParams, innerXTrans, controlParameter);
    double innerTranslationY = Interpolate(controlParams, innerYTrans, controlParameter);
    double innerTranslationZ = Interpolate(controlParams, innerZTrans, controlParameter);
    double outerTranslationX = Interpolate(controlParams, outerXTrans, controlParameter);
    double outerTranslationZ = Interpolate(controlParams, outerZTrans, controlParameter);

    // innerTranslationY on hingePoint1 on purpose, maybe consider setting it to zero as default. See CPACS definition on
    // Path/Step/HingeLineTransformation for more information.
    gp_Pnt outerHingeNew = outerHingeOld.XYZ() + gp_XYZ(outerTranslationX, innerTranslationY, outerTranslationZ);
    gp_Pnt innerHingeNew = innerHingeOld.XYZ() + gp_XYZ(innerTranslationX, innerTranslationY, innerTranslationZ);

    // calculating the needed transformations
    CTiglControlSurfaceTransformation transformation(innerHingeOld, outerHingeOld, innerHingeNew, outerHingeNew,
                                                     rotation);

    return transformation.getTotalTransformation();
}

}