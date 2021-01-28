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
#include <GeomLib_IsPlanarSurface.hxx>

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

void CCPACSWingCell::TrimSpanwise(GeometryCache& cache,
                                  SpanWiseBorder border,
                                  CCPACSWingCellPositionSpanwise const& positioning,
                                  double tol) const
{
    if (!positioning.GetSpanwiseContourCoordinate_choice1()){
        throw CTiglError("Internal Error when trying to create the cell geometry");
    }

    double trim_coordinate = positioning.GetSpanwiseContourCoordinate_choice1().get();

    for(auto row = cache.rgsurface.Root(); row; row = row->UNext()) {
        for (auto current = row; current; current = current->VNext()) {
            if (current->GetAnnotation().keep) {
                if (trim_coordinate <= current->GetAnnotation().scmin) {
                    if (border == SpanWiseBorder::Outer) {
                        // remove this face from the rgsurface
                        current->GetAnnotation().keep = false;
                    }
                }
                else if (trim_coordinate >= current->GetAnnotation().scmax) {
                    if (border == SpanWiseBorder::Inner) {
                        // remove this face from the rgsurface
                        current->GetAnnotation().keep = false;
                    }
                }
                else {
                    //trim the face

                    double alpha = (trim_coordinate - current->GetAnnotation().scmin) / (current->GetAnnotation().scmax - current->GetAnnotation().scmin);
                    double v = (1-alpha)*current->VMin() + alpha * current->VMax();

                    double vmin = current->VMin();
                    double vmax = current->VMax();
                    bool trim = false;
                    if (border == SpanWiseBorder::Inner) {
                        if ( fabs(v - vmin) > tol ) {
                            vmin = v;
                            current->GetAnnotation().scmin = trim_coordinate;
                            trim = true;
                        }
                    }
                    else {
                        if ( fabs(vmax - v) > tol ) {
                            vmax = v;
                            current->GetAnnotation().scmax = trim_coordinate;
                            trim = true;
                        }
                    }

                    if (trim) {
                        current->ReplaceFace(TrimFace(current->GetFace(),
                                                      current->UMin(),
                                                      current->UMax(),
                                                      vmin,
                                                      vmax));
                    }
                }
            }
        }
    }
}

TopoDS_Shape CCPACSWingCell::CutSpanwise(GeometryCache& cache,
                                         TopoDS_Shape const& loftShape,
                                         SpanWiseBorder border,
                                         CCPACSWingCellPositionSpanwise const& positioning,
                                         gp_Dir const& zRefDir,
                                         double tol) const
{

    // Border not defined by contour cooordinate.
    if (positioning.GetSpanwiseContourCoordinate_choice1()){
        throw CTiglError("Internal Error when trying to create the cell geometry");
    }

    // check if border definition allows trimming along contour coordinate anyway,
    // otherwise use Boolean operations

    gp_Pnt le_point, le_point_proj, te_point, te_point_proj;
    if (border == SpanWiseBorder::Inner ){
        le_point = cache.IBLE;
        le_point_proj = cache.projectedIBLE;
        te_point = cache.IBTE;
        te_point_proj = cache.projectedIBTE;
    }
    else {
        le_point = cache.OBLE;
        le_point_proj = cache.projectedOBLE;
        te_point = cache.OBTE;
        te_point_proj = cache.projectedOBTE;
    }

    auto le_intersect = GetFaceAndUV(loftShape, le_point_proj);
    auto te_intersect = GetFaceAndUV(loftShape, te_point_proj);

    if ( !le_intersect || !te_intersect ){
        throw CTiglError("Cannot associate projected point with (u,v)-coordinates");
    }

    gp_Vec te_to_le = gp_Vec(le_point, te_point).Normalized();
    gp_Ax3 border_axis(le_point, zRefDir ^ te_to_le, te_to_le);

    if ( border == SpanWiseBorder::Outer ){
        border_axis.ZReverse();
        cache.border_outer_ax3 = border_axis;
    }
    else {
        cache.border_inner_ax3 = border_axis;
    }

    TopoDS_Shape result;
    if ( te_intersect->face.IsEqual(le_intersect->face) && fabs(te_intersect->v - le_intersect->v ) < tol ){
        // border runs along an isocurve of a single fac => we can trim

        // trim along v
        double v = le_intersect->v;
        TopoDS_Face face = le_intersect->face;

        Standard_Real umin, umax, vmin, vmax;
        BRepTools::UVBounds(face, umin, umax, vmin, vmax);
        if ( border == SpanWiseBorder::Inner ) {
            if (fabs(v - vmin) > tol ) {
                vmin = v;
            }
        }
        else {
            if (fabs(vmax - v) > tol ) {
                vmax = v;
            }
        }
        TopoDS_Face trimmed_face = TrimFace(face, umin, umax, vmin, vmax);
        result = ReplaceFaceInShape(loftShape, trimmed_face, face);

    } else {
        // border is not an isocurve => we must cut

        TopoDS_Shape cuttingShape;
        if (positioning.GetInputType() == CCPACSWingCellPositionSpanwise::InputType::Rib ) {
            // we can use the cutting plane defined by the rib
            gp_Pln pln;
            TopoDS_Shape ribcutface = GetRibCutGeometry(positioning.GetRib());
            Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(ribcutface));
            GeomLib_IsPlanarSurface surf_check(surf);
            if (surf_check.IsPlanar()) {
                pln = surf_check.Plan();
            }
            else {
                throw tigl::CTiglError("Cannot create cutting plane from RibCutGeometry");
            }
            cuttingShape = BRepBuilderAPI_MakeFace(pln);
        } else {

            double eta_le, eta_te, eta_lim;
            if (border == SpanWiseBorder::Inner ) {
                eta_le = m_etaXsiCache->innerLeadingEdgePoint.eta;
                eta_te = m_etaXsiCache->innerTrailingEdgePoint.eta;
                eta_lim = 0.;
            } else {
                eta_le = m_etaXsiCache->outerLeadingEdgePoint.eta;
                eta_te = m_etaXsiCache->outerTrailingEdgePoint.eta;
                eta_lim = 1.;
            }

            if ( fabs(eta_le - eta_lim) < tol && fabs(eta_te - eta_lim) < Precision::Confusion() ) {
                // if the inner border of the cell is the inner border of the Component segment
                // a cutting plane from the inner border of the WCS is created
                // this is necessary due to cutting precision
                CTiglWingStructureReference wsr(m_parent->GetParent()->GetStructure());
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

    // remove all faces on the "outside" of this boundary
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

void CCPACSWingCell::TrimChordwise(GeometryCache& cache,
                                  ChordWiseBorder border,
                                  CCPACSWingCellPositionChordwise const& positioning,
                                  double tol) const
{
    if (!positioning.GetChordwiseContourCoordinate_choice2()){
        throw CTiglError("Internal Error when trying to create the cell geometry");
    }

    double trim_coordinate = positioning.GetChordwiseContourCoordinate_choice2().get();
    bool upper = (m_parent->GetParent()->GetLoftSide() == UPPER_SIDE);

    for(auto row = cache.rgsurface.Root(); row; row = row->UNext()) {
        for (auto current = row; current; current = current->VNext()) {
            if (current->GetAnnotation().keep) {
                if (trim_coordinate <= current->GetAnnotation().ccmin) {
                    if (border == ChordWiseBorder::TE && upper) {
                        // remove this face from the rgsurface
                        current->GetAnnotation().keep = false;
                    }
                    if (border == ChordWiseBorder::LE && !upper) {
                        // remove this face from the rgsurface
                        current->GetAnnotation().keep = false;
                    }
                }
                else if (trim_coordinate >= current->GetAnnotation().ccmax) {
                    if (border == ChordWiseBorder::LE && upper) {
                        // remove this face from the rgsurface
                        current->GetAnnotation().keep = false;
                    }
                    if (border == ChordWiseBorder::TE && !upper) {
                        // remove this face from the rgsurface
                        current->GetAnnotation().keep = false;
                    }
                }
                else {
                    //trim the face

                    double alpha = (trim_coordinate - current->GetAnnotation().ccmin) / (current->GetAnnotation().ccmax - current->GetAnnotation().ccmin);
                    double u = (1-alpha)*current->UMin() + alpha * current->UMax();

                    double umin = current->UMin();
                    double umax = current->UMax();
                    bool trim = false;
                    if (  (border == ChordWiseBorder::LE &&  upper)
                        ||(border == ChordWiseBorder::TE && !upper)) {
                        if ( fabs(u - umin) > tol ) {
                            umin = u;
                            current->GetAnnotation().ccmin = trim_coordinate;
                            trim = true;
                        }
                    }
                    if (  (border == ChordWiseBorder::TE &&  upper)
                        ||(border == ChordWiseBorder::LE && !upper)) {
                        if ( fabs(umax - u) > tol ) {
                            umax = u;
                            current->GetAnnotation().ccmax = trim_coordinate;
                            trim = true;
                        }
                    }

                    if (trim) {
                        current->ReplaceFace(TrimFace(current->GetFace(),
                                                      umin,
                                                      umax,
                                                      current->VMin(),
                                                      current->VMax()));
                    }
                }
            }
        }
    }
}

TopoDS_Shape CCPACSWingCell::CutChordwise(GeometryCache& cache,
                                          TopoDS_Shape const& loftShape,
                                          ChordWiseBorder border,
                                          CCPACSWingCellPositionChordwise const& positioning,
                                          gp_Dir const& zRefDir,
                                          double tol) const
{
    if (positioning.GetChordwiseContourCoordinate_choice2()){
        throw CTiglError("Internal Error when trying to create the cell geometry");
    }

    gp_Pnt ib_point, ib_point_proj, ob_point, ob_point_proj;
    if (border == ChordWiseBorder::LE ) {
        ib_point = cache.IBLE;
        ib_point_proj = cache.projectedIBLE;
        ob_point = cache.OBLE;
        ob_point_proj = cache.projectedOBLE;
    } else {
        ib_point = cache.IBTE;
        ib_point_proj = cache.projectedIBTE;
        ob_point = cache.OBTE;
        ob_point_proj = cache.projectedOBTE;
    }

    auto ib_intersect = GetFaceAndUV(loftShape, ib_point_proj);
    auto ob_intersect = GetFaceAndUV(loftShape, ob_point_proj);

    if ( !ib_intersect || !ob_intersect ){
        throw CTiglError("Cannot associate projected point with (u,v)-coordinates");
    }

    gp_Vec ib_to_ob = gp_Vec(ib_point, ob_point).Normalized();
    gp_Ax3 border_axis(ib_point, zRefDir ^ ib_to_ob, ib_to_ob);

    if ( border == ChordWiseBorder::LE ){
        border_axis.ZReverse();
        cache.border_le_ax3 = border_axis;
    }
    else {
        cache.border_te_ax3 = border_axis;
    }

    TopoDS_Shape result;
    if ( ib_intersect->face.IsEqual(ob_intersect->face) && fabs(ib_intersect->u - ob_intersect->u ) < tol ){
        // border runs along an isocurve of a single fac => we can trim

        // trim along u
        double u = ib_intersect->u;
        TopoDS_Face face = ib_intersect->face;
        Standard_Real umin, umax, vmin, vmax;
        BRepTools::UVBounds(face, umin, umax, vmin, vmax);
        if ( border == ChordWiseBorder::TE ) {
            if (fabs(u-umin) > tol ){
                umin = u;
            }
        }
        else {
            if (fabs(umax-u) > tol ){
                umax = u;
            }
        }
        TopoDS_Face trimmed_face = TrimFace(face, umin, umax, vmin, vmax);
        result = ReplaceFaceInShape(loftShape, trimmed_face, face);

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

    // remove all faces on the "outside" of this boundary
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
            if ( border == ChordWiseBorder::LE ){
                cache.sparShapeLE = sparShape;
            }
            else {
                cache.sparShapeTE = sparShape;
            }
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

    int use_contour_coordinates = (int)(m_positioningInnerBorder.GetInputType()  == CCPACSWingCellPositionSpanwise::Contour);
    use_contour_coordinates    += (int)(m_positioningOuterBorder.GetInputType()  == CCPACSWingCellPositionSpanwise::Contour);
    use_contour_coordinates    += (int)(m_positioningLeadingEdge.GetInputType()  == CCPACSWingCellPositionChordwise::Contour);
    use_contour_coordinates    += (int)(m_positioningTrailingEdge.GetInputType() == CCPACSWingCellPositionChordwise::Contour);

    if ( use_contour_coordinates > 0 && use_contour_coordinates != 4 ) {
        throw CTiglError("If one boundary of a wing cell is defined using contour coordinates, "
                         "all boundaries must be defined using contour coordinates");
    }

    if ( use_contour_coordinates ) {

        // The skin is composed of a rectangular grid of faces. We need to annotate
        // these faces to trim the correct faces at the correct parameters.

        cache.rgsurface.SetShape(loftShape);

        /*
         *  Step 1/3:
         *
         *  create sums of all parameter ranges for every row (in spanwise v-dir)
         *  and every column (in chordwise u-dir) of the grid of faces composing
         *  the upper skin
         *
         *  This is needed to calculate the contour coordinate range of each face,
         *  so that we can translate between local parameter ranges (u,v)
         *  and contour coordinates.
         *
         */
        std::vector<double> row_ranges(cache.rgsurface.NRows(), 0.);
        std::vector<double> col_ranges(cache.rgsurface.NCols(), 0.);

        int j = 0;
        for (auto row = cache.rgsurface.Root(); row; row = row->VNext()) {
            // iterate cells in spanwise direction: Increment v/row/j
            int i = 0;
            for (auto current = row; current; current = current->UNext()){
                //iterate cells in chordwise direction: Increment u/col/i
                row_ranges[j] += current->UMax() - current->UMin();
                col_ranges[i] += current->VMax() - current->VMin();
                i++;
            }
            j++;
        }

        /*
         *  Step 2/3:
         *
         *  Compute the relative contribution of each face to the total parameter
         *  range in u- and v-direction respectively. These relative contributions
         *  correspond to the contour coordinates spanned by each individual face.
         */

        double spanwise_contour_coordinate = 0;
        // coordinates go from 0 to 1 on upper side, and from -1 to 0 on lower side
        int chordwise_contour_start = m_parent->GetParent()->GetLoftSide() == UPPER_SIDE ? 0 : -1;
        j = 0;
        for (auto row = cache.rgsurface.Root(); row; row = row->VNext()) {
            double chordwise_contour_coordinate = chordwise_contour_start;
            int i = 0;
            for (auto current = row; current; current = current->UNext()){
                current->GetAnnotation().scmin = spanwise_contour_coordinate;
                current->GetAnnotation().scmax = spanwise_contour_coordinate
                        + (current->VMax() - current->VMin())/col_ranges[i];
                spanwise_contour_coordinate = current->GetAnnotation().scmax;

                current->GetAnnotation().ccmin = chordwise_contour_coordinate;
                current->GetAnnotation().ccmax = chordwise_contour_coordinate
                        + (current->UMax() - current->UMin())/row_ranges[j];
                chordwise_contour_coordinate = current->GetAnnotation().ccmax;

                i++;
            }
            j++;
        }

        /*
         *  Step 3/3:
         *
         *  Trim all the faces and store the result
         */
        TrimSpanwise(cache, SpanWiseBorder::Inner, m_positioningInnerBorder, 1e-2);
        TrimSpanwise(cache, SpanWiseBorder::Outer, m_positioningOuterBorder, 1e-2);
        TrimChordwise(cache, ChordWiseBorder::LE, m_positioningLeadingEdge, 1e-2);
        TrimChordwise(cache, ChordWiseBorder::TE, m_positioningTrailingEdge, 1e-2);

        TopoDS_Builder builder;
        TopoDS_Compound resultShape;
        builder.MakeCompound(resultShape);

        for (auto row = cache.rgsurface.Root(); row; row = row->VNext()) {
            for (auto current = row; current; current = current->UNext()) {
                if (current->GetAnnotation().keep == true ) {
                    builder.Add(resultShape, current->GetFace());
                }
            }
        }

        // store the result
        cache.skinGeometry = resultShape;

    }
    else {
        // calculate corner points on chord face and project points on loftShape
        // cache points for later use
        cache.IBLE = wsr.GetPoint(m_etaXsiCache->innerLeadingEdgePoint.eta,
                                  m_etaXsiCache->innerLeadingEdgePoint.xsi,
                                  WING_COORDINATE_SYSTEM);
        cache.projectedIBLE = ProjectPointOnShape(loftShape, cache.IBLE, zRefDir);
        cache.IBTE = wsr.GetPoint(m_etaXsiCache->innerTrailingEdgePoint.eta,
                                  m_etaXsiCache->innerTrailingEdgePoint.xsi,
                                  WING_COORDINATE_SYSTEM);
        cache.projectedIBTE = ProjectPointOnShape(loftShape, cache.IBTE, zRefDir);
        cache.OBLE = wsr.GetPoint(m_etaXsiCache->outerLeadingEdgePoint.eta,
                                  m_etaXsiCache->outerLeadingEdgePoint.xsi,
                                  WING_COORDINATE_SYSTEM);
        cache.projectedOBLE = ProjectPointOnShape(loftShape, cache.OBLE, zRefDir);
        cache.OBTE = wsr.GetPoint(m_etaXsiCache->outerTrailingEdgePoint.eta,
                                  m_etaXsiCache->outerTrailingEdgePoint.xsi,
                                  WING_COORDINATE_SYSTEM);
        cache.projectedOBTE = ProjectPointOnShape(loftShape, cache.OBTE, zRefDir);


        // cut the shape at the cell borders
        TopoDS_Shape resultShape = CutSpanwise(cache, loftShape, SpanWiseBorder::Inner, m_positioningInnerBorder, zRefDir, 1e-2);
        resultShape              = CutSpanwise(cache, resultShape, SpanWiseBorder::Outer, m_positioningOuterBorder, zRefDir, 1e-2);
        resultShape              = CutChordwise(cache, resultShape, ChordWiseBorder::LE, m_positioningLeadingEdge, zRefDir, 1e-2);
        resultShape              = CutChordwise(cache, resultShape, ChordWiseBorder::TE, m_positioningTrailingEdge, zRefDir, 1e-2);

        // store the result
        cache.skinGeometry = resultShape;
    }

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

    gp_Ax3 border_le_ax3 = m_geometryCache->border_le_ax3;
    gp_Vec center_loc = pTest.XYZ() - border_le_ax3.Location().XYZ();

    bool sparTest = false, plainTest = false;

    if (m_positioningLeadingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
        sparTest = PointIsInfrontSparGeometry(border_le_ax3.Direction(), pTest, m_geometryCache->sparShapeLE);
    }
    else {
        plainTest = (center_loc).Dot(border_le_ax3.Direction()) > 0;
    }

    if (plainTest || sparTest) {

        gp_Ax3 border_te_ax3 = m_geometryCache->border_te_ax3;
        center_loc = pTest.XYZ() - border_te_ax3.Location().XYZ();

        sparTest  = false;
        plainTest = false;
        if (m_positioningTrailingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
            sparTest = PointIsInfrontSparGeometry(border_le_ax3.Direction(), pTest, m_geometryCache->sparShapeTE);
        }
        else {
            plainTest = (center_loc).Dot(border_te_ax3.Direction()) > 0;
        }

        if (plainTest || sparTest) {

            gp_Ax3 border_inner_ax3 = m_geometryCache->border_inner_ax3;
            center_loc = pTest.XYZ() - border_inner_ax3.Location().XYZ();

            if ((center_loc).Dot(border_inner_ax3.Direction()) > 0) {

                gp_Ax3 border_outer_ax3 = m_geometryCache->border_outer_ax3;
                center_loc = pTest.XYZ() - border_outer_ax3.Location().XYZ();

                if ((center_loc).Dot(border_outer_ax3.Direction()) > 0) {
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
