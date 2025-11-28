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

#include "ControlSurfaceDeviceHelper.h"
#include "tiglcommonfunctions.h"
#include "tiglmathfunctions.h"
#include "tigletaxsifunctions.h"
#include "Debugging.h"

#include <BRepBuilderAPI_Transform.hxx>
#include "CNamedShape.h"

namespace tigl
{

ControlSurfaceDeviceHelper::ControlSurfaceDeviceHelper()
{
}

PNamedShape ControlSurfaceDeviceHelper::GetTransformedFlapShape_helper(PNamedShape deviceShape, gp_Trsf T) const
{
    BRepBuilderAPI_Transform form(deviceShape->Shape(), T);
    deviceShape->SetShape(form.Shape());

    // store the transformation property. Required e.g. for VTK metadata
    gp_GTrsf gT(T);
    CTiglTransformation tiglTrafo(gT);
    unsigned int nFaces = deviceShape->GetFaceCount();
    for (unsigned int iFace = 0; iFace < nFaces; ++iFace) {
        CFaceTraits ft = deviceShape->GetFaceTraits(iFace);
        ft.SetTransformation(tiglTrafo);
        deviceShape->SetFaceTraits(iFace, ft);
    }

    return deviceShape;
}

gp_Vec ControlSurfaceDeviceHelper::GetNormalOfControlSurfaceDevice_helper(const CCPACSWingComponentSegment& compSeg)
{
    gp_Pnt point1                             = compSeg.GetPoint(0, 0);
    gp_Pnt point2                             = compSeg.GetPoint(0, 1);
    gp_Pnt point3                             = compSeg.GetPoint(1, 0);

    gp_Vec dir1to2 = -(gp_Vec(point1.XYZ()) - gp_Vec(point2.XYZ()));
    gp_Vec dir1to3 = -(gp_Vec(point1.XYZ()) - gp_Vec(point3.XYZ()));

    gp_Vec nvV = dir1to2 ^ dir1to3;
    nvV.Normalize();
    return nvV;
}

PNamedShape ControlSurfaceDeviceHelper::outerShapeGetLoft_helper(PNamedShape shapeBox, PNamedShape outerShape, std::string const& uid) const
{
        for (int iFace = 0; iFace < static_cast<int>(outerShape->GetFaceCount()); ++iFace) {
            CFaceTraits ft = outerShape->GetFaceTraits(iFace);
            ft.SetOrigin(shapeBox);
            outerShape->SetFaceTraits(iFace, ft);
        }

#ifdef DEBUG
        DEBUG_SCOPE(debug);
        debug.dumpShape(outerShape->Shape(), _uid);
#endif

        return outerShape;
}

gp_Pnt ControlSurfaceDeviceHelper::calc_hinge_point(
        CCPACSControlSurfaceHingePoint const& hinge_point, 
        CCPACSEtaIsoLine const& etaLE,
        CCPACSWingComponentSegment const& cSegment,
        CTiglUIDManager const* uidMgr,
        std::string const& innerOuter) 
        {

        CTiglWingStructureReference wsr(cSegment);

        double hingeXsi = hinge_point.GetHingeXsi();
        double relHingeHeight = hinge_point.GetHingeRelHeight();
        if (relHingeHeight < 0. || 1. < relHingeHeight) {
            LOG(ERROR) << "The " << innerOuter << "HingeHeight is not between 0. and 1.";
            throw CTiglError("The " + innerOuter +
                             "HingeHeight is not between 0. and 1. in CCPACSLeadingEdgeDevice::buildHingePoints.");
        }
        if (hingeXsi < 0. || 1. < hingeXsi) {
            LOG(ERROR) << "The " << innerOuter << "HingeXsi is not between 0. and 1.";
            throw CTiglError("The " + innerOuter +
                             "HingeXsi is not between 0. and 1. in CCPACSLeadingEdgeDevice::buildHingePoints.");
        }

        // create the hinge line point and normal on the Wing component segment mid plane
        gp_Pnt myHingePoint =
            cSegment.GetPoint(transformEtaToCSOrTed(etaLE, *uidMgr), hingeXsi, WING_COORDINATE_SYSTEM);
        gp_Vec midplaneNormal = cSegment.GetMidplaneNormal(transformEtaToCSOrTed(etaLE, *uidMgr));

        // Project point on the inner and outer face of the Wing loft
        gp_Pnt upper, lower;
        upper = ProjectPointOnShape(wsr.GetUpperShape(WING_COORDINATE_SYSTEM), myHingePoint, midplaneNormal);
        lower = ProjectPointOnShape(wsr.GetLowerShape(WING_COORDINATE_SYSTEM), myHingePoint, midplaneNormal);
        // then get delta through distance of these points multiplied by rel hinge height
        gp_Vec delta(lower, upper);
        delta *= relHingeHeight;
        lower.Translate(delta);

        // do we have to translate the point?
        auto translation = hinge_point.GetTranslation();

        if (translation) {
            lower = lower.XYZ() + translation.value().AsPoint().Get_gp_Pnt().XYZ();
        }
        return lower;
    };

}