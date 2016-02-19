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

#include "CCPACSControlSurfaceSkinCutOut.h"

#include "CTiglError.h"

#include <string>

namespace tigl
{

CCPACSControlSurfaceSkinCutOut::CCPACSControlSurfaceSkinCutOut()
{
}

CCPACSControlSurfaceSkinCutOut::~CCPACSControlSurfaceSkinCutOut()
{
}

void CCPACSControlSurfaceSkinCutOut::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    std::string tmp = xpath + "/xsiInnerBorder";
    if (tixiGetDoubleElement(tixiHandle, tmp.c_str(), &xsiInnerBorder) != SUCCESS) {
        throw CTiglError("Missing xsiInnerBorder element in path " + xpath + "!", TIGL_OPEN_FAILED);
    }
    
    tmp = xpath + "/xsiOuterBorder";
    if (tixiGetDoubleElement(tixiHandle, tmp.c_str(), &xsiOuterBorder) != SUCCESS) {
        throw CTiglError("Missing xsiOuterBorder element in path " + xpath + "!", TIGL_OPEN_FAILED);
    }
}

} // namespace tigl
