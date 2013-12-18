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

#include "CCPACSWingProfile.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "math.h"

#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "gp_Pln.hxx"
#include "Geom2d_Line.hxx"
#include "Geom2d_TrimmedCurve.hxx"
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

namespace tigl {

    // Constructor
    CCPACSWingProfile::CCPACSWingProfile(const std::string& path)
        : ProfileXPath(path),
        invalidated(true)
    {
        profileWireAlgo = WireAlgoPointer(new CTiglInterpolateBsplineWire);
        Cleanup();
    }

    // Destructor
    CCPACSWingProfile::~CCPACSWingProfile(void)
    {
        delete profileWireAlgo;
        Cleanup();
    }

    // Cleanup routine
    void CCPACSWingProfile::Cleanup(void)
    {
        name = "";
        description = "";
        uid = "";

        for (CCPACSCoordinateContainer::size_type i = 0; i < coordinates.size(); i++)
        {
            delete coordinates[i];
        }
        coordinates.clear();
        Invalidate();
    }

    // Read wing profile file
    void CCPACSWingProfile::ReadCPACS(TixiDocumentHandle tixiHandle)
    {
        Cleanup();
        std::string namePath = ProfileXPath + "/name";
        std::string describtionPath = ProfileXPath + "/description";
        std::string elementPath = ProfileXPath + "/pointList";

        try
        {
            // Get profiles "uid"
            char* ptrUID = NULL;
            if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(ProfileXPath.c_str()), "uID", &ptrUID) == SUCCESS)
                uid = ptrUID;

            // Get subelement "name"
            char* ptrName = NULL;
            if (tixiGetTextElement(tixiHandle, const_cast<char*>(namePath.c_str()), &ptrName) == SUCCESS)
                name = ptrName;

            // Get subelement "description"
            char* ptrDescription = NULL;
            if (tixiGetTextElement(tixiHandle, const_cast<char*>(describtionPath.c_str()), &ptrDescription) == SUCCESS)
                description = ptrDescription;

            /* Get point count */
            int   pointCount;
            if (tixiGetNamedChildrenCount(tixiHandle, const_cast<char*>(elementPath.c_str()), "point", &pointCount) != SUCCESS) {
                throw CTiglError("Error: tixiGetNamedChildrenCount failed in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
            }

            if (pointCount > 2)
            {
                // Loop over all points
                for (int i = 1; i <= pointCount; i++)
                {
                    CTiglPoint* point = new CTiglPoint(0.0, 0.0, 0.0);
                    coordinates.push_back(point);

                    std::ostringstream xpath;
                    xpath << elementPath.c_str() << "/point[" << i << "]";
                    std::string x = xpath.str();
                    char * ptrPathChar = const_cast<char*>(x.c_str());

                    if (tixiGetPoint(tixiHandle, ptrPathChar, &(point->x), &(point->y), &(point->z)) != SUCCESS) {
                        throw CTiglError("Error: XML error while reading <point/> in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
                    }
                }
            }
            else // read in vector based point list
            {
                // create xXpath
                std::ostringstream xpath;
                xpath << elementPath.c_str() << "/x";
                std::string xXpath = xpath.str();

                // create yXpath
                std::ostringstream ypath;
                ypath << elementPath.c_str() << "/y";
                std::string yXpath = ypath.str();

                // create zXpath
                std::ostringstream zpath;
                zpath << elementPath.c_str() << "/z";
                std::string zXpath = zpath.str();

                // check the number of elements in all three vectors. It has to be the same, otherwise cancel
                int countX;
                int countY;
                int countZ;
                if (tixiGetVectorSize(tixiHandle, const_cast<char*>(xXpath.c_str()), &countX) != SUCCESS){
                    throw CTiglError("Error: XML error while reading point vector <x> in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetVectorSize(tixiHandle, const_cast<char*>(yXpath.c_str()), &countY) != SUCCESS){
                    throw CTiglError("Error: XML error while reading point vector <y> in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetVectorSize(tixiHandle, const_cast<char*>(zXpath.c_str()), &countZ) != SUCCESS){
                    throw CTiglError("Error: XML error while reading point vector <z> in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
                }

                if (countX != countY || countX != countZ || countY != countZ) {
                    throw CTiglError("Error: Vector size for profile points are not eqal in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
                }

                // read in vectors, vectors are allocated and freed by tixi
                double *xCoordinates = NULL;
                double *yCoordinates = NULL;
                double *zCoordinates = NULL;

                if (tixiGetFloatVector(tixiHandle, const_cast<char*>(xXpath.c_str()), &xCoordinates, countX) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <x> in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetFloatVector(tixiHandle, const_cast<char*>(yXpath.c_str()), &yCoordinates, countY) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <y> in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetFloatVector(tixiHandle, const_cast<char*>(zXpath.c_str()), &zCoordinates, countZ) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <z> in CCPACSWingProfile::ReadCPACS", TIGL_XML_ERROR);
                }

                // Loop over all points in the vector
                for (int i = 0; i < countX; i++)
                {
                    CTiglPoint* point = new CTiglPoint(xCoordinates[i], yCoordinates[i], zCoordinates[i]);
                    coordinates.push_back(point);
                }
            }

        }
        catch (...)
        {
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
        if (!invalidated)
            return;

        BuildWires();
        invalidated = false;
    }

    // Returns the wing profile wire
    TopoDS_Wire CCPACSWingProfile::GetWire(bool forceClosed)
    {
        Update();
        return (forceClosed ? wireClosed : wireOriginal);
    }
    
    // Returns the wing profile upper wire
    TopoDS_Wire CCPACSWingProfile::GetUpperWire()
    {
        Update();
        return upperWire;
    }
    
    // Returns the wing profile lower wire
    TopoDS_Wire CCPACSWingProfile::GetLowerWire()
    {
        Update();
        return lowerWire;
    }
    
    // Returns the wing profile lower and upper wire fused
    TopoDS_Wire CCPACSWingProfile::GetFusedUpperLowerWire()
    {
        Update();
        // rebuild closed wire
        BRepBuilderAPI_MakeWire closedWireBuilder;
        closedWireBuilder.Add(upperWire);
        closedWireBuilder.Add(lowerWire);
        
        return closedWireBuilder.Wire();
    }


    // Returns the leading edge point of the wing profile wire. The leading edge point
    // is already transformed by the wing profile transformation.
    gp_Pnt CCPACSWingProfile::GetLEPoint(void)
    {
        Update();
        return lePoint;
    }

    // Returns the trailing edge point of the wing profile wire. The trailing edge point
    // is already transformed by the wing profile transformation.
    gp_Pnt CCPACSWingProfile::GetTEPoint(void)
    {
        Update();
        return tePoint;
    }

    // Returns a point on the chord line between leading and trailing
    // edge as function of parameter xsi, which ranges from 0.0 to 1.0.
    // For xsi = 0.0 chord point is equal to leading edge, for xsi = 1.0
    // chord point is equal to trailing edge.
    gp_Pnt CCPACSWingProfile::GetChordPoint(double xsi)
    {
        if (xsi < 0.0 || xsi > 1.0)
        {
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

        if (xsi < 0.0 || xsi > 1.0)
        {
            throw CTiglError("Error: Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingProfile::GetPoint", TIGL_ERROR);
        }

        if (xsi < Precision::Confusion())
            return GetLEPoint();
        if ((1.0 - xsi) < Precision::Confusion())
            return GetTEPoint();

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
        BRepTools_WireExplorer wireExplorer;
        for (wireExplorer.Init(GetWire(true)); wireExplorer.More(); wireExplorer.Next())
        {
            const TopoDS_Edge& edge = wireExplorer.Current();
            Standard_Real firstParam;
            Standard_Real lastParam;
            Handle(Geom_Curve) curve3d = BRep_Tool::Curve(edge, firstParam, lastParam);
            // Convert 3d curve to 2d curve lying in the xz-plane
            Handle(Geom2d_Curve) curve2d = GeomAPI::To2d(curve3d, xzPlane);
            // Check if there are intersection points between line2d and curve2d
            Geom2dAPI_InterCurveCurve intersection(line2d, curve2d);
            for (int n = 1; n <= intersection.NbPoints(); n++)
            {
                ipnts2d.push_back(intersection.Point(n));
            }
        }
        if (ipnts2d.size() == 1)
        {
            // There is only one intesection point with the wire
            gp_Pnt2d ipnt2d = ipnts2d[0];
            gp_Pnt ipnt3d(ipnt2d.X(), 0.0, ipnt2d.Y());
            if (fabs(ipnt2d.Y() - chordPoint2d.Y()) < Precision::Confusion())
                return ipnt3d;
            if ((fromUpper && ipnt2d.Y() > chordPoint2d.Y()) || (!fromUpper && ipnt2d.Y() < chordPoint2d.Y()))
            {
                return ipnt3d;
            }
        }
        else if (ipnts2d.size() > 1)
        {
            // There are one or more intersection points with the wire. Find the
            // points with the minimum and maximum y-values.
            gp_Pnt2d minYPnt2d = ipnts2d[0];
            gp_Pnt2d maxYPnt2d = minYPnt2d;
            for (std::vector<gp_Pnt2d>::size_type i = 1; i < ipnts2d.size(); i++)
            {
                gp_Pnt2d currPnt2d = ipnts2d[i];
                if (currPnt2d.Y() < minYPnt2d.Y())
                    minYPnt2d = currPnt2d;
                if (currPnt2d.Y() > maxYPnt2d.Y())
                    maxYPnt2d = currPnt2d;
            }
            gp_Pnt maxYPnt3d(maxYPnt2d.X(), 0.0, maxYPnt2d.Y());
            gp_Pnt minYPnt3d(minYPnt2d.X(), 0.0, minYPnt2d.Y());
            if (fromUpper)
            {
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

    // Builds the wing profile wire. The returned wire is already transformed by the
    // wing profile element transformation.
    void CCPACSWingProfile::BuildWires(void)
    {
        ITiglWireAlgorithm::CPointContainer points;
        for (CCPACSCoordinateContainer::size_type i = 0; i < coordinates.size(); i++)
            points.push_back(coordinates[i]->Get_gp_Pnt());

        // Build wires from wing profile points.
        const ITiglWireAlgorithm& wireBuilder = *profileWireAlgo;

        // CCPACSWingSegment::makeSurfaces cannot handle currently 
        // wire with multiple edges. Thus we get problems if we have
        // a linear interpolated wire consting of many edges.
        if(dynamic_cast<const CTiglInterpolateLinearWire*>(&wireBuilder)){
            LOG(ERROR) << "Linear Wing Profiles are currently not supported" << endl;
            throw CTiglError("Linear Wing Profiles are currently not supported",TIGL_ERROR);
        }

        TopoDS_Wire tempWireClosed   = wireBuilder.BuildWire(points, true);
        TopoDS_Wire tempWireOriginal = wireBuilder.BuildWire(points, false);
        if (tempWireClosed.IsNull() == Standard_True || tempWireOriginal.IsNull() == Standard_True)
            throw CTiglError("Error: TopoDS_Wire is null in CCPACSWingProfile::BuildWire", TIGL_ERROR);

        //@todo: do we really want to remove all y information? this has to be a bug
        // Apply wing profile transformation to wires
        CTiglTransformation transformation;
        transformation.AddProjectionOnXZPlane();

        TopoDS_Shape tempShapeClosed   = transformation.Transform(tempWireClosed);
        TopoDS_Shape tempShapeOriginal = transformation.Transform(tempWireOriginal);

        // Cast shapes to wires, see OpenCascade documentation
        if (tempShapeClosed.ShapeType() != TopAbs_WIRE || tempShapeOriginal.ShapeType() != TopAbs_WIRE)
            throw CTiglError("Error: Wrong shape type in CCPACSWingProfile::BuildWire", TIGL_ERROR);

        wireClosed   = TopoDS::Wire(tempShapeClosed);
        wireOriginal = TopoDS::Wire(tempShapeOriginal);
        
        BuildLETEPoints();
        invalidated = false;
        
        // now create upper and lower wires
        gp_Pnt upperPoint = GetUpperPoint(0.5);
        
        Handle_Geom_BSplineCurve curve = BRepAdaptor_CompCurve(wireOriginal).BSpline();
        double lep_par = GeomAPI_ProjectPointOnCurve(lePoint, curve).LowerDistanceParameter();
        double upperPnt_par  = GeomAPI_ProjectPointOnCurve(upperPoint, curve).LowerDistanceParameter();
        
        TopoDS_Edge upper_edge, lower_edge;
        gp_Pnt te_up, te_down;
        if(upperPnt_par < lep_par){
            //wire goes from top to bottom
            upper_edge = BRepBuilderAPI_MakeEdge(curve,curve->FirstParameter(), lep_par);
            lower_edge = BRepBuilderAPI_MakeEdge(curve,lep_par, curve->LastParameter());
            te_up = curve->StartPoint();
            te_down = curve->EndPoint();
        }
        else {
            //wire goes from bottom to top
            lower_edge = BRepBuilderAPI_MakeEdge(curve,curve->FirstParameter(), lep_par);
            upper_edge = BRepBuilderAPI_MakeEdge(curve,lep_par, curve->LastParameter());
            te_up = curve->EndPoint();
            te_down = curve->StartPoint();
        }

        BRepBuilderAPI_MakeWire upperWireBuilder, lowerWireBuilder;
        //check if we have to close upper and lower wing shells
        if(te_up.Distance(te_down) > Precision::Confusion()){
            lowerWireBuilder.Add(BRepBuilderAPI_MakeEdge(te_up,te_down));
        }

        upperWireBuilder.Add(upper_edge); 
        lowerWireBuilder.Add(lower_edge);
        
        upperWire = upperWireBuilder.Wire();
        lowerWire = lowerWireBuilder.Wire();
    }

    // Builds leading and trailing edge points of the wing profile wire.
    void CCPACSWingProfile::BuildLETEPoints(void)
    {
        // compute TE point
        gp_Pnt firstPnt = coordinates[0]->Get_gp_Pnt();
        gp_Pnt lastPnt  = coordinates[coordinates.size() - 1]->Get_gp_Pnt();
        if( fabs(firstPnt.X() - lastPnt.X()) < Precision::Confusion()){
            double x = (firstPnt.X() + lastPnt.X())/2.;
            double y = (firstPnt.Y() + lastPnt.Y())/2.;
            double z = (firstPnt.Z() + lastPnt.Z())/2.;
            tePoint = gp_Pnt(x,y,z);
        }
        else if(firstPnt.X() > lastPnt.X()) {
            tePoint = firstPnt;
        }
        else {
            tePoint = lastPnt;
        }

        // find the point with the max dist to TE point
        lePoint = tePoint;
        CCPACSCoordinateContainer::iterator pit = coordinates.begin();
        for(; pit != coordinates.end(); ++pit) {
            gp_Pnt point = (*pit)->Get_gp_Pnt();
            if(tePoint.Distance(point) > tePoint.Distance(lePoint)) {
                lePoint = point;
            }
        }
        // project into x-z plane
        lePoint.SetY(0.);
        tePoint.SetY(0.);
    }

    // Returns the profile points as read from TIXI.
    std::vector<CTiglPoint*> CCPACSWingProfile::GetCoordinateContainer()
    {
        std::vector<CTiglPoint*> newPointVector;
        for (CCPACSCoordinateContainer::size_type i = 0; i < coordinates.size(); i++)
        {
            gp_Pnt pnt = coordinates[i]->Get_gp_Pnt();
            //@TODO: i dont't see why we should transform here
            //pnt = TransformPoint(pnt);
            newPointVector.push_back(new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z()));
        }
        return newPointVector;
    }



} // end namespace tigl
