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

#include "CCPACSWingCell.h"

#include <cassert>
#include <cmath>
#include <sstream>

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepExtrema_ExtPF.hxx>

#include <GeomConvert.hxx>
#include <CTiglBSplineAlgorithms.h>

#include "generated/TixiHelper.h"
#include "CCPACSWing.h"
#include "CCPACSWingCells.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSWingRibsDefinitions.h"
#include "CCPACSWingRibsPositioning.h"
#include "CCPACSWingSpars.h"
#include "CCPACSWingSparSegments.h"
#include "CCPACSWingSparSegment.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "tigletaxsifunctions.h"
#include "to_string.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

namespace tigl
{

namespace WingCellInternal
{

    // calculates crossproduct (p1-p3)x(p2-p3) (only "z"-value)
    double sign(Point2D p1, Point2D p2, Point2D p3)
    {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

    // checks if point p is in triangle p1-p2-p3
    bool is_in_trian(Point2D p, Point2D p1, Point2D p2, Point2D p3)
    {
        bool s1 = sign(p, p1, p2) > 0.;
        // outer border
        bool s2 = sign(p, p2, p3) > 0.;
        // leading edge
        bool s3 = sign(p, p3, p1) > 0.;
        return (s1 == s2) && (s2 == s3);
    }

    Point2D toPoint2D(EtaXsi etaXsi)
    {
        Point2D p;
        p.x = etaXsi.eta;
        p.y = etaXsi.xsi;
        return p;
    }

    struct IntersectionResult
    {
        TopoDS_Face face;
        double u, v;
    };

    /**
     * @brief ClosestPntOnShapeAlongDir calculates the closest point on a shape to a
     * given input point along a direction.
     * @param shape Input shape
     * @param pnt Input point
     * @param dir Search direction for the closest point
     * @param tol a tolerance for the squared distance of the projected point
     * @return IntersectionResult instance, containing the face containing the closest point
     * together with the (u,v) coordinates of the closest point on that face
     */
    IntersectionResult ClosestPointOnShapeAlongDir(TopoDS_Shape const& shape,
                                                   gp_Pnt const& pnt,
                                                   gp_Dir const& dir,
                                                   double tol = 1e-3)
    {
        //first, find closest point to the shape
        gp_Pnt pntOnShape = ProjectPointOnShape(shape, pnt, dir);

        //next, find a face that contains the point and query the uv coordinates
        TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(pntOnShape);
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
        for (int f = 1; f <= faceMap.Extent(); f++) {
            TopoDS_Face const& face = TopoDS::Face(faceMap(f));

            BRepExtrema_ExtPF proj(v, face);
            for (auto i=1; i<=proj.NbExt(); ++i) {
                if(proj.SquareDistance(i) < tol) {
                    IntersectionResult res;
                    res.face = face;
                    proj.Parameter(i, res.u, res.v);
                    return res;
                }
            }
        }
        throw tigl::CTiglError("Projection onto lofting surface failed");
    }


}

using namespace WingCellInternal;

CCPACSWingCell::CCPACSWingCell(CCPACSWingCells* parentCells, CTiglUIDManager* uidMgr)
    : generated::CPACSWingCell(parentCells, uidMgr)
    , m_etaXsiCache(*this, &CCPACSWingCell::UpdateEtaXsiValues)
    , m_geometryCache(*this, &CCPACSWingCell::BuildSkinGeometry)
{
    Reset();
}

CCPACSWingCell::~CCPACSWingCell()
{
}

void CCPACSWingCell::InvalidateImpl(const boost::optional<std::string>& source) const
{
    m_geometryCache.clear();
    m_etaXsiCache.clear();
}

void CCPACSWingCell::Reset()
{
    m_uID         = "";
    Invalidate();
}

bool CCPACSWingCell::IsConvex() const
{
    Point2D p1, p2, p3, p4;
    // calculate for all 4 edges the relative position of eta/xsi
    p1 = toPoint2D(GetTrailingEdgeInnerPoint());
    p2 = toPoint2D(GetTrailingEdgeOuterPoint());
    p3 = toPoint2D(GetLeadingEdgeOuterPoint());
    p4 = toPoint2D(GetLeadingEdgeInnerPoint());

    // trailing edge
    bool s1 = sign(p3, p1, p2) > 0.;
    // outer border
    bool s2 = sign(p4, p2, p3) > 0.;
    // leading edge
    bool s3 = sign(p1, p3, p4) > 0.;
    // inner border
    bool s4 = sign(p2, p4, p1) > 0.;

    return (s1 == s2) && (s2 == s3) && (s3 == s4);
}

bool CCPACSWingCell::IsInside(double eta, double xsi) const
{
    Point2D p, p1, p2, p3, p4;
    p.x = eta;
    p.y = xsi;

    p1 = toPoint2D(GetTrailingEdgeInnerPoint());
    p2 = toPoint2D(GetTrailingEdgeOuterPoint());
    p3 = toPoint2D(GetLeadingEdgeOuterPoint());
    p4 = toPoint2D(GetLeadingEdgeInnerPoint());

    if (IsConvex()) {
        // calculate for all 4 edges the relative position of eta/xsi

        // trailing edge
        bool s1 = sign(p, p1, p2) > 0.;
        // outer border
        bool s2 = sign(p, p2, p3) > 0.;
        // leading edge
        bool s3 = sign(p, p3, p4) > 0.;
        // inner border
        bool s4 = sign(p, p4, p1) > 0.;

        // this only works if the quadriangle is convex
        return (s1 == s2) && (s2 == s3) && (s3 == s4);
    }
    else {
        // compute windings of nodes
        bool w1 = sign(p4, p1, p2) > 0.;
        bool w2 = sign(p1, p2, p3) > 0.;
        bool w3 = sign(p2, p3, p4) > 0.;
        bool w4 = sign(p3, p4, p1) > 0.;

        // get main winding, if 3 positive one negative -> 3, else 1
        int iwind = (w1 + w2 + w3 + w4);
        if (iwind != 1 && iwind != 3) {
            throw CTiglError("Error in Quadriangle Winding calculation in CCPACSWingCell::IsInside.", TIGL_MATH_ERROR);
        }

        bool winding = (iwind == 3);

        // determine point with w[i] != winding
        if (w1 != winding || w3 != winding) {
            return is_in_trian(p, p1, p3, p4) || is_in_trian(p, p1, p2, p3);
        }
        else if (w2 != winding || w4 != winding) {
            return is_in_trian(p, p2, p1, p4) || is_in_trian(p, p2, p4, p3);
        }
        else {
            throw CTiglError("Error in Quadriangle Winding calculation in CCPACSWingCell::IsInside.", TIGL_MATH_ERROR);
        }
    }
}

void CCPACSWingCell::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& cellXPath)
{
    Reset();
    generated::CPACSWingCell::ReadCPACS(tixiHandle, cellXPath);
}

EtaXsi CCPACSWingCell::GetLeadingEdgeInnerPoint() const
{
    return EtaXsi(m_etaXsiCache->innerLeadingEdgePoint.eta,
                  m_etaXsiCache->innerLeadingEdgePoint.xsi);
}

EtaXsi CCPACSWingCell::GetLeadingEdgeOuterPoint() const
{
    return EtaXsi(m_etaXsiCache->outerLeadingEdgePoint.eta,
                  m_etaXsiCache->outerLeadingEdgePoint.xsi);
}

EtaXsi CCPACSWingCell::GetTrailingEdgeInnerPoint() const
{
    return EtaXsi(m_etaXsiCache->innerTrailingEdgePoint.eta,
                  m_etaXsiCache->innerTrailingEdgePoint.xsi);
}

EtaXsi CCPACSWingCell::GetTrailingEdgeOuterPoint() const
{
    return EtaXsi(m_etaXsiCache->outerTrailingEdgePoint.eta,
                  m_etaXsiCache->outerTrailingEdgePoint.xsi);
}

void CCPACSWingCell::SetLeadingEdgeInnerPoint(double eta1, double xsi1)
{
    m_positioningInnerBorder.SetEta1(eta1);
    m_positioningLeadingEdge.SetXsi1(xsi1);
}

void CCPACSWingCell::SetLeadingEdgeOuterPoint(double eta1, double xsi2)
{
    m_positioningOuterBorder.SetEta1(eta1);
    m_positioningLeadingEdge.SetXsi2(xsi2);
}

void CCPACSWingCell::SetTrailingEdgeInnerPoint(double eta2, double xsi1)
{
    m_positioningInnerBorder.SetEta2(eta2);
    m_positioningTrailingEdge.SetXsi1(xsi1);
}

void CCPACSWingCell::SetTrailingEdgeOuterPoint(double eta2, double xsi2)
{
    m_positioningOuterBorder.SetEta2(eta2);
    m_positioningTrailingEdge.SetXsi2(xsi2);
}

void CCPACSWingCell::SetLeadingEdgeSpar(const std::string& sparUID)
{
    m_positioningLeadingEdge.SetSparUId(sparUID);
}

void CCPACSWingCell::SetTrailingEdgeSpar(const std::string& sparUID)
{
    m_positioningTrailingEdge.SetSparUId(sparUID);
}

void CCPACSWingCell::SetInnerBorderRib(const std::string& ribDefinitionUID, int ribNumber)
{
    m_positioningInnerBorder.SetRib(ribDefinitionUID, ribNumber);
}

void CCPACSWingCell::SetOuterBorderRib(const std::string& ribDefinitionUID, int ribNumber)
{
    m_positioningOuterBorder.SetRib(ribDefinitionUID, ribNumber);
}

CCPACSMaterialDefinition& CCPACSWingCell::GetMaterial()
{
    return m_skin.GetMaterial();
}

const CCPACSMaterialDefinition& CCPACSWingCell::GetMaterial() const
{
    return m_skin.GetMaterial();
}


EtaXsi CCPACSWingCell::computePositioningEtaXsi(const CCPACSWingCellPositionSpanwise& spanwisePos,
                                                                   const CCPACSWingCellPositionChordwise& chordwisePos,
                                                                   bool inner, bool front) const
{
    double eta, xsi;

    if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Eta &&
        chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Xsi) {
        if (front) {
            eta = spanwisePos.GetEta().first;
        }
        else {
            eta = spanwisePos.GetEta().second;
        }
        if (inner) {
            xsi = chordwisePos.GetXsi().first;
        }
        else {
            xsi = chordwisePos.GetXsi().second;
        }
    }
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Eta &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Spar) {
        if (front) {
            eta = spanwisePos.GetEta().first;
        }
        else {
            eta = spanwisePos.GetEta().second;
        }
        // get the spar from the wing structure
        const CCPACSWingCSStructure& structure = m_parent->GetParent()->GetStructure();
        const CCPACSWingSparSegment& spar      = structure.GetSparSegment(chordwisePos.GetSparUId());
        xsi                                    = computeSparXsiValue(CTiglWingStructureReference(structure), spar, eta);
    }
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Rib &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Xsi) {
        std::string ribUid;
        int ribIndex;
        // get the ribs definition from the wing structure reference
        spanwisePos.GetRib(ribUid, ribIndex);
        const CCPACSWingCSStructure& structure         = m_parent->GetParent()->GetStructure();
        const CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(ribUid);
        if (inner) {
            xsi = chordwisePos.GetXsi().first;
        }
        else {
            xsi = chordwisePos.GetXsi().second;
        }
        eta = computeRibEtaValue(CTiglWingStructureReference(structure), ribsDefinition, ribIndex, xsi);
    }
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Rib &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Spar) {
        // get the spar from the wing structure reference
        const CCPACSWingCSStructure& structure = m_parent->GetParent()->GetStructure();
        const CCPACSWingSparSegment& spar      = structure.GetSparSegment(chordwisePos.GetSparUId());
        // get the ribs definition from the wing structure reference
        std::string ribUid;
        int ribIndex;
        spanwisePos.GetRib(ribUid, ribIndex);
        const CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(ribUid);

        tigl::EtaXsi result = computeRibSparIntersectionEtaXsi(CTiglWingStructureReference(structure), ribsDefinition, ribIndex, spar);
        eta                 = result.eta;
        xsi                 = result.xsi;
    }
    else
        throw CTiglError("Unrecognized combination of chordwise and spanwise positionings");

    return EtaXsi(eta, xsi);
}

void CCPACSWingCell::UpdateEtaXsiValues(EtaXsiCache& cache) const
{
    cache.innerLeadingEdgePoint = computePositioningEtaXsi(m_positioningInnerBorder, m_positioningLeadingEdge, true, true);
    cache.outerLeadingEdgePoint = computePositioningEtaXsi(m_positioningOuterBorder, m_positioningLeadingEdge, false, true);
    cache.innerTrailingEdgePoint = computePositioningEtaXsi(m_positioningInnerBorder, m_positioningTrailingEdge, true, false);
    cache.outerTrailingEdgePoint = computePositioningEtaXsi(m_positioningOuterBorder, m_positioningTrailingEdge, false, false);

}

TopoDS_Shape CCPACSWingCell::GetSkinGeometry(TiglCoordinateSystem cs) const
{
    if (cs == GLOBAL_COORDINATE_SYSTEM) {
        return CTiglWingStructureReference(m_parent->GetParent()->GetStructure())
            .GetWing()
            .GetTransformationMatrix()
            .Transform(m_geometryCache->skinGeometry);
    }
    else if (cs == WING_COORDINATE_SYSTEM) {
        return m_geometryCache->skinGeometry;
    }
    else {
        throw CTiglError("Invalid coordinate system passed to CCPACSWingCell::GetSkinGeometry");
    }
}

TopoDS_Shape CCPACSWingCell::CutSpanwise(TopoDS_Shape const& loftShape,
                                         SpanWiseBorder border,
                                         CCPACSWingCellPositionSpanwise const& positioning,
                                         gp_Dir const& zRefDir,
                                         double tol) const
{
    CTiglWingStructureReference wsr(m_parent->GetParent()->GetStructure());

    double eta_le, eta_te, xsi_le, xsi_te;
    if (border == SpanWiseBorder::Inner ) {
        eta_le = m_etaXsiCache->innerLeadingEdgePoint.eta;
        eta_te = m_etaXsiCache->innerTrailingEdgePoint.eta;
        xsi_le = m_etaXsiCache->innerLeadingEdgePoint.xsi;
        xsi_le = m_etaXsiCache->innerTrailingEdgePoint.xsi;
    } else {
        eta_le = m_etaXsiCache->outerLeadingEdgePoint.eta;
        eta_te = m_etaXsiCache->outerTrailingEdgePoint.eta;
        xsi_le = m_etaXsiCache->outerLeadingEdgePoint.xsi;
        xsi_le = m_etaXsiCache->outerTrailingEdgePoint.xsi;
    }

    // get (u,v) coordinates of closest point on loft for leading edge point
    gp_Pnt le_point = wsr.GetPoint(eta_le, xsi_le, WING_COORDINATE_SYSTEM);
    auto le_intersect = ClosestPointOnShapeAlongDir(loftShape,
                                                    le_point,
                                                    zRefDir);

    // get (u,v) coordinates of closest point on loft for trailing edge point
    gp_Pnt te_point = wsr.GetPoint(eta_te, xsi_te, WING_COORDINATE_SYSTEM);
    auto te_intersect = ClosestPointOnShapeAlongDir(loftShape,
                                                    te_point,
                                                    zRefDir);

    gp_Vec te_to_le = gp_Vec(le_point, te_point).Normalized();
    gp_Ax3 border_axis(le_point, zRefDir ^ te_to_le, te_to_le);

    if ( border == SpanWiseBorder::Inner ){
        border_axis.ZReverse();
    }

    TopoDS_Shape result;
    if ( te_intersect.face.IsEqual(le_intersect.face) && fabs(te_intersect.v - le_intersect.v ) < tol ){
        // border runs along an isocurve of a single fac => we can trim

        // trim along v
        double v = le_intersect.v;
        TopoDS_Face face = le_intersect.face;
        Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
        Standard_Real umin, umax, vmin, vmax;
        BRepTools::UVBounds(face, umin, umax, vmin, vmax);
        Handle(Geom_BSplineSurface) trimmed_surf;
        if ( border == SpanWiseBorder::Inner ) {
            trimmed_surf = fabs(v - vmin) < tol ? GeomConvert::SurfaceToBSplineSurface(surf)
                                                : CTiglBSplineAlgorithms::trimSurface(surf, umin, umax, v, vmax);
        }
        else {
            trimmed_surf = fabs(v - vmax) < tol ? GeomConvert::SurfaceToBSplineSurface(surf)
                                                : CTiglBSplineAlgorithms::trimSurface(surf, umin, umax, vmin, v);
        }
        TopoDS_Face trimmed_face = BRepBuilderAPI_MakeFace(trimmed_surf, Precision::Confusion());

        // create a compound of all faces of the loft shape, where the original face
        // gets replaced by the trimmed face
        TopoDS_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        builder.Add(compound, trimmed_face);

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(loftShape, TopAbs_FACE, faceMap);
        for (int f = 1; f <= faceMap.Extent(); f++) {
            TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
            if( loftFace == face ) {
                continue;
            }
            builder.Add(compound, loftFace);
        }
        result = compound;

    } else {
        // border is not an isocurve => we must cut

        TopoDS_Shape cuttingShape;
        if (positioning.GetInputType() == CCPACSWingCellPositionSpanwise::InputType::Rib ) {
            // we can use the cutting shape defined by the rib
            cuttingShape = GetRibCutGeometry(positioning.GetRib());
        } else {

            double eta_lim = 0;
            if ( border == SpanWiseBorder::Outer ){
                eta_lim = 1;
            }

            if ( fabs(eta_le - eta_lim) < tol && fabs(eta_te - eta_lim) < Precision::Confusion() ) {
                // if the inner border of the cell is the inner border of the Component segment
                // a cutting plane from the inner border of the WCS is created
                // this is necessary due to cutting precision
                BRepAdaptor_Surface surf(wsr.GetInnerFace());
                gp_Pnt p0 = surf.Value(0.5, 0.0);
                gp_Pnt pU = surf.Value(0.5, 1.0);
                gp_Pnt pV = surf.Value(1.0, 0.0);
                gp_Ax3 ax0UV(p0, gp_Vec(p0, pU) ^ gp_Vec(p0, pV), gp_Vec(p0, pU));

                cuttingShape = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
            } else {
                gp_Pln cutPlaneIB   = gp_Pln(border_axis);
                cuttingShape = BRepBuilderAPI_MakeFace(cutPlaneIB).Face();
            }
        }

        result = SplitShape(loftShape, cuttingShape);
    }

    TopoDS_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(result, TopAbs_FACE, faceMap);
    for (int f = 1; f <= faceMap.Extent(); f++) {
        TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
        gp_Pnt faceCenter = GetCentralFacePoint(loftFace);
        gp_Vec center_loc = faceCenter.XYZ() - border_axis.Location().XYZ();
        if ( (center_loc).Dot(border_axis.Direction()) > 0 ) {
            builder.Add(compound, loftFace);
        }
    }

    return compound;
}

TopoDS_Shape CCPACSWingCell::CutChordwise(TopoDS_Shape const& loftShape,
                                          ChordWiseBorder border,
                                          CCPACSWingCellPositionChordwise const& positioning,
                                          gp_Dir const& zRefDir,
                                          double tol) const
{


    CTiglWingStructureReference wsr(m_parent->GetParent()->GetStructure());

    double eta_inner, eta_outer, xsi_inner, xsi_outer;
    if (border == ChordWiseBorder::LE ) {
        eta_inner = m_etaXsiCache->innerLeadingEdgePoint.eta;
        eta_outer = m_etaXsiCache->outerLeadingEdgePoint.eta;
        xsi_inner = m_etaXsiCache->innerLeadingEdgePoint.xsi;
        xsi_outer = m_etaXsiCache->outerLeadingEdgePoint.xsi;
    } else {
        eta_inner = m_etaXsiCache->innerTrailingEdgePoint.eta;
        eta_outer = m_etaXsiCache->outerTrailingEdgePoint.eta;
        xsi_inner = m_etaXsiCache->innerTrailingEdgePoint.xsi;
        xsi_outer = m_etaXsiCache->outerTrailingEdgePoint.xsi;
    }

    // get (u,v) coordinates of closest point on loft for leading edge point
    gp_Pnt ib_point = wsr.GetPoint(eta_inner, xsi_inner, WING_COORDINATE_SYSTEM);
    auto ib_intersect = ClosestPointOnShapeAlongDir(loftShape,
                                                    ib_point,
                                                    zRefDir);

    // get (u,v) coordinates of closest point on loft for trailing edge point
    gp_Pnt ob_point = wsr.GetPoint(eta_outer, xsi_outer, WING_COORDINATE_SYSTEM);
    auto ob_intersect = ClosestPointOnShapeAlongDir(loftShape,
                                                    ob_point,
                                                    zRefDir);
    gp_Vec ib_to_ob = gp_Vec(ib_point, ob_point).Normalized();
    gp_Ax3 border_axis(ib_point, zRefDir ^ ib_to_ob, ib_to_ob);

    if ( border == ChordWiseBorder::TE ){
        border_axis.ZReverse();
    }

    TopoDS_Shape result;
    if ( ib_intersect.face.IsEqual(ob_intersect.face) && fabs(ib_intersect.u - ob_intersect.u ) < tol ){
        // border runs along an isocurve of a single fac => we can trim

        // trim along v
        double u = ib_intersect.u;
        TopoDS_Face face = ib_intersect.face;
        Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
        Standard_Real umin, umax, vmin, vmax;
        BRepTools::UVBounds(face, umin, umax, vmin, vmax);
        Handle(Geom_BSplineSurface) trimmed_surf;
        if ( border == ChordWiseBorder::TE ) {
            trimmed_surf = fabs(u - umin) < tol ? GeomConvert::SurfaceToBSplineSurface(surf)
                                                : CTiglBSplineAlgorithms::trimSurface(surf, u, umax, vmin, vmax);
        }
        else {
            trimmed_surf = fabs(u - umax) < tol ? GeomConvert::SurfaceToBSplineSurface(surf)
                                                : CTiglBSplineAlgorithms::trimSurface(surf, umin, u, vmin, vmax);
        }
        TopoDS_Face trimmed_face = BRepBuilderAPI_MakeFace(trimmed_surf, Precision::Confusion());

        // create a compound of all faces of the loft shape, where the original face
        // gets replaced by the trimmed face
        TopoDS_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        builder.Add(compound, trimmed_face);

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(loftShape, TopAbs_FACE, faceMap);
        for (int f = 1; f <= faceMap.Extent(); f++) {
            TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
            if( loftFace == face ) {
                continue;
            }
            builder.Add(compound, loftFace);
        }
        result = compound;

    } else {
        // border is not an isocurve => we must cut

        TopoDS_Shape cuttingShape;
        if (positioning.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar ) {
            // we can use the cutting shape defined by the spar
            cuttingShape = m_uidMgr->ResolveObject<CCPACSWingSparSegment>(m_positioningLeadingEdge.GetSparUId())
                    .GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        } else {

            gp_Pln cutPlane   = gp_Pln(border_axis);
            cuttingShape = BRepBuilderAPI_MakeFace(cutPlane).Face();
        }

        result = SplitShape(loftShape, cuttingShape);
    }

    TopoDS_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(result, TopAbs_FACE, faceMap);
    for (int f = 1; f <= faceMap.Extent(); f++) {
        TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
        gp_Pnt faceCenter = GetCentralFacePoint(loftFace);

        bool keep_face = false;
        if ( positioning.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar ) {
            TopoDS_Shape sparShape  = m_uidMgr->ResolveObject<CCPACSWingSparSegment>(positioning.GetSparUId())
                              .GetSparGeometry(WING_COORDINATE_SYSTEM);
            keep_face = PointIsInfrontSparGeometry(border_axis.Direction(), faceCenter, sparShape);
        }
        else {
            gp_Vec center_loc = faceCenter.XYZ() - border_axis.Location().XYZ();
            keep_face = (center_loc).Dot(border_axis.Direction()) > 0;
        }
        if (keep_face) {
            builder.Add(compound, loftFace);
        }
    }
    return compound;
}

void CCPACSWingCell::BuildSkinGeometry(GeometryCache& cache) const
{
    const CTiglWingStructureReference wsr(m_parent->GetParent()->GetStructure());
    const gp_Pnt p1 = wsr.GetLeadingEdgePoint(0);
    const gp_Pnt p2 = wsr.GetLeadingEdgePoint(1);

    // create a reference direction without sweep angle
    const gp_Pnt p2stern = gp_Pnt(p1.X(), p2.Y(), p2.Z());
    const gp_Vec yRefDirStern = gp_Vec(p1, p2stern).Normalized();

    const gp_Pnt tePoint0 = wsr.GetTrailingEdgePoint(0);

    const gp_Vec xRefDir(p1, tePoint0);
    const gp_Vec zRefDir = gp_Vec(-yRefDirStern ^ xRefDir).Normalized();

    // get the shape of the skin
    TopoDS_Shape loftShape = m_parent->GetParent()->GetLoftSide() == UPPER_SIDE ? wsr.GetUpperShape() : wsr.GetLowerShape();

    // cut the shape at the cell borders
    TopoDS_Shape resultShape = CutSpanwise(loftShape, SpanWiseBorder::Inner, m_positioningInnerBorder, zRefDir, 1e-2);
    resultShape              = CutSpanwise(resultShape, SpanWiseBorder::Outer, m_positioningOuterBorder, zRefDir, 1e-2);
    resultShape              = CutChordwise(resultShape, ChordWiseBorder::LE, m_positioningLeadingEdge, zRefDir, 1e-2);
    resultShape              = CutChordwise(resultShape, ChordWiseBorder::TE, m_positioningTrailingEdge, zRefDir, 1e-2);

    // store the result
    cache.skinGeometry = resultShape;
}

TopoDS_Shape CCPACSWingCell::GetRibCutGeometry(std::pair<std::string, int> ribUidAndIndex) const
{
    if (!m_uidMgr) {
        throw CTiglError("No uid manager in CCPACSWingCell::GetRibCutGeometry");
    }

    const CCPACSWingRibsDefinition& ribsDefinition = m_uidMgr->ResolveObject<CCPACSWingRibsDefinition>(ribUidAndIndex.first);
    const CCPACSWingRibsDefinition::CutGeometry& cutGeometry = ribsDefinition.GetRibCutGeometry(ribUidAndIndex.second);

    // obtain plane from rib cut geometry, in case this is already the final face
    if (cutGeometry.isTargetFace) {
        BRepAdaptor_Surface surf(cutGeometry.shape);
        return BRepBuilderAPI_MakeFace(surf.Plane()).Face();
    }
    return cutGeometry.shape;
}

std::string CCPACSWingCell::GetDefaultedUID() const
{
    return GetUID();
}

PNamedShape CCPACSWingCell::GetLoft() const
{
    return PNamedShape(new CNamedShape(GetSkinGeometry(GLOBAL_COORDINATE_SYSTEM), GetUID()));
}

TiglGeometricComponentType CCPACSWingCell::GetComponentType() const
{
    return TIGL_COMPONENT_WINGCELL;
}

TiglGeometricComponentIntent CCPACSWingCell::GetComponentIntent() const
{
    return TIGL_INTENT_LOGICAL;
}


} // namespace tigl
