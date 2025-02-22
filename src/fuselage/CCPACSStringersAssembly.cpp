/*
* Copyright (c) 2018 RISC Software GmbH
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

#include "CCPACSStringersAssembly.h"
#include "CCPACSFuselageStringer.h"
#include "CCPACSFuselage.h"
#include "CCPACSDuct.h"

namespace tigl
{

CCPACSStringersAssembly::CCPACSStringersAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSStringersAssembly(parent, uidMgr)
{
}

CCPACSStringersAssembly::CCPACSStringersAssembly(CCPACSDuctStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSStringersAssembly(parent, uidMgr)
{
}

CCPACSStringersAssembly::CCPACSStringersAssembly(CCPACSVesselStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSStringersAssembly(parent, uidMgr)
{
}

void CCPACSStringersAssembly::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 0; i < (int)m_stringers.size(); i++) {
        m_stringers[i]->Invalidate(source);
    }
}

CTiglRelativelyPositionedComponent const* CCPACSStringersAssembly::GetParentComponent() const
{
    if (IsParent<CCPACSDuctStructure>()) {
        return GetParent<CCPACSDuctStructure>()->GetParent();
    }
    if (IsParent<CCPACSFuselageStructure>()) {
        return GetParent<CCPACSFuselageStructure>()->GetParent();
    }
    throw CTiglError("Unexpected error: Parent of CCPACSStringersAssembly must either be CCPACSDuctStructure or CCPACSFuselageStructure.");
}

CTiglTransformation CCPACSStringersAssembly::GetTransformationMatrix() const
{
    return GetParentComponent()->GetTransformationMatrix();
}

ITiglFuselageDuctStructure const* CCPACSStringersAssembly::GetStructureInterface() const
{
    if (IsParent<CCPACSDuctStructure>()) {
        return GetParent<CCPACSDuctStructure>();
    }
    if (IsParent<CCPACSFuselageStructure>()) {
        return GetParent<CCPACSFuselageStructure>();
    }
    throw CTiglError("Unexpected error: Parent of CCPACSStringersAssembly must either be CCPACSDuctStructure or CCPACSFuselageStructure.");
}

} // namespace tigl
