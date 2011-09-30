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
* @brief  Implementation of CPACS wing segment handling routines.
*/

#include <math.h>
#include <iostream>
#include <string>

#include "CCPACSWingSegment.h"
#include "CCPACSWing.h"
#include "CCPACSWingProfile.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Shell.hxx"
#include "TopoDS_Wire.hxx"
#include "gp_Trsf.hxx"
#include "gp_Lin.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "GC_MakeSegment.hxx"
#include "GeomAPI_IntCS.hxx"
#include "Geom_Surface.hxx"
#include "Geom_Line.hxx"
#include "gce_MakeLin.hxx"
#include "IntCurvesFace_Intersector.hxx"
#include "Precision.hxx"
#include "TopLoc_Location.hxx"
#include "Poly_Triangulation.hxx"
#include "Poly_Array1OfTriangle.hxx"
#include "BRep_Tool.hxx"
#include "BRep_Builder.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepMesh.hxx"
#include "BRepTools.hxx"
#include "BRepBndLib.hxx"
#include "BRepGProp.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "Bnd_Box.hxx"
#include "BRepLib_FindSurface.hxx"
#include "ShapeAnalysis_Surface.hxx"
#include "GProp_GProps.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "GeomAdaptor_Surface.hxx"
#include "GC_MakeLine.hxx"

#include "Geom_BSplineCurve.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "GeomAdaptor_HCurve.hxx"
#include "GeomFill_SimpleBound.hxx"
#include "GeomFill_BSplineCurves.hxx"
#include "GeomFill_FillingStyle.hxx"
#include "Geom_BSplineSurface.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"


#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace tigl {

	// Constructor
	CCPACSWingSegment::CCPACSWingSegment(CCPACSWing* aWing, int aSegmentIndex)
		: innerConnection(this)
		, outerConnection(this)
		, wing(aWing)
        , mySegmentIndex(aSegmentIndex)
        , surfacesAreValid(false)
	{
		Cleanup();
	}

	// Destructor
	CCPACSWingSegment::~CCPACSWingSegment(void)
	{
		Cleanup();
	}

	// Invalidates internal state
	void CCPACSWingSegment::Invalidate(void)
	{
		invalidated = true;
		surfacesAreValid = false;
	}

	// Cleanup routine
	void CCPACSWingSegment::Cleanup(void)
	{
		name = "";
		Invalidate();
	}

	// Update internal segment data
	void CCPACSWingSegment::Update(void)
	{
		if (!invalidated)
			return;

		BuildLoft();
		invalidated = false;
	}

	// Read CPACS segment elements
	void CCPACSWingSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
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

		// Inner connection
		tempString = segmentXPath + "/fromElementUID";
		innerConnection.ReadCPACS(tixiHandle, tempString);

		// Inner connection
		tempString = segmentXPath + "/toElementUID";
		outerConnection.ReadCPACS(tixiHandle, tempString);

		Update();
	}

	// Returns the wing this segment belongs to
	CCPACSWing& CCPACSWingSegment::GetWing(void) const
	{
		return *wing;
	}

    // Returns the segment index of this segment
    int CCPACSWingSegment::GetSegmentIndex(void) const
    {
        return mySegmentIndex;
    }

	// Gets the loft between the two segment sections
	TopoDS_Shape CCPACSWingSegment::GetLoft(void)
	{
		Update();
		return loft;
	}

    // helper function to get the inner transformed chord line wire
    TopoDS_Wire CCPACSWingSegment::GetInnerWire(void)
    {
		CCPACSWingProfile & innerProfile = innerConnection.GetProfile();
		TopoDS_Wire innerWire = innerProfile.GetWire(/* closed */ true);

		// Do section element transformations
		TopoDS_Shape innerShape = innerConnection.GetSectionElementTransformation().Transform(innerWire);

		// Do section transformations
		innerShape = innerConnection.GetSectionTransformation().Transform(innerShape);

		// Do positioning transformations (positioning of sections)
		innerShape = innerConnection.GetPositioningTransformation().Transform(innerShape);

		// Cast shapes to wires, see OpenCascade documentation
        if (innerShape.ShapeType() != TopAbs_WIRE) {
			throw CTiglError("Error: Wrong shape type in CCPACSWingSegment::GetInnerWire, called from BuildLoft", TIGL_ERROR);
        }
		TopoDS_Wire returnWire = TopoDS::Wire(innerShape);
        return returnWire;
    }

    // helper function to get the outer transformed chord line wire
    TopoDS_Wire CCPACSWingSegment::GetOuterWire(void)
    {
		CCPACSWingProfile & outerProfile = outerConnection.GetProfile();
		TopoDS_Wire outerWire = outerProfile.GetWire(/* closed */ true);

		// Do section element transformations
		TopoDS_Shape outerShape = outerConnection.GetSectionElementTransformation().Transform(outerWire);

		// Do section transformations
		outerShape = outerConnection.GetSectionTransformation().Transform(outerShape);

		// Do positioning transformations (positioning of sections)
		outerShape = outerConnection.GetPositioningTransformation().Transform(outerShape);

		// Cast shapes to wires, see OpenCascade documentation
        if (outerShape.ShapeType() != TopAbs_WIRE) {
			throw CTiglError("Error: Wrong shape type in CCPACSWingSegment::GetOuterWire, called from BuildLoft", TIGL_ERROR);
        }
		TopoDS_Wire returnWire = TopoDS::Wire(outerShape);
        return returnWire;
    }

	// Builds the loft between the two segment sections
	void CCPACSWingSegment::BuildLoft(void)
	{
        TopoDS_Wire innerWire = GetInnerWire();
        TopoDS_Wire outerWire = GetOuterWire();

		// Build loft
        //BRepOffsetAPI_ThruSections generator(Standard_False, Standard_False, Precision::Confusion());
        BRepOffsetAPI_ThruSections generator(/* is solid (else shell) */ Standard_True, /* ruled (else smoothed out) */ Standard_False, Precision::Confusion());
		generator.AddWire(innerWire);
		generator.AddWire(outerWire);
        generator.CheckCompatibility(/* check (defaults to true) */ Standard_False);
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

		Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
		sfs->Init ( loft );
		sfs->Perform();
		loft = sfs->Shape();

        // Calculate volume
        GProp_GProps System;
        BRepGProp::VolumeProperties(loft, System);
        myVolume = System.Mass();

		// Calculate surface area
        GProp_GProps AreaSystem;
		BRepGProp::SurfaceProperties(loft, AreaSystem);
        mySurfaceArea = AreaSystem.Mass();
	}

	// Gets the upper point in relative wing coordinates for a given eta and xsi
	gp_Pnt CCPACSWingSegment::GetUpperPoint(double eta, double xsi)
	{
        return GetPoint(eta, xsi, true);
	}

    // Gets the lower point in relative wing coordinates for a given eta and xsi
    gp_Pnt CCPACSWingSegment::GetLowerPoint(double eta, double xsi)
	{
        return GetPoint(eta, xsi, false);
	}

    // Returns the inner section UID of this segment
    std::string CCPACSWingSegment::GetInnerSectionUID(void)
    {
        return innerConnection.GetSectionUID();
    }

    // Returns the outer section UID of this segment
    std::string CCPACSWingSegment::GetOuterSectionUID(void)
    {
        return outerConnection.GetSectionUID();
    }

    // Returns the inner section element UID of this segment
    std::string CCPACSWingSegment::GetInnerSectionElementUID(void)
    {
        return innerConnection.GetSectionElementUID();
    }

    // Returns the outer section element UID of this segment
    std::string CCPACSWingSegment::GetOuterSectionElementUID(void)
    {
        return outerConnection.GetSectionElementUID();
    }

    // Returns the inner section index of this segment
    int CCPACSWingSegment::GetInnerSectionIndex(void)
    {
        return innerConnection.GetSectionIndex();
    }

    // Returns the outer section index of this segment
    int CCPACSWingSegment::GetOuterSectionIndex(void)
    {
        return outerConnection.GetSectionIndex();
    }

    // Returns the inner section element index of this segment
    int CCPACSWingSegment::GetInnerSectionElementIndex(void)
    {
        return innerConnection.GetSectionElementIndex();
    }

    // Returns the outer section element index of this segment
    int CCPACSWingSegment::GetOuterSectionElementIndex(void)
    {
        return outerConnection.GetSectionElementIndex();
    }

	// Returns the start section element index of this segment
	CCPACSWingConnection& CCPACSWingSegment::GetInnerConnection(void)
	{
		return( innerConnection );
	}

	// Returns the end section element index of this segment
	CCPACSWingConnection& CCPACSWingSegment::GetOuterConnection(void)
	{
		return( outerConnection );
	}

    // Returns the volume of this segment
    double CCPACSWingSegment::GetVolume(void)
    {
        Update();
        return( myVolume );
    }

	// Returns the surface area of this segment
    double CCPACSWingSegment::GetSurfaceArea(void)
    {
        Update();
        return( mySurfaceArea );
    }

    // Gets the count of segments connected to the inner section of this segment // TODO can this be optimized instead of iterating over all segments?
    int CCPACSWingSegment::GetInnerConnectedSegmentCount(void)
    {
        int count = 0;
        for (int i = 1; i <= GetWing().GetSegmentCount(); i++)
        {
            CCPACSWingSegment& nextSegment = (CCPACSWingSegment &) GetWing().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetInnerSectionUID() == GetInnerSectionUID() ||
                nextSegment.GetOuterSectionUID() == GetInnerSectionUID())
            {
                count++;
            }
        }
        return count;
    }

    // Gets the count of segments connected to the outer section of this segment
    int CCPACSWingSegment::GetOuterConnectedSegmentCount(void)
    {
        int count = 0;
        for (int i = 1; i <= GetWing().GetSegmentCount(); i++)
        {
            CCPACSWingSegment& nextSegment = (CCPACSWingSegment &) GetWing().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetInnerSectionUID() == GetOuterSectionUID() ||
                nextSegment.GetOuterSectionUID() == GetOuterSectionUID())
            {
                count++;
            }
        }
        return count;
    }

    // Gets the index (number) of the n-th segment connected to the inner section
    // of this segment. n starts at 1.
    int CCPACSWingSegment::GetInnerConnectedSegmentIndex(int n)
    {
        if (n < 1 || n > GetInnerConnectedSegmentCount())
        {
            throw CTiglError("Error: Invalid value for parameter n in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_INDEX_ERROR);
        }

        for (int i = 1, count = 0; i <= GetWing().GetSegmentCount(); i++)
        {
            CCPACSWingSegment& nextSegment = (CCPACSWingSegment &) GetWing().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetInnerSectionUID() == GetInnerSectionUID() ||
                nextSegment.GetOuterSectionUID() == GetInnerSectionUID())
            {
                if (++count == n) return nextSegment.GetSegmentIndex();
            }
        }

        throw CTiglError("Error: No connected segment found in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_NOT_FOUND);
    }

    // Gets the index (number) of the n-th segment connected to the outer section
    // of this segment. n starts at 1.
    int CCPACSWingSegment::GetOuterConnectedSegmentIndex(int n)
    {
        if (n < 1 || n > GetOuterConnectedSegmentCount())
        {
            throw CTiglError("Error: Invalid value for parameter n in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_INDEX_ERROR);
        }

        for (int i = 1, count = 0; i <= GetWing().GetSegmentCount(); i++)
        {
            CCPACSWingSegment& nextSegment = (CCPACSWingSegment &) GetWing().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetInnerSectionUID() == GetOuterSectionUID() ||
                nextSegment.GetOuterSectionUID() == GetOuterSectionUID())
            {
                if (++count == n) return nextSegment.GetSegmentIndex();
            }
        }

        throw CTiglError("Error: No connected segment found in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_NOT_FOUND);
    }

    // Returns an upper or lower point on the segment surface in
    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
    // edge on the outer wing profile. If fromUpper is true, a point
    // on the upper surface is returned, otherwise from the lower.
    gp_Pnt CCPACSWingSegment::GetPoint(double eta, double xsi, bool fromUpper)
    {
        if (eta < 0.0 || eta > 1.0)
        {
            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
        }

		CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
		CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

        // Compute points on wing profiles for the given xsi
        gp_Pnt innerProfilePoint;
        gp_Pnt outerProfilePoint;
        if (fromUpper == true)
        {
            innerProfilePoint = innerProfile.GetUpperPoint(xsi);
            outerProfilePoint = outerProfile.GetUpperPoint(xsi);
        }
        else
        {
            innerProfilePoint = innerProfile.GetLowerPoint(xsi);
            outerProfilePoint = outerProfile.GetLowerPoint(xsi);
        }

        // Do section element transformation on points
        innerProfilePoint = innerConnection.GetSectionElementTransformation().Transform(innerProfilePoint);
        outerProfilePoint = outerConnection.GetSectionElementTransformation().Transform(outerProfilePoint);

        // Do section transformations
        innerProfilePoint = innerConnection.GetSectionTransformation().Transform(innerProfilePoint);
        outerProfilePoint = outerConnection.GetSectionTransformation().Transform(outerProfilePoint);

        // Do positioning transformations
        innerProfilePoint = innerConnection.GetPositioningTransformation().Transform(innerProfilePoint);
        outerProfilePoint = outerConnection.GetPositioningTransformation().Transform(outerProfilePoint);

        // Get point on wing segment in dependence of eta by linear interpolation
        Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(innerProfilePoint, outerProfilePoint);
        Standard_Real firstParam = profileLine->FirstParameter();
        Standard_Real lastParam  = profileLine->LastParameter();
        Standard_Real param = (lastParam - firstParam) * eta;
        gp_Pnt profilePoint;
        profileLine->D0(param, profilePoint);

		return profilePoint;
    }

    // Returns the inner profile points as read from TIXI. The points are already transformed.
    std::vector<CTiglPoint*> CCPACSWingSegment::GetRawInnerProfilePoints()
    {
        CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
        std::vector<CTiglPoint*> points = innerProfile.GetCoordinateContainer();
		std::vector<CTiglPoint*> pointsTransformed;
        for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < points.size(); i++) {
			
			gp_Pnt pnt = points[i]->Get_gp_Pnt();

            pnt = innerConnection.GetSectionElementTransformation().Transform(pnt);
            pnt = innerConnection.GetSectionTransformation().Transform(pnt);
            pnt = innerConnection.GetPositioningTransformation().Transform(pnt);

			CTiglPoint *tiglPoint = new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z());
			pointsTransformed.push_back(tiglPoint);

        }
        return pointsTransformed;
    }


    // Returns the outer profile points as read from TIXI. The points are already transformed.
    std::vector<CTiglPoint*> CCPACSWingSegment::GetRawOuterProfilePoints()
    {
        CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
        std::vector<CTiglPoint*> points = outerProfile.GetCoordinateContainer();
		std::vector<CTiglPoint*> pointsTransformed;
        for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < points.size(); i++) {
			
			gp_Pnt pnt = points[i]->Get_gp_Pnt();

            pnt = outerConnection.GetSectionElementTransformation().Transform(pnt);
            pnt = outerConnection.GetSectionTransformation().Transform(pnt);
            pnt = outerConnection.GetPositioningTransformation().Transform(pnt);

			CTiglPoint *tiglPoint = new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z());
			pointsTransformed.push_back(tiglPoint);

        }
        return pointsTransformed;
    }


    // Gets the uid of this segment
    const std::string & CCPACSWingSegment::GetUID(void)
    {
    	return uid;
    }

    // Gets the uid of this segment
    const char* CCPACSWingSegment::GetUIDPtr(void)
    {
    	return uid.c_str();
    }

    // Returns eta as parametric distance from a given point on the surface
    double CCPACSWingSegment::GetEta(gp_Pnt pnt, bool isUpper)
    {
        double eta = 0.0;
        MakeSurfaces();
        if(isUpper) {
            ShapeAnalysis_Surface analysis(upperSurface);
            const gp_Pnt2d uv = analysis.ValueOfUV(pnt, Precision::Confusion());
            eta = uv.Y();
        } else {
            ShapeAnalysis_Surface analysis(lowerSurface);
            const gp_Pnt2d uv = analysis.ValueOfUV(pnt, Precision::Confusion());
            eta = uv.Y();
        }
        return(eta);
    }

    // Returns xsi as parametric distance from a given point on the surface
    double CCPACSWingSegment::GetXsi(gp_Pnt pnt, bool isUpper)
    {
        double xsi = 0.0;
        MakeSurfaces();
        if(isUpper) {
            ShapeAnalysis_Surface analysis(upperSurface);
            const gp_Pnt2d uv = analysis.ValueOfUV(pnt, Precision::Confusion());
            xsi = uv.X();
        } else {
            ShapeAnalysis_Surface analysis(lowerSurface);
            const gp_Pnt2d uv = analysis.ValueOfUV(pnt, Precision::Confusion());
            xsi = uv.X();
        }
        return(xsi);
    }

    // Returns if the given point is ont the Top of the wing or on the lower side.
    bool CCPACSWingSegment::GetIsOnTop(gp_Pnt pnt)
    {
		bool isUpperSide = false;
		double tolerance = 0.03;

		MakeSurfaces();

		// Note: a positive z-axis describe "up" according to wings in CPACS
		gp_Pnt lowerBound = gp_Pnt(pnt.X(), pnt.Y(), pnt.Z() - tolerance);
		gp_Pnt upperBound = gp_Pnt(pnt.X(), pnt.Y(), pnt.Z() + tolerance*2);

		Handle(Geom_TrimmedCurve) line = GC_MakeSegment(lowerBound, upperBound);

		GeomAPI_IntCS inCS;
        inCS.Perform(line, upperSurface);
        if((inCS.IsDone()) && (inCS.NbPoints() > 0) ) {
            isUpperSide = true;
        }

        return isUpperSide;
	}




	// Builds upper/lower surfaces as shapes
	void CCPACSWingSegment::MakeSurfaces()
	{
		if(surfacesAreValid)
			return;

		int numberOfWirePoints = 500;
		double xsi = 0.0;

		BRepBuilderAPI_MakePolygon upperMakePolygonOne,upperMakePolygonTwo;
		BRepBuilderAPI_MakePolygon lowerMakePolygonOne,lowerMakePolygonTwo;

		for(int i = 1; i < numberOfWirePoints; i++) 
		{
			xsi = ((double)i/(double)numberOfWirePoints) * 1.0;

			gp_Pnt p1 = GetUpperPoint(0.0, xsi);
			gp_Pnt p2 = GetUpperPoint(1.0, xsi);
            gp_Pnt p3 = GetLowerPoint(0.0, xsi);
            gp_Pnt p4 = GetLowerPoint(1.0, xsi);

			upperMakePolygonOne.Add(p1);
			upperMakePolygonTwo.Add(p2);
            lowerMakePolygonOne.Add(p3);
            lowerMakePolygonTwo.Add(p4);
		}

		upperMakePolygonOne.Build();
		upperMakePolygonTwo.Build();
        lowerMakePolygonOne.Build();
        lowerMakePolygonTwo.Build();

		BRepOffsetAPI_ThruSections upperSections(Standard_False,Standard_True);
		upperSections.AddWire(upperMakePolygonOne.Wire());
		upperSections.AddWire(upperMakePolygonTwo.Wire());
		upperSections.Build();

		BRepOffsetAPI_ThruSections lowerSections(Standard_False,Standard_True);
		lowerSections.AddWire(lowerMakePolygonOne.Wire());
		lowerSections.AddWire(lowerMakePolygonTwo.Wire());
		lowerSections.Build();

		upperShape = upperSections.Shape();
		lowerShape = lowerSections.Shape();

		// Transform upper/lower shapes
//		upperShape = wing->GetWingTransformation().Transform(upperShape);
//		lowerShape = wing->GetWingTransformation().Transform(lowerShape);

		const BRepLib_FindSurface findSurface(upperShape, /* tolerance */1.01);
		upperSurface = findSurface.Surface();
		const BRepLib_FindSurface findSurface2(lowerShape, /* tolerance */1.01);
		lowerSurface = findSurface2.Surface();

		surfacesAreValid = true;
	}


} // end namespace tigl
