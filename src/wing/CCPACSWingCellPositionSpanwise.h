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
/**
* @file
* @brief  Implementation of CPACS Wing Cell handling routines.
*/

#ifndef CCPACSWINGCELLPOSITIONSPANWISE_H
#define CCPACSWINGCELLPOSITIONSPANWISE_H

#include "CTiglWingSpanwiseBorder.h"


namespace tigl 
{

// forward declarations
class CCPACSWingCell;

class CCPACSWingCellPositionSpanwise : public CTiglWingSpanwiseBorder
{
public:
    TIGL_EXPORT CCPACSWingCellPositionSpanwise(CCPACSWingCell* parent);

protected:
    void invalidateParent();

private:
    CCPACSWingCellPositionSpanwise(const CCPACSWingCellPositionSpanwise&); // = delete;
    void operator=(const CCPACSWingCellPositionSpanwise&); // = delete;

    CCPACSWingCell* m_parent;
};

} // end namespace tigl


#endif // CCPACSWINGCELLPOSITIONSPANWISE_H
