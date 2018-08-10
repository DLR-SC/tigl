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

#include "CTiglFuselageSegmentGuidecurveBuilder.h"

#include "CCPACSFuselage.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSGuideCurveProfile.h"
#include "CCPACSConfiguration.h"
#include "CCPACSGuideCurveAlgo.h"
#include "CCPACSFuselageProfileGetPointAlgo.h"

#include "tiglcommonfunctions.h"

namespace tigl
{

CTiglFuselageSegmentGuidecurveBuilder::CTiglFuselageSegmentGuidecurveBuilder(CCPACSFuselageSegment &segment)
    : m_segment(segment)
{
}

CTiglFuselageSegmentGuidecurveBuilder::~CTiglFuselageSegmentGuidecurveBuilder()
{
}

std::vector<gp_Pnt> CTiglFuselageSegmentGuidecurveBuilder::BuildGuideCurvePnts(const CCPACSGuideCurve * guideCurve) const
{
    assert(guideCurve);

    // get start and end profile
    CTiglFuselageConnection& startConnection = m_segment.GetStartConnection();
    CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
    CTiglFuselageConnection& endConnection = m_segment.GetEndConnection();
    CCPACSFuselageProfile& endProfile   = endConnection.GetProfile();

    // get wire and close it if the profile is not mirror symmetric
    TopoDS_Wire startWire = startProfile.GetWire(!startProfile.GetMirrorSymmetry());
    TopoDS_Wire endWire   = endProfile.GetWire(!endProfile.GetMirrorSymmetry());

    // get profile wires in world coordinates
    startWire = TopoDS::Wire(transformFuselageProfileGeometry(m_segment.GetFuselage().GetTransformationMatrix(), startConnection, startWire));
    endWire   = TopoDS::Wire(transformFuselageProfileGeometry(m_segment.GetFuselage().GetTransformationMatrix(), endConnection, endWire));

    // put wires into container for guide curve algo
    TopTools_SequenceOfShape startWireContainer;
    startWireContainer.Append(startWire);
    TopTools_SequenceOfShape endWireContainer;
    endWireContainer.Append(endWire);

    // get chord lengths for inner profile in word coordinates
    TopoDS_Wire innerChordLineWire = TopoDS::Wire(transformFuselageProfileGeometry(m_segment.GetFuselage().GetTransformationMatrix(), startConnection, startProfile.GetDiameterWire()));
    TopoDS_Wire outerChordLineWire = TopoDS::Wire(transformFuselageProfileGeometry(m_segment.GetFuselage().GetTransformationMatrix(), endConnection, endProfile.GetDiameterWire()));
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
        const CCPACSGuideCurve& neighborGuideCurve = m_segment.GetFuselage().GetGuideCurveSegment(neighborGuideCurveUID);
        // get relative circumference from neighboring guide curve
        fromRelativeCircumference = neighborGuideCurve.GetToRelativeCircumference();
    }

    // get relative circumference of outer profile
    double toRelativeCircumference = guideCurve->GetToRelativeCircumference();
    // get guide curve profile UID
    std::string guideCurveProfileUID = guideCurve->GetGuideCurveProfileUID();

    // get guide curve profile
    CCPACSConfiguration& config = m_segment.GetFuselage().GetConfiguration();
    CCPACSGuideCurveProfile& guideCurveProfile = config.GetGuideCurveProfile(guideCurveProfileUID);

    // get local x-direction for the guide curve
    gp_Dir rxDir = gp_Dir(0., 0., 1.);
    if (guideCurve->GetRXDirection()) {
        rxDir.SetX(guideCurve->GetRXDirection()->GetX());
        rxDir.SetY(guideCurve->GetRXDirection()->GetY());
        rxDir.SetZ(guideCurve->GetRXDirection()->GetZ());
    }


    // construct guide curve algorithm
    std::vector<gp_Pnt> guideCurvePnts = CCPACSGuideCurveAlgo<CCPACSFuselageProfileGetPointAlgo> (startWireContainer,
                                                                                                  endWireContainer,
                                                                                                  fromRelativeCircumference,
                                                                                                  toRelativeCircumference,
                                                                                                  innerScale,
                                                                                                  outerScale,
                                                                                                  rxDir,
                                                                                                  guideCurveProfile);
    return guideCurvePnts;
}

} // namespace tigl
