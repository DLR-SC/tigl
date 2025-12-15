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

#pragma once 

#include "generated/CPACSControlSurfaceHingePoint.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSEtaIsoLine.h"
#include "CTiglUIDManager.h"

namespace tigl
{

namespace ControlSurfaceDeviceHelper
{
    TIGL_EXPORT PNamedShape GetTransformedFlapShape(PNamedShape deviceShape, gp_Trsf T);

    TIGL_EXPORT gp_Vec GetNormalOfControlSurfaceDevice(const CCPACSWingComponentSegment& cSegment);

    TIGL_EXPORT PNamedShape outerShapeGetLoft(PNamedShape shapeBox, PNamedShape outerShape, std::string const& uid);

    TIGL_EXPORT  gp_Pnt calc_hinge_point(CCPACSControlSurfaceHingePoint const& hinge_point, CCPACSEtaIsoLine const& etaLE, CCPACSWingComponentSegment const& cSegment, CTiglUIDManager const* uidMgr, std::string const& innerOuter);

}
}