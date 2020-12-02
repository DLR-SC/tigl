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
#include <IntCurvesFace_ShapeIntersector.hxx>

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

    // calculates the area of a triangle
    double area(Point2D p1, Point2D p2, Point2D p3)
    {
        double area = 0.;
        area += p1.x * (p2.y - p3.y);
        area += p2.x * (p3.y - p1.y);
        area += p3.x * (p1.y - p2.y);

        return fabs(area / 2.);
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

    gp_Pnt pointOnShape(TopoDS_Edge edge)
    {
        BRepAdaptor_Curve curve(edge);
        double u_min = curve.FirstParameter();
        double u_max = curve.LastParameter();

        return curve.Value(u_min + ((u_max - u_min) / 2.));
    }

    gp_Pnt pointOnShape(TopoDS_Face face)
    {
        return GetCentralFacePoint(face);
    }

    Point2D toPoint2D(EtaXsi etaXsi)
    {
        Point2D p;
        p.x = etaXsi.eta;
        p.y = etaXsi.xsi;
        return p;
    }

    gp_Pnt projectCornerPointOntoLoft(const gp_Pnt& p, const gp_Vec& zRefDir, double bboxSize, const TopoDS_Shape& loftShape)
    {
        // build cut edge along z reference axis for intersection points on loft
        const gp_Vec offset = zRefDir * bboxSize;
        const TopoDS_Edge cutEdge = BRepBuilderAPI_MakeEdge(p.Translated(-offset), p.Translated(offset));

        BRepExtrema_DistShapeShape extrema(loftShape, cutEdge, Extrema_ExtFlag_MIN);
        extrema.Perform();
        return extrema.PointOnShape1(1);
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
     * @param start optional starting limit along the search direction
     * @param end optional starting limit along the search direction
     * @return IntersectionResult instance, containing the face containing the closest point
     * together with the (u,v) coordinates of the closest point on that face
     */
    IntersectionResult ClosestPointOnShapeAlongDir(TopoDS_Shape const& shape,
                                                   gp_Pnt const& pnt,
                                                   gp_Dir const& dir,
                                                   double start = std::numeric_limits<double>::min(),
                                                   double end = std::numeric_limits<double>::max())
    {
        IntCurvesFace_ShapeIntersector intersector;
        intersector.Load(shape, Precision::Confusion());
        intersector.PerformNearest(gp_Lin(pnt, dir), start, end);
        if (!intersector.IsDone() || intersector.NbPnt() < 1) {
            LOG(ERROR) << "unable to determine nearest point between face and vertex!";
            throw tigl::CTiglError("unable to determine nearest point between face and vertex!");
        }
        IntersectionResult res;
        res.face = intersector.Face(1);
        res.u = intersector.UParameter(1);
        res.v = intersector.VParameter(1);
        return res;
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
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Eta &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Contour) {
        throw CTiglError("Chordwise Positioning via ContourCoordinate not yet supported");
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
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Rib &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Contour) {
        throw CTiglError("Chordwise Positioning via ContourCoordinate not yet supported");
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
                                         double max_extents,
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
                                                    zRefDir,
                                                    -max_extents,
                                                    max_extents);

    // get (u,v) coordinates of closest point on loft for trailing edge point
    gp_Pnt te_point = wsr.GetPoint(eta_te, xsi_te, WING_COORDINATE_SYSTEM);
    auto te_intersect = ClosestPointOnShapeAlongDir(loftShape,
                                                    te_point,
                                                    zRefDir,
                                                    -max_extents,
                                                    max_extents);

    gp_Vec te_to_le = gp_Vec(le_point, te_point).Normalized();
    gp_Ax3 border_axis(le_point, zRefDir ^ te_to_le, te_to_le);

    if ( border == SpanWiseBorder::Inner ){
        border_axis.ZReverse();
    }

    TopoDS_Shape result;
    if ( (te_intersect.face == le_intersect.face) && fabs(te_intersect.v - le_intersect.v ) < tol ){
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

#ifdef DEBUG
        if ( border == SpanWiseBorder::Inner ) {
       BRepTools::Write(face, "face_untrimmed_inner.brep");
       BRepTools::Write(trimmed_face, "face_trimmed_inner.brep");
        }
        else {
            BRepTools::Write(face, "face_untrimmed_outer.brep");
            BRepTools::Write(trimmed_face, "face_trimmed_outer.brep");
        }
#endif

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

#ifdef DEBUG
    if ( border == SpanWiseBorder::Inner ) {
       BRepTools::Write(compound, "compound_inner.brep");
    }
    else {
       BRepTools::Write(compound, "compound_outer.brep");
    }
#endif
    return compound;
}

TopoDS_Shape CCPACSWingCell::CutChordwise(TopoDS_Shape const& loftShape,
                                          ChordWiseBorder border,
                                          CCPACSWingCellPositionChordwise const& positioning,
                                          gp_Dir const& zRefDir,
                                          double max_extents,
                                          double tol) const
{
    if (positioning.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Contour ) {
        throw CTiglError("Chordwise Positioning via ContourCoordinate not yet supported");
    }
    else {

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
                                                        zRefDir,
                                                        -max_extents,
                                                        max_extents);

        // get (u,v) coordinates of closest point on loft for trailing edge point
        gp_Pnt ob_point = wsr.GetPoint(eta_outer, xsi_outer, WING_COORDINATE_SYSTEM);
#ifdef DEBUG
        BRepTools::Write(loftShape, "loftShape.brep");
#endif
        auto ob_intersect = ClosestPointOnShapeAlongDir(loftShape,
                                                        ob_point,
                                                        zRefDir,
                                                        -max_extents,
                                                        max_extents);

#ifdef DEBUG
        BRepTools::Write(ib_intersect.face, "ib_intersect_face.brep");
        BRepTools::Write(ob_intersect.face, "ob_intersect_face.brep");
#endif

        gp_Vec ib_to_ob = gp_Vec(ib_point, ob_point).Normalized();
        gp_Ax3 border_axis(ib_point, zRefDir ^ ib_to_ob, ib_to_ob);

        if ( border == ChordWiseBorder::TE ){
            border_axis.ZReverse();
        }

        TopoDS_Shape result;
        if ( (ib_intersect.face == ob_intersect.face) && fabs(ib_intersect.u - ob_intersect.u ) < tol ){
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
            gp_Vec center_loc = faceCenter.XYZ() - border_axis.Location().XYZ();
            if ( (center_loc).Dot(border_axis.Direction()) > 0 ) {
                builder.Add(compound, loftFace);
            }
        }
        return compound;

    }
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

    TopoDS_Shape loftShape = m_parent->GetParent()->GetLoftSide() == UPPER_SIDE ? wsr.GetUpperShape() : wsr.GetLowerShape();

    // determine diagonal vector of loft bounding box (e.g. used for size of cut faces)
    Bnd_Box boundingBox;
    BRepBndLib::Add(wsr.GetUpperShape(), boundingBox);
    BRepBndLib::Add(wsr.GetLowerShape(), boundingBox);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    const gp_Vec diagonal(xmax - xmin, ymax - ymin, zmax - zmin);
    const double bboxSize = diagonal.Magnitude();



    // cut at inner and outer border
    TopoDS_Shape resultShape = CutSpanwise(loftShape, SpanWiseBorder::Inner, m_positioningInnerBorder, zRefDir, bboxSize, 1e-2);
    resultShape              = CutSpanwise(resultShape, SpanWiseBorder::Outer, m_positioningOuterBorder, zRefDir, bboxSize, 1e-3);
    resultShape              = CutChordwise(resultShape, ChordWiseBorder::LE, m_positioningLeadingEdge, zRefDir, bboxSize, 1e-2);
    //resultShape              = CutChordwise(resultShape, ChordWiseBorder::TE, m_positioningTrailingEdge, zRefDir, bboxSize, 1e-2);

#ifdef DEBUG
        BRepTools::Write(resultShape, "resultShape.brep");
#endif

    // do the same for m_positioningOuterBorder



    const double LEXsi1 = m_etaXsiCache->innerLeadingEdgePoint.xsi;
    const double LEXsi2 = m_etaXsiCache->outerLeadingEdgePoint.xsi;
    const double TEXsi1 = m_etaXsiCache->innerTrailingEdgePoint.xsi;
    const double TEXsi2 = m_etaXsiCache->outerTrailingEdgePoint.xsi;
    const double IBEta1 = m_etaXsiCache->innerLeadingEdgePoint.eta;
    const double IBEta2 = m_etaXsiCache->innerTrailingEdgePoint.eta;
    const double OBEta1 = m_etaXsiCache->outerLeadingEdgePoint.eta;
    const double OBEta2 = m_etaXsiCache->outerTrailingEdgePoint.eta;

    // Get references to all objects in geometry cache
    gp_Pnt& projectedIBLE = cache.projectedIBLE;
    gp_Pnt& projectedOBLE = cache.projectedOBLE;
    gp_Pnt& projectedIBTE = cache.projectedIBTE;
    gp_Pnt& projectedOBTE = cache.projectedOBTE;
    
    gp_Pln& cutPlaneLE = cache.cutPlaneLE;
    gp_Pln& cutPlaneTE = cache.cutPlaneTE;
    gp_Pln& cutPlaneIB = cache.cutPlaneIB;
    gp_Pln& cutPlaneOB = cache.cutPlaneOB;
    
    TopoDS_Shape& planeShapeLE = cache.planeShapeLE;
    TopoDS_Shape& planeShapeTE = cache.planeShapeTE;
    TopoDS_Shape& planeShapeIB = cache.planeShapeIB;
    TopoDS_Shape& planeShapeOB = cache.planeShapeOB;
    TopoDS_Shape& sparShapeLE  = cache.sparShapeLE;
    TopoDS_Shape& sparShapeTE  = cache.sparShapeTE;

    const gp_Pnt midPlaneIBLE = wsr.GetPoint(IBEta1, LEXsi1, WING_COORDINATE_SYSTEM);
    const gp_Pnt midPlaneOBLE = wsr.GetPoint(OBEta1, LEXsi2, WING_COORDINATE_SYSTEM);
    const gp_Pnt midPlaneIBTE = wsr.GetPoint(IBEta2, TEXsi1, WING_COORDINATE_SYSTEM);
    const gp_Pnt midPlaneOBTE = wsr.GetPoint(OBEta2, TEXsi2, WING_COORDINATE_SYSTEM);

    projectedIBLE = projectCornerPointOntoLoft(midPlaneIBLE, zRefDir, bboxSize, loftShape);
    projectedOBLE = projectCornerPointOntoLoft(midPlaneOBLE, zRefDir, bboxSize, loftShape);
    projectedIBTE = projectCornerPointOntoLoft(midPlaneIBTE, zRefDir, bboxSize, loftShape);
    projectedOBTE = projectCornerPointOntoLoft(midPlaneOBTE, zRefDir, bboxSize, loftShape);

    // check the 3d point coordinates

    if (projectedIBLE.X() > projectedIBTE.X()) {
        throw CTiglError(
            "Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values on the inner border.");
    }
    if (projectedOBLE.X() > projectedOBTE.X()) {
        throw CTiglError(
            "Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values on the outer border.");
    }
    if (projectedIBLE.Y() > projectedOBLE.Y()) {
        throw CTiglError(
            "Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the leading edge.");
    }
    if (projectedIBTE.Y() > projectedOBTE.Y()) {
        throw CTiglError("Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the "
                         "trailing edge.");
    }

    // combine the cell cutting cutPlanes to a compound
    TopoDS_Builder builder;
    TopoDS_Compound cutPlanes;
    builder.MakeCompound(cutPlanes);

    // build the Leading edge cutting plane
    const gp_Vec vCLE = gp_Vec(projectedIBLE, projectedOBLE).Normalized();
    const gp_Ax3 refAxLE((projectedIBLE.XYZ() + projectedOBLE.XYZ()) / 2, -zRefDir ^ vCLE, vCLE);
    cutPlaneLE   = gp_Pln(refAxLE);
    planeShapeLE = BRepBuilderAPI_MakeFace(cutPlaneLE).Face();

    // build the Trailing edge cutting plane
    const gp_Vec vCTE = gp_Vec(projectedIBTE, projectedOBTE).Normalized();
    const gp_Ax3 refAxTE((projectedIBTE.XYZ() + projectedOBTE.XYZ()) / 2, zRefDir ^ vCTE, vCTE);
    cutPlaneTE   = gp_Pln(refAxTE);
    planeShapeTE = BRepBuilderAPI_MakeFace(cutPlaneTE).Face();

    // build the inner border cutting plane
    const gp_Vec vCIB = gp_Vec(projectedIBLE, projectedIBTE).Normalized();
    const gp_Ax3 refAxIB((projectedIBLE.XYZ() + projectedIBTE.XYZ()) / 2, zRefDir ^ vCIB, vCIB);
    cutPlaneIB   = gp_Pln(refAxIB);
    planeShapeIB = BRepBuilderAPI_MakeFace(cutPlaneIB).Face();

    // build the Outer border cutting plane
    const gp_Vec vCOB = gp_Vec(projectedOBLE, projectedOBTE).Normalized();
    const gp_Ax3 refAxOB((projectedOBLE.XYZ() + projectedOBTE.XYZ()) / 2, -zRefDir ^ vCOB, vCOB);
    cutPlaneOB   = gp_Pln(refAxOB);
    planeShapeOB = BRepBuilderAPI_MakeFace(cutPlaneOB).Face();

    if (!m_uidMgr) {
        throw CTiglError("No uid manager in CCPACSWingCell::BuildSkinGeometry");
    }
    
    // If any border is defined by a rib or a spar, the cutting plane is changed
    if (m_positioningLeadingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
        planeShapeLE = m_uidMgr->ResolveObject<CCPACSWingSparSegment>(m_positioningLeadingEdge.GetSparUId())
                          .GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        sparShapeLE  = m_uidMgr->ResolveObject<CCPACSWingSparSegment>(m_positioningLeadingEdge.GetSparUId())
                          .GetSparGeometry(WING_COORDINATE_SYSTEM);
    }

    if (m_positioningTrailingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
        planeShapeTE = m_uidMgr->ResolveObject<CCPACSWingSparSegment>(m_positioningTrailingEdge.GetSparUId())
                          .GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        sparShapeTE  = m_uidMgr->ResolveObject<CCPACSWingSparSegment>(m_positioningTrailingEdge.GetSparUId())
                          .GetSparGeometry(WING_COORDINATE_SYSTEM);
    }

    double u05 = 0.5, u1 = 1., v0 = 0., v1 = 1.;
    if (m_positioningInnerBorder.GetInputType() == CCPACSWingCellPositionSpanwise::InputType::Rib) {
        planeShapeIB = GetRibCutGeometry(m_positioningInnerBorder.GetRib());
    }
    else if (IBEta1 == 0. && IBEta2 == 0.) {
        // if the inner border of the cell is the inner border of the Component segment
        // a cutting plane from the inner border of the WCS is created
        // this is necessary due to cutting precision
        BRepAdaptor_Surface surf(wsr.GetInnerFace());
        gp_Pnt p0 = surf.Value(u05, v0);
        gp_Pnt pU = surf.Value(u05, v1);
        gp_Pnt pV = surf.Value(u1, v0);
        gp_Ax3 ax0UV(p0, gp_Vec(p0, pU) ^ gp_Vec(p0, pV), gp_Vec(p0, pU));

        planeShapeIB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
    }

    if (m_positioningOuterBorder.GetInputType() == CCPACSWingCellPositionSpanwise::InputType::Rib) {
        planeShapeOB = GetRibCutGeometry(m_positioningOuterBorder.GetRib());
    }
    else if (OBEta1 == 1. && OBEta2 == 1.) {
        // if the outer border of the cell is the outer border of the Component segment
        // a cutting plane from the outer border of the WCS is created
        // this is necessary due to cutting precision
        BRepAdaptor_Surface surf(wsr.GetOuterFace());
        gp_Pnt p0 = surf.Value(u05, v0);
        gp_Pnt pU = surf.Value(u05, v1);
        gp_Pnt pV = surf.Value(u1, v0);
        gp_Ax3 ax0UV(p0, gp_Vec(p0, pU) ^ gp_Vec(p0, pV), gp_Vec(p0, pU));

        planeShapeOB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
    }

    builder.Add(cutPlanes, planeShapeLE);
    builder.Add(cutPlanes, planeShapeTE);
    builder.Add(cutPlanes, planeShapeIB);
    builder.Add(cutPlanes, planeShapeOB);

    // cut the lower or upper loft with the cutPlanes
    const TopoDS_Shape result = SplitShape(loftShape, cutPlanes);
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(result, TopAbs_FACE, faceMap);

    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    bool notFound = true;

    for (int f = 1; f <= faceMap.Extent(); f++) {
        const TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
        const gp_Pnt faceCenter = GetCentralFacePoint(loftFace);

        // create each midpoint for the vector basis
        bool test = false;
        if (m_positioningLeadingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
            test = PointIsInfrontSparGeometry(cutPlaneLE.Axis().Direction(), faceCenter, sparShapeLE);
        }
        else {
            // test if the midplane point is behind the leading edge border plane
            // create an Ax1 from the Leading edge plane origin to the midpoint of the current face
            const gp_Pnt midPnt = (projectedIBLE.XYZ() + projectedOBLE.XYZ()) / 2;
            const gp_Vec vTest(midPnt, faceCenter);
            const gp_Ax1 a1Test(midPnt, vTest);
            test = a1Test.Angle(cutPlaneLE.Axis()) < M_PI_2;
        }

        if (test) {
            if (m_positioningTrailingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
                test = PointIsInfrontSparGeometry(cutPlaneTE.Axis().Direction(), faceCenter, sparShapeTE);
            }
            else {
                // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
                const gp_Pnt midPnt = (projectedIBTE.XYZ() + projectedOBTE.XYZ()) / 2;
                const gp_Vec vTest = gp_Vec(midPnt, faceCenter);
                const gp_Ax1 a1Test = gp_Ax1(midPnt, vTest);
                test = a1Test.Angle(cutPlaneTE.Axis()) < M_PI_2;
            }

            if (test) {
                // create an Ax1 from the inner border plane origin to the midpoint of the current face
                const gp_Pnt midPnt = (projectedIBLE.XYZ() + projectedIBTE.XYZ()) / 2;
                const gp_Vec vTest = gp_Vec(midPnt, faceCenter);
                const gp_Ax1 a1Test = gp_Ax1(midPnt, vTest);
                if (a1Test.Angle(cutPlaneIB.Axis()) < M_PI_2) {
                    // create an Ax1 from the outer border plane origin to the midpoint of the current face
                    const gp_Pnt midPnt = (projectedOBLE.XYZ() + projectedOBTE.XYZ()) / 2;
                    const gp_Vec vTest = gp_Vec(midPnt, faceCenter);
                    const gp_Ax1 a1Test = gp_Ax1(midPnt, vTest);
                    if (a1Test.Angle(cutPlaneOB.Axis()) < M_PI_2) {
                        builder.Add(compound, loftFace);
                        notFound = false;
                    }
                }
            }
        }
    }

    if (notFound) {
        throw CTiglError("Can not find a matching edge for cell input CCPACSWingCell::BuildSkinGeometry!");
    }

    cache.skinGeometry = compound;
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

template<class T>
bool CCPACSWingCell::IsPartOfCellImpl(T t)
{
    Bnd_Box bBox1, bBox2;
    BRepBndLib::Add(m_geometryCache->skinGeometry, bBox1);
    TopoDS_Shape t_transformed = CTiglWingStructureReference(m_parent->GetParent()->GetStructure())
                                        .GetWing()
                                        .GetTransformationMatrix()
                                        .Inverted()
                                        .Transform(t);
    BRepBndLib::Add(t_transformed, bBox2);

    if (bBox1.IsOut(bBox2)) {
        return false;
    }

    gp_Pnt pTest = WingCellInternal::pointOnShape(t);

    gp_Pnt midPnt = (m_geometryCache->projectedIBLE.XYZ() + m_geometryCache->projectedOBLE.XYZ()) / 2;
    gp_Vec vTest(midPnt, pTest);
    gp_Ax1 a1Test(midPnt, vTest);

    bool sparTest = false, plainTest = false;

    if (m_positioningLeadingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
        sparTest = PointIsInfrontSparGeometry(m_geometryCache->cutPlaneLE.Axis().Direction(), pTest, m_geometryCache->sparShapeLE);
    }
    else {
        plainTest = a1Test.Angle(m_geometryCache->cutPlaneLE.Axis()) < M_PI_2;
    }

    if (plainTest || sparTest) {
        // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
        midPnt = (m_geometryCache->projectedIBTE.XYZ() + m_geometryCache->projectedOBTE.XYZ()) / 2;
        vTest  = gp_Vec(midPnt, pTest);
        a1Test = gp_Ax1(midPnt, vTest);

        sparTest  = false;
        plainTest = false;
        if (m_positioningTrailingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
            sparTest = PointIsInfrontSparGeometry(m_geometryCache->cutPlaneTE.Axis().Direction(), pTest, m_geometryCache->sparShapeTE);
        }
        else {
            plainTest = a1Test.Angle(m_geometryCache->cutPlaneTE.Axis()) < M_PI_2;
        }

        if (plainTest || sparTest) {
            // create an Ax1 from the inner border plane origin to the midpoint of the current face
            midPnt = (m_geometryCache->projectedIBLE.XYZ() + m_geometryCache->projectedIBTE.XYZ()) / 2;
            vTest  = gp_Vec(midPnt, pTest);
            a1Test = gp_Ax1(midPnt, vTest);
            if (a1Test.Angle(m_geometryCache->cutPlaneIB.Axis()) < M_PI_2) {
                // create an Ax1 from the outer border plane origin to the midpoint of the current face
                midPnt = (m_geometryCache->projectedOBLE.XYZ() + m_geometryCache->projectedOBTE.XYZ()) / 2;
                vTest  = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);
                if (a1Test.Angle(m_geometryCache->cutPlaneOB.Axis()) < M_PI_2) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool CCPACSWingCell::IsPartOfCell(TopoDS_Face f)
{
    return IsPartOfCellImpl(f);
}


bool CCPACSWingCell::IsPartOfCell(TopoDS_Edge e)
{
    return IsPartOfCellImpl(e);
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
