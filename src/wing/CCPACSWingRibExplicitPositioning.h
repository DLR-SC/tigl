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

#pragma once

#include "generated/CPACSWingRibExplicitPositioning.h"

namespace tigl
{
class CCPACSWingRibExplicitPositioning : public generated::CPACSWingRibExplicitPositioning
{
public:
    TIGL_EXPORT CCPACSWingRibExplicitPositioning(CCPACSWingRibsDefinition* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT virtual ~CCPACSWingRibExplicitPositioning();

    // overriding ReadCPACS for handling registration of uid references
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

    TIGL_EXPORT void SetStartCurvePoint(const CCPACSCurvePoint& value);
    TIGL_EXPORT void SetStartEtaXsiPoint(const CCPACSEtaXsiPoint& value);
    TIGL_EXPORT void SetStartSparPositionUID(const std::string& value);
    TIGL_EXPORT void SetEndCurvePoint(const CCPACSCurvePoint& value);
    TIGL_EXPORT void SetEndEtaXsiPoint(const CCPACSEtaXsiPoint& value);
    TIGL_EXPORT void SetEndSparPositionUID(const std::string& value);

    // overriding setter for correct handling of invalidation
    TIGL_EXPORT void SetRibStart(const std::string& value) override;
    TIGL_EXPORT void SetRibEnd(const std::string& value) override;

private:
    void InvalidateParent() const;
};
}
