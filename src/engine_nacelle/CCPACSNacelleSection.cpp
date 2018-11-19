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

#include "CCPACSNacelleSection.h"
#include "CCPACSNacelleProfile.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"

namespace tigl {

TIGL_EXPORT CCPACSNacelleSection::CCPACSNacelleSection(CTiglUIDManager* uidMgr)
   : generated::CPACSNacelleSection(uidMgr)
{};

TIGL_EXPORT TopoDS_Wire CCPACSNacelleSection::GetTransformedWire()
{
    // get untransformed profile wire
    const CCPACSNacelleProfile& profile = m_uidMgr->ResolveObject<CCPACSNacelleProfile>(m_profileUID);
    TopoDS_Shape transformedShape(profile.GetWire());

    // apply polar transformation
    CTiglTransformation trafo = GetTransformationMatrix(m_transformation);
    transformedShape = trafo.Transform(transformedShape);

    return TopoDS::Wire(transformedShape);
}

} //namepsace tigl
