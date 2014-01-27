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
* @brief  Implementation of CPACS fuselage segment handling routines.
*/
#include <iostream>

#include "CTiglLogging.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageProfile.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Shell.hxx"
#include "TopoDS.hxx"
#include "BRep_Tool.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "GC_MakeSegment.hxx"
#include "gp_Lin.hxx"
#include "GeomAPI_IntCS.hxx"
#include "Geom_Surface.hxx"
#include "gce_MakeLin.hxx"
#include "Geom_Line.hxx"
#include "IntCurvesFace_Intersector.hxx"
#include "Precision.hxx"
#include "TopLoc_Location.hxx"
#include "Poly_Triangulation.hxx"
#include "BRep_Builder.hxx"
#include "Poly_Array1OfTriangle.hxx"
#include "gp_Trsf.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepMesh.hxx"
#include "BRepTools.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "BRepAlgoAPI_Section.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "BRepBuilderAPI_Transform.hxx"
#include "ShapeAnalysis_Surface.hxx"
#include "BRepLib_FindSurface.hxx"

#ifdef TIGL_USE_XCAF
#include "XCAFDoc_ShapeTool.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "TDataStd_Name.hxx"
#include "TDataXtd_Shape.hxx"
#endif


#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define _USE_MATH_DEFINES
#include <math.h>

namespace tigl {

    
    // Constructor
    CCPACSFuselageSegment::CCPACSFuselageSegment(CCPACSFuselage* aFuselage, int aSegmentIndex)
        : CTiglAbstractSegment(aSegmentIndex)
        , startConnection(this)
        , endConnection(this)
        , fuselage(aFuselage)
    {
        Cleanup();
    }

    // Destructor
    CCPACSFuselageSegment::~CCPACSFuselageSegment(void)
    {
        Cleanup();
    }

    // Cleanup routine
    void CCPACSFuselageSegment::Cleanup(void)
    {
        name = "";
        myVolume      = 0.;
        mySurfaceArea = 0.;
        myWireLength  = 0.;
        continuity    = C2;
        CTiglAbstractSegment::Cleanup();
    }

    // Update internal segment data
    void CCPACSFuselageSegment::Update(void)
    {
        Invalidate();
    }

    // Read CPACS segment elements
    void CCPACSFuselageSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
    {
        Cleanup();

        char*       elementPath;
        std::string tempString;

        // Get subelement "name"
        char* ptrName = NULL;
        tempString    = segmentXPath + "/name";
        elementPath   = const_cast<char*>(tempString.c_str());
        if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS)
            name          = ptrName;

        // Get attribute "uid"
        char* ptrUID = NULL;
        tempString   = "uID";
        if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(segmentXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS)
            SetUID(ptrUID);

        // Start connection
        tempString = segmentXPath + "/fromElementUID";
        startConnection.ReadCPACS(tixiHandle, tempString);

        // End connection
        tempString = segmentXPath + "/toElementUID";
        endConnection.ReadCPACS(tixiHandle, tempString);

        // Continuity
        tempString = segmentXPath + "/continuity";
        elementPath   = const_cast<char*>(tempString.c_str());
        char* ptrCont = NULL;
        if (tixiGetTextElement(tixiHandle, elementPath, &ptrCont) == SUCCESS)
        {
            if (strcmp(ptrCont, "C0") == 0) {
                continuity = C0;
            }
            else if (strcmp(ptrCont, "C1") == 0) {
                std::cout << "C1" << std::endl;
                continuity = C1;
            }
            else if (strcmp(ptrCont, "C2") == 0) {
                std::cout << "C2" << std::endl;
                continuity = C2;
            }
            else {
                LOG(ERROR) << "Invalid continuity specifier " << ptrCont << " for UID " << GetUID();
                continuity = C2;
            }
        }
        else
        {
            continuity = C2;
        }

        Update();
    }

    // Returns the fuselage this segment belongs to
    CCPACSFuselage& CCPACSFuselageSegment::GetFuselage(void) const
    {
        return *fuselage;
    }

    // Returns the segment index of this segment
    int CCPACSFuselageSegment::GetSegmentIndex(void) const
    {
        return mySegmentIndex;
    }

    // Builds the loft between the two segment sections
    TopoDS_Shape CCPACSFuselageSegment::BuildLoft(void)
    {
        CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
        CCPACSFuselageProfile& endProfile   = endConnection.GetProfile();

        TopoDS_Wire startWire = startProfile.GetWire(true);
        TopoDS_Wire endWire   = endProfile.GetWire(true);

        // Do section element transformations
        TopoDS_Shape startShape = startConnection.GetSectionElementTransformation().Transform(startWire);
        TopoDS_Shape endShape   = endConnection.GetSectionElementTransformation().Transform(endWire);

        // Do section transformations
        startShape = startConnection.GetSectionTransformation().Transform(startShape);
        endShape   = endConnection.GetSectionTransformation().Transform(endShape);

        // Do positioning transformations (positioning of sections)
        startShape = startConnection.GetPositioningTransformation().Transform(startShape);
        endShape   = endConnection.GetPositioningTransformation().Transform(endShape);

        // Cast shapes to wires, see OpenCascade documentation
        if (startShape.ShapeType() != TopAbs_WIRE || endShape.ShapeType() != TopAbs_WIRE) {
            throw CTiglError("Error: Wrong shape type in CCPACSFuselageSegment::BuildLoft", TIGL_ERROR);
        }
        startWire = TopoDS::Wire(startShape);
        endWire   = TopoDS::Wire(endShape);

        // Build loft
        //BRepOffsetAPI_ThruSections generator(Standard_False, Standard_False, Precision::Confusion());
        BRepOffsetAPI_ThruSections generator(Standard_True, Standard_False, Precision::Confusion());
        generator.AddWire(startWire);
        generator.AddWire(endWire);
        generator.CheckCompatibility(Standard_False);
        generator.Build();
        TopoDS_Shape loft = generator.Shape();

        // Calculate volume
        GProp_GProps System;
        BRepGProp::VolumeProperties(loft, System);
        myVolume = System.Mass();

        // Calculate surface area
        GProp_GProps AreaSystem;
        BRepGProp::SurfaceProperties(loft, AreaSystem);
        mySurfaceArea = AreaSystem.Mass();
        
        return loft;
    }


    // Returns the start section UID of this segment
    const std::string& CCPACSFuselageSegment::GetStartSectionUID(void)
    {
        return startConnection.GetSectionUID();
    }

    // Returns the end section UID of this segment
    const std::string& CCPACSFuselageSegment::GetEndSectionUID(void)
    {
        return endConnection.GetSectionUID();
    }

    // Returns the start section index of this segment
    int CCPACSFuselageSegment::GetStartSectionIndex(void)
    {
        return startConnection.GetSectionIndex();
    }

    // Returns the end section index of this segment
    int CCPACSFuselageSegment::GetEndSectionIndex(void)
    {
        return endConnection.GetSectionIndex();
    }

    // Returns the start section element UID of this segment
    const std::string& CCPACSFuselageSegment::GetStartSectionElementUID(void)
    {
        return startConnection.GetSectionElementUID();
    }

    // Returns the end section element UID of this segment
    const std::string& CCPACSFuselageSegment::GetEndSectionElementUID(void)
    {
        return endConnection.GetSectionElementUID();
    }

    // Returns the start section element index of this segment
    int CCPACSFuselageSegment::GetStartSectionElementIndex(void)
    {
        return startConnection.GetSectionElementIndex();
    }

    // Returns the end section element index of this segment
    int CCPACSFuselageSegment::GetEndSectionElementIndex(void)
    {
        return endConnection.GetSectionElementIndex();
    }

    // Returns the start section element index of this segment
    CCPACSFuselageConnection& CCPACSFuselageSegment::GetStartConnection(void)
    {
        return( startConnection );
    }

    // Returns the end section element index of this segment
    CCPACSFuselageConnection& CCPACSFuselageSegment::GetEndConnection(void)
    {
        return( endConnection );
    }

    // Returns the volume of this segment
    double CCPACSFuselageSegment::GetVolume(void)
    {
        return( myVolume );
    }

    // Returns the surface area of this segment
    double CCPACSFuselageSegment::GetSurfaceArea(void)
    {
        return( mySurfaceArea );
    }


    // Gets the count of segments connected to the start section of this segment
    int CCPACSFuselageSegment::GetStartConnectedSegmentCount(void)
    {
        int count = 0;
        for (int i = 1; i <= GetFuselage().GetSegmentCount(); i++)
        {
            CCPACSFuselageSegment& nextSegment = (CCPACSFuselageSegment &) GetFuselage().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetStartSectionUID() == GetStartSectionUID() ||
                nextSegment.GetEndSectionUID() == GetStartSectionUID())
            {
                count++;
            }
        }
        return count;
    }

    // Gets the count of segments connected to the end section of this segment
    int CCPACSFuselageSegment::GetEndConnectedSegmentCount(void)
    {
        int count = 0;
        for (int i = 1; i <= GetFuselage().GetSegmentCount(); i++)
        {
            CCPACSFuselageSegment& nextSegment = (CCPACSFuselageSegment &) GetFuselage().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetStartSectionUID() == GetEndSectionUID() ||
                nextSegment.GetEndSectionUID() == GetEndSectionUID())
            {
                count++;
            }
        }
        return count;
    }

    // Gets the index (number) of the n-th segment connected to the start section
    // of this segment. n starts at 1.
    int CCPACSFuselageSegment::GetStartConnectedSegmentIndex(int n)
    {
        if (n < 1 || n > GetStartConnectedSegmentCount())
        {
            throw CTiglError("Error: Invalid value for parameter n in CCPACSFuselageSegment::GetStartConnectedSegmentIndex", TIGL_INDEX_ERROR);
        }

        for (int i = 1, count = 0; i <= GetFuselage().GetSegmentCount(); i++)
        {
            CCPACSFuselageSegment& nextSegment = (CCPACSFuselageSegment &) GetFuselage().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetStartSectionUID() == GetStartSectionUID() ||
                nextSegment.GetEndSectionUID() == GetStartSectionUID())
            {
                if (++count == n) return nextSegment.GetSegmentIndex();
            }
        }

        throw CTiglError("Error: No connected segment found in CCPACSFuselageSegment::GetStartConnectedSegmentIndex", TIGL_NOT_FOUND);
    }

    // Gets the index (number) of the n-th segment connected to the end section
    // of this segment. n starts at 1.
    int CCPACSFuselageSegment::GetEndConnectedSegmentIndex(int n)
    {
        if (n < 1 || n > GetEndConnectedSegmentCount())
        {
            throw CTiglError("Error: Invalid value for parameter n in CCPACSFuselageSegment::GetEndConnectedSegmentIndex", TIGL_INDEX_ERROR);
        }

        for (int i = 1, count = 0; i <= GetFuselage().GetSegmentCount(); i++)
        {
            CCPACSFuselageSegment& nextSegment = (CCPACSFuselageSegment &) GetFuselage().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetStartSectionUID() == GetEndSectionUID() ||
                nextSegment.GetEndSectionUID() == GetEndSectionUID())
            {
                if (++count == n) return nextSegment.GetSegmentIndex();
            }
        }

        throw CTiglError("Error: No connected segment found in CCPACSFuselageSegment::GetEndConnectedSegmentIndex", TIGL_NOT_FOUND);
    }

    // Gets a point on the fuselage segment in dependence of parameters eta and zeta with
    // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
    // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
    // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
    gp_Pnt CCPACSFuselageSegment::GetPoint(double eta, double zeta)
    {
        if (eta < 0.0 || eta > 1.0)
        {
            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSFuselageSegment::GetPoint", TIGL_ERROR);
        }

        CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
        CCPACSFuselageProfile& endProfile   = endConnection.GetProfile();

        gp_Pnt startProfilePoint = startProfile.GetPoint(zeta);
        gp_Pnt endProfilePoint   = endProfile.GetPoint(zeta);

        // Do section element transformation on points
        startProfilePoint = startConnection.GetSectionElementTransformation().Transform(startProfilePoint);
        endProfilePoint   = endConnection.GetSectionElementTransformation().Transform(endProfilePoint);

        // Do section transformations
        startProfilePoint = startConnection.GetSectionTransformation().Transform(startProfilePoint);
        endProfilePoint   = endConnection.GetSectionTransformation().Transform(endProfilePoint);

        // Do positioning transformations
        startProfilePoint = startConnection.GetPositioningTransformation().Transform(startProfilePoint);
        endProfilePoint   = endConnection.GetPositioningTransformation().Transform(endProfilePoint);

        // Get point on fuselage segment in dependence of eta by linear interpolation
        Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(startProfilePoint, endProfilePoint);
        Standard_Real firstParam = profileLine->FirstParameter();
        Standard_Real lastParam  = profileLine->LastParameter();
        Standard_Real param = (lastParam - firstParam) * eta;
        gp_Pnt profilePoint;
        profileLine->D0(param, profilePoint);

        return profilePoint;
    }



    // Returns the start profile points as read from TIXI. The points are already transformed.
    std::vector<CTiglPoint*> CCPACSFuselageSegment::GetRawStartProfilePoints()
    {
        CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
        std::vector<CTiglPoint*> points = startProfile.GetCoordinateContainer();
        std::vector<CTiglPoint*> pointsTransformed;
        for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < points.size(); i++) {
            gp_Pnt pnt = points[i]->Get_gp_Pnt();

            pnt = startConnection.GetSectionElementTransformation().Transform(pnt);
            pnt = startConnection.GetSectionTransformation().Transform(pnt);
            pnt = startConnection.GetPositioningTransformation().Transform(pnt);

            CTiglPoint *tiglPoint = new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z());
            pointsTransformed.push_back(tiglPoint);
        }
        return pointsTransformed;
    }


    // Returns the outer profile points as read from TIXI. The points are already transformed.
    std::vector<CTiglPoint*> CCPACSFuselageSegment::GetRawEndProfilePoints()
    {
        CCPACSFuselageProfile& endProfile = endConnection.GetProfile();
        std::vector<CTiglPoint*> points = endProfile.GetCoordinateContainer();
        std::vector<CTiglPoint*> pointsTransformed;
        for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < points.size(); i++) {
            gp_Pnt pnt = points[i]->Get_gp_Pnt();

            pnt = endConnection.GetSectionElementTransformation().Transform(pnt);
            pnt = endConnection.GetSectionTransformation().Transform(pnt);
            pnt = endConnection.GetPositioningTransformation().Transform(pnt);

            CTiglPoint *tiglPoint = new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z());
            pointsTransformed.push_back(tiglPoint);
        }
        return pointsTransformed;
     }

    gp_Pnt CCPACSFuselageSegment::GetPointOnYPlane(double eta, double ypos, int pointIndex)
    {
        TopoDS_Shape intersectionWire = getWireOnLoft(eta);

        //build cutting plane for intersection
        gp_Pnt p1(-1.0e7, ypos, -1.0e7);
        gp_Pnt p2( 1.0e7, ypos, -1.0e7);
        gp_Pnt p3( 1.0e7, ypos,  1.0e7);
        gp_Pnt p4(-1.0e7, ypos,  1.0e7);

        Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
        Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
        Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
        Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

        TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
        TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
        TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
        TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

        TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
        TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

        // intersection-points
        BRepExtrema_DistShapeShape distSS;
        distSS.LoadS1(intersectionWire);
        distSS.LoadS2(shaft_face);
        distSS.Perform();

        return distSS.PointOnShape1(pointIndex);
    }


    gp_Pnt CCPACSFuselageSegment::GetPointOnXPlane(double eta, double xpos, int pointIndex)
    {
        TopoDS_Shape intersectionWire = getWireOnLoft(eta);

        //build cutting plane for intersection
        gp_Pnt p1(-1.0e7, -1.0e7, xpos);
        gp_Pnt p2( 1.0e7, -1.0e7, xpos);
        gp_Pnt p3( 1.0e7,  1.0e7, xpos);
        gp_Pnt p4(-1.0e7,  1.0e7, xpos);

        Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
        Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
        Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
        Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

        TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
        TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
        TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
        TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

        TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
        TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

        // intersection-points
        BRepExtrema_DistShapeShape distSS;
        distSS.LoadS1(intersectionWire);
        distSS.LoadS2(shaft_face);
        distSS.Perform();

        return distSS.PointOnShape1(pointIndex);
    }


    // Gets the wire on the loft at a given eta
    TopoDS_Shape CCPACSFuselageSegment::getWireOnLoft(double eta)
    {
        // get eta-x-coordinate
        gp_Pnt tmpPoint = GetPoint(eta, 0.1);
        //fuselage->GetFuselageTransformation().Transform(tmpPoint);

        // Build cutting plane
        gp_Pnt p1(tmpPoint.X(), -1.0e7, -1.0e7);
        gp_Pnt p2(tmpPoint.X(),  1.0e7, -1.0e7);
        gp_Pnt p3(tmpPoint.X(),  1.0e7, 1.0e7);
        gp_Pnt p4(tmpPoint.X(), -1.0e7, 1.0e7);

        Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
        Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
        Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
        Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

        TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
        TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
        TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
        TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

        TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
        TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

        // calculate intersection between loft and cutting plane
        Standard_Real tolerance = 1.0e-7;
        int numWires = 0;                           /* The number of intersection lines */
        TopoDS_Shape intersectionResult;            /* The full Intersection result */
        Handle(TopTools_HSequenceOfShape) Wires;    /* All intersection wires */
        Handle(TopTools_HSequenceOfShape) Edges;    /* All intersection edges */
        Standard_Boolean PerformNow = Standard_False;
        BRepAlgoAPI_Section section(GetLoft(), shaft_face, PerformNow);
        section.ComputePCurveOn1(Standard_True);
        section.Approximation(Standard_True);
        section.Build();
        intersectionResult = section.Shape();

        TopExp_Explorer myEdgeExplorer(intersectionResult, TopAbs_EDGE);

        Edges = new TopTools_HSequenceOfShape();
        Wires = new TopTools_HSequenceOfShape();

        while (myEdgeExplorer.More()) {
            Edges->Append(TopoDS::Edge(myEdgeExplorer.Current()));
            myEdgeExplorer.Next();
            numWires++;
        }

        // connect edges to wires and save them to Wire-sequence
        ShapeAnalysis_FreeBounds::ConnectEdgesToWires(Edges, tolerance, false, Wires);
        if (numWires < 1){
            throw CTiglError("Error: No intersection found in CCPACSFuselageSegment::getWireOnLoft", TIGL_NOT_FOUND);
        }
        return fuselage->GetFuselageTransformation().Transform(TopoDS::Wire(Wires->Value(1)));
    }


    int CCPACSFuselageSegment::GetNumPointsOnYPlane(double eta, double ypos)
    {
        TopoDS_Shape intersectionWire = getWireOnLoft(eta);

        //build cutting plane for intersection
        gp_Pnt p1(-1.0e7, ypos, -1.0e7);
        gp_Pnt p2( 1.0e7, ypos, -1.0e7);
        gp_Pnt p3( 1.0e7, ypos,  1.0e7);
        gp_Pnt p4(-1.0e7, ypos,  1.0e7);

        Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
        Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
        Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
        Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

        TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
        TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
        TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
        TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

        TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
        TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

        // intersection-points
        BRepExtrema_DistShapeShape distSS;
        distSS.LoadS1(intersectionWire);
        distSS.LoadS2(shaft_face);
        distSS.Perform();

        return distSS.NbSolution();
    }


    int CCPACSFuselageSegment::GetNumPointsOnXPlane(double eta, double xpos)
    {
        TopoDS_Shape intersectionWire = getWireOnLoft(eta);

        //build cutting plane for intersection
        gp_Pnt p1(-1.0e7, -1.0e7, xpos);
        gp_Pnt p2( 1.0e7, -1.0e7, xpos);
        gp_Pnt p3( 1.0e7,  1.0e7, xpos);
        gp_Pnt p4(-1.0e7,  1.0e7, xpos);

        Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
        Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
        Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
        Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

        TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
        TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
        TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
        TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

        TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
        TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

        // intersection-points
        BRepExtrema_DistShapeShape distSS;
        distSS.LoadS1(intersectionWire);
        distSS.LoadS2(shaft_face);
        distSS.Perform();

        return distSS.NbSolution();
    }


    // Gets a point on the fuselage segment in dependence of an angle alpha (degree).
    // The origin of the angle could be set via the parameters x_cs and z_cs,
    // but in most cases x_cs and z_cs will be zero get the get center line of the profile.
    gp_Pnt CCPACSFuselageSegment::GetPointAngle(double eta, double alpha, double y_cs, double z_cs)
    {
        // get eta-y-coordinate
        gp_Pnt tmpPoint = GetPoint(eta, 0.0);

        // get outer wire
        TopoDS_Shape intersectionWire = getWireOnLoft(eta);

        // compute approximate cross section of fuselage wire
        Bnd_Box boundingBox;
        BRepBndLib::Add(intersectionWire, boundingBox);
        Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
        boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        double xw = xmax - xmin;
        double yw = ymax - ymin;
        double zw = zmax - zmin;

        double cross_section = max(xw, max(yw, zw));

        // This defines a length of a line intersecting with the fuselage
        double length = cross_section * 2.;
        double angle = alpha/180. * M_PI;
        // build a line
        gp_Pnt initPoint(tmpPoint.X(), y_cs, z_cs);
        gp_Pnt endPoint (tmpPoint.X(), y_cs - length*sin(angle),  z_cs + length*cos(angle));

        BRepBuilderAPI_MakeEdge edge1(initPoint, endPoint);
        TopoDS_Shape lineShape = edge1.Shape();

        // calculate intersection point
        BRepExtrema_DistShapeShape distSS;
        distSS.LoadS1(intersectionWire);
        distSS.LoadS2(lineShape);
        distSS.Perform();

        int numberOfIntersections = distSS.NbSolution();
        if(numberOfIntersections > 1) {
            gp_Pnt p1 = distSS.PointOnShape1(1);
            for (int iSol = 1; iSol <= distSS.NbSolution(); ++iSol){
                if (p1.Distance(distSS.PointOnShape1(1)) > 1e-7){
                    LOG(WARNING) << "Multiple intersection points found in CCPACSFuselageSegment::GetPointAngle. Only the first is returned." << std::endl;
                    break;
                }
            }
            return p1;
        }
        else if(numberOfIntersections <= 0) {
            LOG(ERROR) << "No solution found in CCPACSFuselageSegment::GetPointAngle. Return (0,0,0) instead." << std::endl;
            return gp_Pnt(0., 0., 0.);
        }
        else
            return distSS.PointOnShape1(1);
    }



    // Returns the circumference if the segment at a given eta
    double CCPACSFuselageSegment::GetCircumference(const double eta)
    {
        TopoDS_Shape intersectionWire = getWireOnLoft(eta);

        GProp_GProps System;
        BRepGProp::LinearProperties(intersectionWire,System);
        myWireLength = System.Mass();
        return myWireLength;
    }


#ifdef TIGL_USE_XCAF
    // builds data structure for a TDocStd_Application
    // mostly used for export
    TDF_Label CCPACSFuselageSegment::ExportDataStructure(CCPACSConfiguration&, Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label)
    {
        TopExp_Explorer Ex1;
        int i = 0;

//        gp_Trsf t0;
//        TopLoc_Location location0(t0);
//        myAssembly->AddComponent(label, subLabel, location0);

        Handle_XCAFDoc_ShapeTool newAssembly = XCAFDoc_DocumentTool::ShapeTool (label);
        TDF_Label subLabel = myAssembly->NewShape();

        for ( Ex1.Init(GetLoft(), TopAbs_FACE); Ex1.More(); Ex1.Next() )  {
            //std::string numName = "Face_" + i;
            subLabel = newAssembly->AddSubShape (label, Ex1.Current());
            TDataStd_Name::Set(label, "xxx"); //numName.c_str());
        }
//        // This component
//        TDF_Label aLabel = myAssembly->AddShape(GetLoft(), false);
//        TDataStd_Name::Set (aLabel, GetUID().c_str());
//
//        // Other (sub)-components
//        ChildContainerType::iterator it = childContainer.begin();
//        for(; it != childContainer.end(); ++it){
//            CTiglAbstractPhysicalComponent * pChild = *it;
//            if(pChild) TDF_Label aLabel = pChild->ExportDataStructure(myAssembly);
//        }

        return subLabel;
    }
#endif

} // end namespace tigl
