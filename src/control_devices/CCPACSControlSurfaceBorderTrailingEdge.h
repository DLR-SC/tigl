#pragma once
/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018 Martin Siggel <Martin.Siggel@dlr.de>
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


#include "generated/CPACSControlSurfaceBorderTrailingEdge.h"
#include <TopoDS_Wire.hxx>
#include "PNamedShape.h"
#include "CTiglControlSurfaceBorderCoordinateSystem.h"
#include "CCPACSWingComponentSegment.h"

namespace tigl
{

class CCPACSControlSurfaceBorderTrailingEdge : public generated::CPACSControlSurfaceBorderTrailingEdge
{
public:
    TIGL_EXPORT CCPACSControlSurfaceBorderTrailingEdge(CCPACSControlSurfaceOuterShapeTrailingEdge* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT TopoDS_Wire GetWire(PNamedShape wingShape, gp_Vec upDir) const;

    TIGL_EXPORT CTiglControlSurfaceBorderCoordinateSystem GetCoordinateSystem(gp_Vec upDir) const;

private:


};

} // namespace tigl
