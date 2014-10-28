/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
* @brief  Implementation of CPACS wing profile handling routines.
*/

#include <iostream>
#include <sstream>
#include <vector>

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "math.h"

#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "gp_Pln.hxx"
#include "Geom2d_Line.hxx"
#include "Geom2d_TrimmedCurve.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "TopoDS.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "BRep_Tool.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "GeomAPI.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "ShapeFix_Wire.hxx"
#include "CTiglInterpolateBsplineWire.h"
#include "CTiglInterpolateLinearWire.h"
#include "ITiglWingProfileAlgo.h"
#include "CCPACSWingProfile.h"
#include "CCPACSWingProfileFactory.h"

namespace tigl 
{

// Constructor
CCPACSWingProfile::CCPACSWingProfile(const std::string& path)
    : ProfileXPath(path),
    invalidated(true)
{
    Cleanup();
}

// Destructor
CCPACSWingProfile::~CCPACSWingProfile(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSWingProfile::Cleanup(void)
{
    name = "";
    description = "";
    uid = "";

    if (profileAlgo) {
        profileAlgo->Cleanup();
    }

    Invalidate();
}

// Read wing profile file
void CCPACSWingProfile::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    Cleanup();
    std::string namePath = ProfileXPath + "/name";
    std::string describtionPath = ProfileXPath + "/description";

    try {
        // Get profiles "uid"
        char* ptrUID = NULL;
        if (tixiGetTextAttribute(tixiHandle, ProfileXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
            uid = ptrUID;
        }

        // Get subelement "name"
        char* ptrName = NULL;
        if (tixiGetTextElement(tixiHandle, namePath.c_str(), &ptrName) == SUCCESS) {
            name = ptrName;
        }

        // Get subelement "description"
        char* ptrDescription = NULL;
        if (tixiGetTextElement(tixiHandle, describtionPath.c_str(), &ptrDescription) == SUCCESS) {
            description = ptrDescription;
        }

        // create wing profile algorithm via factory
        profileAlgo=CCPACSWingProfileFactory::Instance().CreateProfileAlgo(tixiHandle, *this, ProfileXPath);
        // read in wing profile data
        profileAlgo->ReadCPACS(tixiHandle);
    }
    catch (...) {
        throw;
    }

    Update();
}

// Returns the name of the wing profile
const std::string& CCPACSWingProfile::GetName(void) const
{
    return name;
}

// Returns the describtion of the wing profile
const std::string& CCPACSWingProfile::GetDescription(void) const
{
    return description;
}

// Returns the UID of the wing profile
const std::string& CCPACSWingProfile::GetUID(void) const
{
    return uid;
}

// Invalidates internal wing profile state
void CCPACSWingProfile::Invalidate(void)
{
    invalidated = true;
}

// Update the internal state, i.g. recalculates wire and le, te points
void CCPACSWingProfile::Update(void)
{
    if (!invalidated) {
        return;
    }

    // build wires
    profileAlgo->Update();
    invalidated = false;
}
    
// Returns the wing profile upper wire
TopoDS_Edge CCPACSWingProfile::GetUpperWire()
{
    Update();
    return profileAlgo->GetUpperWire();
}
    
// Returns the wing profile lower wire
TopoDS_Edge CCPACSWingProfile::GetLowerWire()
{
    Update();
    return profileAlgo->GetLowerWire();
}
    
// Returns the wing profile trailing edge
TopoDS_Edge CCPACSWingProfile::GetTrailingEdge()
{
    Update();
    return profileAlgo->GetTrailingEdge();
}

// Returns the wing profile lower and upper wire fused
TopoDS_Wire CCPACSWingProfile::GetSplitWire()
{
    Update();
    // rebuild closed wire
    BRepBuilderAPI_MakeWire closedWireBuilder;
    closedWireBuilder.Add(profileAlgo->GetLowerWire());
    closedWireBuilder.Add(profileAlgo->GetUpperWire());
    if (!profileAlgo->GetTrailingEdge().IsNull()) {
        closedWireBuilder.Add(profileAlgo->GetTrailingEdge());
    }
    closedWireBuilder.Build();
    
    if (!closedWireBuilder.IsDone()) {
        throw CTiglError("Error creating closed wing profile");
    }
        
    return closedWireBuilder.Wire();
}

TopoDS_Wire CCPACSWingProfile::GetWire()
{
    Update();
    // rebuild closed wire
    BRepBuilderAPI_MakeWire closedWireBuilder;
    closedWireBuilder.Add(profileAlgo->GetUpperLowerWire());
    if (!profileAlgo->GetTrailingEdge().IsNull()) {
        closedWireBuilder.Add(profileAlgo->GetTrailingEdge());
    }
        
    return closedWireBuilder.Wire();
}


// Returns the leading edge point of the wing profile wire. The leading edge point
// is already transformed by the wing profile transformation.
gp_Pnt CCPACSWingProfile::GetLEPoint(void)
{
    Update();
    return profileAlgo->GetLEPoint();
}

// Returns the trailing edge point of the wing profile wire. The trailing edge point
// is already transformed by the wing profile transformation.
gp_Pnt CCPACSWingProfile::GetTEPoint(void)
{
    Update();
    return profileAlgo->GetTEPoint();
}

// Returns a point on the chord line between leading and trailing
// edge as function of parameter xsi, which ranges from 0.0 to 1.0.
// For xsi = 0.0 chord point is equal to leading edge, for xsi = 1.0
// chord point is equal to trailing edge.
gp_Pnt CCPACSWingProfile::GetChordPoint(double xsi)
{
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Error: Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingProfile::GetChordPoint", TIGL_ERROR);
    }

    Handle(Geom2d_TrimmedCurve) chordLine = GetChordLine();
    Standard_Real firstParam = chordLine->FirstParameter();
    Standard_Real lastParam  = chordLine->LastParameter();
    Standard_Real param = (lastParam - firstParam) * xsi;

    gp_Pnt2d chordPoint2d;
    chordLine->D0(param, chordPoint2d);
    return gp_Pnt(chordPoint2d.X(), 0.0, chordPoint2d.Y());
}

// Returns the chord line as a wire
TopoDS_Wire CCPACSWingProfile::GetChordLineWire() 
{
    // convert 2d chordline to 3d
    Handle(Geom2d_TrimmedCurve) chordLine = GetChordLine();
    gp_Pnt origin;
    gp_Dir yDir(0.0, 1.0, 0.0);
    gp_Pln xzPlane(origin, yDir);
    Handle(Geom_Curve) chordLine3d = GeomAPI::To3d(chordLine, xzPlane);
    TopoDS_Edge chordEdge = BRepBuilderAPI_MakeEdge(chordLine3d);
    TopoDS_Wire chordWire = BRepBuilderAPI_MakeWire(chordEdge);
    return chordWire;
}

// Returns a point on the upper wing profile as function of
// parameter xsi, which ranges from 0.0 to 1.0.
// For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
// point is equal to trailing edge.
gp_Pnt CCPACSWingProfile::GetUpperPoint(double xsi)
{
    return GetPoint(xsi, true);
}

// Returns a point on the lower wing profile as function of
// parameter xsi, which ranges from 0.0 to 1.0.
// For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
// point is equal to trailing edge.
gp_Pnt CCPACSWingProfile::GetLowerPoint(double xsi)
{
    return GetPoint(xsi, false);
}

// Returns an upper or lower point on the wing profile in
// dependence of parameter xsi, which ranges from 0.0 to 1.0.
// For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
// point is equal to trailing edge. If fromUpper is true, a point
// on the upper profile is returned, otherwise from the lower.
gp_Pnt CCPACSWingProfile::GetPoint(double xsi, bool fromUpper)
{
    Update();

    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Error: Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingProfile::GetPoint", TIGL_ERROR);
    }

    if (xsi < Precision::Confusion()) {
        return GetLEPoint();
    }
    if ((1.0 - xsi) < Precision::Confusion()) {
        return GetTEPoint();
    }

    gp_Pnt   chordPoint3d = GetChordPoint(xsi);
    gp_Pnt2d chordPoint2d(chordPoint3d.X(), chordPoint3d.Z());

    gp_Pnt le3d = GetLEPoint();
    gp_Pnt te3d = GetTEPoint();
    gp_Pnt2d le2d(le3d.X(), le3d.Z());
    gp_Pnt2d te2d(te3d.X(), te3d.Z());

    // Normal vector on chord line
    gp_Vec2d normalVec2d(-(le2d.Y() - te2d.Y()), (le2d.X() - te2d.X()));

    // Compute 2d line normal to chord line
    Handle(Geom2d_Line) line2d = new Geom2d_Line(chordPoint2d, gp_Dir2d(normalVec2d));

    // Define xz-plane for curve projection
    gp_Pln xzPlane = gce_MakePln(gp_Pnt(0.0, 0.0, 0.0), gp_Pnt(1.0, 0.0, 0.0), gp_Pnt(0.0, 0.0, 1.0));

    // Loop over all edges of the wing profile curve and try to find intersection points
    std::vector<gp_Pnt2d> ipnts2d;
    TopoDS_Edge edge;
    if (fromUpper) {
        edge = GetUpperWire();
    }
    else {
        edge = GetLowerWire();
    }

    Standard_Real firstParam;
    Standard_Real lastParam;
    // get curve and trim it - trimming is important, else it will be infinite
    Handle(Geom_Curve) curve3d = BRep_Tool::Curve(edge, firstParam, lastParam);
    curve3d = new Geom_TrimmedCurve(curve3d, firstParam, lastParam);

    // Convert 3d curve to 2d curve lying in the xz-plane
    Handle(Geom2d_Curve) curve2d = GeomAPI::To2d(curve3d, xzPlane);
    // Check if there are intersection points between line2d and curve2d
    Geom2dAPI_InterCurveCurve intersection(line2d, curve2d);
    for (int n = 1; n <= intersection.NbPoints(); n++) {
        ipnts2d.push_back(intersection.Point(n));
    }

    if (ipnts2d.size() == 1) {
        // There is only one intesection point with the wire
        gp_Pnt2d ipnt2d = ipnts2d[0];
        gp_Pnt ipnt3d(ipnt2d.X(), 0.0, ipnt2d.Y());
            return ipnt3d;
    }
    else if (ipnts2d.size() > 1) {
        // There are one or more intersection points with the wire. Find the
        // points with the minimum and maximum y-values.
        gp_Pnt2d minYPnt2d = ipnts2d[0];
        gp_Pnt2d maxYPnt2d = minYPnt2d;
        for (std::vector<gp_Pnt2d>::size_type i = 1; i < ipnts2d.size(); i++) {
            gp_Pnt2d currPnt2d = ipnts2d[i];
            if (currPnt2d.Y() < minYPnt2d.Y()) {
                minYPnt2d = currPnt2d;
            }
            if (currPnt2d.Y() > maxYPnt2d.Y()) {
                maxYPnt2d = currPnt2d;
            }
        }
        gp_Pnt maxYPnt3d(maxYPnt2d.X(), 0.0, maxYPnt2d.Y());
        gp_Pnt minYPnt3d(minYPnt2d.X(), 0.0, minYPnt2d.Y());
        if (fromUpper) {
            return maxYPnt3d;
        }
        return minYPnt3d;
    }
    throw CTiglError("Error: No intersection point found in CCPACSWingProfile::GetPoint", TIGL_NOT_FOUND);
}

// Helper function to determine the chord line between leading and trailing edge in the profile plane
Handle(Geom2d_TrimmedCurve) CCPACSWingProfile::GetChordLine()
{
    Update();
    gp_Pnt le3d = GetLEPoint();
    gp_Pnt te3d = GetTEPoint();

    gp_Pnt2d le2d(le3d.X(), le3d.Z());  // create point in profile-plane (omitting Y coordinate)
    gp_Pnt2d te2d(te3d.X(), te3d.Z());

    Handle(Geom2d_TrimmedCurve) chordLine = GCE2d_MakeSegment(le2d, te2d);
    return chordLine;
}

// get pointer to profile algorithm
PTiglWingProfileAlgo CCPACSWingProfile::GetProfileAlgo(void) const
{
    return profileAlgo;
}

} // end namespace tigl
