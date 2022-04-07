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
#include "CCPACSDuctAssembly.h"
#include "CCutShape.h"
#include "CNamedShape.h"

#include <algorithm>

namespace tigl {

CCPACSDucts::CCPACSDucts(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDucts(parent, uidMgr)
{}

PNamedShape CCPACSDucts::LoftWithDuctCutouts(PNamedShape const& cleanLoft, std::string const & uid) const
{
    if (m_ductAssemblys.size() == 0) {
        return cleanLoft;
    }

    auto loft = cleanLoft;

    for (auto const & ductAssembly : m_ductAssemblys)
    {
        if (ductAssembly->GetExcludeObjectUIDs()) {

            auto const & excludeVector =ductAssembly->GetExcludeObjectUIDs()->GetUIDs();
            if (any_of(excludeVector.begin(), excludeVector.end(), [&](const std::string& elem) { return elem == uid; })) {
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











} //namespace tigl
