/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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

#include <math.h>
#include <iostream>

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

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif


namespace tigl {

    
	// Constructor
	CCPACSFuselageSegment::CCPACSFuselageSegment(CCPACSFuselage* aFuselage, int aSegmentIndex)
		: startConnection(this)
		, endConnection(this)
		, fuselage(aFuselage)
        , mySegmentIndex(aSegmentIndex)
	{
		Cleanup();
	}

	// Destructor
	CCPACSFuselageSegment::~CCPACSFuselageSegment(void)
	{
		Cleanup();
	}

	// Invalidates internal state
	void CCPACSFuselageSegment::Invalidate(void)
	{
		invalidated = true;
	}

	// Cleanup routine
	void CCPACSFuselageSegment::Cleanup(void)
	{
		name = "";
		Invalidate();
	}

	// Update internal segment data
	void CCPACSFuselageSegment::Update(void)
	{
		if (!invalidated)
			return;

		BuildLoft();
		invalidated = false;
	}

	// Read CPACS segment elements
	void CCPACSFuselageSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
	{
		Cleanup();

		char*       elementPath;
		std::string tempString;

		// Get subelement "name"
		char* ptrName = "";
		tempString    = segmentXPath + "/name";
		elementPath   = const_cast<char*>(tempString.c_str());
		tixiGetTextElement(tixiHandle, elementPath, &ptrName);
		name          = ptrName;

        // Get attribute "uid"
        char* ptrUID = "";
        tempString   = "uID";
        elementPath  = const_cast<char*>(tempString.c_str());
        tixiGetTextAttribute(tixiHandle, const_cast<char*>(segmentXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID);
        uid = ptrUID;

		// Start connection
		tempString = segmentXPath + "/fromElementUID";
		startConnection.ReadCPACS(tixiHandle, tempString);

		// End connection
		tempString = segmentXPath + "/toElementUID";
		endConnection.ReadCPACS(tixiHandle, tempString);

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

	// Gets the loft between the two segment sections
	TopoDS_Shape CCPACSFuselageSegment::GetLoft(void)
	{
		Update();
		return loft;
	}

	// Builds the loft between the two segment sections
	void CCPACSFuselageSegment::BuildLoft(void)
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
		loft = generator.Shape();

        // Now we build the triangulation of the loft. To determine a reasonable
        // value for the deflection (see OpenCascade documentation), we build the
        // bounding box around the loft. The greatest dimension of the bounding
        // box is then used as a measure for the deflection.
        Bnd_Box boundingBox;
        BRepBndLib::Add(loft, boundingBox);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        Standard_Real xdist = xmax - xmin;
        Standard_Real ydist = ymax - ymin;
        Standard_Real zdist = zmax - zmin;

        double dist = max(max(xdist, ydist), zdist);

        double deflection = max(0.001, dist * 0.001);

        BRepTools::Clean(loft);
        BRepMesh::Mesh(loft, deflection);

        // Calculate volume
        GProp_GProps System;
        BRepGProp::VolumeProperties(loft, System);
        myVolume = System.Mass();

		// Calculate surface area
        GProp_GProps AreaSystem;
		BRepGProp::SurfaceProperties(loft, AreaSystem);
        mySurfaceArea = AreaSystem.Mass();
	}


    // Returns the start section UID of this segment
    std::string CCPACSFuselageSegment::GetStartSectionUID(void)
    {
        return startConnection.GetSectionUID();
    }

    // Returns the end section UID of this segment
    std::string CCPACSFuselageSegment::GetEndSectionUID(void)
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
    std::string CCPACSFuselageSegment::GetStartSectionElementUID(void)
    {
        return startConnection.GetSectionElementUID();
    }

    // Returns the end section element UID of this segment
    std::string CCPACSFuselageSegment::GetEndSectionElementUID(void)
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



    // Gets the uid of this segment
    const std::string& CCPACSFuselageSegment::GetUID(void)
    {
        return uid;
    }

    // Gets the uid of this segment
    const char* CCPACSFuselageSegment::GetUIDPtr(void)
    {
        return uid.c_str();
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
        BRep_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);

		// get eta-y-coordinate
		gp_Pnt tmpPoint = GetPoint(eta, 0.0);

        // build a line
		gp_Pnt initPoint(tmpPoint.X(), y_cs, z_cs);
        gp_Pnt endPoint(tmpPoint.X(), y_cs,  z_cs + 500.0);
        BRepBuilderAPI_MakeEdge edge1(initPoint, endPoint);
        builder.Add(compound, edge1);
        TopoDS_Shape lineShape(compound);

		// define the axis of symmetry
		gp_Ax1 axis = gp_Ax1(gp_Pnt(tmpPoint.X(), y_cs, z_cs), gp_Dir(1,0,0)); 

        // now rotate line
        gp_Trsf myTrsf;
		myTrsf.SetRotation(axis, alpha *PI / 180);
        BRepBuilderAPI_Transform xform(lineShape, myTrsf);
        lineShape = xform.Shape();

        // get outer wire and calculate intersection point
        TopoDS_Shape intersectionWire = getWireOnLoft(eta);
        BRepExtrema_DistShapeShape distSS;
        distSS.LoadS1(intersectionWire);
        distSS.LoadS2(lineShape);
        distSS.Perform();

		int numberOfIntersections = distSS.NbSolution();
        if(numberOfIntersections != 1) {
        	return gp_Pnt(0.0, 0.0, 0.0);
        }

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

} // end namespace tigl
