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
#include "CTiglError.h"
#include "CBooleanOperTools.h"
#include "CNamedShape.h"
#include "CCutShape.h"
#include "CMergeShapes.h"
#include "CTrimShape.h"
#include "tiglcommonfunctions.h"

#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shell.hxx>
#include <Standard_Version.hxx>
#if OCC_VERSION_HEX < VERSION_HEX_CODE(7,3,0)
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
    PerformIntersections();
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
    _hasComputedIntersections = false;
    _intersections.clear();
    _result.reset();
    _farfield.reset();
}

// Collects the loft (with mirror geometry merged in, if applicable) of every
// non-duct component that makes up the aircraft.
ListPNamedShape CTiglFusePlane::CollectComponentShapes() const
{
    std::vector<CTiglRelativelyPositionedComponent*> allComponents;
    const RelativeComponentContainerType& allRelComps = _myconfig.GetUIDManager().GetRelativeComponents();
    for (RelativeComponentContainerType::const_iterator it = allRelComps.begin(); it != allRelComps.end(); ++it) {
        if (it->second->GetComponentType() != TIGL_COMPONENT_DUCT) {
            allComponents.push_back(it->second);
        }
    }

    ListPNamedShape compShapes;
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

        compShapes.push_back(compShape);
    }
    return compShapes;
}

void CTiglFusePlane::Perform()
{
    if (_hasPerformed) {
        return;
    }

    ListPNamedShape compShapes = CollectComponentShapes();

    if (compShapes.empty()) {
        _result.reset();
    }
    else if (compShapes.size() == 1) {
        _result = compShapes.front();
    }
    else {
        // Fuse all components in a single n-ary Boolean operation. Doing this
        // pairwise/incrementally (fusing one component at a time into a running
        // result) cannot correctly resolve genuine mutual intersections between
        // three or more components (e.g. a fuselage, HTP and VTP that all meet
        // in the same region): a single BOPAlgo pass over all arguments at once
        // is required to consistently resolve such triple intersections.
        TopTools_ListOfShape arguments, tools;
        ListPNamedShape::const_iterator it = compShapes.begin();
        arguments.Append((*it)->Shape());
        for (++it; it != compShapes.end(); ++it) {
            tools.Append((*it)->Shape());
        }

        BRepAlgoAPI_Fuse fuse;
        fuse.SetArguments(arguments);
        fuse.SetTools(tools);
        fuse.Build();
        if (!fuse.IsDone()) {
            throw CTiglError("Error fusing aircraft components", TIGL_ERROR);
        }

        PNamedShape fusedCompound(new CNamedShape(fuse.Shape(), "BOP_FUSE"));
        for (ListPNamedShape::const_iterator cit = compShapes.begin(); cit != compShapes.end(); ++cit) {
            CBooleanOperTools::MapFaceNamesAfterBOP(fuse, *cit, fusedCompound);
        }

        // BRepAlgoAPI_Fuse returns a TopoDS_COMPOUND wrapping the resulting
        // shell(s); rebuild a genuine TopoDS_SOLID from them, as expected
        // by exporters and other API consumers of the fused shape
        BRepBuilderAPI_MakeSolid solidMaker;
        TopTools_IndexedMapOfShape shellMap;
        TopExp::MapShapes(fusedCompound->Shape(), TopAbs_SHELL, shellMap);
        for (int ishell = 1; ishell <= shellMap.Extent(); ++ishell) {
            solidMaker.Add(TopoDS::Shell(shellMap(ishell)));
        }

        PNamedShape fused(new CNamedShape(solidMaker.Solid(), _myconfig.GetUID().c_str()));
        CBooleanOperTools::MapFaceNamesAfterBOP(solidMaker, fusedCompound, fused);
        _result = fused;
    }

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
    }

    if (_result) {
        _result->SetName(_myconfig.GetUID().c_str());
        _result->SetShortName("AIRCRAFT");
    }
    _hasPerformed = true;
}

// Computes the pairwise intersection curves between all components. This is
// used only for visualization/export (CTiglCADExporter, TIGLCreator) and
// plays no role in the fused solid computed by Perform(); it is therefore
// computed lazily; on request, since it is considerably more expensive than
// the fuse itself (O(n^2) section + O(n^3) cut operations in the worst case).
void CTiglFusePlane::PerformIntersections()
{
    if (_hasComputedIntersections) {
        return;
    }

    ListPNamedShape compShapes = CollectComponentShapes();

    // Each curve is cut by every other component's shape, so that only the
    // part not buried inside a third component remains (e.g. the HTP-VTP
    // seam is trimmed away wherever it also lies inside the fuselage) --
    // otherwise these would show up as stray untrimmed wires in the export.
    for (size_t i = 0; i < compShapes.size(); ++i) {
        for (size_t j = i + 1; j < compShapes.size(); ++j) {
            const PNamedShape& shape1 = compShapes[i];
            const PNamedShape& shape2 = compShapes[j];

            BRepAlgoAPI_Section sectionOp(shape1->Shape(), shape2->Shape());
            if (!sectionOp.IsDone()) {
                continue;
            }

            TopoDS_Shape intersection = sectionOp.Shape();
            if (intersection.IsNull()) {
                continue;
            }

            for (size_t k = 0; k < compShapes.size() && !intersection.IsNull(); ++k) {
                if (k == i || k == j) {
                    continue;
                }
                BRepAlgoAPI_Cut cutOp(intersection, compShapes[k]->Shape());
                if (cutOp.IsDone()) {
                    intersection = cutOp.Shape();
                }
            }

            if (intersection.IsNull()) {
                continue;
            }

            PNamedShape intersectionShape(new CNamedShape(intersection, std::string("INT" + shape1->Name() + shape2->Name()).c_str()));
            intersectionShape->SetShortName(std::string("INT" + shape1->ShortName() + shape2->ShortName()).c_str());
            _intersections.push_back(intersectionShape);
        }
    }

    CCPACSFarField& farfield = _myconfig.GetFarField();
    if (farfield.GetType() != NONE && (_mymode == FULL_PLANE_TRIMMED_FF || _mymode == HALF_PLANE_TRIMMED_FF)) {
        PNamedShape ff = farfield.GetLoft();

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

    _hasComputedIntersections = true;
}

} // namespace tigl
