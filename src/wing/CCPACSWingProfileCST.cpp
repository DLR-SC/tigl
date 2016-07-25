/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-17 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
/**
* @file
* @brief  Implementation of CPACS wing profile as a CST profile
*
* The wing profile is defined by the (C)lass function / (S)hape function (T)ransformation
* geometry representation method.
*/

#include "CCPACSWingProfileCST.h"

#include "CCSTCurveBuilder.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglTransformation.h"
#include "CWireToCurve.h"
#include "TixiSaveExt.h"
#include "math.h"

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Standard_Version.hxx>


namespace tigl
{
// Constructor
CCPACSWingProfileCST::CCPACSWingProfileCST()
{
    trailingEdge.Nullify();
}

// Destructor
CCPACSWingProfileCST::~CCPACSWingProfileCST()
{
}

std::string CCPACSWingProfileCST::CPACSID()
{
    return "cst2D";
}

void CCPACSWingProfileCST::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& profileXPath) {
    ProfileDataXPath = profileXPath;
    generated::CPACSCst2D::ReadCPACS(tixiHandle, profileXPath);
}

void CCPACSWingProfileCST::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& profileXPath) const {
    generated::CPACSCst2D::WriteCPACS(tixiHandle, profileXPath);
}

// Cleanup routine
void CCPACSWingProfileCST::Cleanup()
{
    trailingEdge.Nullify();
}

void CCPACSWingProfileCST::Update()
{
    BuildWires();
}

// Builds the wing profile wire. The returned wire is already transformed by the
// wing profile element transformation.
void CCPACSWingProfileCST::BuildWires()
{
    gp_Trsf yzSwitch;
    yzSwitch.SetMirror(gp_Ax2(gp_Pnt(0.,0.,0.), gp_Dir(0.,-1.,1.)));
    
    // Build upper wire
    CCSTCurveBuilder upperBuilder(m_upperN1, m_upperN2, m_upperB.AsVector());
    Handle_Geom_BSplineCurve upperCurve = upperBuilder.Curve();
    upperCurve->Transform(yzSwitch);
    upperWire = BRepBuilderAPI_MakeEdge(upperCurve);
    
    // Build lower curve
    std::vector<double> binv;
    const auto& lb = m_lowerB.AsVector();
    for (unsigned int i = 0; i < lb.size(); ++i) {
        binv.push_back(-lb[i]);
    }
    
    CCSTCurveBuilder lowerBuilder(m_lowerN1, m_lowerN2, binv);
    Handle_Geom_BSplineCurve lowerCurve = lowerBuilder.Curve();
    lowerCurve->Transform(yzSwitch);
    lowerCurve->Reverse();
    lowerWire = BRepBuilderAPI_MakeEdge(lowerCurve);
    
    BRepBuilderAPI_MakeWire upperLowerWireMaker(lowerWire, upperWire);
    TopoDS_Wire upperLowerWire = upperLowerWireMaker.Wire();
    
    // conatenate wire
    Handle_Geom_Curve upperLowerCurve = CWireToCurve(upperLowerWire).curve();
    upperLowerEdge = BRepBuilderAPI_MakeEdge(upperLowerCurve);
    
    tePoint = gp_Pnt(1,0,0);
    lePoint = gp_Pnt(0,0,0);
}

// Returns sample points
std::vector<const CTiglPoint*> CCPACSWingProfileCST::GetSamplePoints() const {
    return {};
}

const std::vector<CTiglPoint>& CCPACSWingProfileCST::GetSamplePoints2() const {
    static std::vector<CTiglPoint> dummy;
    return dummy;
}

// get profiles CPACS XML path
const std::string & CCPACSWingProfileCST::GetProfileDataXPath() const
{
    return ProfileDataXPath;
}

void CCPACSWingProfileCST::SetProfileDataXPath(const std::string& xpath) {
    ProfileDataXPath = xpath;
}

// Getter for upper wire of closed profile
const TopoDS_Edge& CCPACSWingProfileCST::GetUpperWireClosed() const
{
    return upperWire;
}

// Getter for lower wire of closed profile
const TopoDS_Edge& CCPACSWingProfileCST::GetLowerWireClosed() const
{
    return lowerWire;
}

// Getter for upper wire of opened profile
const TopoDS_Edge& CCPACSWingProfileCST::GetUpperWireOpened() const
{
    throw CTiglError("ERROR: GetUpperWireOpened not implemented yet for CCPACSWingProfileCST!");
}

// Getter for lower wire of opened profile
const TopoDS_Edge& CCPACSWingProfileCST::GetLowerWireOpened() const
{
    throw CTiglError("ERROR: GetLowerWireOpened not implemented yet for CCPACSWingProfileCST!");
}

// get upper wing profile wire
const TopoDS_Edge & CCPACSWingProfileCST::GetUpperWire() const
{
    return upperWire;
}
            
// get lower wing profile wire
const TopoDS_Edge & CCPACSWingProfileCST::GetLowerWire() const
{
    return lowerWire;
}

// gets the upper and lower wing profile into on edge
const TopoDS_Edge & CCPACSWingProfileCST::GetUpperLowerWire() const
{
    return upperLowerEdge;
}

// get trailing edge
const TopoDS_Edge & CCPACSWingProfileCST::GetTrailingEdge() const
{
    return trailingEdge;
}

// get trailing edge
const TopoDS_Edge & CCPACSWingProfileCST::GetTrailingEdgeOpened() const
{
    throw CTiglError("ERROR: GetTrailingEdgeOpened not implemented yet for CCPACSWingProfileCST!");
}

// get leading edge point();
const gp_Pnt & CCPACSWingProfileCST::GetLEPoint() const
{
    return lePoint;
}
        
// get trailing edge point();
const gp_Pnt & CCPACSWingProfileCST::GetTEPoint() const
{
    return tePoint;
}

} // end namespace tigl
