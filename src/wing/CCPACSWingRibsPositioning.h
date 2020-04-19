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
    enum StartEndDefinitionType
    {
        CURVEPOINT_STARTEND,
        ETAXSI_STARTEND,
        SPARPOSITION_STARTEND,
    };

    enum RibCountDefinitionType
    {
        NUMBER_OF_RIBS,
        SPACING
    };

public:
    TIGL_EXPORT CCPACSWingRibsPositioning(CCPACSWingRibsDefinition* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT virtual ~CCPACSWingRibsPositioning();

    // overriding ReadCPACS for handling registration of uid references
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

    TIGL_EXPORT StartEndDefinitionType GetStartDefinitionType() const;

    TIGL_EXPORT StartEndDefinitionType GetEndDefinitionType() const;

    // overriding setter for handling invalidation correctly
    TIGL_EXPORT void SetRibStart(const std::string& value) override;
    TIGL_EXPORT void SetRibEnd(const std::string& value) override;

    TIGL_EXPORT RibCountDefinitionType GetRibCountDefinitionType() const;
    TIGL_EXPORT void SetNumberOfRibs(int);
    TIGL_EXPORT void SetSpacing(double);
    TIGL_EXPORT void SetRibReference(const std::string& value) override;

    TIGL_EXPORT void SetRibCrossingBehaviour(const generated::CPACSRibCrossingBehaviour& value) override;

    TIGL_EXPORT void SetStartCurvePoint(const CCPACSCurvePoint& value);
    TIGL_EXPORT void SetStartEtaXsiPoint(const CCPACSEtaXsiPoint& value);
    TIGL_EXPORT void SetStartSparPositionUID(const std::string& value);
    TIGL_EXPORT void SetEndCurvePoint(const CCPACSCurvePoint& value);
    TIGL_EXPORT void SetEndEtaXsiPoint(const CCPACSEtaXsiPoint& value);
    TIGL_EXPORT void SetEndSparPositionUID(const std::string& value);
private:
    void InvalidateParent() const;

    friend class CCPACSWingRibRotation;
};

} // end namespace tigl

#endif // CCPACSWINGRIBSPOSITIONING_H
