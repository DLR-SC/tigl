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

#include "tixi.h"

#include "CTiglError.h"
#include "CTiglLogging.h"

#include <cmath>

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

CCPACSWingCell::CCPACSWingCell()
{
    reset();
}

const std::string &CCPACSWingCell::GetUID() const
{
    return uid;
}


void CCPACSWingCell::reset()
{
    innerBorderEta1 = 0.;
    innerBorderEta2 = 0.;
    outerBorderEta1 = 0.;
    outerBorderEta2 = 0.;
    leadingEdgeXsi1 = 0.;
    leadingEdgeXsi2 = 0.;
    trailingEdgeXsi1 = 0.;
    trailingEdgeXsi2 = 0.;
    
    uid = "";

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
    
    reset();
    
    // Get UID
    char * nameStr = NULL;
    if ( tixiGetTextAttribute(tixiHandle, cellXPath.c_str(), "uID", &nameStr) != SUCCESS ) {
        throw tigl::CTiglError("No UID given for wing cell " + cellXPath + "!", TIGL_UID_ERROR);
    }
    
    double iBE1, iBE2, oBE1, oBE2, lEX1, lEX2, tEX1, tEX2;
    
    // get postionings of cell
    std::string positioningString;
    positioningString = cellXPath + "/positioningLeadingEdge/sparUID";
    if ( tixiCheckElement(tixiHandle, positioningString.c_str()) == SUCCESS) {
        LOG(WARNING) << "In " << cellXPath << ": Cell positiongs via spars is currently not supported by TiGL. Please use eta/xsi definitions.";
        lEX1 = 0.; lEX2 = 0.;
    }
    else {
        positioningString = cellXPath + "/positioningLeadingEdge/xsi1";
        if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &lEX1) != SUCCESS) {
            throw tigl::CTiglError("No leading edge xsi1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
        }
        
        positioningString = cellXPath + "/positioningLeadingEdge/xsi2";
        if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &lEX2) != SUCCESS) {
            throw tigl::CTiglError("No leading edge xsi2 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
        }
    }

    positioningString = cellXPath + "/positioningTrailingEdge/sparUID";
    if ( tixiCheckElement(tixiHandle, positioningString.c_str()) == SUCCESS) {
        LOG(WARNING) << "In " << cellXPath << ": Cell positiongs via spars is currently not supported by TiGL. Please use eta/xsi definitions.";
        tEX1 = 0.; tEX2 = 0.;
    }
    else {
        positioningString = cellXPath + "/positioningTrailingEdge/xsi1";
        if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &tEX1) != SUCCESS) {
            throw tigl::CTiglError("No leading edge xsi1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
        }
        
        positioningString = cellXPath + "/positioningTrailingEdge/xsi2";
        if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &tEX2) != SUCCESS) {
            throw tigl::CTiglError("No leading edge xsi1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
        }
        
    }
    
    positioningString = cellXPath + "/positioningInnerBorder/ribDefinitionUID";
    if ( tixiCheckElement(tixiHandle, positioningString.c_str()) == SUCCESS) {
        LOG(WARNING) << "In " << cellXPath << ": Cell positiongs via ribs is currently not supported by TiGL. Please use eta/xsi definitions.";
        iBE1 = 0.; iBE2 = 0.;
    }
    else {
        positioningString = cellXPath + "/positioningInnerBorder/eta1";
        if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &iBE1) != SUCCESS) {
            throw tigl::CTiglError("No inner border eta1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
        }
        
        positioningString = cellXPath + "/positioningInnerBorder/eta2";
        if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &iBE2) != SUCCESS) {
            throw tigl::CTiglError("No inner border eta2 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
        }
    }
    
    positioningString = cellXPath + "/positioningOuterBorder/ribDefinitionUID";
    if ( tixiCheckElement(tixiHandle, positioningString.c_str()) == SUCCESS) {
        LOG(WARNING) << "In " << cellXPath << ": Cell positiongs via ribs is currently not supported by TiGL. Please use eta/xsi definitions.";
        oBE1 = 0.; oBE2 = 0.;
    }
    else {
        positioningString = cellXPath + "/positioningOuterBorder/eta1";
        if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &oBE1) != SUCCESS) {
            throw tigl::CTiglError("No outer border eta1 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
        }
        
        positioningString = cellXPath + "/positioningOuterBorder/eta2";
        if ( tixiGetDoubleElement(tixiHandle, positioningString.c_str(), &oBE2) != SUCCESS) {
            throw tigl::CTiglError("No outer border eta2 positioning given for wing cell " + cellXPath + "!", TIGL_ERROR);
        }
    }
    
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
    
    innerBorderEta1 = iBE1;
    innerBorderEta2 = iBE2;
    outerBorderEta1 = oBE1;
    outerBorderEta2 = oBE2;
    leadingEdgeXsi1 = lEX1;
    leadingEdgeXsi2 = lEX2;
    trailingEdgeXsi1 = tEX1;
    trailingEdgeXsi2 = tEX2;
}

void CCPACSWingCell::SetLeadingEdgeInnerPoint(double eta, double xsi)
{
    leadingEdgeXsi1 = xsi;
    innerBorderEta1 = eta;
}

void CCPACSWingCell::SetLeadingEdgeOuterPoint(double eta, double xsi)
{
    leadingEdgeXsi2 = xsi;
    outerBorderEta1 = eta;
}

void CCPACSWingCell::SetTrailingEdgeInnerPoint(double eta, double xsi)
{
    trailingEdgeXsi1 = xsi;
    innerBorderEta2 = eta;
}

void CCPACSWingCell::SetTrailingEdgeOuterPoint(double eta, double xsi)
{
    trailingEdgeXsi2 = xsi;
    outerBorderEta2 = eta;
}

void CCPACSWingCell::GetLeadingEdgeInnerPoint(double* eta, double* xsi) const
{
    *xsi = leadingEdgeXsi1;
    *eta = innerBorderEta1;
}

void CCPACSWingCell::GetLeadingEdgeOuterPoint(double* eta, double* xsi) const
{
    *xsi = leadingEdgeXsi2;
    *eta = outerBorderEta1;
}

void CCPACSWingCell::GetTrailingEdgeInnerPoint(double* eta, double* xsi) const
{
    *xsi = trailingEdgeXsi1;
    *eta = innerBorderEta2;
}

void CCPACSWingCell::GetTrailingEdgeOuterPoint(double* eta, double* xsi) const
{
    *xsi = trailingEdgeXsi2;
    *eta = outerBorderEta2;
}

CCPACSMaterial &CCPACSWingCell::GetMaterial()
{
    return material;
}

} // namespace tigl
