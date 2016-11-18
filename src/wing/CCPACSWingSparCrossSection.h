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
#ifndef CCPACSWINGSPARCROSSSECTION_H
#define CCPACSWINGSPARCROSSSECTION_H

#include "generated/CPACSSparCrossSection.h"
#include "CCPACSWingSparWeb.h"


namespace tigl
{

// forward declarations
class CCPACSWingCSStructure;

class CCPACSWingSparCrossSection : public generated::CPACSSparCrossSection
{
public:
    TIGL_EXPORT CCPACSWingSparCrossSection(CCPACSWingSparSegment* sparSegment);
    TIGL_EXPORT CCPACSWingSparCrossSection(CPACSMainGearSupportBeam* supportBeam);
};

} // end namespace tigl

#endif // CCPACSWINGSPARCROSSSECTION_H
