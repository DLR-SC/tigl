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

namespace {
std::string MakeShortName(tigl::CCPACSConfiguration& config, tigl::CTiglAbstractPhysicalComponent& comp){
    if (comp.GetComponentType() & TIGL_COMPONENT_FUSELAGE) {
        unsigned int index = 0;
        for(int i = 1; i <= config.GetFuselageCount(); ++i) {
            tigl::CCPACSFuselage& f = config.GetFuselage(i);
            if(comp.GetUID() == f.GetUID()) {
                index = i;
                break;
            }
        }
        std::stringstream str;
        str << "F" << index;
        return str.str();
    }
    else if (comp.GetComponentType() & TIGL_COMPONENT_WING) {
        unsigned int index = 0;
        for(int i = 1; i <= config.GetWingCount(); ++i) {
            tigl::CCPACSWing& f = config.GetWing(i);
            if(comp.GetUID() == f.GetUID()) {
                index = i;
                break;
            }
        }
        std::stringstream str;
        str << "W" << index;
        return str.str();
    }

    return "UNKNOWN";
}
}

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
    std::string rootShortName = MakeShortName(_myconfig, *rootComponent);
    PNamedShape rootShape    (new CNamedShape(rootComponent->GetLoft()        , rootName.c_str()));
    PNamedShape rootShapeMirr(new CNamedShape(rootComponent->GetMirroredLoft(), rootName.c_str()));
    rootShape->SetShortName(rootShortName.c_str());
    rootShortName += "M";
    rootShapeMirr->SetShortName(rootShortName.c_str());

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
        std::string childShortName = MakeShortName(_myconfig, *child);
        childShape->SetShortName(childShortName.c_str());
        childShortName += "M";
        childShapeMirr->SetShortName(childShortName.c_str());

        PNamedShape childMerged = CMergeShapes(childShape, childShapeMirr);
        childShapes.push_back(childMerged);
    }
    CFuseShapes fuser(rootMerged, childShapes);
    _result = fuser.NamedShape();
    if(_result) {
        _result->SetName(_myconfig.GetUID().c_str());
        _result->SetShortName("AIRCRAFT");
    }
    _hasPerformed = true;

}

} // namespace tigl
