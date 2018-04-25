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
#include "Geom_Line.hxx"
#include "TopExp_Explorer.hxx"
#include "TopoDS.hxx"
#include "tiglcommonfunctions.h"

namespace tigl
{

CCPACSFuselageProfileGetPointAlgo::CCPACSFuselageProfileGetPointAlgo (const TopTools_SequenceOfShape& wireContainer)
{
    try {
        if (wireContainer.Length()!=1) {
            throw CTiglError("CCPACSWingProfileGetPointAlgo: Number of wires is not equal 1", TIGL_ERROR);
        }
        wire = TopoDS::Wire(wireContainer(1));
    }
    catch(...) {
        throw CTiglError("CCPACSFuselageProfileGetPointAlgo: Conversion of shape to wire failed", TIGL_ERROR);
    }
    wireLength = GetLength(wire);
}

void CCPACSFuselageProfileGetPointAlgo::GetPointTangent(const double& alpha, gp_Pnt& point, gp_Vec& tangent)
{
    // alpha<0.0 : use line in the direction of the tangent at alpha=0.0
    if (alpha<0.0) {
        // get startpoint
        gp_Pnt startpoint;
        WireGetPointTangent(wire, 0.0, startpoint, tangent);
        // length of tangent has to be equal two the length of the profile curve
        tangent = wireLength * tangent/tangent.Magnitude();
        // get direction vector
        gp_Dir dir(-1.0*tangent);
        // construct line
        Geom_Line line(startpoint, dir);
        // map [-infinity, 0.0] to [0.0, infinity] and scale by profile length
        Standard_Real zeta = wireLength*(-1.0*alpha);
        // get point on line at distance zeta from the start point
        line.D0(zeta, point);
    }
    else if (alpha>=0.0 && alpha<=1.0) {
        WireGetPointTangent(wire, alpha, point, tangent);
        // length of tangent has to be equal two the length of the profile curve
        tangent = wireLength * tangent/tangent.Magnitude();
    }
    // alpha>1.0 : use line in the direction of the tangent at alpha=1.0
    else {
        // get startpoint
        gp_Pnt startpoint;
        WireGetPointTangent(wire, 1.0, startpoint, tangent);
        // length of tangent has to be equal two the length of the profile curve
        tangent = wireLength * tangent/tangent.Magnitude();
        // get direction vector
        gp_Dir dir(tangent);
        // construct line
        Geom_Line line(startpoint, dir);
        // map [1.0, infinity] to [0.0, infinity] and scale by profile length
        Standard_Real zeta = wireLength*(alpha - 1.0);
        // get point on line at distance zeta from the start point
        line.D0(zeta, point);
    }
}

} // end namespace tigl






