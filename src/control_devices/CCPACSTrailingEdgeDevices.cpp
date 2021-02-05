/*
* Copyright (C) 2007-2021 German Aerospace Center (DLR/SC)
*
* Created: 2021-02-05 Jan Kleinert <Jan.Kleinert@dlr.de>
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
/**
* @file
* @brief  Implementation of CPACS trailing edge devices
*/

#include "CTiglError.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSTrailingEdgeDevices.h"

namespace tigl {

CCPACSTrailingEdgeDevices::CCPACSTrailingEdgeDevices(CCPACSControlSurfaces* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTrailingEdgeDevices(parent, uidMgr) {}

// Returns the total count of trailing edge devices in a configuration
int CCPACSTrailingEdgeDevices::GetTrailingEdgeDeviceCount() const
{
    return static_cast<int>(m_trailingEdgeDevices.size());
}

// Returns the trailing edge device for a given index.
CCPACSTrailingEdgeDevice& CCPACSTrailingEdgeDevices::GetTrailingEdgeDevice(int index) const
{
    index --;
    if (index < 0 || index >= GetTrailingEdgeDeviceCount()) {
        throw CTiglError("Invalid index in CCPACSTrailingEdgeDevices::GetTrailingEdgeDevice", TIGL_INDEX_ERROR);
    }
    return *m_trailingEdgeDevices[index];
}

// Returns the trailing edge device for a given UID.
CCPACSTrailingEdgeDevice& CCPACSTrailingEdgeDevices::GetTrailingEdgeDevice(const std::string& UID) const
{
    return *m_trailingEdgeDevices[GetTrailingEdgeDeviceIndex(UID)-1];
}

// Returns the trailing edge device index for a given UID.
int CCPACSTrailingEdgeDevices::GetTrailingEdgeDeviceIndex(const std::string& UID) const
{
    for (int i=0; i < GetTrailingEdgeDeviceCount(); i++) {
        const std::string tmpUID(m_trailingEdgeDevices[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Invalid UID in CCPACSTrailingEdgeDevices::GetTrailingEdgeDeviceIndex", TIGL_UID_ERROR);
}

} // namespace tigl
