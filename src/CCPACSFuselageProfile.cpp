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
* @brief  Implementation of CPACS fuselage profile handling routines.
*/

#include "CCPACSFuselageProfile.h"
#include "CTiglError.h"
#include "TopoDS.hxx"
#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "Geom2d_TrimmedCurve.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "Geom2d_Line.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "BRep_Tool.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI.hxx"
#include "gp_Pln.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "CTiglInterpolateBsplineWire.h"

#include "math.h"
#include <iostream>
#include <sstream>


#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace tigl {

    // Constructor
    CCPACSFuselageProfile::CCPACSFuselageProfile(const std::string& path)
        : ProfileXPath(path),
        invalidated(true)
    {
        profileWireAlgo = WireAlgoPointer(new CTiglInterpolateBsplineWire);
        Cleanup();
    }

    // Destructor
    CCPACSFuselageProfile::~CCPACSFuselageProfile(void)
    {
        delete profileWireAlgo;
        Cleanup();
    }

    // Cleanup routine
    void CCPACSFuselageProfile::Cleanup(void)
    {
        name       = "";
        uid        = "";
        description= "";
        wireLength = 0.0;

        for (CCPACSCoordinateContainer::size_type i = 0; i < coordinates.size(); i++)
        {
            delete coordinates[i];
        }
        coordinates.clear();

        Invalidate();
    }

    // Read fuselage profile file
    void CCPACSFuselageProfile::ReadCPACS(TixiDocumentHandle tixiHandle)
    {
        Cleanup();
        std::string namePath        = ProfileXPath + "/name";
        std::string describtionPath = ProfileXPath + "/description";
        std::string elementPath     = ProfileXPath + "/pointList";

        try
        {
            // Get subelement "name"
            char* ptrName = NULL;
            if (tixiGetTextElement(tixiHandle, const_cast<char*>(namePath.c_str()), &ptrName) == SUCCESS)
                name = ptrName;

            // Get profiles "uid"
            char* ptrUID = NULL;
            if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(ProfileXPath.c_str()), "uID", &ptrUID) == SUCCESS)
                uid = ptrUID;

            // Get subelement "description"
            char* ptrDescription = NULL;
            if (tixiGetTextElement(tixiHandle, const_cast<char*>(describtionPath.c_str()), &ptrDescription) == SUCCESS)
                description = ptrDescription;

            /* Get point count */
            int   pointCount;
            if (tixiGetNamedChildrenCount(tixiHandle, const_cast<char*>(elementPath.c_str()), "point", &pointCount) != SUCCESS) {
                throw CTiglError("Error: tixiGetNamedChildrenCount failed in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
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
                    char *ptrPathChar = const_cast<char*>(x.c_str());

                    if (tixiGetPoint(tixiHandle, ptrPathChar, &(point->x), &(point->y), &(point->z)) != SUCCESS) {
                        throw CTiglError("Error: XML error while reading <point/> in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
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
                    throw CTiglError("Error: XML error while reading point vector <x> in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetVectorSize(tixiHandle, const_cast<char*>(yXpath.c_str()), &countY) != SUCCESS){
                    throw CTiglError("Error: XML error while reading point vector <y> in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetVectorSize(tixiHandle, const_cast<char*>(zXpath.c_str()), &countZ) != SUCCESS){
                    throw CTiglError("Error: XML error while reading point vector <z> in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
                }

                if (countX != countY || countX != countZ || countY != countZ) {
                    throw CTiglError("Error: Vector size for profile points are not eqal in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
                }

                // read in vectors, vectors are allocated and freed by tixi
                double *xCoordinates = NULL;
                double *yCoordinates = NULL;
                double *zCoordinates = NULL;

                if (tixiGetFloatVector(tixiHandle, const_cast<char*>(xXpath.c_str()), &xCoordinates, countX) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <x> in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetFloatVector(tixiHandle, const_cast<char*>(yXpath.c_str()), &yCoordinates, countY) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <y> in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
                }
                if (tixiGetFloatVector(tixiHandle, const_cast<char*>(zXpath.c_str()), &zCoordinates, countZ) != SUCCESS) {
                    throw CTiglError("Error: XML error while reading point vector <z> in CCPACSFuselageProfile::ReadCPACS", TIGL_XML_ERROR);
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
            if (tixiHandle != -1)
                tixiCloseDocument(tixiHandle); /* remove me`! */
            throw;
        }

        Update();
    }

    // Returns the filename of the fuselage profile file
    std::string CCPACSFuselageProfile::GetFileName(void) const
    {
        return ProfileXPath;
    }

    // Returns the name of the fuselage profile
    std::string CCPACSFuselageProfile::GetName(void) const
    {
        return name;
    }

    // Returns the UID of the fuselage profile
    std::string CCPACSFuselageProfile::GetUID(void) const
    {
        return uid;
    }

    // Invalidates internal fuselage profile state
    void CCPACSFuselageProfile::Invalidate(void)
    {
        invalidated = true;
    }

    // Update the internal state, i.g. recalculates wire
    void CCPACSFuselageProfile::Update(void)
    {
        if (!invalidated)
            return;

        BuildWires();
        ComputeWireLength();
        invalidated = false;
    }

    // Returns the fuselage profile wire
    TopoDS_Wire CCPACSFuselageProfile::GetWire(bool forceClosed)
    {
        Update();
        return (forceClosed ? wireClosed : wireOriginal);
    }

    // check if the distance between two points are below a fixed value, so that
    // these point could be imaged as "equal".
    bool CCPACSFuselageProfile::checkSamePoints(gp_Pnt pointA, gp_Pnt pointB)
    {
        Standard_Real distance;
        distance = pointA.Distance(pointB);
        if(distance < 0.01) {
            return true;
        } else {
            return false;
        }
    }


    // Builds the fuselage profile wire. The returned wire is already transformed by the
    // fuselage profile element transformation.
    void CCPACSFuselageProfile::BuildWires(void)
    {
        ITiglWireAlgorithm::CPointContainer points;

        if(coordinates.size() < 2)
            throw CTiglError("Error: Number of points is less than 2 in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);

        points.push_back(coordinates[0]->Get_gp_Pnt());
        for (CCPACSCoordinateContainer::size_type i = 1; i < coordinates.size() -1 ; i++) 
        {
            gp_Pnt p1 = coordinates[i-1]->Get_gp_Pnt();
            gp_Pnt p2 = coordinates[i]->Get_gp_Pnt();

            // only take points that are not "the same" 
            if( !checkSamePoints(p1, p2) ) {
                points.push_back(coordinates[i]->Get_gp_Pnt());
            }
        }

        // we always want to include the endpoint, if it's the same as the startpoint
        // we use the startpoint to enforce closing of the spline
        gp_Pnt pStart =  coordinates[0]->Get_gp_Pnt();
        gp_Pnt pEnd   =  coordinates[coordinates.size()-1]->Get_gp_Pnt();
        if(checkSamePoints(pStart,pEnd))
            points.push_back(pStart);
        else
            points.push_back(pEnd);

        // Build wire from fuselage profile points
        const ITiglWireAlgorithm& wireBuilder = *profileWireAlgo;
        const CTiglInterpolateBsplineWire * pSplineBuilder = dynamic_cast<const CTiglInterpolateBsplineWire*>(&wireBuilder);
        if(pSplineBuilder){
            const_cast<CTiglInterpolateBsplineWire*>(pSplineBuilder)->setEndpointContinuity(C1);
        }

        TopoDS_Wire tempWireClosed   = wireBuilder.BuildWire(points, true);
        TopoDS_Wire tempWireOriginal = wireBuilder.BuildWire(points, false);
        if(pSplineBuilder){
            const_cast<CTiglInterpolateBsplineWire*>(pSplineBuilder)->setEndpointContinuity(C0);
        }

        if (tempWireClosed.IsNull() == Standard_True || tempWireOriginal.IsNull() == Standard_True)
            throw CTiglError("Error: TopoDS_Wire is null in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);

        // Apply fuselage profile transformation to wire
        CTiglTransformation transformation;
        TopoDS_Shape tempShapeClosed   = transformation.Transform(tempWireClosed);
        TopoDS_Shape tempShapeOriginal = transformation.Transform(tempWireOriginal);

        // Cast shapes to wires, see OpenCascade documentation
        if (tempShapeClosed.ShapeType() != TopAbs_WIRE || tempShapeOriginal.ShapeType() != TopAbs_WIRE)
            throw CTiglError("Error: Wrong shape type in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);

        wireClosed   = TopoDS::Wire(tempShapeClosed);
        wireOriginal = TopoDS::Wire(tempShapeOriginal);
    }

    // Transforms a point by the fuselage profile transformation
    gp_Pnt CCPACSFuselageProfile::TransformPoint(const gp_Pnt& aPoint) const
    {
        CTiglTransformation transformation;
        return transformation.Transform(aPoint);
    }

    // Computes the length of the fuselage profile wire
    void CCPACSFuselageProfile::ComputeWireLength(void)
    {
        wireLength = 0.0;

        BRepTools_WireExplorer wireExplorer;
        for (wireExplorer.Init(wireOriginal); wireExplorer.More(); wireExplorer.Next())
        {
            Standard_Real firstParam;
            Standard_Real lastParam;
            TopoDS_Edge edge = wireExplorer.Current();
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);
            GeomAdaptor_Curve adaptorCurve(curve);
            wireLength += GCPnts_AbscissaPoint::Length(adaptorCurve);
        }
    }

    // Get length of fuselage profile wire
    double CCPACSFuselageProfile::GetWireLength(void)
    {
        Update();
        return wireLength;
    }

    // Gets a point on the fuselage profile wire in dependence of a parameter zeta with
    // 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the wire start point,
    // for zeta = 1.0 the last wire point.
    gp_Pnt CCPACSFuselageProfile::GetPoint(double zeta)
    {
        Update();

        if (zeta < 0.0 || zeta > 1.0)
        {
            throw CTiglError("Error: Parameter zeta not in the range 0.0 <= zeta <= 1.0 in CCPACSFuselageProfile::GetPoint", TIGL_ERROR);
        }

        double length = wireLength * zeta;

        // Get the first edge of the wire
        BRepTools_WireExplorer wireExplorer(wireOriginal);
        if (!wireExplorer.More())
        {
            throw CTiglError("Error: Not enough edges found in CCPACSFuselageProfile::GetPoint", TIGL_ERROR);
        }
        Standard_Real firstParam;
        Standard_Real lastParam;
        TopoDS_Edge edge = wireExplorer.Current();
        wireExplorer.Next();
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);

        // Length of current edge
        GeomAdaptor_Curve adaptorCurve;
        adaptorCurve.Load(curve);
        double currLength = GCPnts_AbscissaPoint::Length(adaptorCurve);

        // Length of complete wire up to now
        double sumLength = currLength;

        while (length > sumLength)
        {
            if (!wireExplorer.More())
                break;
            edge = wireExplorer.Current();
            wireExplorer.Next();

            curve = BRep_Tool::Curve(edge, firstParam, lastParam);
            adaptorCurve.Load(curve);

            // Length of current edge
            currLength = GCPnts_AbscissaPoint::Length(adaptorCurve);

            // Length of complete wire up to now
            sumLength += currLength;
        }

        // Distance of searched point from end point of current edge
        double currEndDelta = max((sumLength - length), 0.0);

        // Distance of searched point from start point of current edge
        double currDist = max((currLength - currEndDelta), 0.0);

        GCPnts_AbscissaPoint abscissaPoint(adaptorCurve, currDist, adaptorCurve.FirstParameter());
        gp_Pnt point = adaptorCurve.Value(abscissaPoint.Parameter());

        return point;
    }


    // Returns the profile points as read from TIXI.
    std::vector<CTiglPoint*> CCPACSFuselageProfile::GetCoordinateContainer()
    {
        std::vector<CTiglPoint*> newPointVector;
        for (CCPACSCoordinateContainer::size_type i = 0; i < coordinates.size(); i++)
        {
            gp_Pnt pnt = coordinates[i]->Get_gp_Pnt();
            pnt = TransformPoint(pnt);
            newPointVector.push_back(new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z()));
        }
        return newPointVector;
    }

} // end namespace tigl
