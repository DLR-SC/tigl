/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-21 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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
* @brief  Implementation of CPACS wings handling routines.
*/

#include "CCPACSGenericSystems.h"

#include "CCPACSGenericSystem.h"
#include "CCPACSACSystems.h"
#include "CTiglError.h"


namespace tigl
{

// Constructor
CCPACSGenericSystems::CCPACSGenericSystems(CCPACSACSystems* parent, CTiglUIDManager* uidMgr)
: generated::CPACSGenericSystems(parent, uidMgr)
{
}

// Destructor
CCPACSGenericSystems::~CCPACSGenericSystems()
{
}

CCPACSConfiguration& CCPACSGenericSystems::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

} // end namespace tigl
