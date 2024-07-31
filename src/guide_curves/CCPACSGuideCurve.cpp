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
#include "CCPACSGuideCurve.h"

#include <TopoDS_Edge.hxx>

namespace tigl
{

// Constructor
CCPACSGuideCurve::CCPACSGuideCurve(CCPACSGuideCurves* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSGuideCurve(parent, uidMgr)
    , guideCurveTopo(*this, &CCPACSGuideCurve::BuildCurve)
{
    Cleanup();
}

// Destructor
CCPACSGuideCurve::~CCPACSGuideCurve(void)
{
    Cleanup();
}

void CCPACSGuideCurve::InvalidateImpl(const boost::optional<std::string>& source) const
{
    guideCurveTopo.clear();
}

CCPACSGuideCurve::FromOrToDefinition CCPACSGuideCurve::GetFromDefinition() const {
    if (!ValidateChoices()) {
        throw CTiglError("Choices not valid", TIGL_XML_ERROR);
    }
    if (m_fromRelativeCircumference_choice2_1) {
        return FromOrToDefinition::CIRCUMFERENCE;
    }
    if (m_fromParameter_choice2_2) {
        return FromOrToDefinition::PARAMETER;
    }
    if (m_fromGuideCurveUID_choice1) {
        return FromOrToDefinition::UID;
    }
    throw CTiglError("Logic error in GetFromDefinition detection");
}

CCPACSGuideCurve::FromOrToDefinition CCPACSGuideCurve::GetToDefinition() const {
    if (!ValidateChoices()) {
        throw CTiglError("Choices not valid", TIGL_XML_ERROR);
    }
    if (m_toRelativeCircumference_choice1) {
        return FromOrToDefinition::CIRCUMFERENCE;
    }
    if (m_toParameter_choice2) {
        return FromOrToDefinition::PARAMETER;
    }
    throw CTiglError("Logic error in GetToDefinition detection");
}

double CCPACSGuideCurve::GetFromDefinitionValue() const
{
    if ( GetFromGuideCurveUID_choice1() ) {
        CCPACSGuideCurve& pred = m_uidMgr->ResolveObject<CCPACSGuideCurve>(*GetFromGuideCurveUID_choice1());
        return pred.GetToDefinitionValue();
    }
    else if (GetFromRelativeCircumference_choice2_1()) {
        return *GetFromRelativeCircumference_choice2_1();
    }
    else if (GetFromParameter_choice2_2()) {
        return *GetFromParameter_choice2_2();
    }
    else {
        throw CTiglError("CCPACSGuideCurve::GetFromDefinitionValue(): Either a fromRelativeCircumference, a fromParameter or a fromGuideCurveUID must be present", TIGL_NOT_FOUND);
    }
}

double CCPACSGuideCurve::GetToDefinitionValue() const
{
    if (GetToRelativeCircumference_choice1()) {
        return *GetToRelativeCircumference_choice1();
    }
    else if (GetToParameter_choice2()) {
        return *GetToParameter_choice2();
    }
    else {
        throw CTiglError("CCPACSGuideCurve::GetToDefinitionValue(): Either a toRelativeCircumference or a toParameter must be present", TIGL_NOT_FOUND);
    }
}

// Cleanup routine
void CCPACSGuideCurve::Cleanup(void)
{
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

CCPACSGuideCurve* CCPACSGuideCurve::GetConnectedCurve() const
{
    std::vector<CCPACSGuideCurve*> curves = m_uidMgr->ResolveObjects<CCPACSGuideCurve>();
    for (std::size_t i = 0; i < curves.size(); i++) {
        if (curves[i]->GetFromGuideCurveUID_choice1() == m_uID) {
            return curves[i];
        }
    }
    return NULL;
}

CCPACSGuideCurve const* CCPACSGuideCurve::GetRootCurve() const
{
    if ( GetFromGuideCurveUID_choice1() ) {
        CCPACSGuideCurve& pred = m_uidMgr->ResolveObject<CCPACSGuideCurve>(*GetFromGuideCurveUID_choice1());
        return pred.GetRootCurve();
    }
    else {
        if( ! (GetFromRelativeCircumference_choice2_1() || GetFromParameter_choice2_2()) ) {
            throw CTiglError("CCPACSGuideCurve::GetRootCurve(): Either a fromCircumference, a fromParameter or a fromGuideCurveUID must be present", TIGL_NOT_FOUND);
        } else {
            return this;
        }
    }
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

IGuideCurveBuilder::~IGuideCurveBuilder()
{
}

} // end namespace tigl
