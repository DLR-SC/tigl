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
#include "CCPACSWingCells.h"
#include "CCPACSWingShell.h"

#include "tixi.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"
#include "CTiglCommon.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"

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

// [[CAS_AES]] added pointer to parent
// [[CAS_AES]] added initialization of stringer
CCPACSWingCell::CCPACSWingCell(CCPACSWingCells* nParentCells)
: mParentCells(nParentCells),
  stringer(NULL)
{
    Reset();
}

// [[CAS_AES]] added destructor
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
    mPositionLeadingEdge.Invalidate();
    mPositionTrailingEdge.Invalidate();
    mPositionInnerBorder.Invalidate();
    mPositionOuterBorder.Invalidate();
    geometryValid = false;
}

void CCPACSWingCell::Reset()
{
    // [[CAS_AES]] clean up positionings instead of local eta/xsi variables
    mPositionLeadingEdge.Init(this);
    mPositionTrailingEdge.Init(this);
    mPositionInnerBorder.Init(this);
    mPositionOuterBorder.Init(this);

    uid = "";
    
    geometryValid = false;

    // [[CAS_AES]] free memory for stringer
    if (stringer != NULL) {
        delete stringer;
        stringer = NULL;
    }
    for(int i = 0; i < explicitStringers.size(); i++) {
        delete explicitStringers[i];
    }
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
    // check path
    if ( tixiCheckElement(tixiHandle, cellXPath.c_str()) != SUCCESS) {
        LOG(ERROR) << "Wing Cell " << cellXPath << " not found in CPACS file!" << std::endl;
        return;
    }
    
    Reset();
    
    // Get UID
    char * nameStr = NULL;
    if ( tixiGetTextAttribute(tixiHandle, cellXPath.c_str(), "uID", &nameStr) != SUCCESS ) {
        throw tigl::CTiglError("No UID given for wing cell " + cellXPath + "!", TIGL_UID_ERROR);
    }
    
    // [[CAS_AES]] removed variables for eta/xsi values, are read in the CCPACSWingCellPosition... members

    // [[CAS_AES]] Get subelement stringer
    int count;
    std::string elementPath  = cellXPath;
    ReturnCode tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "stringer", &count);
    if (tixiRet == SUCCESS && count == 1) {
        stringer = new CCPACSWingStringer(mParentCells->GetParentElement());
        elementPath = cellXPath + "/stringer";
        stringer->ReadCPACS(tixiHandle, elementPath);
        stringer->SetParentCell(this);
    }

    // [[CAS_AES]] replaced original read routines by our implementation supporting positioning classes
    // [[CAS_AES]] BEGIN
    // Get subelement positioningLeadingEdge
    elementPath = cellXPath;
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "positioningLeadingEdge", &count);

    if (tixiRet == SUCCESS && count == 1) {
        elementPath = cellXPath + "/positioningLeadingEdge";
        mPositionLeadingEdge.Init(this);
        mPositionLeadingEdge.ReadCPACS(tixiHandle, elementPath);
    }
    else {
        throw CTiglError("XML error: reading <positioningLeadingEdge> failed in CCPACSWingCell::ReadCPACS", TIGL_XML_ERROR);
    }

    // Get subelement positioningTrailingEdge
    elementPath = cellXPath;
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "positioningTrailingEdge", &count);

    if (tixiRet == SUCCESS && count == 1) {
        elementPath = cellXPath + "/positioningTrailingEdge";
        mPositionTrailingEdge.Init(this);
        mPositionTrailingEdge.ReadCPACS(tixiHandle, elementPath);
    }
    else {
        throw CTiglError("XML error: reading <positioningTrailingEdge> failed in CCPACSWingCell::ReadCPACS", TIGL_XML_ERROR);
    }

    // Get subelement positioningInnerBorder
    elementPath = cellXPath;
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "positioningInnerBorder", &count);

    if (tixiRet == SUCCESS && count == 1) {
        elementPath = cellXPath + "/positioningInnerBorder";
        mPositionInnerBorder.Init(this);
        mPositionInnerBorder.ReadCPACS(tixiHandle, elementPath);
    }
    else {
        throw CTiglError("XML error: reading <positioningInnerBorder> failed in CCPACSWingCell::ReadCPACS", TIGL_XML_ERROR);
    }

    // Get subelement positioningOuterBorder
    elementPath = cellXPath;
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "positioningOuterBorder", &count);

    if (tixiRet == SUCCESS && count == 1) {
        elementPath = cellXPath + "/positioningOuterBorder";
        mPositionOuterBorder.Init(this);
        mPositionOuterBorder.ReadCPACS(tixiHandle, elementPath);
    }
    else {
        throw CTiglError("XML error: reading <positioningOuterBorder> failed in CCPACSWingCell::ReadCPACS", TIGL_XML_ERROR);
    }
    // [[CAS_AES]] END

    // read material
    std::string materialString;
    materialString = cellXPath + "/skin/material";
    if ( tixiCheckElement(tixiHandle, materialString.c_str()) == SUCCESS) {
        material.ReadCPACS(tixiHandle, materialString.c_str());
    }
    else {
        // @todo: should that be an error?
        LOG(WARNING) << "No material definition found for cell " << cellXPath;
    }
    
    // apply everything
    uid = nameStr;

    // read explicit Stringers
    elementPath = cellXPath;
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "explicitStringers", &count);

    if (tixiRet == SUCCESS && count == 1) {

        elementPath = cellXPath + "/explicitStringers";
        tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "explicitStringer", &count);

        if (tixiRet == SUCCESS && count > 0) {
            for (int i = 1; i <= count; i++) {
                std::stringstream stream;
                stream << elementPath << "/" << "explicitStringer[" << i << "]";

                if ( tixiCheckElement(tixiHandle, stream.str().c_str()) == SUCCESS) {
                    CCPACSExplicitWingStringer* newExStringer = new CCPACSExplicitWingStringer(mParentCells->GetParentElement());
                    newExStringer->SetParentCell(this);
                    newExStringer->ReadCPACS(tixiHandle, stream.str().c_str());
                    explicitStringers.push_back(newExStringer);
                }
            }
        }
    }
    
    // apply everything
    uid = nameStr;

    // [[CAS_AES]] removed local eta/xsi variables, stored in CCPACSWingCellPosition... members
}

// [[CAS_AES]] Write CPACS wings elements
void CCPACSWingCell::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & WingCellDefinitionXPath)
{
    std::string elementPath;
    std::string subelementPath;

    // Set subelement "uID"
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, WingCellDefinitionXPath.c_str(), "uID", GetUID().c_str());

    // save stringer
    if (stringer) {
        elementPath = WingCellDefinitionXPath + "/stringer";
        TixiSaveExt::TixiSaveElement(tixiHandle, WingCellDefinitionXPath.c_str(), "stringer");
        stringer->WriteCPACS(tixiHandle, elementPath);
    }

    // save positionings
    elementPath = WingCellDefinitionXPath + "/positioningLeadingEdge";
    TixiSaveExt::TixiSaveElement(tixiHandle, WingCellDefinitionXPath.c_str(), "positioningLeadingEdge");
    mPositionLeadingEdge.WriteCPACS(tixiHandle, elementPath);

    elementPath = WingCellDefinitionXPath + "/positioningTrailingEdge";
    TixiSaveExt::TixiSaveElement(tixiHandle, WingCellDefinitionXPath.c_str(), "positioningTrailingEdge");
    mPositionTrailingEdge.WriteCPACS(tixiHandle, elementPath);

    elementPath = WingCellDefinitionXPath + "/positioningInnerBorder";
    TixiSaveExt::TixiSaveElement(tixiHandle, WingCellDefinitionXPath.c_str(), "positioningInnerBorder");
    mPositionInnerBorder.WriteCPACS(tixiHandle, elementPath);

    elementPath = WingCellDefinitionXPath + "/positioningOuterBorder";
    TixiSaveExt::TixiSaveElement(tixiHandle, WingCellDefinitionXPath.c_str(), "positioningOuterBorder");
    mPositionOuterBorder.WriteCPACS(tixiHandle, elementPath);

    if (HasExplicitStringer()) {
        TixiSaveExt::TixiSaveElement(tixiHandle, WingCellDefinitionXPath.c_str(), "explicitStringers");
    
        for (int i = 1; i <= NumberOfExplicitStringer(); i++) {
            std::stringstream stream;
            stream << WingCellDefinitionXPath << "/explicitStringers/" << "explicitStringer[" << i << "]";

            explicitStringers[i-1]->WriteCPACS(tixiHandle, stream.str());
        }
    }

    // TODO: write material
}

void CCPACSWingCell::SetLeadingEdgeInnerPoint(double eta, double xsi)
{
    // [[CAS_AES]] using eta/xsi from CCPACSWingCellPosition... members
    double innerBorderEta1, innerBorderEta2;
    double leadingEdgeXsi1, leadingEdgeXsi2;

    CalcEtaXsi();
    mPositionInnerBorder.getEtaDefinition(innerBorderEta1, innerBorderEta2);
    mPositionLeadingEdge.getXsiInput(leadingEdgeXsi1, leadingEdgeXsi2);

    leadingEdgeXsi1 = xsi;
    innerBorderEta1 = eta;
    // [[CAS_AES]] set values in positioning class
    mPositionInnerBorder.setEtaDefinition(innerBorderEta1, innerBorderEta2);
    mPositionLeadingEdge.setXsiInput(leadingEdgeXsi1, leadingEdgeXsi2);
}

void CCPACSWingCell::SetLeadingEdgeOuterPoint(double eta, double xsi)
{
    // [[CAS_AES]] using eta/xsi from CCPACSWingCellPosition... members
    double outerBorderEta1, outerBorderEta2;
    double leadingEdgeXsi1, leadingEdgeXsi2;

    CalcEtaXsi();
    mPositionOuterBorder.getEtaDefinition(outerBorderEta1, outerBorderEta2);
    mPositionLeadingEdge.getXsiInput(leadingEdgeXsi1, leadingEdgeXsi2);

    leadingEdgeXsi2 = xsi;
    outerBorderEta1 = eta;
    // [[CAS_AES]] set values in positioning class
    mPositionOuterBorder.setEtaDefinition(outerBorderEta1, outerBorderEta2);
    mPositionLeadingEdge.setXsiInput(leadingEdgeXsi1, leadingEdgeXsi2);
}

void CCPACSWingCell::SetTrailingEdgeInnerPoint(double eta, double xsi)
{
    // [[CAS_AES]] using eta/xsi from CCPACSWingCellPosition... members
    double innerBorderEta1, innerBorderEta2;
    double trailingEdgeXsi1, trailingEdgeXsi2;

    CalcEtaXsi();
    mPositionInnerBorder.getEtaDefinition(innerBorderEta1, innerBorderEta2);
    mPositionTrailingEdge.getXsiInput(trailingEdgeXsi1, trailingEdgeXsi2);

    trailingEdgeXsi1 = xsi;
    innerBorderEta2 = eta;
    // [[CAS_AES]] set values in positioning class
    mPositionInnerBorder.setEtaDefinition(innerBorderEta1, innerBorderEta2);
    mPositionTrailingEdge.setXsiInput(trailingEdgeXsi1, trailingEdgeXsi2);
}

void CCPACSWingCell::SetTrailingEdgeOuterPoint(double eta, double xsi)
{
    // [[CAS_AES]] using eta/xsi from CCPACSWingCellPosition... members
    double outerBorderEta1, outerBorderEta2;
    double trailingEdgeXsi1, trailingEdgeXsi2;

    CalcEtaXsi();
    mPositionOuterBorder.getEtaDefinition(outerBorderEta1, outerBorderEta2);
    mPositionTrailingEdge.getXsiInput(trailingEdgeXsi1, trailingEdgeXsi2);

    trailingEdgeXsi2 = xsi;
    outerBorderEta2 = eta;
    // [[CAS_AES]] set values in positioning class
    mPositionOuterBorder.setEtaDefinition(outerBorderEta1, outerBorderEta2);
    mPositionTrailingEdge.setXsiInput(trailingEdgeXsi1, trailingEdgeXsi2);
}

void CCPACSWingCell::GetLeadingEdgeInnerPoint(double* eta, double* xsi) const
{
    // [[CAS_AES]] using eta/xsi from CCPACSWingCellPosition... members
    double innerBorderEta1, innerBorderEta2;
    double leadingEdgeXsi1, leadingEdgeXsi2;

    CalcEtaXsi();
    mPositionInnerBorder.getEtaDefinition(innerBorderEta1, innerBorderEta2);
    mPositionLeadingEdge.getXsiInput(leadingEdgeXsi1, leadingEdgeXsi2);

    *xsi = leadingEdgeXsi1;
    *eta = innerBorderEta1;
}

void CCPACSWingCell::GetLeadingEdgeOuterPoint(double* eta, double* xsi) const
{
    // [[CAS_AES]] using eta/xsi from CCPACSWingCellPosition... members
    double outerBorderEta1, outerBorderEta2;
    double leadingEdgeXsi1, leadingEdgeXsi2;

    CalcEtaXsi();
    mPositionOuterBorder.getEtaDefinition(outerBorderEta1, outerBorderEta2);
    mPositionLeadingEdge.getXsiInput(leadingEdgeXsi1, leadingEdgeXsi2);

    *xsi = leadingEdgeXsi2;
    *eta = outerBorderEta1;
}

void CCPACSWingCell::GetTrailingEdgeInnerPoint(double* eta, double* xsi) const
{
    // [[CAS_AES]] using eta/xsi from CCPACSWingCellPosition... members
    double innerBorderEta1, innerBorderEta2;
    double trailingEdgeXsi1, trailingEdgeXsi2;

    CalcEtaXsi();
    mPositionInnerBorder.getEtaDefinition(innerBorderEta1, innerBorderEta2);
    mPositionTrailingEdge.getXsiInput(trailingEdgeXsi1, trailingEdgeXsi2);

    *xsi = trailingEdgeXsi1;
    *eta = innerBorderEta2;
}

void CCPACSWingCell::GetTrailingEdgeOuterPoint(double* eta, double* xsi) const
{
    // [[CAS_AES]] using eta/xsi from CCPACSWingCellPosition... members
    double outerBorderEta1, outerBorderEta2;
    double trailingEdgeXsi1, trailingEdgeXsi2;

    CalcEtaXsi();
    mPositionOuterBorder.getEtaDefinition(outerBorderEta1, outerBorderEta2);
    mPositionTrailingEdge.getXsiInput(trailingEdgeXsi1, trailingEdgeXsi2);

    *xsi = trailingEdgeXsi2;
    *eta = outerBorderEta2;
}

CCPACSMaterial &CCPACSWingCell::GetMaterial()
{
    return material;
}

// [[CAS_AES]] Added setter for the leading edge positioning
void CCPACSWingCell::SetPositionLeadingEdge(CCPACSWingCellPositionChordwise& nPosLE)
{
    double X1 = 0., X2 = 0.;
    nPosLE.getXsiInput(X1, X2);
    std::string SUId = nPosLE.getSparUId();

    if (nPosLE.XsiInput()) {
        mPositionLeadingEdge.setXsiInput(X1, X2);
    }
    else if (nPosLE.SparInput()) {
        mPositionLeadingEdge.setSparUId(SUId);
    }
}

// [[CAS_AES]] Added getter for the leading edge positioning
CCPACSWingCellPositionChordwise& CCPACSWingCell::GetPositionLeadingEdge(void)
{
    return mPositionLeadingEdge;
}

// [[CAS_AES]] Added setter for the trailing edge positioning
void CCPACSWingCell::SetPositionTrailingEdge(CCPACSWingCellPositionChordwise& nPosTE)
{
    double X1 = 0.,X2 = 0.;
    nPosTE.getXsiInput(X1, X2);
    std::string SUId = nPosTE.getSparUId();

    if (nPosTE.XsiInput()) {
        mPositionTrailingEdge.setXsiInput(X1, X2);
    }
    else if (nPosTE.SparInput()) {
        mPositionTrailingEdge.setSparUId(SUId);
    }
}

// [[CAS_AES]] Added getter for the trailing edge positioning
CCPACSWingCellPositionChordwise& CCPACSWingCell::GetPositionTrailingEdge(void)
{
    return mPositionTrailingEdge;
}

// [[CAS_AES]] Added setter for inner border positioning
void CCPACSWingCell::SetPositionInnerBorder(CCPACSWingCellPositionSpanwise& nPosIB)
{
    double E1 = 0.,E2 = 0.;
    int nR = 0;
    std::string RUId("");

    nPosIB.getEtaDefinition(E1, E2);
    nPosIB.getRibDefinition(RUId, nR);

    if (nPosIB.EtaInput()) {
        mPositionInnerBorder.setEtaDefinition(E1, E2);
    }
    else if (nPosIB.RibInput()) {
        mPositionInnerBorder.setRibDefinition(RUId, nR);
    }
}

// [[CAS_AES]] Added getter for inner border positioning
CCPACSWingCellPositionSpanwise& CCPACSWingCell::GetPositionInnerBorder(void)
{
    return mPositionInnerBorder;
}

// [[CAS_AES]] Added setter for outer border positioning
void CCPACSWingCell::SetPositionOuterBorder(CCPACSWingCellPositionSpanwise& nPosOB)
{
    double E1 = 0.,E2 = 0.;
    int nR = 0;
    std::string RUId("");

    nPosOB.getEtaDefinition(E1, E2);
    nPosOB.getRibDefinition(RUId, nR);

    if (nPosOB.EtaInput()) {
        mPositionOuterBorder.setEtaDefinition(E1, E2);
    }
    else if (nPosOB.RibInput()) {
        mPositionOuterBorder.setRibDefinition(RUId, nR);
    }
}

// [[CAS_AES]] Added getter for outer border positioning
CCPACSWingCellPositionSpanwise& CCPACSWingCell::GetPositionOuterBorder(void)
{
    return mPositionOuterBorder;
}

// [[CAS_AES]] get Pointer to parent cells element
CCPACSWingCells* CCPACSWingCell::GetParentElement(void)
{
    return mParentCells;
}

// [[CAS_AES]] Getter for the stringer
CCPACSWingStringer& CCPACSWingCell::GetStringer()
{
    if (stringer == NULL) {
        throw CTiglError("CCPACSWingCell::GetStringer: no stringer defined!");
    }

    return *stringer;
}

// [[CAS_AES]] Returns whether a stringer definition exists or not
bool CCPACSWingCell::HasStringer()
{
    return (stringer != NULL);
}

// [[CAS_AES]] calculates the Eta/Xsi values of the Positions, if they are defined by rib or spar
void CCPACSWingCell::CalcEtaXsi() const
{

    double IBEta1, IBEta2, OBEta1, OBEta2;

    if (mPositionInnerBorder.RibInput() && !mPositionInnerBorder.EtaInput()) {
        mPositionInnerBorder.calcEta();
    }

    if (mPositionOuterBorder.RibInput() && !mPositionOuterBorder.EtaInput()) {
        mPositionOuterBorder.calcEta();
    }

    mPositionInnerBorder.getEtaDefinition(IBEta1, IBEta2);
    mPositionOuterBorder.getEtaDefinition(OBEta1, OBEta2);

    if (mPositionLeadingEdge.SparInput() && !mPositionLeadingEdge.XsiInput()) {
        mPositionLeadingEdge.calcXsi(IBEta1, OBEta1);
    }

    if (mPositionTrailingEdge.SparInput() && !mPositionTrailingEdge.XsiInput()) {
        mPositionTrailingEdge.calcXsi(IBEta2, OBEta2);
    }

}

bool CCPACSWingCell::HasExplicitStringer()
{
    return (NumberOfExplicitStringer() > 0);
}

int CCPACSWingCell::NumberOfExplicitStringer()
{
    return explicitStringers.size();
}

CCPACSExplicitWingStringer* CCPACSWingCell::GetExplicitStringerbyIndex(int id)
{

    if (id < 1 || id > NumberOfExplicitStringer()) {
        throw CTiglError("CCPACSWingCell::getExplicitStringerbyID: id is wrong!");
    }

    return explicitStringers[id-1];
}

void CCPACSWingCell::Update()
{
    if (geometryValid) {
        return;
    }
    
    BuildSkinGeometry();
    geometryValid = true;
}

TopoDS_Shape CCPACSWingCell::GetCellSkinGeometry(bool transform)
{
    Update();

    if (transform) {
        return mParentCells->GetParentElement()->GetWingStructureReference().GetComponentSegment()->GetWing().GetTransformation().Transform(cellSkinGeometry);
    }
    else {
        return cellSkinGeometry;
    }
}

void CCPACSWingCell::BuildSkinGeometry()
{
    // variable for uv bounds
    double u_min=0, u_max=0, v_min=0, v_max=0;
    
    BRep_Builder builder;

    CalcEtaXsi();

    gp_Pnt p1 = mParentCells->GetParentElement()->GetWingStructureReference().GetLeadingEdgePoint(0);
    gp_Pnt p2 = mParentCells->GetParentElement()->GetWingStructureReference().GetLeadingEdgePoint(1);
    gp_Vec yRefDir(p1,p2);
    yRefDir.Normalize();

    // create a reference direction without sweep angle
    gp_Pnt p2stern = gp_Pnt(p1.X(), p2.Y(), p2.Z());
    gp_Vec yRefDirStern(p1,p2stern);
    yRefDirStern.Normalize();

    double sweepAngle = yRefDir.Angle(yRefDirStern);
    
    if (p2.X() < p1.X()) {
        sweepAngle = -sweepAngle;
    }
    
    gp_Pnt p5 = mParentCells->GetParentElement()->GetWingStructureReference().GetTrailingEdgePoint(0);
    gp_Pnt p6 = mParentCells->GetParentElement()->GetWingStructureReference().GetTrailingEdgePoint(1);

    gp_Vec xRefDir(p1,p5);
    gp_Vec zRefDir(-yRefDirStern^xRefDir);
    zRefDir.Normalize();
    
    // Step 8: find the correct part of the loft
    TopoDS_Shape loftShape;
    TiglLoftSide side = mParentCells->GetParentElement()->GetLoftSide();
    if (side == UPPER_SIDE) {
        loftShape = mParentCells->GetParentElement()->GetWingStructureReference().GetUpperShape();
    }
    else {
        loftShape = mParentCells->GetParentElement()->GetWingStructureReference().GetLowerShape();
    }
    
    // determine diagonal vector of loft bounding box (e.g. used for size of cut faces)
    Bnd_Box boundingBox;
    BRepBndLib::Add(mParentCells->GetParentElement()->GetWingStructureReference().GetUpperShape(), boundingBox);
    BRepBndLib::Add(mParentCells->GetParentElement()->GetWingStructureReference().GetLowerShape(), boundingBox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    gp_Vec diagonal(xmax-xmin, ymax-ymin, zmax-zmin);
    Standard_Real bboxSize = diagonal.Magnitude();
    
    gp_Pnt pC1, pC2, pC3, pC4;
    double LEXsi1, LEXsi2, TEXsi1, TEXsi2;
    double IBEta1, IBEta2, OBEta1, OBEta2;
    
    CCPACSWingCellPositionChordwise& PosLE = GetPositionLeadingEdge();
    CCPACSWingCellPositionChordwise& PosTE = GetPositionTrailingEdge();

    CCPACSWingCellPositionSpanwise& PosIB = GetPositionInnerBorder();
    CCPACSWingCellPositionSpanwise& PosOB = GetPositionOuterBorder();

    PosLE.getXsiInput(LEXsi1, LEXsi2);
    PosTE.getXsiInput(TEXsi1, TEXsi2);
    PosIB.getEtaDefinition(IBEta1, IBEta2);
    PosOB.getEtaDefinition(OBEta1, OBEta2);

    pC1 = mParentCells->GetParentElement()->GetWingStructureReference().GetMidplanePoint(IBEta1, LEXsi1);
    pC2 = mParentCells->GetParentElement()->GetWingStructureReference().GetMidplanePoint(OBEta1, LEXsi2);
    pC3 = mParentCells->GetParentElement()->GetWingStructureReference().GetMidplanePoint(IBEta2, TEXsi1);
    pC4 = mParentCells->GetParentElement()->GetWingStructureReference().GetMidplanePoint(OBEta2, TEXsi2);
    
   
     // project the cornerpoints on the lower or upper surface
    TopoDS_Shape cutEdge1, cutEdge2, cutEdge3, cutEdge4;
    // build line along z reference axis for intersection points on loft
    gp_Vec offset(zRefDir*bboxSize);
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
        throw CTiglError("Error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values on the inner border.");
    }
    if (pC2.X() > pC4.X()) {
        throw CTiglError("Error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values on the outer border.");
    }
    if (pC1.Y() > pC2.Y()) {
        throw CTiglError("Error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the leading edge.");
    }
    if (pC3.Y() > pC4.Y()) {
        throw CTiglError("Error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the trailing edge.");
    }

// combine the cell cutting planes to a compound
    TopoDS_Compound Planecomp;
    builder.MakeCompound(Planecomp);

    gp_Pln cutPlaneLE, cutPlaneTE, cutPlaneIB, cutPlaneOB;

// build the Leading edge cutting plane
    gp_Vec vCLE(pC1, pC2);
    vCLE.Normalize();
    gp_Pnt midPnt = CTiglCommon::getMidpoint(pC1, pC2);
    gp_Ax3 refAxLE(midPnt, -zRefDir^vCLE, vCLE);
    cutPlaneLE = gp_Pln(refAxLE);
    TopoDS_Shape planeShapeLE = BRepBuilderAPI_MakeFace(cutPlaneLE).Face();
// build the Trailing edge cutting plane
    gp_Vec vCTE(pC3, pC4);
    vCTE.Normalize();
    midPnt = CTiglCommon::getMidpoint(pC3, pC4);
    gp_Ax3 refAxTE(midPnt, zRefDir^vCTE, vCTE);
    cutPlaneTE = gp_Pln(refAxTE);
    TopoDS_Shape planeShapeTE = BRepBuilderAPI_MakeFace(cutPlaneTE).Face();

    // build the inner border cutting plane
    gp_Vec vCIB(pC1, pC3);
    vCIB.Normalize();
    midPnt = CTiglCommon::getMidpoint(pC1, pC3);
    gp_Ax3 refAxIB(midPnt, zRefDir^vCIB, vCIB);
    cutPlaneIB = gp_Pln(refAxIB);
    TopoDS_Shape planeShapeIB = BRepBuilderAPI_MakeFace(cutPlaneIB).Face();

// build the Outer border cutting plane
    gp_Vec vCOB(pC2, pC4);
    vCOB.Normalize();
    midPnt = CTiglCommon::getMidpoint(pC2, pC4);
    gp_Ax3 refAxOB(midPnt, -zRefDir^vCOB, vCOB);
    cutPlaneOB = gp_Pln(refAxOB);
    TopoDS_Shape planeShapeOB = BRepBuilderAPI_MakeFace(cutPlaneOB).Face();


// If any border is defined by a rib or a spar, the cutting plane is changed

    double u05 = 0.5, u1 = 1., v0 = 0., v1 = 1.;

    if (PosLE.SparInput()) {
        planeShapeLE = PosLE.sparCuttingplane();
    }

    if (PosTE.SparInput()) {
        planeShapeTE = PosTE.sparCuttingplane();
    }

    if (PosIB.RibInput()) {
        planeShapeIB = PosIB.getRibCuttingPlane();
    }
    // if the inner border of the cell is the inner border of the Component segment
    // a cutting plane from the inner border of the WCS is created
    // this is necessary due to cutting precision
    else if (IBEta1 == 0. && IBEta2 == 0.) {
        BRepAdaptor_Surface surf(mParentCells->GetParentElement()->GetWingStructureReference().GetInnerFace());
        gp_Pnt p0 = surf.Value(u05, v0);
        gp_Pnt pU = surf.Value(u05, v1);
        gp_Pnt pV = surf.Value(u1, v0);
        gp_Ax3 ax0UV(p0, gp_Vec(p0, pU)^gp_Vec(p0, pV), gp_Vec(p0, pU));

        planeShapeIB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
    }

    if (PosOB.RibInput()) {
        planeShapeOB = PosOB.getRibCuttingPlane();
    }
    // if the outer border of the cell is the outer border of the Component segment
    // a cutting plane from the outer border of the WCS is created
    // this is necessary due to cutting precision
    else if (OBEta1 == 1. && OBEta2 == 1.) {
        BRepAdaptor_Surface surf(mParentCells->GetParentElement()->GetWingStructureReference().GetOuterFace());
        gp_Pnt p0 = surf.Value(u05, v0);
        gp_Pnt pU = surf.Value(u05, v1);
        gp_Pnt pV = surf.Value(u1, v0);
        gp_Ax3 ax0UV(p0, gp_Vec(p0, pU)^gp_Vec(p0, pV), gp_Vec(p0, pU));

        planeShapeOB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
    }

    builder.Add(Planecomp, planeShapeLE);
    builder.Add(Planecomp, planeShapeTE);
    builder.Add(Planecomp, planeShapeIB);
    builder.Add(Planecomp, planeShapeOB);

// cut the lower or upper loft with the planes
    TopoDS_Shape result = CTiglCommon::splitShape(loftShape, Planecomp);
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(result, TopAbs_FACE, faceMap);
    
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    bool notFound = true;

    mCutPlaneLE = cutPlaneLE;
    mCutPlaneTE = cutPlaneTE;
    mCutPlaneIB = cutPlaneIB;
    mCutPlaneOB = cutPlaneOB;

    mPlaneShapeLE = planeShapeLE;
    mPlaneShapeTE = planeShapeTE;
    mPlaneShapeIB = planeShapeIB;
    mPlaneShapeOB = planeShapeOB;

    mPC1 = pC1;
    mPC2 = pC2;
    mPC3 = pC3;
    mPC4 = pC4;
    
    for (int f = 1; f <= faceMap.Extent(); f++) {

        TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
        Handle(Geom_Surface) surf = BRep_Tool::Surface(loftFace);
        BRepTools::UVBounds(loftFace, u_min, u_max, v_min, v_max);
        gp_Pnt pTest = surf->Value(u_min + ((u_max - u_min) / 4), v_min + ((v_max - v_min) / 4));

    // create each midpoint for the vector basis

        // test if the midplane point is behind the leading edge border plane
        // create an Ax1 from the Leading edge plane origin to the midpoint of the current face
        gp_Pnt midPnt = CTiglCommon::getMidpoint(pC1, pC2);
        gp_Vec vTest(midPnt, pTest);
        gp_Ax1 a1Test(midPnt, vTest);
        
        bool sparTest = false, plainTest = false;

        if (PosLE.SparInput()) {
            sparTest = mParentCells->GetParentElement()->SparSegmentsTest(cutPlaneLE.Axis(), pTest, planeShapeLE);
        }
        else {
            plainTest = a1Test.Angle(cutPlaneLE.Axis()) < (90.0 * (M_PI / 180.));
        }

        if(plainTest || sparTest) {
        // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
            midPnt = CTiglCommon::getMidpoint(pC3, pC4);
            vTest = gp_Vec(midPnt, pTest);
            a1Test = gp_Ax1(midPnt, vTest);

            sparTest = false;
            plainTest = false;
            if (PosTE.SparInput()) {
                sparTest = mParentCells->GetParentElement()->SparSegmentsTest(cutPlaneTE.Axis(), pTest, planeShapeTE);
            }
            else {
                plainTest = a1Test.Angle(cutPlaneTE.Axis()) < (90.0 * (M_PI / 180.));
            }

            if (plainTest || sparTest) {
                // create an Ax1 from the inner border plane origin to the midpoint of the current face
                midPnt = CTiglCommon::getMidpoint(pC1, pC3);
                vTest = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);
                if (a1Test.Angle(cutPlaneIB.Axis()) < (90.0 * (M_PI / 180.))) {
                // create an Ax1 from the outer border plane origin to the midpoint of the current face
                    midPnt = CTiglCommon::getMidpoint(pC2, pC4);
                    vTest = gp_Vec(midPnt, pTest);
                    a1Test = gp_Ax1(midPnt, vTest);
                    if (a1Test.Angle(cutPlaneOB.Axis()) < (90.0 * (M_PI / 180.))) {
                        builder.Add(compound, loftFace);
                        notFound = false;
                    }
                }
            }
        }
    }

    if (notFound) {
        throw CTiglError("Error: Can not find a matching edge for cell input CCPACSWingCell::BuildSkinGeometry!");
    }
    
    cellSkinGeometry = compound;
}


TIGL_EXPORT bool CCPACSWingCell::IsPartOfCell(TopoDS_Face f)
{
    Update();

    Bnd_Box bBox1, bBox2;
    BRepBndLib::Add(cellSkinGeometry, bBox1);
    TopoDS_Face face = TopoDS::Face(mParentCells->GetParentElement()->GetWingStructureReference().GetComponentSegment()->GetWing().GetTransformation().Inverted().Transform(f));
    BRepBndLib::Add(face, bBox2);

    if (bBox1.IsOut(bBox2)) {
        return false;
    }

    double u_min = 0., u_max = 0., v_min = 0., v_max = 0.;
    Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
    BRepTools::UVBounds(face, u_min, u_max, v_min, v_max);
    gp_Pnt pTest = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));

    CCPACSWingCellPositionChordwise& PosLE = GetPositionLeadingEdge();
    CCPACSWingCellPositionChordwise& PosTE = GetPositionTrailingEdge();
    CCPACSWingCellPositionSpanwise& PosIB = GetPositionInnerBorder();
    CCPACSWingCellPositionSpanwise& PosOB = GetPositionOuterBorder();

    gp_Pnt midPnt = CTiglCommon::getMidpoint(mPC1, mPC2);
    gp_Vec vTest(midPnt, pTest);
    gp_Ax1 a1Test(midPnt, vTest);

    bool sparTest = false, plainTest = false;

    if (PosLE.SparInput()) {
        sparTest = mParentCells->GetParentElement()->SparSegmentsTest(mCutPlaneLE.Axis(), pTest, mPlaneShapeLE);
    }
    else {
        plainTest = a1Test.Angle(mCutPlaneLE.Axis()) < (90.0 * (M_PI / 180.));
    }

    if (plainTest || sparTest) {
        // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
        midPnt = CTiglCommon::getMidpoint(mPC3, mPC4);
        vTest = gp_Vec(midPnt, pTest);
        a1Test = gp_Ax1(midPnt, vTest);

        sparTest = false;
        plainTest = false;
        if (PosTE.SparInput()) {
            sparTest = mParentCells->GetParentElement()->SparSegmentsTest(mCutPlaneTE.Axis(), pTest, mPlaneShapeTE);
        }
        else {
            plainTest = a1Test.Angle(mCutPlaneTE.Axis()) < (90.0 * (M_PI / 180.));
        }

        if (plainTest || sparTest) {
            // create an Ax1 from the inner border plane origin to the midpoint of the current face
            midPnt = CTiglCommon::getMidpoint(mPC1, mPC3);
            vTest = gp_Vec(midPnt, pTest);
            a1Test = gp_Ax1(midPnt, vTest);
            if (a1Test.Angle(mCutPlaneIB.Axis()) < (90.0 * (M_PI / 180.))) {
                // create an Ax1 from the outer border plane origin to the midpoint of the current face
                midPnt = CTiglCommon::getMidpoint(mPC2, mPC4);
                vTest = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);
                if (a1Test.Angle(mCutPlaneOB.Axis()) < (90.0 * (M_PI / 180.))) {
                    return true;
                }
            }
        }
    }

    return false;
}


TIGL_EXPORT bool CCPACSWingCell::IsPartOfCell(TopoDS_Edge e)
{
    Update();

    Bnd_Box bBox1, bBox2;
    BRepBndLib::Add(cellSkinGeometry, bBox1);
    TopoDS_Edge edge = TopoDS::Edge(mParentCells->GetParentElement()->GetWingStructureReference().GetComponentSegment()->GetWing().GetTransformation().Inverted().Transform(e));
    BRepBndLib::Add(edge, bBox2);

    if (bBox1.IsOut(bBox2)) {
        return false;
    }

    double u_min = 0., u_max = 0.;

    BRepAdaptor_Curve curve(edge);
    u_min = curve.FirstParameter();
    u_max = curve.LastParameter();

    gp_Pnt pTest = curve.Value(u_min + ((u_max - u_min) / 2));

    CCPACSWingCellPositionChordwise& PosLE = GetPositionLeadingEdge();
    CCPACSWingCellPositionChordwise& PosTE = GetPositionTrailingEdge();
    CCPACSWingCellPositionSpanwise& PosIB = GetPositionInnerBorder();
    CCPACSWingCellPositionSpanwise& PosOB = GetPositionOuterBorder();

    gp_Pnt midPnt = CTiglCommon::getMidpoint(mPC1, mPC2);
    gp_Vec vTest(midPnt, pTest);
    gp_Ax1 a1Test(midPnt, vTest);

    bool sparTest = false, plainTest = false;

    if (PosLE.SparInput()) {
        sparTest = mParentCells->GetParentElement()->SparSegmentsTest(mCutPlaneLE.Axis(), pTest, mPlaneShapeLE);
    }
    else {
        plainTest = a1Test.Angle(mCutPlaneLE.Axis()) < (90.0 * (M_PI / 180.));
    }

    if (plainTest || sparTest) {
        // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
        midPnt = CTiglCommon::getMidpoint(mPC3, mPC4);
        vTest = gp_Vec(midPnt, pTest);
        a1Test = gp_Ax1(midPnt, vTest);

        sparTest = false;
        plainTest = false;
        if (PosTE.SparInput()) {
            sparTest = mParentCells->GetParentElement()->SparSegmentsTest(mCutPlaneTE.Axis(), pTest, mPlaneShapeTE);
        }
        else {
            plainTest = a1Test.Angle(mCutPlaneTE.Axis()) < (90.0 * (M_PI / 180.));
        }

        if (plainTest || sparTest) {
            // create an Ax1 from the inner border plane origin to the midpoint of the current face
            midPnt = CTiglCommon::getMidpoint(mPC1, mPC3);
            vTest = gp_Vec(midPnt, pTest);
            a1Test = gp_Ax1(midPnt, vTest);
            if (a1Test.Angle(mCutPlaneIB.Axis()) < (90.0 * (M_PI / 180.))) {
                // create an Ax1 from the outer border plane origin to the midpoint of the current face
                midPnt = CTiglCommon::getMidpoint(mPC2, mPC4);
                vTest = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);
                if (a1Test.Angle(mCutPlaneOB.Axis()) < (90.0 * (M_PI / 180.))) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

} // namespace tigl
