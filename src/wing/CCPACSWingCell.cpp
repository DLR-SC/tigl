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
#include "CCPACSWingShell.h"
#include "CCPACSWingSpars.h"
#include "CCPACSWingSparSegments.h"
#include "CCPACSWingSparSegment.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "tigletaxsifunctions.h"
#include "to_string.h"


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
        area += p1.x*(p2.y - p3.y);
        area += p2.x*(p3.y - p1.y);
        area += p3.x*(p1.y - p2.y);
        
        return fabs(area/2.);
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
}

using namespace WingCellInternal;

CCPACSWingCell::CCPACSWingCell(CCPACSWingCells* parentCells, CTiglUIDManager* uidMgr)
    : generated::CPACSWingCell(parentCells, uidMgr) {
    Reset();
}

CCPACSWingCell::~CCPACSWingCell() {}

void CCPACSWingCell::Invalidate()
{
    cache = boost::none;
}

void CCPACSWingCell::Reset()
{
    m_uID = "";
    
    cache = boost::none;
}

bool CCPACSWingCell::IsConvex() const
{
    Point2D p1, p2, p3, p4;
    
    // calculate for all 4 edges the relative position of eta/xsi
    GetTrailingEdgeInnerPoint(&p1.x, &p1.y);
    GetTrailingEdgeOuterPoint(&p2.x, &p2.y);
    GetLeadingEdgeOuterPoint (&p3.x, &p3.y);
    GetLeadingEdgeInnerPoint (&p4.x, &p4.y);
    
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
    p.x = eta; p.y = xsi;

    GetTrailingEdgeInnerPoint(&p1.x, &p1.y);
    GetTrailingEdgeOuterPoint(&p2.x, &p2.y);
    GetLeadingEdgeOuterPoint (&p3.x, &p3.y);
    GetLeadingEdgeInnerPoint (&p4.x, &p4.y);
    
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
        int  iwind = (w1 + w2 + w3 + w4);
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

void CCPACSWingCell::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string &cellXPath)
{
    Reset();
    generated::CPACSWingCell::ReadCPACS(tixiHandle, cellXPath);
}

void CCPACSWingCell::GetLeadingEdgeInnerPoint(double* eta, double* xsi) const
{
    UpdateCache();
    *eta = cache.value().innerLeadingEdgePoint.eta;
    *xsi = cache.value().innerLeadingEdgePoint.xsi;
}

void CCPACSWingCell::GetLeadingEdgeOuterPoint(double* eta, double* xsi) const
{
    UpdateCache();
    *eta = cache.value().outerLeadingEdgePoint.eta;
    *xsi = cache.value().outerLeadingEdgePoint.xsi;
}

void CCPACSWingCell::GetTrailingEdgeInnerPoint(double* eta, double* xsi) const
{
    UpdateCache();
    *eta = cache.value().innerTrailingEdgePoint.eta;
    *xsi = cache.value().innerTrailingEdgePoint.xsi;
}

void CCPACSWingCell::GetTrailingEdgeOuterPoint(double* eta, double* xsi) const
{
    UpdateCache();
    *eta = cache.value().outerTrailingEdgePoint.eta;
    *xsi = cache.value().outerTrailingEdgePoint.xsi;
}

void CCPACSWingCell::SetLeadingEdgeInnerPoint(double eta, double xsi)
{
    UpdateCache();
    m_positioningInnerBorder.SetEta(eta, cache.value().innerTrailingEdgePoint.eta);
    m_positioningLeadingEdge.SetXsi(xsi, cache.value().outerLeadingEdgePoint.xsi);
}

void CCPACSWingCell::SetLeadingEdgeOuterPoint(double eta, double xsi)
{
    UpdateCache();
    m_positioningOuterBorder.SetEta(eta, cache.value().outerTrailingEdgePoint.eta);
    m_positioningLeadingEdge.SetXsi(cache.value().innerLeadingEdgePoint.xsi, xsi);
}

void CCPACSWingCell::SetTrailingEdgeInnerPoint(double eta, double xsi)
{
    UpdateCache();
    m_positioningInnerBorder.SetEta(cache.value().innerLeadingEdgePoint.eta, eta);
    m_positioningTrailingEdge.SetXsi(xsi, cache.value().outerTrailingEdgePoint.xsi);
}

void CCPACSWingCell::SetTrailingEdgeOuterPoint(double eta, double xsi)
{
    UpdateCache();
    m_positioningOuterBorder.SetEta(cache.value().outerLeadingEdgePoint.eta, eta);
    m_positioningTrailingEdge.SetXsi(cache.value().innerTrailingEdgePoint.xsi, xsi);
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

void CCPACSWingCell::UpdateCache() const
{
    if (!cache) {
        UpdateEtaXsiValues();
    }
    assert(cache);
}


std::pair<double, double> CCPACSWingCell::computePositioningEtaXsi(const CCPACSWingCellPositionSpanwise& spanwisePos,
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
        const CCPACSWingSparSegment& spar = structure.GetSparSegment(chordwisePos.GetSparUId());
        xsi = computeSparXsiValue(structure.GetWingStructureReference(), spar, eta);
    }
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Rib &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Xsi) {
        std::string ribUid;
        int ribIndex;
        // get the ribs definition from the wing structure reference
        spanwisePos.GetRib(ribUid, ribIndex);
        const CCPACSWingCSStructure& structure = m_parent->GetParentElement()->GetStructure();
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
        CTiglWingStructureReference wsr = structure.GetWingStructureReference();
        const CCPACSWingSparSegment& spar = structure.GetSparSegment(chordwisePos.GetSparUId());
        // get the ribs definition from the wing structure reference
        std::string ribUid;
        int ribIndex;
        spanwisePos.GetRib(ribUid, ribIndex);
        const CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(ribUid);

        tigl::EtaXsi result = computeRibSparIntersectionEtaXsi(wsr, ribsDefinition, ribIndex, spar);
        eta = result.eta;
        xsi = result.xsi;
    }

    return std::make_pair(eta, xsi);
}

void CCPACSWingCell::UpdateEtaXsiValues() const
{
    cache.emplace();

    std::pair<double, double> innerLePoint = computePositioningEtaXsi(m_positioningInnerBorder, m_positioningLeadingEdge, true, true);
    cache->innerLeadingEdgePoint.eta = innerLePoint.first;
    cache->innerLeadingEdgePoint.xsi = innerLePoint.second;

    std::pair<double, double> outerLePoint = computePositioningEtaXsi(m_positioningOuterBorder, m_positioningLeadingEdge, false, true);
    cache->outerLeadingEdgePoint.eta = outerLePoint.first;
    cache->outerLeadingEdgePoint.xsi = outerLePoint.second;

    std::pair<double, double> innerTePoint = computePositioningEtaXsi(m_positioningInnerBorder, m_positioningTrailingEdge, true, false);
    cache->innerTrailingEdgePoint.eta = innerTePoint.first;
    cache->innerTrailingEdgePoint.xsi = innerTePoint.second;

    std::pair<double, double> outerTePoint = computePositioningEtaXsi(m_positioningOuterBorder, m_positioningTrailingEdge, false, false);
    cache->outerTrailingEdgePoint.eta = outerTePoint.first;
    cache->outerTrailingEdgePoint.xsi = outerTePoint.second;
}

void CCPACSWingCell::Update() const
{
    // TODO: update geometry
}

} // namespace tigl
