/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-23 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef CCPACSCONTROLSURFACESKINCUTOUTBORDER_H
#define CCPACSCONTROLSURFACESKINCUTOUTBORDER_H

#include "tigl_internal.h"
#include "CSharedPtr.h"

#include <string>
#include <tixi.h>

namespace tigl
{

class CCPACSControlSurfaceSkinCutoutBorder
{
public:
    TIGL_EXPORT CCPACSControlSurfaceSkinCutoutBorder();
    TIGL_EXPORT ~CCPACSControlSurfaceSkinCutoutBorder();
    
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
                               const std::string & controlSurfaceSkinCutoutBorderXPath);
    
    TIGL_EXPORT double etaLE() const;
    TIGL_EXPORT double etaTE() const;
    
private:
    double _etaLE, _etaTE;
};

typedef CSharedPtr<CCPACSControlSurfaceSkinCutoutBorder> CCPACSControlSurfaceSkinCutoutBorderPtr;

} // namespace tigl

#endif // CCPACSCONTROLSURFACESKINCUTOUTBORDER_H
