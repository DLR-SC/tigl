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

#ifndef CCPACSLeadingEdgeDeviceS_H
#define CCPACSLeadingEdgeDeviceS_H

#include "generated/CPACSTrailingEdgeDevices.h"

namespace tigl {

class CCPACSLeadingEdgeDevices : public generated::CPACSTrailingEdgeDevices {
public:
  TIGL_EXPORT CCPACSLeadingEdgeDevices(CCPACSControlSurfaces *parent,
                                       CTiglUIDManager *uidMgr);

  // Returns the total count of trailing edge devices in a configuration
  TIGL_EXPORT int GetTrailingEdgeDeviceCount() const;

  // Returns the trailing edge device for a given index.
  TIGL_EXPORT CCPACSLeadingEdgeDevice &GetTrailingEdgeDevice(int index) const;

  // Returns the trailing edge device for a given UID.
  TIGL_EXPORT CCPACSLeadingEdgeDevice &
  GetTrailingEdgeDevice(const std::string &UID) const;

  // Returns the trailing edge device index for a given UID.
  TIGL_EXPORT int GetTrailingEdgeDeviceIndex(const std::string &UID) const;
};

} // namespace tigl

#endif
