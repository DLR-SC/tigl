/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2019-01-31 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSEnginePosition.h"
#include "generated/CPACSEngine.h"
#include "CTiglUIDManager.h"
#include "CTiglEngineNacelleBuilder.h"

namespace tigl {

CCPACSEnginePosition::CCPACSEnginePosition(CCPACSEnginePositions* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEnginePosition(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation)
{
}

std::string CCPACSEnginePosition::GetDefaultedUID() const
{
    return generated::CPACSEnginePosition::GetUID();
}

void CCPACSEnginePosition::Invalidate()
{
}

PNamedShape CCPACSEnginePosition::BuildLoft() const
{
    try {
        CCPACSEngine& engine = m_uidMgr->ResolveObject<CCPACSEngine>(m_engineUID);
        if (engine.GetNacelle()) {
            boost::optional<CCPACSEngineNacelle>& nacelle = engine.GetNacelle();
            CTiglEngineNacelleBuilder builder(*nacelle, CTiglRelativelyPositionedComponent::GetTransformationMatrix());
            return builder.BuildShape();
        }
        else {
            throw CTiglError("Cannot build geometry of engine " + m_engineUID + " without a nacelle definition.");
        }
    }
    catch(...) {
        throw(CTiglError("CCPACSEnginePosition: Unable to build nacelle for the engine with UID " + m_engineUID + ".", TIGL_ERROR));
    }
}

} //namespace tigl
