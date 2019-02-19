/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#pragma once

#include "generated/CPACSNacelleSection.h"
#include "CCPACSNacelleProfile.h"
#include "CTiglTransformation.h"
#include "TopoDS_Wire.hxx"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSNacelleSection : public generated::CPACSNacelleSection
{
public:
    TIGL_EXPORT CCPACSNacelleSection(CCPACSNacelleSections* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT const CCPACSNacelleProfile& GetProfile() const;

    //TODO this seems like unneccessary code duplication
    TIGL_EXPORT TopoDS_Wire GetTransformedWire() const;
    TIGL_EXPORT TopoDS_Edge GetTransformedUpperWire() const;
    TIGL_EXPORT TopoDS_Edge GetTransformedLowerWire() const;
    TIGL_EXPORT TopoDS_Edge GetTransformedTrailingEdge() const;

    TIGL_EXPORT double GetChordLength() const;

    TIGL_EXPORT CTiglTransformation GetTransformationMatrix() const;

};

} //namespace tigl
