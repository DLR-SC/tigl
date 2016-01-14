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
#include "CTiglTypeRegistry.h"
#include "CTiglLogging.h"
#include "CTiglTransformation.h"
#include "CWireToCurve.h"
#include "CCPACSWingProfileFactory.h"
#include "math.h"

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Geom_BSplineCurve.hxx>




namespace tigl
{

namespace
{
    // helper function to read in tixi vector
    std::vector<double> readTixiVector (TixiDocumentHandle tixiHandle, const std::string& xpath, const std::string& name, const std::string& funcname)
    {
        std::string fullpath = xpath + name;
        int count;
        if (tixiGetVectorSize(tixiHandle, fullpath.c_str(), &count) != SUCCESS) {
            throw CTiglError("Error: XML error while reading vector <" + name + "> in " + funcname, TIGL_XML_ERROR);
        }
        double *tmp;
        if (tixiGetFloatVector(tixiHandle, fullpath.c_str(), &tmp, count) != SUCCESS)  {
            throw CTiglError("Error: XML error while reading vector <" + name + "> in " + funcname, TIGL_XML_ERROR);
        }
        std::vector<double> res(tmp, tmp+count);
        return res;
    }
    // helper function to read in tixi double
    double readTixiDouble (TixiDocumentHandle tixiHandle, const std::string& xpath, const std::string& name, const std::string& funcname)
    {
        std::string fullpath = xpath + name;
        double res;
        if (tixiGetDoubleElement(tixiHandle, fullpath.c_str(), &res) != SUCCESS) {
            throw CTiglError("Error: XML error while reading double <" + name + "> in " + funcname, TIGL_XML_ERROR);
        }
        return res;
    }
} // anonymous namespace


// register profile algo at factory
PTiglWingProfileAlgo CreateProfileCST(const CCPACSWingProfile& profile, const std::string& cpacsPath)
{
    return PTiglWingProfileAlgo(new CCPACSWingProfileCST(profile, cpacsPath));
}

AUTORUN(CCPACSWingProfileCST)
{
    return CCPACSWingProfileFactory::Instance().RegisterAlgo(CCPACSWingProfileCST::CPACSID(), CreateProfileCST);
}

// Constructor
CCPACSWingProfileCST::CCPACSWingProfileCST(const CCPACSWingProfile&, const std::string& path)
{
    ProfileDataXPath=path + "/" + CPACSID();
    trailingEdge.Nullify();
}

// Destructor
CCPACSWingProfileCST::~CCPACSWingProfileCST(void)
{
}

std::string CCPACSWingProfileCST::CPACSID()
{
    return "cst2D";
}

// Cleanup routine
void CCPACSWingProfileCST::Cleanup(void)
{
    upperB.clear();
    lowerB.clear();
    trailingEdge.Nullify();
}

void CCPACSWingProfileCST::Update(void)
{
    BuildWires();
}

// Read wing profile file
void CCPACSWingProfileCST::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    upperB=readTixiVector(tixiHandle, ProfileDataXPath, "/upperB", "CCPACSWingProfileCST::ReadCPACS"); 
    lowerB=readTixiVector(tixiHandle, ProfileDataXPath, "/lowerB", "CCPACSWingProfileCST::ReadCPACS"); 
    upperN1=readTixiDouble(tixiHandle, ProfileDataXPath, "/upperN1", "CCPACSWingProfileCST::ReadCPACS"); 
    upperN2=readTixiDouble(tixiHandle, ProfileDataXPath, "/upperN2", "CCPACSWingProfileCST::ReadCPACS"); 
    lowerN1=readTixiDouble(tixiHandle, ProfileDataXPath, "/lowerN1", "CCPACSWingProfileCST::ReadCPACS"); 
    lowerN2=readTixiDouble(tixiHandle, ProfileDataXPath, "/lowerN2", "CCPACSWingProfileCST::ReadCPACS"); 
}

// Builds the wing profile wire. The returned wire is already transformed by the
// wing profile element transformation.
void CCPACSWingProfileCST::BuildWires()
{
    gp_Trsf yzSwitch;
    yzSwitch.SetMirror(gp_Ax2(gp_Pnt(0.,0.,0.), gp_Dir(0.,-1.,1.)));
    
    // Build upper wire
    CCSTCurveBuilder upperBuilder(upperN1, upperN2, upperB);
    Handle(Geom_BSplineCurve) upperCurve = upperBuilder.Curve();
    upperCurve->Transform(yzSwitch);
    upperWire = BRepBuilderAPI_MakeEdge(upperCurve);
    
    // Build lower curve, 
    std::vector<double> binv;
    for (unsigned int i = 0; i < lowerB.size(); ++i) {
        binv.push_back(-lowerB[i]);
    }
    
    CCSTCurveBuilder lowerBuilder(lowerN1, lowerN2, binv);
    Handle(Geom_BSplineCurve) lowerCurve = lowerBuilder.Curve();
    lowerCurve->Transform(yzSwitch);
    lowerCurve->Reverse();
    lowerWire = BRepBuilderAPI_MakeEdge(lowerCurve);
    
    BRepBuilderAPI_MakeWire upperLowerWireMaker(lowerWire, upperWire);
    TopoDS_Wire upperLowerWire = upperLowerWireMaker.Wire();
    
    // conatenate wire
    Handle(Geom_Curve) upperLowerCurve = CWireToCurve(upperLowerWire).curve();
    upperLowerEdge = BRepBuilderAPI_MakeEdge(upperLowerCurve);
    
    tePoint = gp_Pnt(1,0,0);
    lePoint = gp_Pnt(0,0,0);
}

// Returns sample points
std::vector<CTiglPoint*> CCPACSWingProfileCST::GetSamplePoints() const
{
    std::vector<CTiglPoint*> empty;
    return empty;
}

// get profiles CPACS XML path
const std::string & CCPACSWingProfileCST::GetProfileDataXPath() const
{
    return ProfileDataXPath;
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
