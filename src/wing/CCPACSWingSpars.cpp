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

#include "CCPACSWingSpars.h"

namespace tigl
{

CCPACSWingSpars::CCPACSWingSpars(CCPACSWingCSStructure* structure)
: generated::CPACSWingSpar(structure)
{
}

void CCPACSWingSpars::Invalidate()
{
    m_sparSegments.Invalidate();
}

const CCPACSWingCSStructure& CCPACSWingSpars::GetStructure() const
{
    return *GetParent();
}

CCPACSWingCSStructure& CCPACSWingSpars::GetStructure()
{
    return *GetParent();
}

} // end namespace tigl
