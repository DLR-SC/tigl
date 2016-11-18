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

namespace tigl
{

// Constructor
CCPACSGuideCurve::CCPACSGuideCurve()
{
    Cleanup();
}

// Destructor
CCPACSGuideCurve::~CCPACSGuideCurve()
{
    Cleanup();
}

// Cleanup routine
void CCPACSGuideCurve::Cleanup()
{
    m_name       = "";
    m_uID        = "";
    m_description= "";
    m_fromGuideCurveUID_choice1.destroy();
    m_fromRelativeCircumference_choice2.destroy();
    fromRelativeCircumferenceIsSet = true;
}

// Read guide curve file
void CCPACSGuideCurve::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    Cleanup();
    generated::CPACSGuideCurve::ReadCPACS(tixiHandle, xpath);

    if (HasFromRelativeCircumference_choice2() && HasFromGuideCurveUID_choice1()) {
        throw CTiglError("Error: It is forbidden to give fromRelativeCircumference AND fromGuideCurveUID in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    } else if (HasFromRelativeCircumference_choice2()) {
        fromRelativeCircumferenceIsSet = true;
    } else if (HasFromGuideCurveUID_choice1()) {
        fromRelativeCircumferenceIsSet = false;
    } else {
        throw CTiglError("Error: Attribute fromRelativeCircumference OR fromGuideCurveUID missing in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
}

// Check if fromRelativeCircumference is set
bool CCPACSGuideCurve::GetFromRelativeCircumferenceIsSet() const
{
    return fromRelativeCircumferenceIsSet;
}

// Returns the relative circumference of the starting profile
double CCPACSGuideCurve::GetFromRelativeCircumference() const
{
    return std::stod(*m_fromRelativeCircumference_choice2);
}

// Returns the relative circumference of the end profile
double CCPACSGuideCurve::GetToRelativeCircumference() const
{
    return std::stod(m_toRelativeCircumference);
}

const std::string& CCPACSGuideCurve::GetFromGuideCurveUID() const {
    return GetFromGuideCurveUID_choice1();
}

} // end namespace tigl
