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
#include "CTrimShape.h"
#include "tiglcommonfunctions.h"

#include <BOPCol_ListOfShape.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>


#include <string>
#include <cassert>

namespace tigl
{

CTiglFusePlane::CTiglFusePlane(CCPACSConfiguration& config)
    : _myconfig(config)
{
    _mymode = HALF_PLANE;
    Invalidate();
}

void CTiglFusePlane::SetResultMode(TiglFuseResultMode mode)
{
    if (mode != _mymode) {
        Invalidate();
        _mymode = mode;
    }
}

const PNamedShape CTiglFusePlane::FusedPlane()
{
    Perform();
    return _result;
}



const ListPNamedShape& CTiglFusePlane::Intersections()
{
    Perform();
    return _intersections;
}

const PNamedShape CTiglFusePlane::FarField()
{
    Perform();
    return _farfield;
}

void CTiglFusePlane::Invalidate()
{
    _hasPerformed = false;
    _intersections.clear();
    _result.reset();
    _farfield.reset();
}

PNamedShape CTiglFusePlane::FuseWithChilds(CTiglAbstractPhysicalComponent* parent)
{
    assert(parent != NULL);
    
    PNamedShape parentShape (parent->GetLoft());
    if (_mymode == FULL_PLANE || _mymode == FULL_PLANE_TRIMMED_FF) {
        PNamedShape rootShapeMirr = parent->GetMirroredLoft();
        parentShape = CMergeShapes(parentShape, rootShapeMirr);
    }

    CTiglAbstractPhysicalComponent::ChildContainerType childs = parent->GetChildren(false);
    CTiglAbstractPhysicalComponent::ChildContainerType::iterator childIt;
    
    if (childs.size() == 0) {
        return parentShape;
    }

    ListPNamedShape childShapes;
    for (childIt = childs.begin(); childIt != childs.end(); ++childIt) {
        CTiglAbstractPhysicalComponent* child = *childIt;
        if (!child) {
            continue;
        }

        PNamedShape childShape = FuseWithChilds(child);
        childShapes.push_back(childShape);
    }
    CFuseShapes fuser(parentShape, childShapes);
    PNamedShape result = fuser.NamedShape();

    // trim previous intersections
    ListPNamedShape::iterator intIt = _intersections.begin();
    ListPNamedShape newInts;
    for (; intIt != _intersections.end(); ++intIt) {
        PNamedShape inters = *intIt;
        if (!inters) {
            continue;
        }

        TopoDS_Shape sh = inters->Shape();
        sh = BRepAlgoAPI_Cut(sh, parentShape->Shape());
        if (! sh.IsNull()) {
            inters->SetShape(sh);
            newInts.push_back(inters);
        }
    }
    _intersections = newInts;

    // insert intersections
    ListPNamedShape::const_iterator it = fuser.Intersections().begin();
    for (; it != fuser.Intersections().end(); it++) {
        if (*it) {
            _intersections.push_back(*it);
        }
    }
    
    return result;
}

/**
 * @todo: it would be nice if this algorithm would support
 * some progress bar interface
 */
void CTiglFusePlane::Perform()
{
    if (_hasPerformed) {
        return;
    }

    CTiglUIDManager& uidManager = _myconfig.GetUIDManager();
    CTiglAbstractPhysicalComponent* rootComponent = uidManager.GetRootComponent();
    if (!rootComponent) {
        LOG(ERROR) << "Root component of plane not found. Cannot create fused plane.";
        return;
    }

    _result = FuseWithChilds(rootComponent);

    CCPACSFarField& farfield = _myconfig.GetFarField();
    if (farfield.GetFieldType() != NONE && (_mymode == FULL_PLANE_TRIMMED_FF || _mymode == HALF_PLANE_TRIMMED_FF)) {
        PNamedShape ff = farfield.GetLoft();

        BOPCol_ListOfShape aLS;
        aLS.Append(_result->Shape());
        aLS.Append(ff->Shape());

        BOPAlgo_PaveFiller dsfill;
        dsfill.SetArguments(aLS);
        dsfill.Perform();
        CTrimShape trim1(_result, ff, dsfill, INCLUDE);
        PNamedShape resulttrimmed = trim1.NamedShape();

        CTrimShape trim2(ff, _result, dsfill, EXCLUDE);
        _farfield = trim2.NamedShape();

        _result = resulttrimmed;

        // trim intersections with far field
        ListPNamedShape::iterator intIt = _intersections.begin();
        ListPNamedShape newInts;
        for (; intIt != _intersections.end(); ++intIt) {
            PNamedShape inters = *intIt;
            if (!inters) {
                continue;
            }

            TopoDS_Shape sh = inters->Shape();
            sh = BRepAlgoAPI_Common(sh, ff->Shape());
            if (! sh.IsNull()) {
                inters->SetShape(sh);
                newInts.push_back(inters);
            }
        }
        _intersections = newInts;
    }

    if (_result) {
        _result->SetName(_myconfig.GetUID().c_str());
        _result->SetShortName("AIRCRAFT");
    }
    _hasPerformed = true;

}

} // namespace tigl
