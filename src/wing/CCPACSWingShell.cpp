/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSWingShell.h"

#include "CCPACSWingCSStructure.h"
#include "CTiglError.h"
#include "CCPACSWingCell.h"
#include "tiglcommonfunctions.h"

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <GeomLProp_SLProps.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>


namespace tigl
{

CCPACSWingShell::CCPACSWingShell(CCPACSWingCSStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingShell(parent, uidMgr) {}

int CCPACSWingShell::GetCellCount() const
{
    if (m_cells)
        return m_cells->GetCellCount();
    else
        return 0;
}

const CCPACSWingCell& CCPACSWingShell::GetCell(int index) const
{
    return m_cells->GetCell(index);
}

CCPACSWingCell& CCPACSWingShell::GetCell(int index)
{
    // forward call to const method
    return const_cast<CCPACSWingCell&>(static_cast<const CCPACSWingShell&>(*this).GetCell(index));
}

const CCPACSMaterialDefinition& CCPACSWingShell::GetMaterial() const
{
    return m_skin.GetMaterial();
}

CCPACSMaterialDefinition& CCPACSWingShell::GetMaterial()
{
    return m_skin.GetMaterial();
}

const CCPACSWingCSStructure& CCPACSWingShell::GetStructure() const
{
    return *m_parent;
}

CCPACSWingCSStructure& CCPACSWingShell::GetStructure()
{
    return *m_parent;
}

void CCPACSWingShell::Invalidate()
{
    geometryCache = boost::none;
    if (m_cells)
        m_cells->Invalidate();
}

bool CCPACSWingShell::IsValid() const
{
    return geometryCache.is_initialized();
}

void CCPACSWingShell::Update() const
{
    if (geometryCache) {
        return;
    }
    geometryCache.emplace();

    // TODO: build stringer geometry
}

TiglLoftSide CCPACSWingShell::GetLoftSide() const
{
    if (&GetParent()->GetLowerShell() == this)
        return LOWER_SIDE;
    if (&GetParent()->GetUpperShell() == this)
        return UPPER_SIDE;
    throw CTiglError("Cannot determine loft side, this shell is neither lower nor upper shell of parent");
}

bool CCPACSWingShell::SparSegmentsTest(gp_Ax1 nNormal, gp_Pnt nTestPoint, TopoDS_Shape nSparSegments)
{
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(nSparSegments, TopAbs_FACE, faceMap);
    double u_min = 0., u_max = 0., v_min = 0., v_max = 0.;

    // for symmetry
    nTestPoint.SetY(fabs(nTestPoint.Y()));

    for (int f = 1; f <= faceMap.Extent(); f++) {
        TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
        BRepAdaptor_Surface surf(loftFace);
        Handle(Geom_Surface) geomSurf = BRep_Tool::Surface(loftFace);

        BRepTools::UVBounds(TopoDS::Face(faceMap(f)), u_min, u_max, v_min, v_max);

        gp_Pnt startPnt = surf.Value(u_min, v_min + ((v_max - v_min) / 2));
        gp_Pnt endPnt   = surf.Value(u_max, v_min + ((v_max - v_min) / 2));

        // if the U / V direction of the spar plane is changed
        gp_Ax1 a1Test0   = gp_Ax1(startPnt, gp_Vec(startPnt, endPnt));
        gp_Ax1 a1TestZ   = gp_Ax1(startPnt, gp_Vec(gp_Pnt(0., 0., 0.), gp_Pnt(0., 0., 1.)));
        gp_Ax1 a1Test_mZ = gp_Ax1(startPnt, gp_Vec(gp_Pnt(0., 0., 0.), gp_Pnt(0., 0., -1.)));

        if (a1Test0.Angle(a1TestZ) < Radians(20.0) || a1Test0.Angle(a1Test_mZ) < Radians(20.0)) {
            startPnt = surf.Value(u_min + ((u_max - u_min) / 2), v_min);
            endPnt   = surf.Value(u_min + ((u_max - u_min) / 2), v_max);
        }

        // Here it is checked if the stringer is in the Y area of the corresponding spar face

        if (endPnt.Y() > startPnt.Y()) {
            if (startPnt.Y() > nTestPoint.Y() || endPnt.Y() < nTestPoint.Y()) {
                continue;
            }
        }
        else {
            if (startPnt.Y() < fabs(nTestPoint.Y()) || endPnt.Y() > fabs(nTestPoint.Y())) {
                continue;
            }
        }

        // project test point onto the surface
        Handle(ShapeAnalysis_Surface) SA_surf = new ShapeAnalysis_Surface(geomSurf);
        gp_Pnt2d uv                           = SA_surf->ValueOfUV(nTestPoint, 0.0);
        gp_Pnt pTestProj                      = surf.Value(uv.X(), uv.Y());

        GeomLProp_SLProps prop(geomSurf, uv.X(), uv.Y(), 1, 0.01);
        gp_Dir normal = prop.Normal();

        gp_Ax1 planeNormal(pTestProj, normal);

        if (nNormal.Angle(planeNormal) > Radians(90.0)) {
            planeNormal = planeNormal.Reversed();
        }

        gp_Vec vTest = gp_Vec(pTestProj, nTestPoint);

        if (vTest.Magnitude() == 0.) {
            continue;
        }

        gp_Ax1 a1Test = gp_Ax1(pTestProj, vTest);

        if (a1Test.Angle(planeNormal) < Radians(89.0)) {
            return true;
        }
        else {
            continue;
        }
    }

    return false;
}

} // namespace tigl
