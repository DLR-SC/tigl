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
#ifndef CCPACSWINGRIBSPOSITIONING_H
#define CCPACSWINGRIBSPOSITIONING_H

#include "generated/CPACSWingRibsPositioning.h"
#include "CCPACSWingRibRotation.h"

namespace tigl
{

// forward declaration
class CCPACSWingRibsDefinition;

class CCPACSWingRibsPositioning : public generated::CPACSWingRibsPositioning
{
public:
    enum RibCountDefinitionType
    {
        NUMBER_OF_RIBS,
        SPACING
    };

public:
    TIGL_EXPORT CCPACSWingRibsPositioning(CCPACSWingRibsDefinition* parent);
    
    TIGL_EXPORT void SetEtaStart(double);
    TIGL_EXPORT void SetEtaEnd(double);

    TIGL_EXPORT void SetRibStart(const std::string&);
    TIGL_EXPORT void SetRibEnd(const std::string&);

    TIGL_EXPORT RibCountDefinitionType GetRibCountDefinitionType() const;

    TIGL_EXPORT int GetNumberOfRibs() const;
    TIGL_EXPORT void SetNumberOfRibs(int);

    TIGL_EXPORT double GetSpacing() const;
    TIGL_EXPORT void SetSpacing(double);

private:
    void invalidateStructure();
};

} // end namespace tigl

#endif // CCPACSWINGRIBSPOSITIONING_H
