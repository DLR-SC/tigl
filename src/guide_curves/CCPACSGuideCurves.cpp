/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS guide curve container handling routines
*/

#include "CCPACSGuideCurves.h"
#include "CCPACSGuideCurve.h"


namespace tigl
{

CCPACSGuideCurves::CCPACSGuideCurves(CCPACSFuselageSegment* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSGuideCurves(parent, uidMgr) {}

CCPACSGuideCurves::CCPACSGuideCurves(CCPACSWingSegment* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSGuideCurves(parent, uidMgr) {}


void CCPACSGuideCurves::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {
    generated::CPACSGuideCurves::ReadCPACS(tixiHandle, xpath);

    // sort by uid as some unit tests rely on this (TODO: should we fix the tests?)
    // WARN: this destroys the order of the guide curves as stored in the CPACS file
    std::sort(m_guideCurves.begin(), m_guideCurves.end(), [](const std::unique_ptr<CCPACSGuideCurve>& a, const std::unique_ptr<CCPACSGuideCurve>& b) {
        return a->GetUID() < b->GetUID();
    });
}

// Returns the total count of guide curves in this configuration
int CCPACSGuideCurves::GetGuideCurveCount() const
{
    return static_cast<int>(m_guideCurves.size());
}

// Returns the guide curve for a given index
const CCPACSGuideCurve& CCPACSGuideCurves::GetGuideCurve(int index) const
{
    index--;
    if (index < 0 || index >= GetGuideCurveCount()) {
        throw CTiglError("Invalid index in CCPACSGuideCurves::GetGuideCurve", TIGL_INDEX_ERROR);
    }
    return *m_guideCurves[index];
}

// Returns the guide curve for a given index
CCPACSGuideCurve& CCPACSGuideCurves::GetGuideCurve(int index)
{
    return const_cast<CCPACSGuideCurve&>(static_cast<const CCPACSGuideCurves*>(this)->GetGuideCurve(index));
}

// Returns the guide curve for a given uid.
const CCPACSGuideCurve& CCPACSGuideCurves::GetGuideCurve(std::string uid) const
{
    for (std::size_t i = 0; i < m_guideCurves.size(); i++) {
        if (m_guideCurves[i]->GetUID() == uid) {
            return *m_guideCurves[i];
        }
    }
    throw CTiglError("CCPACSGuideCurve::GetGuideCurve: Guide curve \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

// Returns the guide curve for a given uid.
CCPACSGuideCurve& CCPACSGuideCurves::GetGuideCurve(std::string uid)
{
    return const_cast<CCPACSGuideCurve&>(static_cast<const CCPACSGuideCurves*>(this)->GetGuideCurve(uid));
}

// Returns the guide curve for a given uid.
bool CCPACSGuideCurves::GuideCurveExists(std::string uid) const
{
    for (std::size_t i = 0; i < m_guideCurves.size(); i++) {
        if (m_guideCurves[i]->GetUID() == uid) {
            return true;
        }
    }
    return false;
}

void CCPACSGuideCurves::GetRelativeCircumferenceRange(double relCirc,
                                                      double& relCircStart,
                                                      double& relCircEnd,
                                                      int& idx) const
{
    std::vector<double> relCircs;
    for (int iguide = 1; iguide <=  GetGuideCurveCount(); ++iguide) {
        const CCPACSGuideCurve* root = GetGuideCurve(iguide).GetRootCurve();
        relCircs.push_back(*root->GetFromRelativeCircumference_choice2());
    }
    if ( relCircs.back() < 1.0 ) {
        relCircs.push_back(1.0);
    }

    std::sort(relCircs.begin(), relCircs.end());

    // probably best to assert for performance reasons...
    assert( relCircs.size() > 0 );
    assert( relCirc >= relCircs[0] );
    assert( relCirc <= relCircs.back() );

    for (size_t i = 1; i < relCircs.size(); ++i ) {
        if (relCircs[i] >= relCirc ) {
            relCircStart = relCircs[i-1];
            relCircEnd = relCircs[i];
            idx = static_cast<int>(i-1);
            break;
        }
    }
}

} // end namespace tigl


