/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#pragma once

#include "generated/CPACSNacelleSections.h"
#include "generated/CPACSNacelleProfiles.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSNacelleSections : public generated::CPACSNacelleSections
{
public:
    TIGL_EXPORT CCPACSNacelleSections(CCPACSNacelleCowl* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT size_t GetSectionCount() const;
    TIGL_EXPORT CCPACSNacelleSection& GetSection(size_t index) const;
    TIGL_EXPORT CCPACSNacelleSection& GetSection(std::string const uid) const;
    TIGL_EXPORT size_t GetSectionIndex(std::string const uid) const;
};

} //namespace tigl
