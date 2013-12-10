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
    _hasPerformed = false;
}

void CTiglFusePlane::SetResultMode(TiglFuseResultMode mode)
{
    _mymode = mode;
}

const PNamedShape CTiglFusePlane::NamedShape()
{
    Perform();
    return _result;
}

void CTiglFusePlane::Perform()
{
    if(_hasPerformed) {
        return;
    }

    CTiglUIDManager& uidManager = _myconfig.GetUIDManager();
    CTiglAbstractPhysicalComponent* rootComponent = uidManager.GetRootComponent();
    if(!rootComponent) {
        LOG(ERROR) << "Root component of plane not found. Cannot create fused plane.";
        return;
    }

    std::string rootName = rootComponent->GetUID();
    PNamedShape rootShape    (new CNamedShape(rootComponent->GetLoft()        , rootName.c_str()));
    PNamedShape rootShapeMirr(new CNamedShape(rootComponent->GetMirroredLoft(), rootName.c_str()));

    PNamedShape rootMerged = CMergeShapes(rootShape, rootShapeMirr);

    CTiglAbstractPhysicalComponent::ChildContainerType childs = rootComponent->GetChildren(false);
    CTiglAbstractPhysicalComponent::ChildContainerType::iterator childIt;

    ListCNamedShape childShapes;
    for(childIt = childs.begin(); childIt != childs.end(); ++childIt) {
        CTiglAbstractPhysicalComponent* child = *childIt;
        if(!child) {
            continue;
        }

        PNamedShape childShape    (new CNamedShape(child->GetLoft()        , child->GetUID().c_str()));
        PNamedShape childShapeMirr(new CNamedShape(child->GetMirroredLoft(), child->GetUID().c_str()));

        PNamedShape childMerged = CMergeShapes(childShape, childShapeMirr);
        childShapes.push_back(childMerged);
    }
    CFuseShapes fuser(rootMerged, childShapes);
    _result = fuser.NamedShape();
    _hasPerformed = true;

}

} // namespace tigl
