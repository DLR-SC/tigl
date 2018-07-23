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
}

using namespace WingCellInternal;

CCPACSWingCell::CCPACSWingCell(CCPACSWingCells* parentCells, CTiglUIDManager* uidMgr)
    : generated::CPACSWingCell(parentCells, uidMgr)
{
    Reset();
}

CCPACSWingCell::~CCPACSWingCell()
{
}

void CCPACSWingCell::Invalidate()
{
    m_geometryCache = boost::none;
    m_etaXsiCache = boost::none;
}

void CCPACSWingCell::Reset()
{
    m_uID         = "";
    m_geometryCache = boost::none;
    m_etaXsiCache = boost::none;
}

bool CCPACSWingCell::IsConvex() const
{
    Point2D p1, p2, p3, p4;

    // calculate for all 4 edges the relative position of eta/xsi
    GetTrailingEdgeInnerPoint(&p1.x, &p1.y);
    GetTrailingEdgeOuterPoint(&p2.x, &p2.y);
    GetLeadingEdgeOuterPoint(&p3.x, &p3.y);
    GetLeadingEdgeInnerPoint(&p4.x, &p4.y);

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

    GetTrailingEdgeInnerPoint(&p1.x, &p1.y);
    GetTrailingEdgeOuterPoint(&p2.x, &p2.y);
    GetLeadingEdgeOuterPoint(&p3.x, &p3.y);
    GetLeadingEdgeInnerPoint(&p4.x, &p4.y);

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

void CCPACSWingCell::GetLeadingEdgeInnerPoint(double* eta, double* xsi) const
{
    UpdateEtaXsiValues();
    *eta = m_etaXsiCache.value().innerLeadingEdgePoint.eta;
    *xsi = m_etaXsiCache.value().innerLeadingEdgePoint.xsi;
}

void CCPACSWingCell::GetLeadingEdgeOuterPoint(double* eta, double* xsi) const
{
    UpdateEtaXsiValues();
    *eta = m_etaXsiCache.value().outerLeadingEdgePoint.eta;
    *xsi = m_etaXsiCache.value().outerLeadingEdgePoint.xsi;
}

void CCPACSWingCell::GetTrailingEdgeInnerPoint(double* eta, double* xsi) const
{
    UpdateEtaXsiValues();
    *eta = m_etaXsiCache.value().innerTrailingEdgePoint.eta;
    *xsi = m_etaXsiCache.value().innerTrailingEdgePoint.xsi;
}

void CCPACSWingCell::GetTrailingEdgeOuterPoint(double* eta, double* xsi) const
{
    UpdateEtaXsiValues();
    *eta = m_etaXsiCache.value().outerTrailingEdgePoint.eta;
    *xsi = m_etaXsiCache.value().outerTrailingEdgePoint.xsi;
}

void CCPACSWingCell::SetLeadingEdgeInnerPoint(double eta1, double xsi1)
{
    UpdateEtaXsiValues();
    const double eta2 = m_etaXsiCache.value().innerTrailingEdgePoint.eta;
    const double xsi2 = m_etaXsiCache.value().outerLeadingEdgePoint.xsi;
    m_positioningInnerBorder.SetEta(eta1, eta2);
    m_positioningLeadingEdge.SetXsi(xsi1, xsi2);
}

void CCPACSWingCell::SetLeadingEdgeOuterPoint(double eta1, double xsi2)
{
    UpdateEtaXsiValues();
    const double eta2 = m_etaXsiCache.value().outerTrailingEdgePoint.eta;
    const double xsi1 = m_etaXsiCache.value().innerLeadingEdgePoint.xsi;
    m_positioningOuterBorder.SetEta(eta1, eta2);
    m_positioningLeadingEdge.SetXsi(xsi1, xsi2);
}

void CCPACSWingCell::SetTrailingEdgeInnerPoint(double eta2, double xsi1)
{
    UpdateEtaXsiValues();
    const double eta1 = m_etaXsiCache.value().innerLeadingEdgePoint.eta;
    const double xsi2 = m_etaXsiCache.value().outerTrailingEdgePoint.xsi;
    m_positioningInnerBorder.SetEta(eta1, eta2);
    m_positioningTrailingEdge.SetXsi(xsi1, xsi2);
}

void CCPACSWingCell::SetTrailingEdgeOuterPoint(double eta2, double xsi2)
{
    UpdateEtaXsiValues();
    const double eta1 = m_etaXsiCache.value().outerLeadingEdgePoint.eta;
    const double xsi1 = m_etaXsiCache.value().innerTrailingEdgePoint.xsi;
    m_positioningOuterBorder.SetEta(eta1, eta2);
    m_positioningTrailingEdge.SetXsi(xsi1, xsi2);
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
        const CCPACSWingCSStructure& structure = m_parent->GetParentElement()->GetStructure();
        const CCPACSWingSparSegment& spar      = structure.GetSparSegment(chordwisePos.GetSparUId());
        xsi                                    = computeSparXsiValue(structure.GetWingStructureReference(), spar, eta);
    }
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Rib &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Xsi) {
        std::string ribUid;
        int ribIndex;
        // get the ribs definition from the wing structure reference
        spanwisePos.GetRib(ribUid, ribIndex);
        const CCPACSWingCSStructure& structure         = m_parent->GetParentElement()->GetStructure();
        const CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(ribUid);
        if (inner) {
            xsi = chordwisePos.GetXsi().first;
        }
        else {
            xsi = chordwisePos.GetXsi().second;
        }
        eta = computeRibEtaValue(structure.GetWingStructureReference(), ribsDefinition, ribIndex, xsi);
    }
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Rib &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Spar) {
        // get the spar from the wing structure reference
        const CCPACSWingCSStructure& structure = m_parent->GetParentElement()->GetStructure();
        CTiglWingStructureReference wsr        = structure.GetWingStructureReference();
        const CCPACSWingSparSegment& spar      = structure.GetSparSegment(chordwisePos.GetSparUId());
        // get the ribs definition from the wing structure reference
        std::string ribUid;
        int ribIndex;
        spanwisePos.GetRib(ribUid, ribIndex);
        const CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(ribUid);

        tigl::EtaXsi result = computeRibSparIntersectionEtaXsi(wsr, ribsDefinition, ribIndex, spar);
        eta                 = result.eta;
        xsi                 = result.xsi;
    }

    return EtaXsi(eta, xsi);
}

void CCPACSWingCell::UpdateEtaXsiValues() const
{
    if (m_etaXsiCache) {
        return;
    }

    m_etaXsiCache.emplace();

    EtaXsi innerLePoint =
        computePositioningEtaXsi(m_positioningInnerBorder, m_positioningLeadingEdge, true, true);
    m_etaXsiCache->innerLeadingEdgePoint.eta = innerLePoint.eta;
    m_etaXsiCache->innerLeadingEdgePoint.xsi = innerLePoint.xsi;

    EtaXsi outerLePoint =
        computePositioningEtaXsi(m_positioningOuterBorder, m_positioningLeadingEdge, false, true);
    m_etaXsiCache->outerLeadingEdgePoint.eta = outerLePoint.eta;
    m_etaXsiCache->outerLeadingEdgePoint.xsi = outerLePoint.xsi;

    EtaXsi innerTePoint =
        computePositioningEtaXsi(m_positioningInnerBorder, m_positioningTrailingEdge, true, false);
    m_etaXsiCache->innerTrailingEdgePoint.eta = innerTePoint.eta;
    m_etaXsiCache->innerTrailingEdgePoint.xsi = innerTePoint.xsi;

    EtaXsi outerTePoint =
        computePositioningEtaXsi(m_positioningOuterBorder, m_positioningTrailingEdge, false, false);
    m_etaXsiCache->outerTrailingEdgePoint.eta = outerTePoint.eta;
    m_etaXsiCache->outerTrailingEdgePoint.xsi = outerTePoint.xsi;
}

void CCPACSWingCell::Update() const
{
    if (m_geometryCache) {
        return;
    }

    BuildSkinGeometry();
}

TopoDS_Shape CCPACSWingCell::GetCellSkinGeometry(TiglCoordinateSystem cs) const
{
    Update();

    if (cs == GLOBAL_COORDINATE_SYSTEM) {
        return m_parent->GetParentElement()
            ->GetStructure()
            .GetWingStructureReference()
            .GetWingComponentSegment()
            .GetWing()
            .GetTransformationMatrix()
            .Transform(m_geometryCache.value().cellSkinGeometry);
    }
    else if (cs == WING_COORDINATE_SYSTEM) {
        return m_geometryCache.value().cellSkinGeometry;
    }
    else {
        throw CTiglError("Invalid coordinate system passed to CCPACSWingCell::GetCellSkinGeometry");
    }
}

void CCPACSWingCell::BuildSkinGeometry() const
{

    BRep_Builder builder;

    const CTiglWingStructureReference& wsr = m_parent->GetParentElement()->GetStructure().GetWingStructureReference();
    gp_Pnt p1                              = wsr.GetLeadingEdgePoint(0);
    gp_Pnt p2                              = wsr.GetLeadingEdgePoint(1);
    gp_Vec yRefDir(p1, p2);
    yRefDir.Normalize();

    // create a reference direction without sweep angle
    gp_Pnt p2stern = gp_Pnt(p1.X(), p2.Y(), p2.Z());
    gp_Vec yRefDirStern(p1, p2stern);
    yRefDirStern.Normalize();

    double sweepAngle = yRefDir.Angle(yRefDirStern);

    if (p2.X() < p1.X()) {
        sweepAngle = -sweepAngle;
    }

    gp_Pnt p5 = wsr.GetTrailingEdgePoint(0);
    gp_Pnt p6 = wsr.GetTrailingEdgePoint(1);

    gp_Vec xRefDir(p1, p5);
    gp_Vec zRefDir(-yRefDirStern ^ xRefDir);
    zRefDir.Normalize();

    // Step 8: find the correct part of the loft
    TopoDS_Shape loftShape;
    TiglLoftSide side = m_parent->GetParentElement()->GetLoftSide();
    if (side == UPPER_SIDE) {
        loftShape = wsr.GetUpperShape();
    }
    else {
        loftShape = wsr.GetLowerShape();
    }

    // determine diagonal vector of loft bounding box (e.g. used for size of cut faces)
    Bnd_Box boundingBox;
    BRepBndLib::Add(wsr.GetUpperShape(), boundingBox);
    BRepBndLib::Add(wsr.GetLowerShape(), boundingBox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    gp_Vec diagonal(xmax - xmin, ymax - ymin, zmax - zmin);
    Standard_Real bboxSize = diagonal.Magnitude();

    if (!m_etaXsiCache) {
        UpdateEtaXsiValues();
    }
    assert(m_etaXsiCache);

    double LEXsi1, LEXsi2, TEXsi1, TEXsi2;
    double IBEta1, IBEta2, OBEta1, OBEta2;

    LEXsi1 = m_etaXsiCache.value().innerLeadingEdgePoint.xsi;
    LEXsi2 = m_etaXsiCache.value().outerLeadingEdgePoint.xsi;
    TEXsi1 = m_etaXsiCache.value().innerTrailingEdgePoint.xsi;
    TEXsi2 = m_etaXsiCache.value().outerTrailingEdgePoint.xsi;
    IBEta1 = m_etaXsiCache.value().innerLeadingEdgePoint.eta;
    IBEta2 = m_etaXsiCache.value().innerTrailingEdgePoint.eta;
    OBEta1 = m_etaXsiCache.value().outerLeadingEdgePoint.eta;
    OBEta2 = m_etaXsiCache.value().outerTrailingEdgePoint.eta;

    m_geometryCache.emplace();

    // Get references to all objects in geometry cache
    gp_Pnt& pC1 = m_geometryCache->pC1;
    gp_Pnt& pC2 = m_geometryCache->pC2;
    gp_Pnt& pC3 = m_geometryCache->pC3;
    gp_Pnt& pC4 = m_geometryCache->pC4;
    
    gp_Pln& cutPlaneLE = m_geometryCache->cutPlaneLE;
    gp_Pln& cutPlaneTE = m_geometryCache->cutPlaneTE;
    gp_Pln& cutPlaneIB = m_geometryCache->cutPlaneIB;
    gp_Pln& cutPlaneOB = m_geometryCache->cutPlaneOB;
    
    TopoDS_Shape& planeShapeLE = m_geometryCache->planeShapeLE;
    TopoDS_Shape& planeShapeTE = m_geometryCache->planeShapeTE;
    TopoDS_Shape& planeShapeIB = m_geometryCache->planeShapeIB;
    TopoDS_Shape& planeShapeOB = m_geometryCache->planeShapeOB;
    TopoDS_Shape& sparShapeLE  = m_geometryCache->sparShapeLE;
    TopoDS_Shape& sparShapeTE  = m_geometryCache->sparShapeTE;

    pC1 = wsr.GetPoint(IBEta1, LEXsi1, WING_COORDINATE_SYSTEM);
    pC2 = wsr.GetPoint(OBEta1, LEXsi2, WING_COORDINATE_SYSTEM);
    pC3 = wsr.GetPoint(IBEta2, TEXsi1, WING_COORDINATE_SYSTEM);
    pC4 = wsr.GetPoint(OBEta2, TEXsi2, WING_COORDINATE_SYSTEM);

    // project the cornerpoints on the lower or upper surface
    TopoDS_Shape cutEdge1, cutEdge2, cutEdge3, cutEdge4;
    // build line along z reference axis for intersection points on loft
    gp_Vec offset(zRefDir * bboxSize);
    BRepBuilderAPI_MakeEdge edgeBuilder1(pC1.Translated(-offset), pC1.Translated(offset));
    cutEdge1 = edgeBuilder1.Edge();
    BRepBuilderAPI_MakeEdge edgeBuilder2(pC2.Translated(-offset), pC2.Translated(offset));
    cutEdge2 = edgeBuilder2.Edge();
    BRepBuilderAPI_MakeEdge edgeBuilder3(pC3.Translated(-offset), pC3.Translated(offset));
    cutEdge3 = edgeBuilder3.Edge();
    BRepBuilderAPI_MakeEdge edgeBuilder4(pC4.Translated(-offset), pC4.Translated(offset));
    cutEdge4 = edgeBuilder4.Edge();

    // find intersection points on loft geometry, use minimum distance for stability
    BRepExtrema_DistShapeShape ex1(loftShape, cutEdge1, Extrema_ExtFlag_MIN);
    ex1.Perform();
    pC1 = ex1.PointOnShape1(1);
    BRepExtrema_DistShapeShape ex2(loftShape, cutEdge2, Extrema_ExtFlag_MIN);
    ex2.Perform();
    pC2 = ex2.PointOnShape1(1);
    BRepExtrema_DistShapeShape ex3(loftShape, cutEdge3, Extrema_ExtFlag_MIN);
    ex3.Perform();
    pC3 = ex3.PointOnShape1(1);
    BRepExtrema_DistShapeShape ex4(loftShape, cutEdge4, Extrema_ExtFlag_MIN);
    ex4.Perform();
    pC4 = ex4.PointOnShape1(1);

    // check the 3d point coordinates

    if (pC1.X() > pC3.X()) {
        throw CTiglError(
            "Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values on the inner border.");
    }
    if (pC2.X() > pC4.X()) {
        throw CTiglError(
            "Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values on the outer border.");
    }
    if (pC1.Y() > pC2.Y()) {
        throw CTiglError(
            "Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the leading edge.");
    }
    if (pC3.Y() > pC4.Y()) {
        throw CTiglError("Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the "
                         "trailing edge.");
    }

    // combine the cell cutting planes to a compound
    TopoDS_Compound Planecomp;
    builder.MakeCompound(Planecomp);

    // build the Leading edge cutting plane
    gp_Vec vCLE(pC1, pC2);
    vCLE.Normalize();
    gp_Pnt midPnt = (pC1.XYZ() + pC2.XYZ()) / 2;
    gp_Ax3 refAxLE(midPnt, -zRefDir ^ vCLE, vCLE);
    cutPlaneLE   = gp_Pln(refAxLE);
    planeShapeLE = BRepBuilderAPI_MakeFace(cutPlaneLE).Face();
    // build the Trailing edge cutting plane
    gp_Vec vCTE(pC3, pC4);
    vCTE.Normalize();
    midPnt = (pC3.XYZ() + pC4.XYZ()) / 2;
    gp_Ax3 refAxTE(midPnt, zRefDir ^ vCTE, vCTE);
    cutPlaneTE   = gp_Pln(refAxTE);
    planeShapeTE = BRepBuilderAPI_MakeFace(cutPlaneTE).Face();

    // build the inner border cutting plane
    gp_Vec vCIB(pC1, pC3);
    vCIB.Normalize();
    midPnt = (pC1.XYZ() + pC3.XYZ()) / 2;
    gp_Ax3 refAxIB(midPnt, zRefDir ^ vCIB, vCIB);
    cutPlaneIB   = gp_Pln(refAxIB);
    planeShapeIB = BRepBuilderAPI_MakeFace(cutPlaneIB).Face();

    // build the Outer border cutting plane
    gp_Vec vCOB(pC2, pC4);
    vCOB.Normalize();
    midPnt = (pC2.XYZ() + pC4.XYZ()) / 2;
    gp_Ax3 refAxOB(midPnt, -zRefDir ^ vCOB, vCOB);
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
    // if the inner border of the cell is the inner border of the Component segment
    // a cutting plane from the inner border of the WCS is created
    // this is necessary due to cutting precision
    else if (IBEta1 == 0. && IBEta2 == 0.) {
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
    // if the outer border of the cell is the outer border of the Component segment
    // a cutting plane from the outer border of the WCS is created
    // this is necessary due to cutting precision
    else if (OBEta1 == 1. && OBEta2 == 1.) {
        BRepAdaptor_Surface surf(wsr.GetOuterFace());
        gp_Pnt p0 = surf.Value(u05, v0);
        gp_Pnt pU = surf.Value(u05, v1);
        gp_Pnt pV = surf.Value(u1, v0);
        gp_Ax3 ax0UV(p0, gp_Vec(p0, pU) ^ gp_Vec(p0, pV), gp_Vec(p0, pU));

        planeShapeOB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
    }

    builder.Add(Planecomp, planeShapeLE);
    builder.Add(Planecomp, planeShapeTE);
    builder.Add(Planecomp, planeShapeIB);
    builder.Add(Planecomp, planeShapeOB);

    // cut the lower or upper loft with the planes
    TopoDS_Shape result = SplitShape(loftShape, Planecomp);
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(result, TopAbs_FACE, faceMap);

    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    bool notFound = true;

    for (int f = 1; f <= faceMap.Extent(); f++) {

        TopoDS_Face loftFace      = TopoDS::Face(faceMap(f));
        Handle(Geom_Surface) surf = BRep_Tool::Surface(loftFace);

        // variable for uv bounds
        double u_min = 0, u_max = 0, v_min = 0, v_max = 0;
        BRepTools::UVBounds(loftFace, u_min, u_max, v_min, v_max);
        gp_Pnt pTest = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));

        // create each midpoint for the vector basis

        // test if the midplane point is behind the leading edge border plane
        // create an Ax1 from the Leading edge plane origin to the midpoint of the current face
        gp_Pnt midPnt = (pC1.XYZ() + pC2.XYZ()) / 2;
        gp_Vec vTest(midPnt, pTest);
        gp_Ax1 a1Test(midPnt, vTest);

        bool sparTest = false, plainTest = false;

        if (m_positioningLeadingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
            sparTest = PointIsInfrontSparGeometry(cutPlaneLE.Axis(), pTest, sparShapeLE);
        }
        else {
            plainTest = a1Test.Angle(cutPlaneLE.Axis()) < M_PI_2;
        }

        if (plainTest || sparTest) {
            // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
            midPnt = (pC3.XYZ() + pC4.XYZ()) / 2;
            vTest  = gp_Vec(midPnt, pTest);
            a1Test = gp_Ax1(midPnt, vTest);

            sparTest  = false;
            plainTest = false;
            if (m_positioningTrailingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
                sparTest = PointIsInfrontSparGeometry(cutPlaneTE.Axis(), pTest, sparShapeTE);
            }
            else {
                plainTest = a1Test.Angle(cutPlaneTE.Axis()) < M_PI_2;
            }

            if (plainTest || sparTest) {
                // create an Ax1 from the inner border plane origin to the midpoint of the current face
                midPnt = (pC1.XYZ() + pC3.XYZ()) / 2;
                vTest  = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);
                if (a1Test.Angle(cutPlaneIB.Axis()) < M_PI_2) {
                    // create an Ax1 from the outer border plane origin to the midpoint of the current face
                    midPnt = (pC2.XYZ() + pC4.XYZ()) / 2;
                    vTest  = gp_Vec(midPnt, pTest);
                    a1Test = gp_Ax1(midPnt, vTest);
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

    m_geometryCache->cellSkinGeometry = compound;
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
    Update();

    Bnd_Box bBox1, bBox2;
    BRepBndLib::Add(m_geometryCache.value().cellSkinGeometry, bBox1);
    TopoDS_Shape t_transformed = m_parent->GetParentElement()
                                        ->GetStructure()
                                        .GetWingStructureReference()
                                        .GetWingComponentSegment()
                                        .GetWing()
                                        .GetTransformationMatrix()
                                        .Inverted()
                                        .Transform(t);
    BRepBndLib::Add(t_transformed, bBox2);

    if (bBox1.IsOut(bBox2)) {
        return false;
    }

    gp_Pnt pTest = WingCellInternal::pointOnShape(t);

    gp_Pnt midPnt = (m_geometryCache.value().pC1.XYZ() + m_geometryCache.value().pC2.XYZ()) / 2;
    gp_Vec vTest(midPnt, pTest);
    gp_Ax1 a1Test(midPnt, vTest);

    bool sparTest = false, plainTest = false;

    if (m_positioningLeadingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
        sparTest = PointIsInfrontSparGeometry(m_geometryCache.value().cutPlaneLE.Axis(), pTest, m_geometryCache.value().sparShapeLE);
    }
    else {
        plainTest = a1Test.Angle(m_geometryCache.value().cutPlaneLE.Axis()) < M_PI_2;
    }

    if (plainTest || sparTest) {
        // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
        midPnt = (m_geometryCache.value().pC3.XYZ() + m_geometryCache.value().pC4.XYZ()) / 2;
        vTest  = gp_Vec(midPnt, pTest);
        a1Test = gp_Ax1(midPnt, vTest);

        sparTest  = false;
        plainTest = false;
        if (m_positioningTrailingEdge.GetInputType() == CCPACSWingCellPositionChordwise::InputType::Spar) {
            sparTest = PointIsInfrontSparGeometry(m_geometryCache.value().cutPlaneTE.Axis(), pTest, m_geometryCache.value().sparShapeTE);
        }
        else {
            plainTest = a1Test.Angle(m_geometryCache.value().cutPlaneTE.Axis()) < M_PI_2;
        }

        if (plainTest || sparTest) {
            // create an Ax1 from the inner border plane origin to the midpoint of the current face
            midPnt = (m_geometryCache.value().pC1.XYZ() + m_geometryCache.value().pC3.XYZ()) / 2;
            vTest  = gp_Vec(midPnt, pTest);
            a1Test = gp_Ax1(midPnt, vTest);
            if (a1Test.Angle(m_geometryCache.value().cutPlaneIB.Axis()) < M_PI_2) {
                // create an Ax1 from the outer border plane origin to the midpoint of the current face
                midPnt = (m_geometryCache.value().pC2.XYZ() + m_geometryCache.value().pC4.XYZ()) / 2;
                vTest  = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);
                if (a1Test.Angle(m_geometryCache.value().cutPlaneOB.Axis()) < M_PI_2) {
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

PNamedShape CCPACSWingCell::GetLoft()
{
    return PNamedShape(new CNamedShape(GetCellSkinGeometry(GLOBAL_COORDINATE_SYSTEM), GetUID()));
}

TiglGeometricComponentType CCPACSWingCell::GetComponentType() const
{
    return TIGL_COMPONENT_LOGICAL;
}

} // namespace tigl
