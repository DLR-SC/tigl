/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-19 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @file
* @brief  Implementation of a CPACS guide curve
*/

#include "tigl.h"
#include "CCPACSGuideCurve.h"
#include "CTiglError.h"
#include "CTiglPoint.h"
#include "CTiglLogging.h"
#include "tiglcommonfunctions.h"

#include <TopoDS_Edge.hxx>

namespace tigl
{

// Constructor
CCPACSGuideCurve::CCPACSGuideCurve(CTiglUIDManager* uidMgr)
    : generated::CPACSGuideCurve(uidMgr)
    , guideCurveTopo(*this, &CCPACSGuideCurve::BuildCurve)
{
    Cleanup();
}

// Destructor
CCPACSGuideCurve::~CCPACSGuideCurve(void)
{
    Cleanup();
}

CCPACSGuideCurve::FromDefinition CCPACSGuideCurve::GetFromDefinition() const {
    if (!ValidateChoices())
        throw CTiglError("Choices not valid", TIGL_XML_ERROR);
    if (m_fromRelativeCircumference_choice2)
        return FromDefinition::CIRCUMFERENCE;
    if (m_fromGuideCurveUID_choice1)
        return FromDefinition::UID;
    throw CTiglError("Logic error in FromDefinition detection");
}

// Cleanup routine
void CCPACSGuideCurve::Cleanup(void)
{
    //nextGuideSegment = NULL;
    guideCurveTopo.clear();
    m_builder = NULL;
}

TopoDS_Edge CCPACSGuideCurve::GetCurve() const
{
    return *guideCurveTopo;
}

std::vector<gp_Pnt> CCPACSGuideCurve::GetCurvePoints() const
{
    if (!m_builder) {
        throw CTiglError("Cannot get Guide Curve Points: Null pointer to guide curve builder", TIGL_NULL_POINTER);
    }
    std::vector<gp_Pnt> guideCurvePnts = m_builder->BuildGuideCurvePnts(this);
    return guideCurvePnts;
}

//void CCPACSGuideCurve::ConnectToCurve(CCPACSGuideCurve *guide)
//{
//    if (!guide) {
//        throw CTiglError("Null pointer guide curve in CCPACSGuideCurve::ConnectToCurve", TIGL_ERROR);
//    }
//    
//    if (guide->GetFromGuideCurveUID_choice1() != m_uID) {
//        throw CTiglError("Guide curves cannot be connected. Mismatching uids.", TIGL_ERROR);
//    }
//    
//    nextGuideSegment = guide;
//}

CCPACSGuideCurve* CCPACSGuideCurve::GetConnectedCurve() const
{
    auto curves = m_uidMgr->ResolveObjects<CCPACSGuideCurve>();
    for (auto& c : curves)
        if (c->GetFromGuideCurveUID_choice1() == m_uID)
            return c;
    return nullptr;
}

void CCPACSGuideCurve::SetGuideCurveBuilder(IGuideCurveBuilder& b)
{
    m_builder = &b;
}

void CCPACSGuideCurve::BuildCurve(TopoDS_Edge& cache) const
{
    if (m_builder) {
        // interpolate B-Spline curve through guide curve points
        cache = EdgeSplineFromPoints(GetCurvePoints());
    }
}

} // end namespace tigl
