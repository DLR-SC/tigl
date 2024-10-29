/*
* Copyright (c) 2020 RISC Software GmbH
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

#include "CTiglUIDObject.h"

#include "CTiglUIDManager.h"
#include "ReentryGuard.h"

namespace tigl
{

void CTiglUIDObject::Invalidate(const boost::optional<std::string>& source) const
{
    const ReentryGuard guard(m_isInvalidating);
    if (guard) {
        InvalidateImpl(source);
        InvalidateReferences(source);
    }
}

void CTiglUIDObject::InvalidateReferences(const boost::optional<std::string>& source) const
{
    const CTiglUIDManager& uidMgr = GetUIDManager();
    if (GetObjectUID()) {
        const std::string uid = *GetObjectUID();
        if (!uid.empty() && uidMgr.IsReferenced(uid)) {
            for (const CTiglUIDObject* obj : uidMgr.GetReferences(uid)) {
                obj->Invalidate(uid);
            }
        }
    }
}

}