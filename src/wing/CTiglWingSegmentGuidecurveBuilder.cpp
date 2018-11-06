/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-08-21 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglWingSegmentGuidecurveBuilder.h"

#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CCPACSGuideCurveProfile.h"
#include "CCPACSConfiguration.h"
#include "CCPACSGuideCurveAlgo.h"
#include "CCPACSWingProfileGetPointAlgo.h"
#include "CTiglRelativelyPositionedComponent.h"

#include "tiglcommonfunctions.h"

namespace tigl
{

CTiglWingSegmentGuidecurveBuilder::CTiglWingSegmentGuidecurveBuilder(CCPACSWingSegment &segment)
    : m_segment(segment)
{
}

CTiglWingSegmentGuidecurveBuilder::~CTiglWingSegmentGuidecurveBuilder()
{
}

std::vector<gp_Pnt> CTiglWingSegmentGuidecurveBuilder::BuildGuideCurvePnts(const CCPACSGuideCurve * guideCurve) const
{
    assert(guideCurve);

    if (!m_segment.GetGuideCurves()) {
        throw CTiglError("No guide curves defined for segment \"" + m_segment.GetUID() + "\".");
    }

    const tigl::CTiglTransformation& wingTransform = m_segment.GetParentTransformation();

    // get upper and lower part of inner profile in world coordinates
    CTiglWingConnection& innerConnection = m_segment.GetInnerConnection();
    CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
    TopoDS_Edge upperInnerWire = TopoDS::Edge(transformWingProfileGeometry(wingTransform, innerConnection, innerProfile.GetUpperWire()));
    TopoDS_Edge lowerInnerWire = TopoDS::Edge(transformWingProfileGeometry(wingTransform, innerConnection, innerProfile.GetLowerWire()));

    // get upper and lower part of outer profile in world coordinates
    CTiglWingConnection& outerConnection = m_segment.GetOuterConnection();
    CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
    TopoDS_Edge upperOuterWire = TopoDS::Edge(transformWingProfileGeometry(wingTransform, outerConnection, outerProfile.GetUpperWire()));
    TopoDS_Edge lowerOuterWire = TopoDS::Edge(transformWingProfileGeometry(wingTransform, outerConnection, outerProfile.GetLowerWire()));

    // concatenate inner profile wires for guide curve construction algorithm
    TopTools_SequenceOfShape concatenatedInnerWires;
    concatenatedInnerWires.Append(lowerInnerWire);
    concatenatedInnerWires.Append(upperInnerWire);

    // concatenate outer profile wires for guide curve construction algorithm
    TopTools_SequenceOfShape concatenatedOuterWires;
    concatenatedOuterWires.Append(lowerOuterWire);
    concatenatedOuterWires.Append(upperOuterWire);

    // get chord lengths for inner profile in word coordinates
    TopoDS_Wire innerChordLineWire = TopoDS::Wire(transformWingProfileGeometry(wingTransform, innerConnection, innerProfile.GetChordLineWire()));
    TopoDS_Wire outerChordLineWire = TopoDS::Wire(transformWingProfileGeometry(wingTransform, outerConnection, outerProfile.GetChordLineWire()));
    double innerScale = GetLength(innerChordLineWire);
    double outerScale = GetLength(outerChordLineWire);


    double fromRelativeCircumference;
    // check if fromRelativeCircumference is given in the current guide curve
    if (guideCurve->GetFromRelativeCircumference_choice2()) {
        fromRelativeCircumference = *guideCurve->GetFromRelativeCircumference_choice2();
    }
    // otherwise get relative circumference from neighboring segment guide curve
    else {
        // get neighboring guide curve UID
        std::string neighborGuideCurveUID = *guideCurve->GetFromGuideCurveUID_choice1();
        // get neighboring guide curve
        const CCPACSGuideCurve& neighborGuideCurve = m_segment.GetGuideCurves()->GetGuideCurve(neighborGuideCurveUID);
        // get relative circumference from neighboring guide curve
        fromRelativeCircumference = neighborGuideCurve.GetToRelativeCircumference();
    }

    // get relative circumference of outer profile
    double toRelativeCircumference = guideCurve->GetToRelativeCircumference();
    // get guide curve profile UID
    std::string guideCurveProfileUID = guideCurve->GetGuideCurveProfileUID();
    // get relative circumference of inner profile

    // get guide curve profile
    CCPACSGuideCurveProfile& guideCurveProfile = m_segment.GetUIDManager().ResolveObject<CCPACSGuideCurveProfile>(guideCurveProfileUID);

    // get local x-direction for the guide curve
    gp_Dir rxDir = gp_Dir(1., 0., 0.);
    if (guideCurve->GetRXDirection()) {
        rxDir.SetX(guideCurve->GetRXDirection()->GetX());
        rxDir.SetY(guideCurve->GetRXDirection()->GetY());
        rxDir.SetZ(guideCurve->GetRXDirection()->GetZ());
    }

    // construct guide curve algorithm
    std::vector<gp_Pnt> guideCurvePnts = CCPACSGuideCurveAlgo<CCPACSWingProfileGetPointAlgo> (concatenatedInnerWires,
                                                                                              concatenatedOuterWires,
                                                                                              fromRelativeCircumference,
                                                                                              toRelativeCircumference,
                                                                                              innerScale,
                                                                                              outerScale,
                                                                                              rxDir,
                                                                                              guideCurveProfile);
    return guideCurvePnts;
}

} // namespace tigl
