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

#include "CTiglStringerFrameBorderedObject.h"

#include "CCPACSFuselage.h"
#include "CCPACSFrame.h"
#include "CCPACSFuselageStringer.h"
#include "CTiglUIDManager.h"

#include <TopoDS_Compound.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepExtrema_ExtCC.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include "tiglcommonfunctions.h"

namespace tigl
{
CTiglStringerFrameBorderedObject::CTiglStringerFrameBorderedObject(
    const CTiglUIDManager& uidMgr, const CCPACSFuselage& fuselage, std::string& startFrameUID, std::string& endFrameUID,
    std::string& startStringerUID, boost::variant<std::string&, boost::optional<std::string>&> endStringerUID)
    : m_uidMgr(uidMgr)
    , m_fuselage(fuselage)
    , m_startFrameUID(startFrameUID)
    , m_endFrameUID(endFrameUID)
    , m_startStringerUID(startStringerUID)
    , m_endStringerUID(endStringerUID)
{
}

void CTiglStringerFrameBorderedObject::Invalidate()
{
    m_borderCache = boost::none;
    m_geometry    = boost::none;
}

/*
TopoDS_Shape CTiglStringerFrameBorderedObject::GetGeometry(TiglCoordinateSystem referenceCS)
{
    if (!m_geometry)
        BuildGeometry();

    if (referenceCS == GLOBAL_COORDINATE_SYSTEM)
        return m_fuselage.GetTransformationMatrix().Transform(m_geometry.value());
    else
        return m_geometry.value();
}
*/

bool CTiglStringerFrameBorderedObject::Contains(const TopoDS_Face& face)
{
    if (!m_borderCache)
        UpdateBorders();

    // compute center point of face
    double u_min = 0., u_max = 0., v_min = 0., v_max = 0.;
    BRepTools::UVBounds(face, u_min, u_max, v_min, v_max);
    Handle(Geom_Surface) testSurf = BRep_Tool::Surface(face);
    const gp_Pnt center           = testSurf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));

    return Contains(center);
}

bool CTiglStringerFrameBorderedObject::Contains(const TopoDS_Edge& edge)
{
    if (!m_borderCache)
        UpdateBorders();

    // compute center point of edge
    double u_min = 0., u_max = 0.;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, u_min, u_max);
    gp_Pnt center            = curve->Value(u_min + ((u_max - u_min) / 2));

    return Contains(center);
}

bool CTiglStringerFrameBorderedObject::Contains(const gp_Pnt& point)
{
    const double _45DegInRad = Radians(45.0);

    const BorderCache& c = m_borderCache.value();
    gp_Ax1 test1(c.sFrame_sStringer.Location(), gp_Vec(c.sFrame_sStringer.Location(), point));
    if (test1.Angle(c.sFrame_sStringer) < _45DegInRad) {
        gp_Ax1 test2(c.sFrame_eStringer.Location(), gp_Vec(c.sFrame_eStringer.Location(), point));
        if (test2.Angle(c.sFrame_eStringer) < _45DegInRad) {
            gp_Ax1 test3(c.eFrame_sStringer.Location(), gp_Vec(c.eFrame_sStringer.Location(), point));
            if (test3.Angle(c.eFrame_sStringer) < _45DegInRad) {
                gp_Ax1 test4(c.eFrame_eStringer.Location(), gp_Vec(c.eFrame_eStringer.Location(), point));
                if (test4.Angle(c.eFrame_eStringer) < _45DegInRad) {
                    return true;
                }
            }
        }
    }

    return false;
}
/*
void CTiglStringerFrameBorderedObject::BuildGeometry()
{
    if (!m_borderCache)
        UpdateBorders();

    CCPACSFrame& sFrame               = m_uidMgr.ResolveObject<CCPACSFrame>(m_startFrameUID);
    CCPACSFrame& eFrame               = m_uidMgr.ResolveObject<CCPACSFrame>(m_endFrameUID);
    CCPACSFuselageStringer& sStringer = m_uidMgr.ResolveObject<CCPACSFuselageStringer>(m_startStringerUID);
    CCPACSFuselageStringer& eStringer = m_uidMgr.ResolveObject<CCPACSFuselageStringer>(GetEndStringerUid());

    // create cut geometry from stringers and frames
    TopoDS_Builder builder;
    TopoDS_Compound cutCompound;
    builder.MakeCompound(cutCompound);

    builder.Add(cutCompound, sFrame.GetCutGeometry(FUSELAGE_COORDINATE_SYSTEM));
    builder.Add(cutCompound, eFrame.GetCutGeometry(FUSELAGE_COORDINATE_SYSTEM));
    builder.Add(cutCompound, sStringer.GetCutGeometry(FUSELAGE_COORDINATE_SYSTEM));
    builder.Add(cutCompound, eStringer.GetCutGeometry(FUSELAGE_COORDINATE_SYSTEM));

    // split fuselage loft
    const TopoDS_Shape loft         = m_fuselage.GetLoftFromFusedSegments();
    const TopoDS_Shape splittedLoft = SplitShape(loft, cutCompound);

    // find door face
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(splittedLoft, TopAbs_FACE, faceMap);

    const CTiglTransformation& trafo = m_fuselage.GetTransformationMatrix();

    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    for (int i = 1; i <= faceMap.Extent(); i++) {
        const TopoDS_Shape f = faceMap(i);
        if (Contains(TopoDS::Face(trafo.Transform(f)))) {
            builder.Add(compound, f);
        }
    }

    m_geometry = compound;
}
*/

void CTiglStringerFrameBorderedObject::UpdateBorders()
{
    try {
        CCPACSFrame& sFrame               = m_uidMgr.ResolveObject<CCPACSFrame>(m_startFrameUID);
        CCPACSFrame& eFrame               = m_uidMgr.ResolveObject<CCPACSFrame>(m_endFrameUID);
        CCPACSFuselageStringer& sStringer = m_uidMgr.ResolveObject<CCPACSFuselageStringer>(m_startStringerUID);
        CCPACSFuselageStringer& eStringer = m_uidMgr.ResolveObject<CCPACSFuselageStringer>(GetEndStringerUid());

        m_borderCache.emplace();
        BorderCache& c = *m_borderCache;

        // get the intersection Points between stringer and frames
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
        throw;
    }
}

void CTiglStringerFrameBorderedObject::UpdateBorder(gp_Ax1& b, TopoDS_Shape frame, TopoDS_Shape stringer)
{
    intersectionPointList intersections;
    if (GetIntersectionPoint(TopoDS::Wire(frame), TopoDS::Wire(stringer), intersections,
                             std::numeric_limits<double>::max())) {
        b.SetLocation(intersections.front().Center);
        return;
    }

    throw CTiglError("No intersection between frame and stringer");
}

std::string CTiglStringerFrameBorderedObject::GetEndStringerUid() const
{
    struct Visitor : boost::static_visitor<std::string> {
        std::string operator()(const std::string& s)
        {
            return s;
        }
        std::string operator()(const boost::optional<std::string>& s)
        {
            return s.value(); // TODO(bgruber): we do not yet support empty end stringer uids
        }
    } v;
    return m_endStringerUID.apply_visitor(v);
}

} // namespace tigl
