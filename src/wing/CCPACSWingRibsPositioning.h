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

#include <string>

#include <tixi.h>

#include "CCPACSWingRibRotation.h"
#include "tigl_internal.h"


namespace tigl
{

// forward declaration
class CCPACSWingRibsDefinition;

class CCPACSWingRibsPositioning
{
public:
    enum CrossingBehaviour 
    { 
        CROSSING_CROSS,
        CROSSING_END
    };
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
    TIGL_EXPORT CCPACSWingRibsPositioning(CCPACSWingRibsDefinition& parent);
    TIGL_EXPORT virtual ~CCPACSWingRibsPositioning();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & ribsPositioningXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & ribsPositioningXPath) const;

    TIGL_EXPORT const std::string& GetRibReference() const;
    TIGL_EXPORT void SetRibReference(const std::string&);

    TIGL_EXPORT StartDefinitionType GetStartDefinitionType() const;
    
    TIGL_EXPORT double GetEtaStart() const;
    TIGL_EXPORT void SetEtaStart(double);

    TIGL_EXPORT const std::string& GetElementStartUID() const;
    TIGL_EXPORT void SetElementStartUID(const std::string&);

    // NOTE: definition via spar position not conform with CPACS format (v2.3)
    TIGL_EXPORT const std::string& GetSparPositionStartUID() const;
    TIGL_EXPORT void SetSparPositionStartUID(const std::string&);

    TIGL_EXPORT EndDefinitionType GetEndDefinitionType() const;

    TIGL_EXPORT double GetEtaEnd() const;
    TIGL_EXPORT void SetEtaEnd(double);

    TIGL_EXPORT const std::string& GetElementEndUID() const;
    TIGL_EXPORT void SetElementEndUID(const std::string&);

    // NOTE: definition via spar position not conform with CPACS format (v2.3)
    TIGL_EXPORT const std::string& GetSparPositionEndUID() const;
    TIGL_EXPORT void SetSparPositionEndUID(const std::string&);

    TIGL_EXPORT const std::string& GetRibStart() const;
    TIGL_EXPORT void SetRibStart(const std::string&);

    TIGL_EXPORT const std::string& GetRibEnd() const;
    TIGL_EXPORT void SetRibEnd(const std::string&);

    TIGL_EXPORT RibCountDefinitionType GetRibCountDefinitionType() const;

    TIGL_EXPORT int GetNumberOfRibs() const;
    TIGL_EXPORT void SetNumberOfRibs(int);

    TIGL_EXPORT double GetSpacing() const;
    TIGL_EXPORT void SetSpacing(double);

    TIGL_EXPORT CrossingBehaviour GetRibCrossingBehaviour() const;
    TIGL_EXPORT void SetRibCrossingBehaviour(CrossingBehaviour);

    TIGL_EXPORT const CCPACSWingRibRotation& GetRibRotation() const;
    TIGL_EXPORT CCPACSWingRibRotation& GetRibRotation();

protected:
    void Cleanup();

private:
    CCPACSWingRibsPositioning(const CCPACSWingRibsPositioning&);
    void operator=(const CCPACSWingRibsPositioning&);

private:
    CCPACSWingRibsDefinition& parent;
    std::string ribReference;
    StartDefinitionType startDefinitionType;
    double etaStart;
    std::string elementStartUID;
    std::string sparPositionStartUID;
    EndDefinitionType endDefinitionType;
    double etaEnd;
    std::string elementEndUID;
    std::string sparPositionEndUID;
    std::string ribStart;
    std::string ribEnd;
    RibCountDefinitionType ribCountDefinitionType;
    int numberOfRibs;
    double spacing;
    CrossingBehaviour ribCrossingBehaviour;
    CCPACSWingRibRotation ribRotation;
};

} // end namespace tigl

#endif // CCPACSWINGRIBSPOSITIONING_H
