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

#include "CCPACSProfileBasedStructuralElement.h"

#include <BRepOffsetAPI_MakePipeShell.hxx>

#include "generated/CPACSStructuralProfile.h"
#include "generated/CPACSSheetList.h"
#include "CTiglUIDManager.h"

namespace tigl
{
CCPACSProfileBasedStructuralElement::CCPACSProfileBasedStructuralElement(CTiglUIDManager* uidMgr)
    : generated::CPACSProfileBasedStructuralElement(uidMgr)
{
}

TopoDS_Shape CCPACSProfileBasedStructuralElement::makeFromWire(const TopoDS_Wire& spine,
                                                               const gp_Pln& profilePlane) const
{
    const CCPACSSheetList& sheetList =
        m_uidMgr->ResolveObject<CCPACSStructuralProfile>(m_structuralProfileUID_choice1.value()).GetSheetList();
    const TopoDS_Wire& profile = sheetList.CreateProfileWire(profilePlane);

    BRepOffsetAPI_MakePipeShell frameShell(spine);
    // frameShell.SetMode(gp_Dir(1,0,0)); // force the profile plane being not twisted (profile stay parallel to X axis)
    frameShell.SetDiscreteMode();
    frameShell.Add(profile);
    frameShell.Build();
    if (!frameShell.IsDone())
        throw CTiglError("Failed to construct profile based structural element");

    return frameShell.Shape();
}

} // namespace tigl
