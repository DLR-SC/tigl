/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-12 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS wing profile as a point list.
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
#include "Bnd_Box.hxx"
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
#include "BRepBndLib.hxx"
#include "ShapeFix_Wire.hxx"
#include "CTiglInterpolateBsplineWire.h"
#include "CTiglInterpolateLinearWire.h"

#include "ITiglWingProfileAlgo.h"
#include "CCPACSWingProfilePointList.h"

namespace tigl 
{

    // Constructor
    CCPACSWingProfilePointList::CCPACSWingProfilePointList(const std::string& path)
    {
        ProfileDataXPath=path;
        profileWireAlgo = WireAlgoPointer(new CTiglInterpolateBsplineWire);
    }

    // Destructor
    CCPACSWingProfilePointList::~CCPACSWingProfilePointList(void)
    {
        delete profileWireAlgo;
    }

    // Cleanup routine
    void CCPACSWingProfilePointList::Cleanup(void)
    {
        for (CCPACSCoordinateContainer::size_type i = 0; i < coordinates.size(); i++)
        {
            delete coordinates[i];
        }
        coordinates.clear();
    }

    // Read wing profile file
    void CCPACSWingProfilePointList::ReadCPACS(TixiDocumentHandle tixiHandle)
    {
        try
        {
            /* Get point count */
            int   pointCount;
            if (tixiGetNamedChildrenCount(tixiHandle, ProfileDataXPath.c_str(), "point", &pointCount) != SUCCESS) {
                throw CTiglError("Error: tixiGetNamedChildrenCount failed in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
            }

            if (pointCount > 2)
            {
                // Loop over all points
                for (int i = 1; i <= pointCount; i++)
                {
                    CTiglPoint* point = new CTiglPoint(0.0, 0.0, 0.0);
                    coordinates.push_back(point);

                    std::ostringstream xpath;
                    xpath << ProfileDataXPath.c_str() << "/point[" << i << "]";
                    std::string x = xpath.str();
                    char * ptrPathChar = const_cast<char*>(x.c_str());

                    if (tixiGetPoint(tixiHandle, ptrPathChar, &(point->x), &(point->y), &(point->z)) != SUCCESS) {
                        throw CTiglError("Error: XML error while reading <point/> in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
                    }
                }
            }
            else // read in vector based point list
            {
                std::string xXpath = ProfileDataXPath +"/x";
                std::string yXpath = ProfileDataXPath +"/y";
                std::string zXpath = ProfileDataXPath +"/z";

                // check the number of elements in all three vectors. It has to be the same, otherwise cancel
                int countX;
                int countY;
                int countZ;
                if (tixiGetVectorSize(tixiHandle, xXpath.c_str(), &countX) != SUCCESS){
                    throw CTiglError("Error: XML error while reading point vector <x> in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetVectorSize(tixiHandle, yXpath.c_str(), &countY) != SUCCESS){
                    throw CTiglError("Error: XML error while reading point vector <y> in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetVectorSize(tixiHandle, zXpath.c_str(), &countZ) != SUCCESS){
                    throw CTiglError("Error: XML error while reading point vector <z> in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
                }

                if (countX != countY || countX != countZ || countY != countZ) {
                    throw CTiglError("Error: Vector size for profile points are not eqal in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
                }

                // read in vectors, vectors are allocated and freed by tixi
                double *xCoordinates = NULL;
                double *yCoordinates = NULL;
                double *zCoordinates = NULL;

                if (tixiGetFloatVector(tixiHandle, xXpath.c_str(), &xCoordinates, countX) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <x> in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetFloatVector(tixiHandle, yXpath.c_str(), &yCoordinates, countY) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <y> in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetFloatVector(tixiHandle, zXpath.c_str(), &zCoordinates, countZ) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <z> in CCPACSWingProfilePointList::ReadCPACS", TIGL_XML_ERROR);
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
    }

    // Builds the wing profile wire. The returned wire is already transformed by the
    // wing profile element transformation.
    void CCPACSWingProfilePointList::BuildWires()
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
        if (tempWireClosed.IsNull() == Standard_True)
            throw CTiglError("Error: TopoDS_Wire is null in CCPACSWingProfilePointList::BuildWire", TIGL_ERROR);

        //@todo: do we really want to remove all y information? this has to be a bug
        // Apply wing profile transformation to wires
        CTiglTransformation transformation;
        transformation.AddProjectionOnXZPlane();

        TopoDS_Shape tempShapeClosed   = transformation.Transform(tempWireClosed);

        // Cast shapes to wires, see OpenCascade documentation
        if (tempShapeClosed.ShapeType() != TopAbs_WIRE)
            throw CTiglError("Error: Wrong shape type in CCPACSWingProfilePointList::BuildWire", TIGL_ERROR);

        wireClosed   = TopoDS::Wire(tempShapeClosed);
        
        BuildLETEPoints();

        // Create upper and lower wires
        // Get BSpline curve
        Handle_Geom_BSplineCurve curve = BRepAdaptor_CompCurve(wireClosed).BSpline();
        // Get Leading edge parameter on curve
        double lep_par = GeomAPI_ProjectPointOnCurve(lePoint, curve).LowerDistanceParameter();
        
        // upper and lower edges
        TopoDS_Edge edge1, edge2;
        edge1 = BRepBuilderAPI_MakeEdge(curve,curve->FirstParameter(), lep_par);
        edge2 = BRepBuilderAPI_MakeEdge(curve,lep_par, curve->LastParameter());

        // Get maximal z-values of both edges via bounding box
        Bnd_Box boundingBox1;
        Bnd_Box boundingBox2;
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax1, zmax2;
        BRepBndLib::Add(edge1, boundingBox1);
        BRepBndLib::Add(edge2, boundingBox2);
        boundingBox1.Get(xmin, ymin, zmin, xmax, ymax, zmax1);
        boundingBox2.Get(xmin, ymin, zmin, xmax, ymax, zmax2);


        // Trailing edge points
        gp_Pnt te_up, te_down;
        // Find out which edge is on top and asign upper and lower edge
        TopoDS_Edge lower_edge, upper_edge;
        if(zmax2<zmax1)
        {
            //wire goes from top to bottom
            upper_edge = edge1;
            lower_edge = edge2;
            te_up = curve->StartPoint();
            te_down = curve->EndPoint();
        }
        else {
            //wire goes from bottom to top
            lower_edge = edge1;
            upper_edge = edge2;
            te_up = curve->EndPoint();
            te_down = curve->StartPoint();
        }
        // Wire builder
        BRepBuilderAPI_MakeWire upperWireBuilder, lowerWireBuilder;

        //check if we have to close upper and lower wing shells
        if(te_up.Distance(te_down) > Precision::Confusion())
        {
            lowerWireBuilder.Add(BRepBuilderAPI_MakeEdge(te_up,te_down));
        }

        upperWireBuilder.Add(upper_edge); 
        lowerWireBuilder.Add(lower_edge);
        
        upperWire = upperWireBuilder.Wire();
        lowerWire = lowerWireBuilder.Wire();
    }

    // Builds leading and trailing edge points of the wing profile wire.
    void CCPACSWingProfilePointList::BuildLETEPoints(void)
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
    std::vector<CTiglPoint*> CCPACSWingProfilePointList::GetSamplePoints() const
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

    // get profiles CPACS XML path
    const std::string & CCPACSWingProfilePointList::GetProfileDataXPath()
    {
        return ProfileDataXPath;
    }

    // get forced closed wing profile wire
    const TopoDS_Wire & CCPACSWingProfilePointList::GetWireClosed()
    {
        return wireClosed;
    }
        
    // get upper wing profile wire
    const TopoDS_Wire & CCPACSWingProfilePointList::GetUpperWire()
    {
        return upperWire;
    }
            
    // get lower wing profile wire
    const TopoDS_Wire & CCPACSWingProfilePointList::GetLowerWire()
    {
        return lowerWire;
    }

    // get leading edge point();
    const gp_Pnt & CCPACSWingProfilePointList::GetLEPoint()
    {
        return lePoint;
    }
        
    // get trailing edge point();
    const gp_Pnt & CCPACSWingProfilePointList::GetTEPoint()
    {
        return tePoint;
    }


} // end namespace tigl
