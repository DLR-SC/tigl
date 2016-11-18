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
#ifndef CCPACSWINGRIBCROSSSECTION_H
#define CCPACSWINGRIBCROSSSECTION_H

#include <string>

#include <tixi.h>

#include "generated/CPACSWingRibCrossSection.h"
#include "CCPACSMaterial.h"
#include "tigl_internal.h"


namespace tigl
{

// forward declarations
class CCPACSWingRibsDefinition;

class CCPACSWingRibCrossSection : public generated::CPACSWingRibCrossSection
{
public:
    TIGL_EXPORT CCPACSWingRibCrossSection(CPACSPylonRibsDefinition* parent);
    TIGL_EXPORT CCPACSWingRibCrossSection(CCPACSWingRibsDefinition* parent);

    TIGL_EXPORT double GetXRotation() const;
    TIGL_EXPORT void SetXRotation(double);
};

} // end namespace tigl

#endif // CCPACSWINGRIBCROSSSECTION_H
