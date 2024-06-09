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

#include "CCPACSFramesAssembly.h"
#include "CTiglTransformation.h"
#include "CCPACSFrame.h"
#include "CCPACSFuselage.h"
#include "CCPACSDuct.h"
#include "ITiglFuselageDuctStructure.h"

namespace tigl
{
CCPACSFramesAssembly::CCPACSFramesAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFramesAssembly(parent, uidMgr)
{
}

CCPACSFramesAssembly::CCPACSFramesAssembly(CCPACSDuctStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFramesAssembly(parent, uidMgr)
{
}

CCPACSFramesAssembly::CCPACSFramesAssembly(CCPACSHullStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFramesAssembly(parent, uidMgr)
{
}

void CCPACSFramesAssembly::Invalidate(const boost::optional<std::string>& source) const
{
    for (size_t i = 0; i < m_frames.size(); i++)
        m_frames[i]->Invalidate(source);
}

// ToDo: Add hulls
CTiglRelativelyPositionedComponent const* CCPACSFramesAssembly::GetParentComponent() const
{
    if (IsParent<CCPACSDuctStructure>()) {
        return GetParent<CCPACSDuctStructure>()->GetParent();
    }
    if (IsParent<CCPACSFuselageStructure>()) {
        return GetParent<CCPACSFuselageStructure>()->GetParent();
    }
    throw CTiglError("Unexpected error: Parent of CCPACSFramesAssembly must either be CCPACSDuctStructure or CCPACSFuselageStructure.");
}

CTiglTransformation CCPACSFramesAssembly::GetTransformationMatrix() const
{
    return GetParentComponent()->GetTransformationMatrix();
}

ITiglFuselageDuctStructure const* CCPACSFramesAssembly::GetStructureInterface() const
{
    if (IsParent<CCPACSDuctStructure>()) {
        return GetParent<CCPACSDuctStructure>();
    }
    if (IsParent<CCPACSFuselageStructure>()) {
        return GetParent<CCPACSFuselageStructure>();
    }
    throw CTiglError("Unexpected error: Parent of CCPACSFramesAssembly must either be CCPACSDuctStructure or CCPACSFuselageStructure.");
}

} // namespace tigl
