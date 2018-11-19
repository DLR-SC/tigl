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

#include "CCPACSCrossBeamAssemblyPosition.h"

#include <BRepPrimAPI_MakePrism.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <ShapeFix_ShapeTolerance.hxx>

#include "CNamedShape.h"
#include "generated/CPACSStructuralProfile.h"
#include "generated/CPACSSheetList.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CCPACSFrame.h"
#include "CCPACSProfileBasedStructuralElement.h"
#include "CCPACSCrossBeamStrutAssemblyPosition.h"
#include "CCPACSLongFloorBeam.h"
#include "CCPACSLongFloorBeamPosition.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"
//#include "Debugging.h"

namespace tigl
{
CCPACSCrossBeamAssemblyPosition::CCPACSCrossBeamAssemblyPosition(CCPACSCargoCrossBeamsAssembly* parent,
                                                                 CTiglUIDManager* uidMgr)
    : generated::CPACSCrossBeamAssemblyPosition(parent, uidMgr)
    , m_geometry1D(*this, &CCPACSCrossBeamAssemblyPosition::BuildGeometry1D)
    , m_geometry3D(*this, &CCPACSCrossBeamAssemblyPosition::BuildGeometry3D)
    , m_cutGeometry(*this, &CCPACSCrossBeamAssemblyPosition::BuildCutGeometry)
{
}

std::string CCPACSCrossBeamAssemblyPosition::GetDefaultedUID() const
{
    return GetUID();
}

PNamedShape CCPACSCrossBeamAssemblyPosition::GetLoft() const
{
    return PNamedShape( new CNamedShape(GetGeometry(true), GetDefaultedUID()) );
}

TiglGeometricComponentType CCPACSCrossBeamAssemblyPosition::GetComponentType() const
{
    return TIGL_COMPONENT_GENERICSYSTEM;
}

TiglGeometricComponentIntent CCPACSCrossBeamAssemblyPosition::GetComponentIntent() const
{
    return TIGL_INTENT_PHYSICAL | TIGL_INTENT_INNER_STRUCTURE;
}



void CCPACSCrossBeamAssemblyPosition::Invalidate()
{
    m_geometry1D.clear();
    m_geometry3D.clear();
    m_cutGeometry.clear();
}

TopoDS_Shape CCPACSCrossBeamAssemblyPosition::GetGeometry(bool just1DElements, TiglCoordinateSystem cs) const
{
    TopoDS_Shape shape = just1DElements ? *m_geometry1D : *m_geometry3D;
    if (cs == GLOBAL_COORDINATE_SYSTEM) {
        CTiglTransformation trafo = m_parent->GetParent()->GetParent()->GetTransformationMatrix();
        return trafo.Transform(shape);
    }
    else {
        return shape;
    }
}

TopoDS_Shape CCPACSCrossBeamAssemblyPosition::GetCutGeometry(TiglCoordinateSystem cs) const
{
    TopoDS_Shape shape = *m_cutGeometry;
    if (cs == GLOBAL_COORDINATE_SYSTEM) {
        CTiglTransformation trafo = m_parent->GetParent()->GetParent()->GetTransformationMatrix();
        return trafo.Transform(shape);
    }
    else {
        return shape;
    }
}

void CCPACSCrossBeamAssemblyPosition::BuildGeometry1D(TopoDS_Shape& cache) const
{
    BuildGeometry(cache, true);
}

void CCPACSCrossBeamAssemblyPosition::BuildGeometry3D(TopoDS_Shape& cache) const
{
    BuildGeometry(cache, false);
}

void CCPACSCrossBeamAssemblyPosition::BuildGeometry(TopoDS_Shape& cache, bool just1DElements) const
{
    if (just1DElements) {
        //DEBUG_SCOPE(debug);

        // the crossBeam is an edge resulting from the intersection between the face of a frame and a plane
        CCPACSFrame& frame = m_uidMgr->ResolveObject<CCPACSFrame>(m_frameUID);

        const TopoDS_Shape frameGeometry = frame.GetGeometry(true, FUSELAGE_COORDINATE_SYSTEM);
        //debug.addShape(frameGeometry, "frameGeometry");
        const TopoDS_Shape frameFace = BRepBuilderAPI_MakeFace(CloseWire(TopoDS::Wire(frameGeometry)));

        // for some rediculous reason, BRepBuilderAPI_MakeFace alters the tolerance of the underlying wire's edges and vertices,
        // causing subsequent boolean operations to fail (self intersections)
        ShapeFix_ShapeTolerance().SetTolerance(frameFace, Precision::Confusion());
        //debug.addShape(frameFace, "frameFace");

        const TopoDS_Shape cutGeom = GetCutGeometry(FUSELAGE_COORDINATE_SYSTEM);
        //debug.addShape(cutGeom, "cutGeom");
        const TopoDS_Shape cutResult = CutShapes(cutGeom, frameFace);
        //debug.addShape(cutResult, "cutResult");

        TopTools_IndexedMapOfShape typeMap;
        TopExp::MapShapes(cutResult, TopAbs_EDGE, typeMap);
        if (typeMap.IsEmpty())
            throw CTiglError("The cut between the frame geometry and the cross beam plane yielded no edges. Do they really intersect?", TIGL_XML_ERROR);

        // make sure the geometry is a single wire
        const TopoDS_Wire wire = BuildWireFromEdges(cutResult);
        //debug.addShape(wire, "wire");

        cache = wire;
    }
    else {
        // get the profile based structural element for the first position
        CCPACSProfileBasedStructuralElement& structuralElement =
            m_uidMgr->ResolveObject<CCPACSProfileBasedStructuralElement>(GetStructuralElementUID());
        CCPACSStructuralProfile& structuralProfile = m_uidMgr->ResolveObject<CCPACSStructuralProfile>(
                                                         structuralElement.GetStructuralProfileUID_choice1().value());

        TopoDS_Compound compound;
        TopoDS_Builder builder;
        builder.MakeCompound(compound);

        // we have to build the profile, to orientate it, and to sweep it all along the path
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(GetGeometry(true), TopAbs_EDGE, edgeMap);
        for (int i = 1; i <= edgeMap.Extent(); i++) { //for each peace of the crossBeam
            TopTools_IndexedMapOfShape vertexMap;
            TopExp::MapShapes(TopoDS::Edge(edgeMap(i)), TopAbs_VERTEX, vertexMap);
            if (vertexMap.Extent() > 2) {
                throw CTiglError("Error during the crossBeam 3D gereration");
            }

            // get extrem points of the cross beam and order them
            gp_Pnt pnt1 = BRep_Tool::Pnt(TopoDS::Vertex(vertexMap(1)));
            gp_Pnt pnt2 = BRep_Tool::Pnt(TopoDS::Vertex(vertexMap(2)));
            if (pnt1.Y() < pnt2.Y()) {
                std::swap(pnt1, pnt2);
            }

            // create profile plane
            gp_Vec crossBeamVec(pnt1, pnt2);
            gp_Pln profilePlan(gp_Ax3(gp_Pnt(0., 0., 0.), crossBeamVec, gp_Vec(1., 0., 0.)));
            profilePlan.Translate(gp_Vec(pnt1.XYZ()));

            // sweep profile
            TopoDS_Wire profile = structuralProfile.GetSheetList().CreateProfileWire(profilePlan);
            BRepPrimAPI_MakePrism frameShell(profile, crossBeamVec);
            frameShell.Build();
            if (!frameShell.IsDone()) {
                throw CTiglError("Failed to sweep cross beam");
            }
            builder.Add(compound, frameShell.Shape());
        }
        cache = compound;
    }
}

void CCPACSCrossBeamAssemblyPosition::BuildCutGeometry(TopoDS_Shape& cache) const
{
    gp_Ax3 cutAxis(gp_Pnt(0., 0., m_positionZ), gp_Vec(0., 0., 1.), gp_Vec(1., 0., 0.));
    cache = BRepBuilderAPI_MakeFace(gp_Pln(cutAxis));
}

} // namespace tigl
