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

#include <tixi.h>

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
#include "IOHelper.h"
#include "tigletaxsifunctions.h"
#include "TixiSaveExt.h"
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

CCPACSWingCell::CCPACSWingCell(CCPACSWingCells* parentCells)
: parent(parentCells),
  positionLeadingEdge(this),
  positionTrailingEdge(this),
  positionInnerBorder(this),
  positionOuterBorder(this)
{
    Reset();
}

CCPACSWingCell::~CCPACSWingCell()
{
    Reset();
}

const std::string& CCPACSWingCell::GetUID() const
{
    return uid;
}

void CCPACSWingCell::Invalidate()
{
    cache.valid = false;
}

void CCPACSWingCell::Reset()
{
    uid = "";
    
    cache.valid = false;

    positionLeadingEdge.Reset();
    positionTrailingEdge.Reset();
    positionInnerBorder.Reset();
    positionOuterBorder.Reset();
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

void CCPACSWingCell::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &cellXPath)
{
    Reset();
    generated::CPACSWingCell::ReadCPACS(tixiHandle, cellXPath);

    // TODO: implement check that sparUID is not supported yet

    //double iBE1, iBE2, oBE1, oBE2, lEX1, lEX2, tEX1, tEX2;
    //// get postionings of cell
    //std::string positioningString;
    //positioningString = cellXPath + "/positioningLeadingEdge/sparUID";
    //if ( tixiCheckElement(tixiHandle, positioningString.c_str()) == SUCCESS) {
    //    LOG(WARNING) << "In " << cellXPath << ": Cell positiongs via spars is currently not supported by TiGL. Please use eta/xsi definitions.";
    //    lEX1 = 0.; lEX2 = 0.;
    //}
    //else {
    //    positioningString = cellXPath + "/positioningLeadingEdge/xsi1";
    //    if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &lEX1) != SUCCESS) {
    //        throw tigl::CTiglError("No leading edge xsi1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
    //    }
    //    
    //    positioningString = cellXPath + "/positioningLeadingEdge/xsi2";
    //    if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &lEX2) != SUCCESS) {
    //        throw tigl::CTiglError("No leading edge xsi2 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
    //    }
    //}

    //positioningString = cellXPath + "/positioningTrailingEdge/sparUID";
    //if ( tixiCheckElement(tixiHandle, positioningString.c_str()) == SUCCESS) {
    //    LOG(WARNING) << "In " << cellXPath << ": Cell positiongs via spars is currently not supported by TiGL. Please use eta/xsi definitions.";
    //    tEX1 = 0.; tEX2 = 0.;
    //}
    //else {
    //    positioningString = cellXPath + "/positioningTrailingEdge/xsi1";
    //    if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &tEX1) != SUCCESS) {
    //        throw tigl::CTiglError("No leading edge xsi1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
    //    }
    //    
    //    positioningString = cellXPath + "/positioningTrailingEdge/xsi2";
    //    if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &tEX2) != SUCCESS) {
    //        throw tigl::CTiglError("No leading edge xsi1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
    //    }
    //    
    //}
    //
    //positioningString = cellXPath + "/positioningInnerBorder/ribDefinitionUID";
    //if ( tixiCheckElement(tixiHandle, positioningString.c_str()) == SUCCESS) {
    //    LOG(WARNING) << "In " << cellXPath << ": Cell positiongs via ribs is currently not supported by TiGL. Please use eta/xsi definitions.";
    //    iBE1 = 0.; iBE2 = 0.;
    //}
    //else {
    //    positioningString = cellXPath + "/positioningInnerBorder/eta1";
    //    if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &iBE1) != SUCCESS) {
    //        throw tigl::CTiglError("No inner border eta1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
    //    }
    //    
    //    positioningString = cellXPath + "/positioningInnerBorder/eta2";
    //    if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &iBE2) != SUCCESS) {
    //        throw tigl::CTiglError("No inner border eta2 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
    //    }
    //}
    //
    //positioningString = cellXPath + "/positioningOuterBorder/ribDefinitionUID";
    //if ( tixiCheckElement(tixiHandle, positioningString.c_str()) == SUCCESS) {
    //    LOG(WARNING) << "In " << cellXPath << ": Cell positiongs via ribs is currently not supported by TiGL. Please use eta/xsi definitions.";
    //    oBE1 = 0.; oBE2 = 0.;
    //}
    //else {
    //    positioningString = cellXPath + "/positioningOuterBorder/eta1";
    //    if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &oBE1) != SUCCESS) {
    //        throw tigl::CTiglError("No outer border eta1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
    //    }
    //    
    //    positioningString = cellXPath + "/positioningOuterBorder/eta2";
    //    if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &oBE2) != SUCCESS) {
    //        throw tigl::CTiglError("No outer border eta2 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
    //    }
    //}
    //
    //// read material
    //std::string materialString;
    //materialString = cellXPath + "/skin/material";
    //if ( tixiCheckElement(tixiHandle, materialString.c_str()) == SUCCESS) {
    //    material.ReadCPACS(tixiHandle, materialString.c_str());
    //}
    //else {
    //    // @todo: should that be an error?
    //    LOG(WARNING) << "No material definition found for cell " << cellXPath;
    //}
    //
    //// apply everything
    //innerBorderEta1 = iBE1;
    //innerBorderEta2 = iBE2;
    //outerBorderEta1 = oBE1;
    //outerBorderEta2 = oBE2;
    //leadingEdgeXsi1 = lEX1;
    //leadingEdgeXsi2 = lEX2;
    //trailingEdgeXsi1 = tEX1;
    //trailingEdgeXsi2 = tEX2;
}

void CCPACSWingCell::GetLeadingEdgeInnerPoint(double* eta, double* xsi) const
{
    UpdateCache();
    *eta = cache.innerLeadingEdgePoint.eta;
    *xsi = cache.innerLeadingEdgePoint.xsi;
}

void CCPACSWingCell::GetLeadingEdgeOuterPoint(double* eta, double* xsi) const
{
    UpdateCache();
    *eta = cache.outerLeadingEdgePoint.eta;
    *xsi = cache.outerLeadingEdgePoint.xsi;
}

void CCPACSWingCell::GetTrailingEdgeInnerPoint(double* eta, double* xsi) const
{
    UpdateCache();
    *eta = cache.innerTrailingEdgePoint.eta;
    *xsi = cache.innerTrailingEdgePoint.xsi;
}

void CCPACSWingCell::GetTrailingEdgeOuterPoint(double* eta, double* xsi) const
{
    UpdateCache();
    *eta = cache.outerTrailingEdgePoint.eta;
    *xsi = cache.outerTrailingEdgePoint.xsi;
}

void CCPACSWingCell::SetLeadingEdgeInnerPoint(double eta, double xsi)
{
    UpdateCache();
    positionInnerBorder.SetEta(eta, cache.innerTrailingEdgePoint.eta);
    positionLeadingEdge.SetXsi(xsi, cache.outerLeadingEdgePoint.xsi);
}

void CCPACSWingCell::SetLeadingEdgeOuterPoint(double eta, double xsi)
{
    UpdateCache();
    positionOuterBorder.SetEta(eta, cache.outerTrailingEdgePoint.eta);
    positionLeadingEdge.SetXsi(cache.innerLeadingEdgePoint.xsi, xsi);
}

void CCPACSWingCell::SetTrailingEdgeInnerPoint(double eta, double xsi)
{
    UpdateCache();
    positionInnerBorder.SetEta(cache.innerLeadingEdgePoint.eta, eta);
    positionTrailingEdge.SetXsi(xsi, cache.outerTrailingEdgePoint.xsi);
}

void CCPACSWingCell::SetTrailingEdgeOuterPoint(double eta, double xsi)
{
    UpdateCache();
    positionOuterBorder.SetEta(cache.outerLeadingEdgePoint.eta, eta);
    positionTrailingEdge.SetXsi(cache.innerTrailingEdgePoint.xsi, xsi);
}

void CCPACSWingCell::SetLeadingEdgeSpar(const std::string& sparUID)
{
    positionLeadingEdge.SetSparUId(sparUID);
}

void CCPACSWingCell::SetTrailingEdgeSpar(const std::string& sparUID)
{
    positionTrailingEdge.SetSparUId(sparUID);
}

void CCPACSWingCell::SetInnerBorderRib(const std::string& ribDefinitionUID, int ribNumber)
{
    positionInnerBorder.SetRib(ribDefinitionUID, ribNumber);
}

void CCPACSWingCell::SetOuterBorderRib(const std::string& ribDefinitionUID, int ribNumber)
{
    positionOuterBorder.SetRib(ribDefinitionUID, ribNumber);
}

const CCPACSMaterial &CCPACSWingCell::GetMaterial()
{
    return m_skin.GetMaterial();
}

const CCPACSMaterial &CCPACSWingCell::GetMaterial() const
{
    return material;
}

void CCPACSWingCell::UpdateCache() const
{
    if (!cache.valid) {
        UpdateEtaXsiValues();
    }
    assert(cache.valid);
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
        const CCPACSWingCSStructure& structure = parent->GetParentElement()->GetStructure();
        CCPACSWingSparSegment& spar = structure.GetSparSegment(chordwisePos.GetSparUId());
        xsi = computeSparXsiValue(structure.GetWingStructureReference(), spar, eta);
    }
    else if (spanwisePos.GetInputType() == CCPACSWingCellPositionSpanwise::Rib &&
             chordwisePos.GetInputType() == CCPACSWingCellPositionChordwise::Xsi) {
        std::string ribUid;
        int ribIndex;
        // get the ribs definition from the wing structure reference
        spanwisePos.GetRib(ribUid, ribIndex);
        const CCPACSWingCSStructure& structure = parent->GetParentElement()->GetStructure();
        CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(ribUid);
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
        const CCPACSWingCSStructure& structure = parent->GetParentElement()->GetStructure();
        CTiglWingStructureReference wsr = structure.GetWingStructureReference();
        CCPACSWingSparSegment& spar = structure.GetSparSegment(chordwisePos.GetSparUId());
        // get the ribs definition from the wing structure reference
        std::string ribUid;
        int ribIndex;
        spanwisePos.GetRib(ribUid, ribIndex);
        CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(ribUid);

        tigl::EtaXsi result = computeRibSparIntersectionEtaXsi(wsr, ribsDefinition, ribIndex, spar);
        eta = result.eta;
        xsi = result.xsi;
    }

    return std::make_pair(eta, xsi);
}

void CCPACSWingCell::UpdateEtaXsiValues() const
{
    cache.valid = false;

    std::pair<double, double> innerLePoint = computePositioningEtaXsi(positionInnerBorder, positionLeadingEdge, true, true);
    cache.innerLeadingEdgePoint.eta = innerLePoint.first;
    cache.innerLeadingEdgePoint.xsi = innerLePoint.second;

    std::pair<double, double> outerLePoint = computePositioningEtaXsi(positionOuterBorder, positionLeadingEdge, false, true);
    cache.outerLeadingEdgePoint.eta = outerLePoint.first;
    cache.outerLeadingEdgePoint.xsi = outerLePoint.second;

    std::pair<double, double> innerTePoint = computePositioningEtaXsi(positionInnerBorder, positionTrailingEdge, true, false);
    cache.innerTrailingEdgePoint.eta = innerTePoint.first;
    cache.innerTrailingEdgePoint.xsi = innerTePoint.second;

    std::pair<double, double> outerTePoint = computePositioningEtaXsi(positionOuterBorder, positionTrailingEdge, false, false);
    cache.outerTrailingEdgePoint.eta = outerTePoint.first;
    cache.outerTrailingEdgePoint.xsi = outerTePoint.second;

    cache.valid = true;
}

void CCPACSWingCell::Update() const
{
    // TODO: update geometry
}

} // namespace tigl
