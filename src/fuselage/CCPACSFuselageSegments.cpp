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
#include "CTiglUIDManager.h"
#include "CCPACSFuselageSectionElement.h"

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

    if (NeedReordering()) {
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

bool CCPACSFuselageSegments::NeedReordering() const
{
    if ( GetSegmentCount() <= 1 ) {
        return false;
    }

    bool mustReorderSegments = false;
    std::string prevElementUID = GetSegment(1).GetToElementUID();
    for (int i = 2; i <= GetSegmentCount(); ++i) {
        const CCPACSFuselageSegment& segment = GetSegment(i);
        if (prevElementUID != segment.GetFromElementUID()) {
            mustReorderSegments = true;
        }
        prevElementUID = segment.GetToElementUID();
    }
    return mustReorderSegments;
}

std::vector<std::string> CCPACSFuselageSegments::GetElementUIDsInOrder() const
{
    std::vector<std::string> elementUIDs;
    std::string tempStartUID;
    std::string tempEndUID;
    for (int i = 0; i < m_segments.size(); i++) {
        tempStartUID = m_segments[i]->GetFromElementUID();
        tempEndUID   = m_segments[i]->GetToElementUID();
        if (std::find(elementUIDs.begin(), elementUIDs.end(), tempStartUID) == elementUIDs.end()) {
            elementUIDs.push_back(tempStartUID);
        }
        if (std::find(elementUIDs.begin(), elementUIDs.end(), tempEndUID) == elementUIDs.end()) {
            elementUIDs.push_back(tempEndUID);
        }
    }
    return elementUIDs;
}


CCPACSFuselageSegment& CCPACSFuselageSegments::SplitSegment(const std::string& segmentUID, const std::string& splitterElementUID)
{
    CCPACSFuselageSegment& segment = this->GetSegment(segmentUID);
    CCPACSFuselageSectionElement& splitterElement = GetUIDManager().ResolveObject<CCPACSFuselageSectionElement>(splitterElementUID);

    // create a additional segment
    CCPACSFuselageSegment& additionalSegment =  this->AddSegment();
    std::string additionalSegmentUID = GetUIDManager().MakeUIDUnique(segment.GetUID() + "bis" );
    additionalSegment.SetUID(additionalSegmentUID);
    additionalSegment.SetName(additionalSegmentUID);

    // set the segment from splitter element to the old end
    additionalSegment.SetFromElementUID(splitterElement.GetUID());
    additionalSegment.SetToElementUID(segment.GetToElementUID());

    // reset the old segment to end at the splitter
    segment.SetToElementUID(splitterElement.GetUID());

    Invalidate();
    // Reordering is necessary: After splitting a segment and adding another one, the order is not correct
    // Without reordering, this results in a wrong shape visible in the TiGLViewer
    ReorderSegments();

    return additionalSegment;
}

CCPACSFuselageSegment& CCPACSFuselageSegments::GetSegmentFromTo(const std::string &fromElementUID,
                                                                const std::string toElementUID)
{

    for (int i = 0; i < m_segments.size(); i++) {
        if ( m_segments[i]->GetFromElementUID() == fromElementUID && m_segments[i]->GetToElementUID() == toElementUID ) {
            return GetSegment(i+1);
        }

    }

    throw  CTiglError("Segment with the given from and to UID not found", TIGL_UID_ERROR);

}

} // end namespace tigl
