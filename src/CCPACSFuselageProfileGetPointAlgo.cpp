/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-12 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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

/**
 * @file   CCPACSFuselageProfileGetPointAlgo.cpp
 * @brief  Get point and tangent depending on the parameter on a fuselage guide curves
 *
 * This class takes a fuselage profile wire and provides the point and the tangent
 * for a given parameter along the curve, as it is needed by
 * The curve starts at alpha=0 and ends at alpha=1
 */

#include "tigl_internal.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CCPACSFuselageProfileGetPointAlgo.h"
#include "BRepTools_WireExplorer.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "tiglcommonfunctions.h"

namespace tigl
{

CCPACSFuselageProfileGetPointAlgo::CCPACSFuselageProfileGetPointAlgo (TopoDS_Wire& w)
{
    wire = w;
}

void CCPACSFuselageProfileGetPointAlgo::GetPointTangent(const double& alpha, gp_Pnt& point, gp_Vec& tangent)
{
    if (alpha>=0.0 && alpha<=1.0) {
        WireGetPointTangent2(wire, alpha, point, tangent);
    }
    else
    {
        throw CTiglError("Error: CCPACSFuselageProfileGetPointAlgo::GetPoint: Parameter out of range [-1,1]", TIGL_ERROR);
    }
}

} // end namespace tigl

