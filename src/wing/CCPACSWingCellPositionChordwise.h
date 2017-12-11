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

#ifndef CCPACSWINGCELLPOSITIONCHORDWISE_H
#define CCPACSWINGCELLPOSITIONCHORDWISE_H

#include <utility>

#include "generated/CPACSCellPositioningChordwise.h"

namespace tigl
{
// forward declarations
class CCPACSWingCell;

class CCPACSWingCellPositionChordwise : public generated::CPACSCellPositioningChordwise
{
public:
    enum InputType
    {
        Xsi,
        Spar,
        None
    };

    TIGL_EXPORT CCPACSWingCellPositionChordwise(CCPACSWingCell* parent);

    TIGL_EXPORT InputType GetInputType() const;

    TIGL_EXPORT void GetXsi(double& xsi1, double& xsi2) const;
    TIGL_EXPORT std::pair<double, double> GetXsi() const;
    TIGL_EXPORT void SetXsi(double xsi1, double xsi2);

    TIGL_EXPORT const std::string& GetSparUId() const;
    TIGL_EXPORT void SetSparUId(std::string nSparUId);
};

} // end namespace tigl
#endif // CCPACSWINGCELLPOSITIONCHORDWISE_H
