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

#include "CCPACSCrossBeamStrutAssemblyPosition.h"

#include "generated/CPACSStructuralProfile.h"
#include "CCPACSFuselage.h"
#include "CCPACSCrossBeamAssemblyPosition.h"
#include "CCPACSProfileBasedStructuralElement.h"
#include "CCPACSFrame.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakePrism.hxx>

namespace tigl
{
CCPACSCrossBeamStrutAssemblyPosition::CCPACSCrossBeamStrutAssemblyPosition(CCPACSCargoCrossBeamStrutsAssembly* parent,
                                                                           CTiglUIDManager* uidMgr)
    : generated::CPACSCrossBeamStrutAssemblyPosition(parent, uidMgr)
    , m_geometry1D(*this, &CCPACSCrossBeamStrutAssemblyPosition::BuildGeometry1D)
    , m_geometry3D(*this, &CCPACSCrossBeamStrutAssemblyPosition::BuildGeometry3D)
{
}

std::string CCPACSCrossBeamStrutAssemblyPosition::GetDefaultedUID() const
{
    return GetUID();
}

PNamedShape CCPACSCrossBeamStrutAssemblyPosition::GetLoft() const
{
    return PNamedShape(new CNamedShape(GetGeometry(true), GetDefaultedUID()));
}

TiglGeometricComponentType CCPACSCrossBeamStrutAssemblyPosition::GetComponentType() const
{
    return TIGL_COMPONENT_CROSS_BEAM_STRUT;
}

TiglGeometricComponentIntent CCPACSCrossBeamStrutAssemblyPosition::GetComponentIntent() const
{
    return TIGL_INTENT_PHYSICAL | TIGL_INTENT_INNER_STRUCTURE;
}


void CCPACSCrossBeamStrutAssemblyPosition::Invalidate()
{
    m_geometry1D.clear();
    m_geometry3D.clear();
}

TopoDS_Shape CCPACSCrossBeamStrutAssemblyPosition::GetGeometry(bool just1DElements, TiglCoordinateSystem cs) const
{
    TopoDS_Shape shape = just1DElements ? *m_geometry1D : *m_geometry3D;
    if (cs == GLOBAL_COORDINATE_SYSTEM)
        return m_parent->GetParent()->GetParent()->GetTransformationMatrix().Transform(shape);
    else
        return shape;
}

void CCPACSCrossBeamStrutAssemblyPosition::BuildGeometry1D(TopoDS_Shape& cache) const {
    BuildGeometry(cache, true);
}

void CCPACSCrossBeamStrutAssemblyPosition::BuildGeometry3D(TopoDS_Shape& cache) const {
    BuildGeometry(cache, false);
}

void CCPACSCrossBeamStrutAssemblyPosition::BuildGeometry(TopoDS_Shape& cache, bool just1DElements) const
{
    // get the frame geometry
    CCPACSFrame& frame = m_uidMgr->ResolveObject<CCPACSFrame>(m_frameUID);
    const TopoDS_Wire frameWire = TopoDS::Wire(frame.GetGeometry(true, FUSELAGE_COORDINATE_SYSTEM));

    // get the cargoCrossBeam geometry
    CCPACSCrossBeamAssemblyPosition& crossBeam =
        m_uidMgr->ResolveObject<CCPACSCrossBeamAssemblyPosition>(m_crossBeamUID);
    const TopoDS_Wire crossBeamWire = TopoDS::Wire(crossBeam.GetGeometry(true, TiglCoordinateSystem::FUSELAGE_COORDINATE_SYSTEM));

    // create the cutting plane
    const gp_Pln crossBeamCutPlane(gp_Ax3(gp_Pnt(0., m_positionYAtCrossBeam, 0.), gp_Vec(0., -1., 0.), gp_Vec(1., 0., 0.)));
    const TopoDS_Face crossBeamCutFace = BRepBuilderAPI_MakeFace(crossBeamCutPlane);

    gp_Pnt pointOnCrossBeam;
    if (!GetIntersectionPoint(crossBeamCutFace, crossBeamWire, pointOnCrossBeam)) {
        throw tigl::CTiglError(
            "Error in CPACS crossBeamStrut geometry generation.\nNo intersection between Cross beam and Strut",
            TIGL_XML_ERROR);
    }

    // the frame cutting plane is the rotated crossBeam cutting plane bounded at the rotation axis
    // angle reference is the crossBeam (-Y axis) and clockwise (down first), around gR axis (X direction)
    gp_Ax1 rotAx(pointOnCrossBeam, gp_Vec(1., 0., 0.));
    const gp_Pln frameCutPlane = crossBeamCutPlane.Rotated(rotAx, Radians(m_angleX.value_or(0) - 90.));
    const double largerThanAnyFrame = 10000;
    const TopoDS_Face frameCutFace = BRepBuilderAPI_MakeFace(frameCutPlane, -largerThanAnyFrame, largerThanAnyFrame, -largerThanAnyFrame / 2, 0);

    // cut the frame, this yields up to 2 vertices
    const TopoDS_Shape frameIntersections = CutShapes(frameCutFace, frameWire);
    const std::vector<TopoDS_Shape> vertices = GetSubShapes(frameIntersections, TopAbs_VERTEX);

    if (vertices.size() != 1) {
        throw CTiglError("Unexpected number of vertices when cutting plane with frame");
    }

    const gp_Pnt pointOnFrame = BRep_Tool::Pnt(TopoDS::Vertex(vertices[0]));

    if (just1DElements)
        cache = BRepBuilderAPI_MakeEdge(pointOnCrossBeam, pointOnFrame).Shape();
    else {
        CCPACSProfileBasedStructuralElement& structuralElement =
            m_uidMgr->ResolveObject<CCPACSProfileBasedStructuralElement>(GetStructuralElementUID());
        CCPACSStructuralProfile& structuralProfile =
            m_uidMgr->ResolveObject<CCPACSStructuralProfile>(*structuralElement.GetStructuralProfileUID_choice1());

        // create profile plane
        const gp_Vec vector(pointOnCrossBeam, pointOnFrame);
        const gp_Pln profilePlan(gp_Ax3(pointOnCrossBeam, vector, gp_Vec(1., 0., 0.)));

        // sweep profile
        const TopoDS_Wire profile = structuralProfile.GetSheetList().CreateProfileWire(profilePlan);
        BRepPrimAPI_MakePrism frameShell(profile, vector);
        frameShell.Build();
        if (!frameShell.IsDone())
            throw CTiglError("Failed to sweep cross beam");

        cache = frameShell.Shape();
    }
}
} // namespace tigl
