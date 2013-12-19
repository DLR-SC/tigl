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
    Invalidate();
}

void CTiglFusePlane::SetResultMode(TiglFuseResultMode mode)
{
    if(mode != _mymode) {
        Invalidate();
        _mymode = mode;
    }
}

const PNamedShape CTiglFusePlane::NamedShape()
{
    Perform();
    return _result;
}

const ListPNamedShape& CTiglFusePlane::SubShapes() {
    Perform();
    return _subShapes;
}


const ListPNamedShape& CTiglFusePlane::Intersections() {
    Perform();
    return _intersections;
}

void CTiglFusePlane::Invalidate()
{
    _hasPerformed = false;
    _subShapes.clear();
    _intersections.clear();
    _result.reset();
}

/**
 * @todo: it would be nice if this algorithm would support
 * some progress bar interface
 */
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
    rootShape->SetShortName(rootShortName.c_str());

    if(_mymode == FULL_PLANE || _mymode == FULL_PLANE_TRIMMED_FF) {
        PNamedShape rootShapeMirr(new CNamedShape(rootComponent->GetMirroredLoft(), rootName.c_str()));
        rootShortName += "M";
        rootShapeMirr->SetShortName(rootShortName.c_str());
        rootShape = CMergeShapes(rootShape, rootShapeMirr);
    }

    CTiglAbstractPhysicalComponent::ChildContainerType childs = rootComponent->GetChildren(false);
    CTiglAbstractPhysicalComponent::ChildContainerType::iterator childIt;

    ListPNamedShape childShapes;
    for(childIt = childs.begin(); childIt != childs.end(); ++childIt) {
        CTiglAbstractPhysicalComponent* child = *childIt;
        if(!child) {
            continue;
        }

        std::string childShortName = MakeShortName(_myconfig, *child);
        PNamedShape childShape    (new CNamedShape(child->GetLoft()        , child->GetUID().c_str()));
        childShape->SetShortName(childShortName.c_str());

        if(_mymode == FULL_PLANE || _mymode == FULL_PLANE_TRIMMED_FF) {
            PNamedShape childShapeMirr(new CNamedShape(child->GetMirroredLoft(), child->GetUID().c_str()));
            childShortName += "M";
            childShapeMirr->SetShortName(childShortName.c_str());
            childShape = CMergeShapes(childShape, childShapeMirr);
        }
        childShapes.push_back(childShape);
    }
    CFuseShapes fuser(rootShape, childShapes);
    _result = fuser.NamedShape();

    // insert trimmed shapes from fusing
    if (fuser.TrimmedParent()) {
        _subShapes.push_back(fuser.TrimmedParent());
    }
    ListPNamedShape::const_iterator it = fuser.TrimmedChilds().begin();
    for(; it != fuser.TrimmedChilds().end(); it++) {
        if(*it) {
            _subShapes.push_back(*it);
        }
    }
    it = fuser.Intersections().begin();
    for(; it != fuser.Intersections().end(); it++) {
        if(*it) {
            _intersections.push_back(*it);
        }
    }

    if(_result) {
        _result->SetName(_myconfig.GetUID().c_str());
        _result->SetShortName("AIRCRAFT");
    }
    _hasPerformed = true;

}

} // namespace tigl
