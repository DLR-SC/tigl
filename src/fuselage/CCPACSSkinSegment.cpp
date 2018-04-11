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

    double x_mn = 0, x_mx = 0, y_mn = 0, y_mx = 0, z_mn = 0, z_mx = 0;
    Bnd_Box boundingBox;
    BRepBndLib::Add(m_parent->GetParent()->GetParent()->GetParent()->GetLoft()->Shape(), boundingBox);
    boundingBox.Get(x_mn, y_mn, z_mn, x_mx, y_mx, z_mx);

    double ym = (y_mx + y_mn) / 2;

    const BorderCache& c = m_borderCache.value();
    gp_Ax1 test1(c.sFrame_sStringer.Location(), gp_Vec(c.sFrame_sStringer.Location(), point));
    if (test1.Angle(c.sFrame_sStringer) < (45.0 * (M_PI / 180.))) {
        gp_Ax1 test2(c.sFrame_eStringer.Location(), gp_Vec(c.sFrame_eStringer.Location(), point));
        if (test2.Angle(c.sFrame_eStringer) < (45.0 * (M_PI / 180.))) {
            gp_Ax1 test3(c.eFrame_sStringer.Location(), gp_Vec(c.eFrame_sStringer.Location(), point));
            if (test3.Angle(c.eFrame_sStringer) < (45.0 * (M_PI / 180.))) {
                gp_Ax1 test4(c.eFrame_eStringer.Location(), gp_Vec(c.eFrame_eStringer.Location(), point));
                if (test4.Angle(c.eFrame_eStringer) < (45.0 * (M_PI / 180.))) {

                    gp_Pnt mPnt1 = (c.sFrame_sStringer.Location().XYZ() + c.eFrame_sStringer.Location().XYZ()) / 2;
                    gp_Pnt mPnt2 = (c.eFrame_sStringer.Location().XYZ() + c.eFrame_eStringer.Location().XYZ()) / 2;
                    gp_Pnt mPnt3 = (c.eFrame_eStringer.Location().XYZ() + c.sFrame_eStringer.Location().XYZ()) / 2;
                    gp_Pnt mPnt4 = (c.sFrame_eStringer.Location().XYZ() + c.sFrame_sStringer.Location().XYZ()) / 2;
                    gp_Pnt mPnt5 = (c.sFrame_sStringer.Location().XYZ() + c.eFrame_eStringer.Location().XYZ()) / 2;
                    gp_Pnt mPnt6(mPnt5.X(), ym, mPnt5.Z());

                    gp_Ax1 ref1(mPnt1, gp_Vec(mPnt1, mPnt3));
                    gp_Ax1 ref2(mPnt2, gp_Vec(mPnt2, mPnt4));
                    gp_Ax1 ref3(mPnt3, gp_Vec(mPnt3, mPnt1));
                    gp_Ax1 ref4(mPnt4, gp_Vec(mPnt4, mPnt2));
                    gp_Ax1 ref5(mPnt6, gp_Vec(mPnt6, mPnt5));

                    gp_Ax1 test5(mPnt1, gp_Vec(mPnt1, point));
                    if (test5.Angle(ref1) < (89.0 * (M_PI / 180.))) {
                        gp_Ax1 test6(mPnt2, gp_Vec(mPnt2, point));
                        if (test6.Angle(ref2) < (89.0 * (M_PI / 180.))) {
                            gp_Ax1 test7(mPnt3, gp_Vec(mPnt3, point));
                            if (test7.Angle(ref3) < (89.0 * (M_PI / 180.))) {
                                gp_Ax1 test8(mPnt4, gp_Vec(mPnt4, point));
                                if (test8.Angle(ref4) < (89.0 * (M_PI / 180.))) {
                                    double maxAngle = 0;

                                    gp_Ax1 a1(mPnt6, gp_Vec(mPnt6, c.sFrame_sStringer.Location()));
                                    if (a1.Angle(ref5) > maxAngle)
                                        maxAngle = a1.Angle(ref5);
                                    gp_Ax1 a2(mPnt6, gp_Vec(mPnt6, c.eFrame_sStringer.Location()));
                                    if (a2.Angle(ref5) > maxAngle)
                                        maxAngle = a2.Angle(ref5);
                                    gp_Ax1 a3(mPnt6, gp_Vec(mPnt6, c.sFrame_eStringer.Location()));
                                    if (a3.Angle(ref5) > maxAngle)
                                        maxAngle = a3.Angle(ref5);
                                    gp_Ax1 a4(mPnt6, gp_Vec(mPnt6, c.eFrame_eStringer.Location()));
                                    if (a4.Angle(ref5) > maxAngle)
                                        maxAngle = a4.Angle(ref5);

                                    gp_Ax1 test9(mPnt6, gp_Vec(mPnt6, point));
                                    if (test9.Angle(ref5) < maxAngle) {
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
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
