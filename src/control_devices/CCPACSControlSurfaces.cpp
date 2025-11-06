/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSControlSurfaces.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSLeadingEdgeDevice.h"

namespace tigl
{

CCPACSControlSurfaces::CCPACSControlSurfaces(CCPACSWingComponentSegment* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaces(parent, uidMgr)
{
}

size_t CCPACSControlSurfaces::ControlSurfaceCount() const
{
    size_t count = 0;
    if (GetTrailingEdgeDevices()) {
        count += GetTrailingEdgeDevices()->GetTrailingEdgeDevices().size();
    }
    if (GetLeadingEdgeDevices()) {
        count += GetLeadingEdgeDevices()->GetLeadingEdgeDevices().size();
    }

    return count;
}

const std::string& CCPACSControlSurfaces::GetControlSurfaceUID(size_t index) const
{
    // trailing edge devices first
    if (GetTrailingEdgeDevices()) {
        const auto& teds = GetTrailingEdgeDevices()->GetTrailingEdgeDevices();
        if (index < teds.size()) {
            return teds.at(index)->GetUID();
        }
        index -= teds.size();
    }

    if (GetLeadingEdgeDevices()) {
        const auto& leds = GetLeadingEdgeDevices()->GetLeadingEdgeDevices();
        if (index < leds.size()) {
            return leds.at(index)->GetUID();
        }
    }

    throw CTiglError("GetControlSurfaceUID: index out of range", TIGL_INDEX_ERROR);
}

} // namespace tigl
