/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSWINGCSSTRUCTURE_H
#define CCPACSWINGCSSTRUCTURE_H

#include "generated/CPACSWingComponentSegmentStructure.h"
#include "CTiglWingStructureReference.h"


namespace tigl
{

// forward declarations
class CCPACSWingSpars;
class CCPACSWingRibsDefinitions;

// shouldn't his be CCPACSWingComponentSegmentStructure
class CCPACSWingCSStructure : public generated::CPACSWingComponentSegmentStructure
{
public:
    TIGL_EXPORT CCPACSWingCSStructure(CCPACSWingComponentSegment* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSWingCSStructure(CCPACSTrailingEdgeDevice* parent, CTiglUIDManager* uidMgr);

    // Returns number of spar segments
    TIGL_EXPORT int GetSparSegmentCount() const;

    // Returns spar segment with passed index
    TIGL_EXPORT CCPACSWingSparSegment& GetSparSegment(int index);
    TIGL_EXPORT const CCPACSWingSparSegment& GetSparSegment(int index) const;

    // Returns spar segment with passed uid
    TIGL_EXPORT CCPACSWingSparSegment& GetSparSegment(const std::string& uid);
    TIGL_EXPORT const CCPACSWingSparSegment& GetSparSegment(const std::string& uid) const;

    // Returns number of ribs definitions
    TIGL_EXPORT int GetRibsDefinitionCount() const;

    // Returns ribs definition with passed index
    TIGL_EXPORT CCPACSWingRibsDefinition& GetRibsDefinition(int index);
    TIGL_EXPORT const CCPACSWingRibsDefinition& GetRibsDefinition(int index) const;

    // Returns ribs definition with passed uid
    TIGL_EXPORT CCPACSWingRibsDefinition& GetRibsDefinition(const std::string& uid);
    TIGL_EXPORT const CCPACSWingRibsDefinition& GetRibsDefinition(const std::string& uid) const;

    TIGL_EXPORT void Invalidate();
};

} // namespace tigl

#endif // CCPACSWINGCSSTRUCTURE_H
