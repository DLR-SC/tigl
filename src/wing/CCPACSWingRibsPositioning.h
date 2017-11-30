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
    // NOTE: definition of start/end of rib via spar position not conform with CPACS format (v2.3)
    enum StartDefinitionType
    { 
        ELEMENT_START,
        ETA_START,
        SPARPOSITION_START
    };
    enum EndDefinitionType
    {
        ELEMENT_END,
        ETA_END,
        SPARPOSITION_END
    };
    enum RibCountDefinitionType
    {
        NUMBER_OF_RIBS,
        SPACING
    };

public:
    TIGL_EXPORT CCPACSWingRibsPositioning(CCPACSWingRibsDefinition* parent);

    TIGL_EXPORT virtual void SetRibReference(const std::string& value) OVERRIDE;
    TIGL_EXPORT virtual void SetRibStart(const std::string& value) OVERRIDE;
    TIGL_EXPORT virtual void SetRibEnd(const std::string& value) OVERRIDE;

    TIGL_EXPORT StartDefinitionType GetStartDefinitionType() const;
    TIGL_EXPORT void SetEtaStart(double);
    TIGL_EXPORT void SetElementStartUID(const std::string&);
    TIGL_EXPORT void SetSparPositionStartUID(const std::string&); // NOTE: definition via spar position not conform with CPACS format (v2.3)

    TIGL_EXPORT EndDefinitionType GetEndDefinitionType() const;
    TIGL_EXPORT void SetEtaEnd(double);
    TIGL_EXPORT void SetElementEndUID(const std::string&);
    TIGL_EXPORT void SetSparPositionEndUID(const std::string&); // NOTE: definition via spar position not conform with CPACS format (v2.3)

    TIGL_EXPORT RibCountDefinitionType GetRibCountDefinitionType() const;
    TIGL_EXPORT void SetNumberOfRibs(int);
    TIGL_EXPORT void SetSpacing(double);

    TIGL_EXPORT virtual void SetRibCrossingBehaviour(const generated::CPACSRibCrossingBehaviour& value) OVERRIDE;

private:
    friend class CCPACSWingRibRotation;

    void invalidateStructure();
};

} // end namespace tigl

#endif // CCPACSWINGRIBSPOSITIONING_H
