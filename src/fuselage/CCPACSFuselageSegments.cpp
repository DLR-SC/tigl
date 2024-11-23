/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @file 
* @brief  Implementation of CPACS fuselage segments handling routines.
*/

#include "CCPACSFuselageSegments.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include "CCPACSFuselageSegment.h"
#include "CCPACSFuselage.h"
#include "CCPACSDuct.h"
#include "CCPACSVessel.h"
#include "CTiglError.h"
#include "sorting.h"
#include "CTiglLogging.h"
#include "CTiglBSplineAlgorithms.h"
#include "tiglcommonfunctions.h"
#include "CTiglCurveConnector.h"

namespace
{
    bool segment_follows(const std::unique_ptr<tigl::CCPACSFuselageSegment>& s2, const std::unique_ptr<tigl::CCPACSFuselageSegment>& s1)
    {
        if (!s2 || !s1) {
            return false;
        }

        return s2->GetFromElementUID() == s1->GetToElementUID();
    }
}

namespace tigl
{

CCPACSFuselageSegments::CCPACSFuselageSegments(CCPACSDuct* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSegments(parent, uidMgr)
    , guideCurves(*this, &CCPACSFuselageSegments::BuildGuideCurves)
{}

CCPACSFuselageSegments::CCPACSFuselageSegments(CCPACSFuselage* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSegments(parent, uidMgr)
    , guideCurves(*this, &CCPACSFuselageSegments::BuildGuideCurves)
{}

CCPACSFuselageSegments::CCPACSFuselageSegments(CCPACSVessel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSegments(parent, uidMgr)
    , guideCurves(*this, &CCPACSFuselageSegments::BuildGuideCurves)
{}

CCPACSConfiguration const& CCPACSFuselageSegments::GetConfiguration() const
{
    if (IsParent<CCPACSFuselage>()) {
        return GetParent<CCPACSFuselage>()->GetConfiguration();
    }
    else if (IsParent<CCPACSDuct>()) {
        return GetParent<CCPACSDuct>()->GetConfiguration();
    }
    else if (IsParent<CCPACSVessel>()) {
        return GetParent<CCPACSVessel>()->GetConfiguration();
    }
    else
    {
        throw CTiglError("CCPACSFuselageSegments: Unknown parent.");
    }
}

// Invalidates internal state
void CCPACSFuselageSegments::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 1; i <= GetSegmentCount(); i++) {
        GetSegment(i).Invalidate(source);
    }
    guideCurves.clear();
}

// Gets a segment by index. 
CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(int index)
{
    return const_cast<CCPACSFuselageSegment&>(static_cast<const CCPACSFuselageSegments&>(*this).GetSegment(index));
}

const CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(int index) const
{
    index--;
    if (index < 0 || index >= GetSegmentCount()) {
        throw CTiglError("Invalid index value in CCPACSFuselageSegments::GetSegment", TIGL_INDEX_ERROR);
    }
    return *m_segments[index];
}

// Gets a segment by uid. 
CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(const std::string& segmentUID)
{
    for (std::size_t i = 0; i < m_segments.size(); i++) {
        if (m_segments[i]->GetUID() == segmentUID) {
            return *m_segments[i];
        }
    }
    throw CTiglError("Invalid uid in CCPACSFuselageSegments::GetSegment", TIGL_UID_ERROR);
}

// Gets total segment count
int CCPACSFuselageSegments::GetSegmentCount() const
{
    return static_cast<int>(m_segments.size());
}


CTiglRelativelyPositionedComponent const* CCPACSFuselageSegments::GetParentComponent() const
{
    if (IsParent<CCPACSFuselage>()) {
        return GetParent<CCPACSFuselage>();
    }
    else if (IsParent<CCPACSDuct>()) {
        return GetParent<CCPACSDuct>();
    }
    else if (IsParent<CCPACSVessel>()) {
        return GetParent<CCPACSVessel>();
    }
    else {
        throw CTiglError("Unknown parent type for CCPACSFuselageSegments.");
    }
}

void tigl::CCPACSFuselageSegments::ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath)
{
    tigl::generated::CPACSFuselageSegments::ReadCPACS(tixiHandle, xpath);

    if (GetSegmentCount() <= 0) {
        return;
    }

    // check order of segments - each segment must start with the element of the previous segment
    bool mustReorderSegments = false;
    std::string prevElementUID = GetSegment(1).GetToElementUID();
    for (int i = 2; i <= GetSegmentCount(); ++i) {
        CCPACSFuselageSegment& segment = GetSegment(i);
        if (prevElementUID != segment.GetFromElementUID()) {
            mustReorderSegments = true;
        }
        prevElementUID = segment.GetToElementUID();
    }

    if (mustReorderSegments) {
        LOG(WARNING) << "Fuselage segments in wrong order! Trying to reorder.";
        ReorderSegments();
    }

}

void CCPACSFuselageSegments::ReorderSegments()
{
    try {
        tigl::follow_sort(GetSegments().begin(), GetSegments().end(), segment_follows);
    } catch (std::invalid_argument) {
        throw CTiglError("Fuselage segments not continuous.");
    }
}

void CCPACSFuselageSegments::BuildGuideCurves(TopoDS_Compound& cache) const
{
    std::map<double, const CCPACSGuideCurve*> roots;

    // get section centers for the centripetal parametrization
    std::vector<gp_Pnt> sectionCenters(GetSegmentCount()+1);

    // get center of inner section of first segment
    const CCPACSFuselageSegment& innerSegment = GetSegment(1);
    sectionCenters[0] = innerSegment.GetTransformedProfileOriginStart();

    // find roots and connect the belonging guide curve segments
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        const CCPACSFuselageSegment& segment = GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        // get center of outer section
        sectionCenters[isegment] = segment.GetTransformedProfileOriginEnd();

        const CCPACSGuideCurves& segmentCurves = *segment.GetGuideCurves();
        for (int iguide = 1; iguide <=  segmentCurves.GetGuideCurveCount(); ++iguide) {
            const CCPACSGuideCurve& curve = segmentCurves.GetGuideCurve(iguide);
            if (!curve.GetFromGuideCurveUID_choice1()) {
                // this is a root curve
                double relCirc;
                if (curve.GetFromRelativeCircumference_choice2_1()) {
                    relCirc = *curve.GetFromRelativeCircumference_choice2_1();
                }
                else if(curve.GetFromParameter_choice2_2()) {
                    relCirc = *curve.GetFromParameter_choice2_2();
                }
                else {
                    throw CTiglError("CCPACSFuselageSegments::BuildGuideCurves(): Either a fromCircumference or a fromParameter must be present", TIGL_NOT_FOUND);
                }
                //TODO: determine if half fuselage or not. If not
                //the guide curve at relCirc=1 should be inserted at relCirc=0
                roots.insert(std::make_pair(relCirc, &curve));
            }
        }
    }

    // get the parameters at the section centers
    std::vector<double> sectionParams = CTiglBSplineAlgorithms::computeParamsBSplineCurve(OccArray(sectionCenters), 0., 1., 0.5);

    // connect guide curve segments to a spline with given continuity conditions and tangents
    CTiglCurveConnector connector(roots, sectionParams);
    cache = connector.GetConnectedGuideCurves();
}

const TopoDS_Compound &CCPACSFuselageSegments::GetGuideCurveWires() const
{
    return *guideCurves;
}

} // end namespace tigl
