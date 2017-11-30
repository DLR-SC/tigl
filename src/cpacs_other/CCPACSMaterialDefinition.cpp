/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSMaterialDefinition.h"

#include "CTiglError.h"

namespace tigl
{
CCPACSMaterialDefinition::CCPACSMaterialDefinition()
    : generated::CPACSMaterialDefinition() {}

bool CCPACSMaterialDefinition::isComposite() const
{
    if (m_compositeUID_choice1) {
        return true;
    } else if (m_materialUID_choice2) {
        return false;
    } else {
        throw CTiglError("Neither materialUID nor compositeUID specified", TIGL_ERROR);
    }
}

const std::string& CCPACSMaterialDefinition::GetUID() const
{
    if (isComposite())
        return *GetCompositeUID_choice1();
    else
        return *GetMaterialUID_choice2();
}

void CCPACSMaterialDefinition::SetUID(const std::string& uid)
{
    if (isComposite())
        return SetCompositeUID_choice1(uid);
    else
        return SetMaterialUID_choice2(uid);
}

} // namespace tigl
