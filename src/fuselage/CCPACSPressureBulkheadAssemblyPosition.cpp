/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#include "CCPACSPressureBulkheadAssemblyPosition.h"

#include <gp_Pln.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopExp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Ax3.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <ShapeFix_ShapeTolerance.hxx>

#include <numeric>

#include "CCPACSFuselage.h"
#include "CCPACSFrame.h"
#include "CCPACSPressureBulkhead.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"
#include "to_string.h"

namespace tigl
{
CCPACSPressureBulkheadAssemblyPosition::CCPACSPressureBulkheadAssemblyPosition(CCPACSPressureBulkheadAssembly* parent,
                                                                               CTiglUIDManager* uidMgr)
    : generated::CPACSPressureBulkheadAssemblyPosition(parent, uidMgr)
    , m_geometry(*this, &CCPACSPressureBulkheadAssemblyPosition::BuildGeometry)
{
}

void CCPACSPressureBulkheadAssemblyPosition::SetFrameUID(const std::string& value)
{
    generated::CPACSPressureBulkheadAssemblyPosition::SetFrameUID(value);
    Invalidate();
}

void CCPACSPressureBulkheadAssemblyPosition::SetPressureBulkheadElementUID(const std::string& value)
{
    generated::CPACSPressureBulkheadAssemblyPosition::SetPressureBulkheadElementUID(value);
    Invalidate();
}

std::string CCPACSPressureBulkheadAssemblyPosition::GetDefaultedUID() const
{
    return m_uID;
}

PNamedShape CCPACSPressureBulkheadAssemblyPosition::GetLoft() const
{
    return PNamedShape(new CNamedShape(GetGeometry(GLOBAL_COORDINATE_SYSTEM), GetDefaultedUID()));
}

TiglGeometricComponentType CCPACSPressureBulkheadAssemblyPosition::GetComponentType() const
{
    return TIGL_COMPONENT_PHYSICAL | TIGL_COMPONENT_PRESSURE_BULKHEAD;
}

void CCPACSPressureBulkheadAssemblyPosition::Invalidate()
{
    m_geometry.clear();
}

TopoDS_Shape CCPACSPressureBulkheadAssemblyPosition::GetGeometry(TiglCoordinateSystem cs) const
{
    if (cs == GLOBAL_COORDINATE_SYSTEM) {
        CTiglTransformation trafo = m_parent->GetParent()->GetParent()->GetTransformationMatrix();
        return trafo.Transform(*m_geometry);
    }
    else {
        return *m_geometry;
    }
}

void CCPACSPressureBulkheadAssemblyPosition::BuildGeometry(TopoDS_Shape& cache) const
{
    CCPACSFrame& frame = m_uidMgr->ResolveObject<CCPACSFrame>(m_frameUID);

    if (frame.GetFramePositions().size() == 1) {
        TopTools_IndexedMapOfShape wireMap;
        TopExp::MapShapes(frame.GetGeometry(true, FUSELAGE_COORDINATE_SYSTEM), TopAbs_WIRE, wireMap);

        if (wireMap.Extent() != 1) {
            throw CTiglError("1D frame geometry should have exactly one wire");
        }

        cache = BRepBuilderAPI_MakeFace(TopoDS::Wire(wireMap(1)));
    }
    else if (frame.GetFramePositions().size() >= 2) {
        CCPACSFuselage& fuselage = *m_parent->GetParent()->GetParent();

        std::vector<gp_Pnt> refPoints;
        for (size_t i = 0; i < frame.GetFramePositions().size(); i++) {
            const CCPACSFuselageStringerFramePosition& framePosition = *frame.GetFramePositions()[i];
            const gp_Pnt refPoint = framePosition.GetRefPoint();
            refPoints.push_back(refPoint);
        }

        // build wire, starting with frame wire
        BRepBuilderAPI_MakeWire wireMaker;
        wireMaker.Add(TopoDS::Wire(frame.GetGeometry(true, FUSELAGE_COORDINATE_SYSTEM)));

        if (!wireMaker.IsDone()) {
            throw CTiglError("Wire generation failed");
        }
        if (wireMaker.Error() != BRepBuilderAPI_WireDone) {
            throw CTiglError("Wire generation failed: " + std_to_string(wireMaker.Error()));
        }

        TopoDS_Wire wire = wireMaker.Wire();

        ShapeAnalysis_Wire analysis;
        analysis.Load(wire);
        if (analysis.CheckSelfIntersection()) {
            throw CTiglError("Wire is self-intersecting. This may be caused by an unfortunate positioning of the frame positions");
        }

        wire = CloseWire(wire);

        BRepBuilderAPI_MakeFace makeFace(wire);
        if (!makeFace.IsDone()) {
            LOG(WARNING) << "Failed to create shape for bulkhead \"" << GetDefaultedUID() << "\", retrying planar one";

            // retry with xz plane put at average ref point
            const gp_Pnt avgRefPoint = std::accumulate(refPoints.begin(), refPoints.end(), gp_Pnt(), [](gp_Pnt a, gp_Pnt b) {
                return gp_Pnt(a.XYZ() + b.XYZ());
            }).XYZ() / static_cast<double>(refPoints.size());
            const gp_Pln plane(avgRefPoint, gp_Dir(1, 0, 0));
            makeFace = BRepBuilderAPI_MakeFace(plane, wire);
            if (!makeFace.IsDone()) {
                throw CTiglError("Face generation failed");
            }
            if (makeFace.Error()) {
                throw CTiglError("Face generation failed: " + std_to_string(makeFace.Error()));
            }
        }

        TopoDS_Face face = makeFace.Face();

        // for some rediculous reason, BRepBuilderAPI_MakeFace alters the tolerance of the underlying wire's edges and vertices,
        // causing subsequent boolean operations to fail (self intersections)
        ShapeFix_ShapeTolerance().SetTolerance(face, Precision::Confusion());

        cache = face;
    }
}

} // namespace tigl
