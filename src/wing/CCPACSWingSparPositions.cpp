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
#include "CCPACSWingSparPositions.h"

#include "CCPACSWingSparPosition.h"
#include "CCPACSWingSpars.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "IOHelper.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingSparPositions::CCPACSWingSparPositions(CCPACSWingSpars* parent)
: generated::CPACSSparPositions(parent)
{
}

const CCPACSWingSparPosition& CCPACSWingSparPositions::GetSparPosition(const std::string uid) const
{
    for (std::vector<unique_ptr<CCPACSWingSparPosition>>::const_iterator it = m_sparPosition.begin(); it != m_sparPosition.end(); ++it) {
        if ((*it)->GetUID() == uid) {
            return *(*it);
        }
    }

    LOG(ERROR) << "SparPosition with UID \"" << uid << "\" not found! Please check the CPACS document!";
    throw CTiglError("SparPosition with UID \"" + uid + "\" not found! Please check the CPACS document!");
}

CCPACSWingSparPosition& CCPACSWingSparPositions::GetSparPosition(const std::string uid)
{
    // forward call to const method
    return const_cast<CCPACSWingSparPosition&>(static_cast<const CCPACSWingSparPositions&>(*this).GetSparPosition(uid));
}

} // end namespace tigl
