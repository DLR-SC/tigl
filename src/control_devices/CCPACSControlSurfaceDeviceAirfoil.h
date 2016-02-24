/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-24 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef CCPACSCONTROLSURFACEDEVICEAIRFOIL_H
#define CCPACSCONTROLSURFACEDEVICEAIRFOIL_H

#include "tigl_internal.h"
#include "CSharedPtr.h"

#include <string>
#include <tixi.h>

namespace tigl
{

class CCPACSConfiguration;

class CCPACSControlSurfaceDeviceAirfoil
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDeviceAirfoil(CCPACSConfiguration*);
    TIGL_EXPORT ~CCPACSControlSurfaceDeviceAirfoil();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle, const std::string& CCPACSControlSurfaceDeviceAirfoilXPath);

protected:
    std::string _airfoilUID;
    double _rotX; // Rotation around the x axis in degree, default 90
    double _scalZ; // Thickness scaling

private:
    CCPACSConfiguration* _config;
};

typedef CSharedPtr<CCPACSControlSurfaceDeviceAirfoil> CCPACSControlSurfaceDeviceAirfoilPtr;

} // namespace tigl

#endif // CCPACSCONTROLSURFACEDEVICEAIRFOIL_H
