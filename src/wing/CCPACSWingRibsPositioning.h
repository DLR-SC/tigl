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
    TIGL_EXPORT CCPACSWingRibsPositioning(CCPACSWingRibsDefinition* parent);


    TIGL_EXPORT StartEndDefinitionType GetStartDefinitionType() const;

    TIGL_EXPORT StartEndDefinitionType GetEndDefinitionType() const;

    TIGL_EXPORT RibCountDefinitionType GetRibCountDefinitionType() const;
    TIGL_EXPORT void SetNumberOfRibs(int);
    TIGL_EXPORT void SetSpacing(double);

    TIGL_EXPORT void SetRibCrossingBehaviour(const generated::CPACSRibCrossingBehaviour& value) OVERRIDE;

    /// Reset the cached structural variables. Must be executed after changing the rib refinition
    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT void SetStartCurvePoint(const CCPACSCurvePoint& value);
    TIGL_EXPORT void SetStartEtaXsiPoint(const CCPACSEtaXsiPoint& value);
    TIGL_EXPORT void SetStartSparPositionUID(const std::string& value);
    TIGL_EXPORT void SetEndCurvePoint(const CCPACSCurvePoint& value);
    TIGL_EXPORT void SetEndEtaXsiPoint(const CCPACSEtaXsiPoint& value);
    TIGL_EXPORT void SetEndSparPositionUID(const std::string& value);
private:
    friend class CCPACSWingRibRotation;
};

} // end namespace tigl

#endif // CCPACSWINGRIBSPOSITIONING_H
