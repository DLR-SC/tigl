/* 
* Copyright (C) 2016 Airbus Defence and Space
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

#include "CCPACSWingSparPositionUIDs.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "to_string.h"


namespace tigl
{

CCPACSWingSparPositionUIDs::CCPACSWingSparPositionUIDs()
{
}

int CCPACSWingSparPositionUIDs::GetSparPositionUIDCount() const
{
    return static_cast<int>(m_sparPositionUIDs.size());
}

const std::string& CCPACSWingSparPositionUIDs::GetSparPositionUID(int index) const
{
    const int idx = index - 1;
    if (idx < 0 || idx >= GetSparPositionUIDCount()) {
        LOG(ERROR) << "Invalid index value";
        throw CTiglError("Invalid index value in CCPACSWingSparPositionUIDs::GetSparPositionUID", TIGL_INDEX_ERROR);
    }
    return (m_sparPositionUIDs[idx]);
}

} // end namespace tigl
