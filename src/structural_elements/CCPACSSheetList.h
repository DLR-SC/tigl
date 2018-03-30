/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#pragma once

#include <TopoDS_Wire.hxx>
#include <gp_Pln.hxx>

#include "generated/CPACSSheetList.h"

namespace tigl
{
class CCPACSSheetList : public generated::CPACSSheetList
{
public:
    TIGL_EXPORT CCPACSSheetList(CCPACSStructuralProfile* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT TopoDS_Wire CreateProfileWire(const gp_Pln& profilePlane) const;
};

} // namespace tigl
