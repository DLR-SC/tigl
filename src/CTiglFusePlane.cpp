/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-11-26 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglFusePlane.h"

#include "CCPACSConfiguration.h"
#include "CTiglUIDManager.h"
#include "CTiglLogging.h"
#include "CFuseShapes.h"
#include "CCutShape.h"
#include "CMergeShapes.h"

#include <string>

namespace tigl {

CTiglFusePlane::CTiglFusePlane(CCPACSConfiguration& config)
    : _myconfig(config)
{
    _mymode = HALF_PLANE;
}

void CTiglFusePlane::SetResultMode(TiglFuseResultMode mode)
{
    _mymode = mode;
}

void CTiglFusePlane::Perform()
{
    CTiglUIDManager& uidManager = _myconfig.GetUIDManager();
    CTiglAbstractPhysicalComponent* rootComponent = uidManager.GetRootComponent();
    if(!rootComponent) {
        LOG(ERROR) << "Root component of plane not found. Cannot create fused plane.";
        return;
    }

    std::string rootName = rootComponent->GetUID();
    CNamedShape rootShape    (rootComponent->GetLoft()        , rootName.c_str());
    CNamedShape rootShapeMirr(rootComponent->GetMirroredLoft(), rootName.c_str());

    CNamedShape rootMerged = CMergeShapes(rootShape, rootShapeMirr);

    CTiglAbstractPhysicalComponent::ChildContainerType childs = rootComponent->GetChildren(false);
    for(unsigned int ichild = 0; ichild < childs.size(); ++ichild) {

    }

}

} // namespace tigl
