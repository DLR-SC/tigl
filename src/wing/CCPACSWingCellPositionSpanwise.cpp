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

#include "CCPACSWingCellPositionSpanwise.h"

#include "CCPACSWingCell.h"


namespace tigl
{

// Constructor
CCPACSWingCellPositionSpanwise::CCPACSWingCellPositionSpanwise(CCPACSWingCell* parent)
: CTiglWingSpanwiseBorder(),
  m_parent(parent)
{
    m_eta1CPACSName = "eta1";
    m_eta2CPACSName = "eta2";
}

void CCPACSWingCellPositionSpanwise::invalidateParent()
{
    m_parent->Invalidate();
}

}
