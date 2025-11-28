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

#include  "generated/CPACSControlSurfaceStep.h"
#include  "generated/CPACSControlSurfaceSteps.h"

#include "CTiglControlSurfaceTransformation.h"
#include  "CCPACSWing.h"
#include  "tigl.h"

namespace tigl
{

class CPACSControlSurfacePath;

class CCPACSControlSurfaceSteps : public generated::CPACSControlSurfaceSteps
{
public:
    TIGL_EXPORT CCPACSControlSurfaceSteps(generated::CPACSControlSurfacePath* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT gp_Trsf GetTransformation(double controlParameter, gp_Pnt innerHingeOld , gp_Pnt outerHingeOld) const;

};
}
