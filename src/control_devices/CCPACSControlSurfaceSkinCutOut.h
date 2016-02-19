/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-19 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef CCPACSCONTROLSURFACESKINCUTOUT_H
#define CCPACSCONTROLSURFACESKINCUTOUT_H

#include "tigl_internal.h"
#include "tixi.h"

#include <string>

namespace tigl
{

class CCPACSControlSurfaceSkinCutOut
{
public:
    TIGL_EXPORT CCPACSControlSurfaceSkinCutOut();
    TIGL_EXPORT ~CCPACSControlSurfaceSkinCutOut();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
                               const std::string& CCPACSControlSurfaceSkinCutOutXPath);
    
    TIGL_EXPORT double xsiInnerBorder() const;
    TIGL_EXPORT double xsiOuterBorder() const;

protected:
    double _xsiInnerBorder, _xsiOuterBorder;
};

}

#endif // CCPACSCONTROLSURFACESKINCUTOUT_H
