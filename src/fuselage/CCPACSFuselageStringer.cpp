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

#include "CCPACSFuselageStringer.h"

#include <gp_Pln.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <GeomLProp_SLProps.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include "tiglcommonfunctions.h"
#include "CCPACSProfileBasedStructuralElement.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CCPACSFrame.h"
#include "CNamedShape.h"

namespace tigl
{
CCPACSFuselageStringer::CCPACSFuselageStringer(CCPACSStringersAssembly* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSStringer(parent, uidMgr)
    , m_geomCache1D(*this, &CCPACSFuselageStringer::BuildGeometry1D)
    , m_geomCache3D(*this, &CCPACSFuselageStringer::BuildGeometry3D)
    , m_cutGeomCache(*this, &CCPACSFuselageStringer::BuildCutGeometry)
{
}

void CCPACSFuselageStringer::Invalidate()
{
    m_geomCache1D.clear();
    m_geomCache3D.clear();
    m_cutGeomCache.clear();
}

TopoDS_Shape CCPACSFuselageStringer::GetGeometry(bool just1DElements, TiglCoordinateSystem cs) const
{
    const TopoDS_Shape shape = just1DElements ? *m_geomCache1D : *m_geomCache3D;
    if (cs == GLOBAL_COORDINATE_SYSTEM) {
        CTiglTransformation trafo = m_parent->GetParent()->GetParent()->GetTransformationMatrix();
        return trafo.Transform(shape);
    }
    else
        return shape;
}

TopoDS_Shape CCPACSFuselageStringer::GetCutGeometry(TiglCoordinateSystem cs) const
{
    TopoDS_Shape shape = *m_cutGeomCache;
    if (cs == TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM) {
        CTiglTransformation trafo = m_parent->GetParent()->GetParent()->GetTransformationMatrix();
        return trafo.Transform(shape);
    }
    else
        return shape;
}


void CCPACSFuselageStringer::BuildGeometry1D(TopoDS_Shape& cache) const
{
    BuildGeometry(cache, true);
}

void CCPACSFuselageStringer::BuildGeometry3D(TopoDS_Shape& cache) const
{
    BuildGeometry(cache, false);
}

void CCPACSFuselageStringer::BuildGeometry(TopoDS_Shape& cache, bool just1DElements) const
{
    if (m_stringerPositions.size() < 2)
        throw CTiglError("Cannot build stringer geometry, less than 2 stringer positions defined in XML",
                         TIGL_XML_ERROR);

    // Build the geometry of the inner structure in 2 steps:
    // 1) path definition (projection on the fuselage)
    // 2) if not just 1D element, build and sweep the profile all along the path

    // -1) place every points in the fuselage loft
    CCPACSFuselage& fuselage  = *m_parent->GetParent()->GetParent();
    std::vector<gp_Lin> pointList;
    for (size_t i = 0; i < m_stringerPositions.size(); i++) {
        pointList.push_back(fuselage.Intersection(*m_stringerPositions[i]));
    }

    // creation of the profile plane
    // the plan axis system is defined by : the location, the normal vector, the x vector. It's a right-handed system
    // the rotation is necessary to have a normal vector tangent to the fuselage
    const gp_Ax1 xAxe(pointList.front().Location(),
                      gp_Dir(1, 0, 0)); // parallel to x Axis => used to rotate the profile plane
    gp_Pln profilePlane = gp_Pln(
        gp_Ax3(pointList.front().Location(), pointList.front().Rotated(xAxe, M_PI / 2.).Direction(), gp_Dir(1, 0, 0)));
    profilePlane.Rotate(gp_Ax1(pointList.front().Location(), pointList.front().Direction()),
                        M_PI / 2.); // correct the orientation of the stringer plane (parallel to the Y-Z reference)

    TopoDS_Compound compound;
    TopoDS_Builder builder;
    builder.MakeCompound(compound);
    for (size_t i = 0; i < pointList.size() - 1; i++) {
        const gp_Pnt p1 = pointList.at(i + 0).Location();
        const gp_Pnt p3 = pointList.at(i + 1).Location();

        // determine the direction of the projection
        const gp_Pnt midPointOnStringer = (p1.XYZ() + p3.XYZ()) / 2;
        const gp_Pnt midPointRefs =
            (m_stringerPositions[i]->GetRefPoint().XYZ() + m_stringerPositions[i + 1]->GetRefPoint().XYZ()) / 2.;
        const gp_Dir interDir(0, midPointOnStringer.Y() - midPointRefs.Y(), midPointOnStringer.Z() - midPointRefs.Z());

        // then, we project the segment on the fuselage, and get the resulting wire
        const TopoDS_Wire path = fuselage.projectParallel(BRepBuilderAPI_MakeEdge(p1, p3).Edge(), interDir);

        if (just1DElements) {
            builder.Add(compound, path);
        }
        else {
            const CCPACSProfileBasedStructuralElement& pbse = m_uidMgr->ResolveObject<CCPACSProfileBasedStructuralElement>(
                m_stringerPositions.front()->GetStructuralElementUID());
            builder.Add(compound, pbse.makeFromWire(path, profilePlane));
        }
    }

    if (just1DElements)
        cache = BuildWireFromEdges(compound); // make sure the geometry is a single wire
    else
        cache = compound;
}

void CCPACSFuselageStringer::BuildCutGeometry(TopoDS_Shape& cache) const
{
    const TopoDS_Shape fuselageLoft = m_parent->GetParent()->GetParent()->GetLoft(FUSELAGE_COORDINATE_SYSTEM)->Shape();

    Bnd_Box fuselageBox;
    BRepBndLib::Add(fuselageLoft, fuselageBox);

    TopTools_IndexedMapOfShape edgeMap;
    TopExp::MapShapes(GetGeometry(true, FUSELAGE_COORDINATE_SYSTEM), TopAbs_EDGE, edgeMap);

    TopoDS_Edge normalEdge;
    BRepBuilderAPI_MakeWire wireBuilder;
    bool stringerOnFace = false;
    gp_Vec normalVector;
    for (int e = 1; e <= edgeMap.Extent(); e++) {
        const TopoDS_Edge& edge  = TopoDS::Edge(edgeMap(e));
        const gp_Pnt pntA = GetFirstPoint(edge);

        //  Loop over all fuselage faces
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(fuselageLoft, TopAbs_FACE, faceMap);

        for (int k = 1; k <= faceMap.Extent(); k++) // check if the pntA is on the surface and then create the normal
        {
            // check if it is there
            Bnd_Box faceBox;
            BRepBndLib::Add(TopoDS::Face(faceMap(k)), faceBox);

            if (!faceBox.IsOut(pntA)) {
                // project the point of the edge on the face
                // face im geom surface umwandeln
                Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(faceMap(k))); //convert Face into Surface
                Handle(ShapeAnalysis_Surface) SA_surf =
                    new ShapeAnalysis_Surface(surf); //convert Surface into ShapeAnalysis-Surface

                // uv koordinaten errechen
                const gp_Pnt2d uv = SA_surf->ValueOfUV(pntA, 0);
                // einen 3d punkt mit diesen uv koords zurueckrechnen
                const gp_Pnt pnt2            = SA_surf->Value(uv.X(), uv.Y());
                const double pointEqualEpsilon = fuselageBox.SquareExtent() * 1e-6;
                if (pntA.IsEqual(pnt2, pointEqualEpsilon)) {
                    stringerOnFace      = true;
                    const gp_Vec normal = gp_Vec(GeomLProp_SLProps(surf, uv.X(), uv.Y(), 1, 0.01).Normal());
                    const double cutPlaneDepth =
                        fuselageBox.SquareExtent() * 1e-6; // fuselageBox.SquareExtent()*1.994e-7 + 0.125;
                    normalEdge = BRepBuilderAPI_MakeEdge(pntA.Translated(-normal * cutPlaneDepth),
                                                         pntA.Translated(normal * cutPlaneDepth));
                    wireBuilder.Add(edge);

                    normalVector = normal;
                    break;
                }
            }
        }
    }

    if (stringerOnFace) {
        // in case of a curve line
        BRepOffsetAPI_MakePipeShell frameShell(wireBuilder.Wire());
        // frameShell.SetMode(gp_Dir(1,0,0)); // to force the profile plane being not twisted (profile stay perpendicular to X axis)
        // frameShell.SetMode(false);
        frameShell.SetMode(gp_Dir(normalVector));
        frameShell.Add(BRepBuilderAPI_MakeWire(normalEdge).Wire());
        frameShell.Build();
        if (!frameShell.IsDone())
            throw CTiglError("Error during the frame cut geometry sweeping");
        cache = frameShell.Shape();
    }
    else {
        TopoDS_Compound empty;
        TopoDS_Builder builder;
        builder.MakeCompound(empty);
        cache = empty;
    }
}
} // namespace tigl
