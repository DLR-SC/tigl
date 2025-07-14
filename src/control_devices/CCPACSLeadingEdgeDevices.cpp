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

#include "CCPACSLeadingEdgeDevices.h"
#include "CCPACSLeadingEdgeDevice.h"
#include "CTiglError.h"

namespace tigl {

CCPACSLeadingEdgeDevices::CCPACSLeadingEdgeDevices(
    CCPACSControlSurfaces *parent, CTiglUIDManager *uidMgr)
    : generated::CPACSLeadingEdgeDevices(parent, uidMgr) {}

// Returns the total count of trailing edge devices in a configuration
int CCPACSLeadingEdgeDevices::GetleadingEdgeDeviceCount() const {
  return static_cast<int>(m_leadingEdgeDevices.size());
}

// Returns the trailing edge device for a given index.
CCPACSLeadingEdgeDevice &
CCPACSLeadingEdgeDevices::GetleadingEdgeDevice(int index) const {
  index--;
  if (index < 0 || index >= GetleadingEdgeDeviceCount()) {
    throw CTiglError(
        "Invalid index in CCPACSLeadingEdgeDevices::GetleadingEdgeDevice",
        TIGL_INDEX_ERROR);
  }
  return *m_leadingEdgeDevices[index];
}

// Returns the trailing edge device for a given UID.
CCPACSLeadingEdgeDevice &
CCPACSLeadingEdgeDevices::GetleadingEdgeDevice(const std::string &UID) const {
  return *m_leadingEdgeDevices[GetleadingEdgeDeviceIndex(UID) - 1];
}

// Returns the trailing edge device index for a given UID.
int CCPACSLeadingEdgeDevices::GetleadingEdgeDeviceIndex(
    const std::string &UID) const {
  for (int i = 0; i < GetleadingEdgeDeviceCount(); i++) {
    const std::string tmpUID(m_leadingEdgeDevices[i]->GetUID());
    if (tmpUID == UID) {
      return i + 1;
    }
  }

  // UID not there
  throw CTiglError(
      "Invalid UID in CCPACSLeadingEdgeDevices::GetleadingEdgeDeviceIndex",
      TIGL_UID_ERROR);
}

} // namespace tigl
