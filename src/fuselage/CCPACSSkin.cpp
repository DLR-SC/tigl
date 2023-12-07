/*
* Copyright (C) 2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-16 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include "CCPACSSkin.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CCPACSDuct.h"
#include "CCPACSFuselage.h"
#include "CCPACSDuctStructure.h"
#include "CCPACSFuselageStructure.h"

namespace tigl
{

CCPACSSkin::CCPACSSkin(CCPACSDuctStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSkin(parent, uidMgr) {}

CCPACSSkin::CCPACSSkin(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSkin(parent, uidMgr) {}

CCPACSSkin::CCPACSSkin(CCPACSHullSkinLayers* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSkin(parent, uidMgr) {}


CTiglRelativelyPositionedComponent const* CCPACSSkin::GetParentComponent() const
{
    if (IsParent<CCPACSDuctStructure>()) {
        return GetParent<CCPACSDuctStructure>()->GetParent();
    }
    else if (IsParent<CCPACSFuselageStructure>()) {
        return GetParent<CCPACSFuselageStructure>()->GetParent();
    }
    else {
        throw CTiglError("Unknown parent type for CCPACSSkin.");
    }
}

} //namespace tigl
