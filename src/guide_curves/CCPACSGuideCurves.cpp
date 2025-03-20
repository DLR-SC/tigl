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

std::vector<double> CCPACSGuideCurves::GetRelativeCircumferenceParameters() const
{
    std::vector<double> relCircs;

    // CPACS 3.3 requires guideCurves to be present. To prevent a hard crash if this is NOT the case,
    // we shoud exit here (or at least before the call to relCircs.back())
    if (GetGuideCurveCount() == 0) {
        return relCircs;
    }

    for (int iguide = 1; iguide <=  GetGuideCurveCount(); ++iguide) {
        const CCPACSGuideCurve* root = GetGuideCurve(iguide).GetRootCurve();
        if(root->GetFromRelativeCircumference_choice2_1()) {
            relCircs.push_back(*root->GetFromRelativeCircumference_choice2_1());
        }
        else if(root->GetFromParameter_choice2_2()) {
            relCircs.push_back(*root->GetFromParameter_choice2_2());
        }
    }

    std::sort(relCircs.begin(), relCircs.end());

    if (std::abs(relCircs.back() - 1.0) >= 1e-3 ) {
        relCircs.push_back(1.0);
    }

    return relCircs;
}

void CCPACSGuideCurves::GetRelativeCircumferenceRange(double relCirc,
                                                      double& relCircStart,
                                                      double& relCircEnd,
                                                      int& idx) const
{
    std::vector<double> relCircs = GetRelativeCircumferenceParameters();

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


