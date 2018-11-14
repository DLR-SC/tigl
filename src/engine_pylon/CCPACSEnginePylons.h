/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-10-31 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef CCPACSENGINEPYLONS_H
#define CCPACSENGINEPYLONS_H

#include "generated/CPACSEnginePylons.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSEnginePylons : public generated::CPACSEnginePylons
{
public:
    // Constructor
    TIGL_EXPORT CCPACSEnginePylons(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Gets a pylon by index or UID.
    TIGL_EXPORT CCPACSEnginePylon& GetEnginePylon(const int index);
    TIGL_EXPORT const CCPACSEnginePylon& GetEnginePylon(const int index) const;
    TIGL_EXPORT CCPACSEnginePylon& GetEnginePylon(const std::string& segmentUID);
    TIGL_EXPORT const CCPACSEnginePylon& GetEnginePylon(const std::string& segmentUID) const;

    // Gets total pylon count
    TIGL_EXPORT int GetPylonCount() const;
};

} // end namespace tigl

#endif // CCPACSENGINEPYLONS_H
