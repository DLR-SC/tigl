/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-04-06 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief  Implementation of CPACS ducts handling routines.
*/

#include "CCPACSDucts.h"
#include "CCPACSDuct.h"
#include "CCutShape.h"
#include "CNamedShape.h"
#include "CTiglError.h"

#include <algorithm>

namespace tigl {

CCPACSDucts::CCPACSDucts(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDucts(parent, uidMgr)
    , enabled(false)
{}

void CCPACSDucts::RegisterInvalidationCallback(std::function<void()> const& fn){
    invalidationCallbacks.push_back(fn);
}

PNamedShape CCPACSDucts::LoftWithDuctCutouts(PNamedShape const& cleanLoft, std::string const & uid) const
{
    if (!enabled || m_ductAssemblys.size() == 0) {
        return cleanLoft;
    }

    auto loft = cleanLoft;

    for (auto const & ductAssembly : m_ductAssemblys)
    {
        if (ductAssembly->GetExcludeObjectUIDs()) {

            auto const & excludeVector =ductAssembly->GetExcludeObjectUIDs()->GetUIDs();
            if (std::any_of(excludeVector.begin(), excludeVector.end(), [&](const std::string& elem) { return elem == uid; })) {
                continue;
            }
        }

        loft = CCutShape(loft, ductAssembly->GetLoft()).NamedShape();

        // Mark the clean loft as origin, rather than the duct loft
        for (int iFace = 0; iFace < static_cast<int>(loft->GetFaceCount()); ++iFace) {
            CFaceTraits ft = loft->GetFaceTraits(iFace);
            ft.SetOrigin(cleanLoft);
            loft->SetFaceTraits(iFace, ft);
        }
    }

    return loft;
}

CCPACSDuct const& CCPACSDucts::GetDuct(std::string const& uid) const
{
    auto it = std::find_if(m_ducts.begin(), m_ducts.end(), [&](std::unique_ptr<CCPACSDuct> const& v){ return v->GetUID() == uid; });

    if ( it != std::end(m_ducts)) {
        return **it;
    }
    throw CTiglError("Could not find duct with uid" + uid);
}

CCPACSDuct& CCPACSDucts::GetDuct(std::string const& uid)
{
    return const_cast<CCPACSDuct&>(GetDuct(uid));
}


bool CCPACSDucts::IsEnabled() const
{
    return enabled;
}

void CCPACSDucts::SetEnabled(bool val)
{
    if (enabled != val) {
        for (auto const& invalidator: invalidationCallbacks) {
            invalidator();
        }
        enabled = val;
    }
}

} //namespace tigl
