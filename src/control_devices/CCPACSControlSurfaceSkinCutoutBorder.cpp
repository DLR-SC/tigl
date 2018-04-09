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

#include "CCPACSControlSurfaceSkinCutoutBorder.h"

#include "CTiglError.h"

namespace tigl
{

CCPACSControlSurfaceSkinCutoutBorder::CCPACSControlSurfaceSkinCutoutBorder()
{
}

CCPACSControlSurfaceSkinCutoutBorder::~CCPACSControlSurfaceSkinCutoutBorder()
{
}

void CCPACSControlSurfaceSkinCutoutBorder::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    if (tixiGetDoubleElement(tixiHandle, (xpath + "/etaLE").c_str(), &_etaLE) != SUCCESS) {
        throw CTiglError("Missing etaLE element in path: " + xpath + "!", TIGL_OPEN_FAILED);
    }
    
    if (tixiGetDoubleElement(tixiHandle, (xpath + "/etaTE").c_str(), &_etaTE) != SUCCESS) {
        throw CTiglError("Missing etaTE element in path: " + xpath + "!", TIGL_OPEN_FAILED);
    }
}

double CCPACSControlSurfaceSkinCutoutBorder::etaLE() const
{
    return _etaLE;
}

double CCPACSControlSurfaceSkinCutoutBorder::etaTE() const
{
    return _etaTE;
}


} // namespace tigl
