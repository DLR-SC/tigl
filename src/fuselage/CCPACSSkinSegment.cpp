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

#include "CCPACSSkinSegment.h"

#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Tool.hxx>
#include <BRepExtrema_ExtCC.hxx>
#include <TopExp.hxx>
#include <BRepTools.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Geom_Surface.hxx>

#include "generated/CPACSSkinSegments.h"
#include "CCPACSFrame.h"
#include "CCPACSFuselageStringer.h"
#include "CTiglError.h"
#include "CCPACSFuselage.h"
#include "CTiglUIDManager.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"

namespace tigl
{
CCPACSSkinSegment::CCPACSSkinSegment(CCPACSSkinSegments* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSkinSegment(parent, uidMgr)
{
}

void CCPACSSkinSegment::Invalidate()
{
    m_borderCache = boost::none;
}

bool CCPACSSkinSegment::Contains(const TopoDS_Face& face)
{
    double u_min = 0., u_max = 0., v_min = 0., v_max = 0.;
    BRepTools::UVBounds(face, u_min, u_max, v_min, v_max);
    Handle(Geom_Surface) testSurf = BRep_Tool::Surface(face);
    gp_Pnt point                  = testSurf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));
    return Contains(point);
}

bool CCPACSSkinSegment::Contains(const TopoDS_Edge& edge)
{
    double u_min = 0., u_max = 0.;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, u_min, u_max);
    gp_Pnt point             = curve->Value(u_min + ((u_max - u_min) / 2));
    return Contains(point);
}

bool CCPACSSkinSegment::Contains(const gp_Pnt& point)
{
    if (!m_borderCache)
        UpdateBorders();

    const double angleLower = Radians(45.);
    const double angleUpper = Radians(89.);

    const BorderCache& c = m_borderCache.value();
    if (gp_Ax1(c.sFrame_sStringer.Location(), gp_Vec(c.sFrame_sStringer.Location(), point)).Angle(c.sFrame_sStringer) >= angleLower) return false;
    if (gp_Ax1(c.sFrame_eStringer.Location(), gp_Vec(c.sFrame_eStringer.Location(), point)).Angle(c.sFrame_eStringer) >= angleLower) return false;
    if (gp_Ax1(c.eFrame_sStringer.Location(), gp_Vec(c.eFrame_sStringer.Location(), point)).Angle(c.eFrame_sStringer) >= angleLower) return false;
    if (gp_Ax1(c.eFrame_eStringer.Location(), gp_Vec(c.eFrame_eStringer.Location(), point)).Angle(c.eFrame_eStringer) >= angleLower) return false;

    const gp_Pnt p1 = (c.sFrame_sStringer.Location().XYZ() + c.eFrame_sStringer.Location().XYZ()) / 2;
    const gp_Pnt p2 = (c.eFrame_sStringer.Location().XYZ() + c.eFrame_eStringer.Location().XYZ()) / 2;
    const gp_Pnt p3 = (c.eFrame_eStringer.Location().XYZ() + c.sFrame_eStringer.Location().XYZ()) / 2;
    const gp_Pnt p4 = (c.sFrame_eStringer.Location().XYZ() + c.sFrame_sStringer.Location().XYZ()) / 2;
    const gp_Pnt p5 = (c.sFrame_sStringer.Location().XYZ() + c.eFrame_eStringer.Location().XYZ()) / 2;

    const TopoDS_Shape fuselageLoft = m_parent->GetParent()->GetParent()->GetParent()->GetLoft()->Shape();
    Bnd_Box boundingBox;
    BRepBndLib::Add(fuselageLoft, boundingBox);
    const double yMid = (boundingBox.CornerMax().Y() + boundingBox.CornerMin().Y()) / 2;
    const gp_Pnt p6(p5.X(), yMid, p5.Z());

    const gp_Ax1 ref1(p1, gp_Vec(p1, p3));
    const gp_Ax1 ref2(p2, gp_Vec(p2, p4));
    const gp_Ax1 ref3(p3, gp_Vec(p3, p1));
    const gp_Ax1 ref4(p4, gp_Vec(p4, p2));
    const gp_Ax1 ref5(p6, gp_Vec(p6, p5));

    if (gp_Ax1(p1, gp_Vec(p1, point)).Angle(ref1) >= angleUpper) return false;
    if (gp_Ax1(p2, gp_Vec(p2, point)).Angle(ref2) >= angleUpper) return false;
    if (gp_Ax1(p3, gp_Vec(p3, point)).Angle(ref3) >= angleUpper) return false;
    if (gp_Ax1(p4, gp_Vec(p4, point)).Angle(ref4) >= angleUpper) return false;

    double maxAngle = 0;
    maxAngle = std::max(maxAngle, gp_Ax1(p6, gp_Vec(p6, c.sFrame_sStringer.Location())).Angle(ref5));
    maxAngle = std::max(maxAngle, gp_Ax1(p6, gp_Vec(p6, c.eFrame_sStringer.Location())).Angle(ref5));
    maxAngle = std::max(maxAngle, gp_Ax1(p6, gp_Vec(p6, c.sFrame_eStringer.Location())).Angle(ref5));
    maxAngle = std::max(maxAngle, gp_Ax1(p6, gp_Vec(p6, c.eFrame_eStringer.Location())).Angle(ref5));

    return gp_Ax1(p6, gp_Vec(p6, point)).Angle(ref5) < maxAngle;
}

void CCPACSSkinSegment::UpdateBorders()
{
    try {
        CCPACSFrame& sFrame               = m_uidMgr->ResolveObject<CCPACSFrame>(m_startFrameUID);
        CCPACSFrame& eFrame               = m_uidMgr->ResolveObject<CCPACSFrame>(m_endFrameUID);
        CCPACSFuselageStringer& sStringer = m_uidMgr->ResolveObject<CCPACSFuselageStringer>(m_startStringerUID);
        CCPACSFuselageStringer& eStringer = m_uidMgr->ResolveObject<CCPACSFuselageStringer>(m_endStringerUID.value());

        m_borderCache  = BorderCache();
        BorderCache& c = *m_borderCache;

        // get the intersection points between stringer and frames
        UpdateBorder(c.sFrame_sStringer, sFrame.GetGeometry(true), sStringer.GetGeometry(true));
        UpdateBorder(c.sFrame_eStringer, sFrame.GetGeometry(true), eStringer.GetGeometry(true));
        UpdateBorder(c.eFrame_sStringer, eFrame.GetGeometry(true), sStringer.GetGeometry(true));
        UpdateBorder(c.eFrame_eStringer, eFrame.GetGeometry(true), eStringer.GetGeometry(true));

        // generate directions to the opposite intersection point
        c.sFrame_sStringer.SetDirection(gp_Vec(c.sFrame_sStringer.Location(), c.eFrame_eStringer.Location()));
        c.sFrame_eStringer.SetDirection(gp_Vec(c.sFrame_eStringer.Location(), c.eFrame_sStringer.Location()));
        c.eFrame_sStringer.SetDirection(gp_Vec(c.eFrame_sStringer.Location(), c.sFrame_eStringer.Location()));
        c.eFrame_eStringer.SetDirection(gp_Vec(c.eFrame_eStringer.Location(), c.sFrame_sStringer.Location()));
    }
    catch (...) {
        m_borderCache = boost::none;
    }
}

void CCPACSSkinSegment::UpdateBorder(gp_Ax1& b, TopoDS_Shape s1, TopoDS_Shape s2)
{
    b.SetLocation(GetIntersectionPoint(s1, s2));
}

gp_Pnt CCPACSSkinSegment::GetIntersectionPoint(TopoDS_Shape s1, TopoDS_Shape s2) const
{
    TopTools_IndexedMapOfShape edgeMap1;
    edgeMap1.Clear();
    TopExp::MapShapes(s1, TopAbs_EDGE, edgeMap1);

    TopTools_IndexedMapOfShape edgeMap2;
    edgeMap2.Clear();
    TopExp::MapShapes(s2, TopAbs_EDGE, edgeMap2);

    for (int i = 1; i <= edgeMap1.Extent(); i++) {
        for (int j = 1; j <= edgeMap2.Extent(); j++) {
            BRepExtrema_ExtCC pG(TopoDS::Edge(edgeMap1(i)), TopoDS::Edge(edgeMap2(j)));
            if (pG.NbExt() == 1) {
                return pG.PointOnE1(1);
            }
        }
    }

    throw CTiglError("No intersection between frame and stringer");
}

} // namespace tigl
