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
#include "CCPACSGuideCurve.h"

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

void CCPACSFuselageProfileGetPointAlgo::GetPointTangent(const double& alpha, gp_Pnt& point,
                                                        gp_Vec& tangent, const CCPACSGuideCurve::FromOrToDefinition& fromOrToDefinition)
{
    Standard_Real umin, umax;
    TopoDS_Edge edge;
    if (fromOrToDefinition == CCPACSGuideCurve::FromOrToDefinition::PARAMETER) {
        // For some reasons, the choice of wire as shape to choose the parameter on, did not work.
        // <BRepAdaptor_CompCurve>.D1( alpha, point, tangent) based on a wire did not produce the expected result.
        // That is why, the first edge is extracted. It is implicitely assumed that the wire consists of exactly one edge.
        // More than one edge might lead to unexpected result when used in combination with PARAMETER
        if (GetNumberOfEdges(wire) > 1) {
            LOG(WARNING) << "CCPACSFuselageProfileGetPointAlgo::GetPointTangent: Defining start or end point of guide curve via parameter on wires consisting of 2 or more edges might lead to unexpected results.";
        }
        // Get parameter range of edge
        edge = GetEdge(wire, 0);
        BRep_Tool::Range(edge, umin, umax);
    }
    else if (fromOrToDefinition == CCPACSGuideCurve::FromOrToDefinition::CIRCUMFERENCE || fromOrToDefinition == CCPACSGuideCurve::FromOrToDefinition::UID) {
        umin = 0.;
        umax = 1.;
    }
    else {
        throw CTiglError("CCPACSFuselageProfileGetPointAlgo::GetPointTangent(): Either a from/toCircumference, a from/toParameter or a from/toGuideCurveUID must be present", TIGL_NOT_FOUND);
    }

    // alpha<umin : use line in the direction of the tangent at alpha=umin
    if (alpha<umin) {
        // get startpoint
        gp_Pnt startpoint;
        WireGetPointTangent(wire, 0.0, startpoint, tangent);
        // length of tangent has to be equal two the length of the profile curve
        tangent = wireLength * tangent/tangent.Magnitude();
        // get direction vector
        gp_Dir dir(-1.0*tangent);
        // construct line
        Geom_Line line(startpoint, dir);
        // map [-infinity, umin] to [umin, infinity] and scale by profile length
        Standard_Real zeta = wireLength*(-1.0*alpha);
        // get point on line at distance zeta from the start point
        line.D0(zeta, point);
    }
    else if (alpha>=umin && alpha<=umax) {

        // When the alpha is based on the parameter (and not on the circumference), a different way of computing the point (and therefore tangent) is chosen
        if (fromOrToDefinition == CCPACSGuideCurve::FromOrToDefinition::PARAMETER) {
            EdgeGetPointTangentBasedOnParam(edge, alpha, point, tangent);
        }
        else {
            WireGetPointTangent(wire, alpha, point, tangent);
        }
        // length of tangent has to be equal two the length of the profile curve
        tangent = wireLength * tangent/tangent.Magnitude();
    }
    // alpha>umax : use line in the direction of the tangent at alpha=umax
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
        // map [umax, infinity] to [umin, infinity] and scale by profile length
        Standard_Real zeta = wireLength*(alpha - 1.0);
        // get point on line at distance zeta from the start point
        line.D0(zeta, point);
    }
}

} // end namespace tigl






