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
 * @file   CCPACSWingProfileGetPointAlgo.cpp
 * @brief  Get point and tangent depending on the parameter on a fuselage guide curves
 *
 * This class takes the concatenated lower and upper wing profile wire (in this order)
 * and provides the point and the tangent for a given parameter alpha along the curve.
 * The curve starts at the trailing edge (alpha=-1), runs through the upper fuselage profile towards
 * the leading edge (alpha=0) and returns to the trailing edge (alpha=1) via the lower fuselage profile
 */

#include "tigl_internal.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CCPACSWingProfileGetPointAlgo.h"
#include "BRepTools_WireExplorer.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "TopExp_Explorer.hxx"
#include "TopoDS.hxx"
#include "Geom_Line.hxx"
#include "tiglcommonfunctions.h"

namespace tigl
{

CCPACSWingProfileGetPointAlgo::CCPACSWingProfileGetPointAlgo (const TopTools_SequenceOfShape& wireContainer)
{
    try {
        if (wireContainer.Length()!=2) {
            throw CTiglError("CCPACSWingProfileGetPointAlgo: Number of wires is not equal 2", TIGL_ERROR);
        }
        lowerWire = TopoDS::Edge(wireContainer(1));
        upperWire = TopoDS::Edge(wireContainer(2));
    }
    catch(...) {
        throw CTiglError("CCPACSWingProfileGetPointAlgo: Separation of upper and lower profiles failed", TIGL_ERROR);
    }

    lowerWireLength = GetLength(lowerWire);
    upperWireLength = GetLength(upperWire);
}

void CCPACSWingProfileGetPointAlgo::GetPointTangent(const double& alpha, gp_Pnt& point, gp_Vec& tangent)
{
    // alpha<-1.0 : use line in the direction of the tangent at alpha=-1.0
    if (alpha<-1.0) {
        // get startpoint
        gp_Pnt startpoint;
        EdgeGetPointTangent(lowerWire, 0.0, startpoint, tangent);
        // length of tangent has to be equal two the length of the lower profile curve
        tangent = lowerWireLength * tangent/tangent.Magnitude();
        // get direction vector
        gp_Dir dir(-1.0*tangent);
        // construct line
        Geom_Line line(startpoint, dir);
        // map [-infinity, -1.0] to [0.0, infinity] and scale by profile length
        Standard_Real zeta = lowerWireLength*(-alpha - 1.0);
        // get point on line at distance zeta from the start point
        line.D0(zeta, point);
    }
    else if (alpha>=-1.0 && alpha<=0.0) {
        // mapping [-1,0] to [0,1]
        double zeta = alpha + 1.0;
        EdgeGetPointTangent(lowerWire, zeta, point, tangent);
        // length of tangent has to be equal two the length of the lower profile curve
        tangent = lowerWireLength * tangent/tangent.Magnitude();
    }
    else if (alpha>0.0 && alpha<=1.0) {
        EdgeGetPointTangent(upperWire, alpha, point, tangent);
        // length of tangent has to be equal two the length of the upper profile curve
        tangent = upperWireLength * tangent/tangent.Magnitude();
    }
    // alpha>1.0 : use line in the direction of the tangent at alpha=1.0
    else {
        // get startpoint
        gp_Pnt startpoint;
        EdgeGetPointTangent(upperWire, 1.0, startpoint, tangent);
        // length of tangent has to be equal two the length of the upper profile curve
        tangent = upperWireLength * tangent/tangent.Magnitude();
        // get direction vector
        gp_Dir dir(tangent);
        // construct line
        Geom_Line line(startpoint, dir);
        // map [1.0, infinity] to [0.0, infinity] and scale by the upper profile length
        Standard_Real zeta = upperWireLength*(alpha - 1.0);
        // get point on line at distance zeta from the start point
        line.D0(zeta, point);
    }
}

} // end namespace tigl






