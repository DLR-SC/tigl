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
* @brief  Implementation of CPACS wings handling routines.
*/

#include "CCPACSWings.h"

#include "CTiglError.h"
#include "CCPACSConfiguration.h"
#include "CCPACSAircraftModel.h"

namespace tigl
{

// Invalidates internal state
void CCPACSWings::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 1; i <= GetWingCount(); i++) {
        GetWing(i).Invalidate(source);
    }
}

CCPACSWings::CCPACSWings(CCPACSRotorcraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWings(parent, uidMgr) {}

CCPACSWings::CCPACSWings(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWings(parent, uidMgr) {}

// Returns the count of wings in a configuration with the property isRotorBlade set to true
int CCPACSWings::GetRotorBladeCount() const
{
    int nRotorBlades = 0;
    for (size_t i = 1; i <= GetWingCount(); i++) {
        if (GetWing(i).IsRotorBlade()) {
            nRotorBlades++;
        }
    }
    return nRotorBlades;
}

bool CCPACSWings::HasWing(const std::string & uid) const
{
    for (std::size_t i = 0; i < m_wings.size(); i++)
        if (m_wings[i]->GetUID() == uid)
            return true;
    return false;
}

} // end namespace tigl
