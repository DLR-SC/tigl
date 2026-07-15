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

#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
#include <TopTools_ListOfShape.hxx>
#else
#include <BOPCol_ListOfShape.hxx>
#endif


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


void CTiglFusePlane::Perform()
{
    if (_hasPerformed) {
        return;
    }

    std::vector<CTiglRelativelyPositionedComponent*> allComponents;
    const RelativeComponentContainerType& allRelComps = _myconfig.GetUIDManager().GetRelativeComponents();
    for (RelativeComponentContainerType::const_iterator it = allRelComps.begin(); it != allRelComps.end(); ++it) {
        if (it->second->GetComponentType() != TIGL_COMPONENT_DUCT) {
            allComponents.push_back(it->second);
        }
    }

    PNamedShape result;
    for (size_t i = 0; i < allComponents.size(); ++i) {
        CTiglRelativelyPositionedComponent* comp = allComponents[i];

        PNamedShape compShape = comp->GetLoft();
        if (!compShape) {
            continue;
        }

        if (_mymode == FULL_PLANE || _mymode == FULL_PLANE_TRIMMED_FF) {
            PNamedShape mirLoft = comp->GetMirroredLoft();
            if (mirLoft) {
                compShape = CMergeShapes(compShape, mirLoft);
            }
        }

        if (!result) {
            result = compShape;
        }
        else {
            // trim existing intersections with this component
            for (ListPNamedShape::iterator intIt = _intersections.begin(); intIt != _intersections.end(); ++intIt) {
                PNamedShape inters = *intIt;
                if (!inters) {
                    continue;
                }

                TopoDS_Shape sh = inters->Shape();
                if (!sh.IsNull()) {
                    sh = BRepAlgoAPI_Cut(sh, compShape->Shape());
                }
                if (!sh.IsNull()) {
                    inters->SetShape(sh);
                }
            }

            CFuseShapes fuser(result, ListPNamedShape{compShape});
            result = fuser.NamedShape();

            ListPNamedShape::const_iterator it2 = fuser.Intersections().begin();
            for (; it2 != fuser.Intersections().end(); ++it2) {
                if (*it2) {
                    _intersections.push_back(*it2);
                }
            }
        }
    }
    _result = result;

    CCPACSFarField& farfield = _myconfig.GetFarField();
    if (farfield.GetType() != NONE && (_mymode == FULL_PLANE_TRIMMED_FF || _mymode == HALF_PLANE_TRIMMED_FF)) {
        PNamedShape ff = farfield.GetLoft();
        assert(_result);

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
        TopTools_ListOfShape aLS;
#else
        BOPCol_ListOfShape aLS;
#endif
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