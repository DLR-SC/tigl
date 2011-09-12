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
* @brief  Implementation of CPACS wing ComponentSegment handling routines.
*/

#include <math.h>
#include <iostream>
#include <string>

#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
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
	CCPACSWingComponentSegment::CCPACSWingComponentSegment(CCPACSWing* aWing, int aSegmentIndex)
		: wing(aWing)
        , mySegmentIndex(aSegmentIndex)
        , surfacesAreValid(false)
	{
		//Cleanup();
	}

	// Destructor
	CCPACSWingComponentSegment::~CCPACSWingComponentSegment(void)
	{
		Cleanup();
	}

	// Invalidates internal state
	void CCPACSWingComponentSegment::Invalidate(void)
	{
		invalidated = true;
		surfacesAreValid = false;
	}

	// Cleanup routine
	void CCPACSWingComponentSegment::Cleanup(void)
	{
		name = "";
		Invalidate();
	}

	// Update internal segment data
	void CCPACSWingComponentSegment::Update(void)
	{
		if (!invalidated)
			return;

		BuildLoft();
		invalidated = false;
	}

	// Read CPACS segment elements
	void CCPACSWingComponentSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
	{
		//Cleanup();

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

		// Get fromElementUID
        char* ptrFromElementUID = "";
		tempString = segmentXPath + "/fromElementUID";
		elementPath   = const_cast<char*>(tempString.c_str());
		tixiGetTextElement(tixiHandle, elementPath, &ptrFromElementUID);
		fromElementUID = ptrFromElementUID;

		// Get toElementUID
		char* ptrToElementUID = "";
		tempString = segmentXPath + "/toElementUID";
		elementPath   = const_cast<char*>(tempString.c_str());
		tixiGetTextElement(tixiHandle, elementPath, &ptrToElementUID);
		toElementUID = ptrToElementUID;

		Update();
	}

	// Returns the wing this segment belongs to
	CCPACSWing& CCPACSWingComponentSegment::GetWing(void) const
	{
		return *wing;
	}


	// Gets the loft between the two segment sections
	TopoDS_Shape CCPACSWingComponentSegment::GetLoft(void)
	{
		Update();
		return loft;
	}



	// Builds the loft between the two segment sections
	void CCPACSWingComponentSegment::BuildLoft(void)
	{

		BRepOffsetAPI_ThruSections generator(Standard_True, Standard_True, Precision::Confusion() );

		int i;
		bool inComponentSection = false;
		int segmentCount = wing->GetSegmentCount();

		for (i=1; i <= segmentCount; i++)
		{
			CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(i);
			std::string x = segment.GetInnerSectionElementUID();
			
			// if we found the outer section, break...
			if(segment.GetInnerSectionElementUID() == toElementUID) {
				i++;
				break;
			}

			// Ok, we found the first segment of this componentSegment
			if(segment.GetInnerSectionElementUID() == fromElementUID) {
				inComponentSection = true;
			}

			// try next segment if this is not withing the componentSegment
			if (!inComponentSection) continue;
			
			CCPACSWingConnection& startConnection = segment.GetInnerConnection();

			CCPACSWingProfile& startProfile = startConnection.GetProfile();
			TopoDS_Wire startWire = startProfile.GetWire(true);

			// Do section element transformations
			TopoDS_Shape startShape = startConnection.GetSectionElementTransformation().Transform(startWire);

			// Do section transformations
			startShape = startConnection.GetSectionTransformation().Transform(startShape);

			// Do positioning transformations (positioning of sections)
			startShape = startConnection.GetPositioningTransformation().Transform(startShape);

			// Cast shapes to wires, see OpenCascade documentation
			if (startShape.ShapeType() != TopAbs_WIRE) {
				throw CTiglError("Error: Wrong shape type in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
			}
			startWire = TopoDS::Wire(startShape);

			generator.AddWire(startWire);
		}

		// somethings goes wrong, we could not find the starting Segment.
		if (!inComponentSection) {
			throw CTiglError("Error: Could not find fromSectionElement in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
		}

		CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(--i);

		if(segment.GetOuterSectionElementUID() == toElementUID) 
		{
			CCPACSWingConnection& endConnection = segment.GetOuterConnection();
			CCPACSWingProfile& endProfile = endConnection.GetProfile();
			TopoDS_Wire endWire = endProfile.GetWire(true);
			TopoDS_Shape endShape = endConnection.GetSectionElementTransformation().Transform(endWire);
			endShape = endConnection.GetSectionTransformation().Transform(endShape);
			endShape = endConnection.GetPositioningTransformation().Transform(endShape);
			endWire = TopoDS::Wire(endShape);
			generator.AddWire(endWire);
		} else if(segment.GetInnerSectionElementUID() == toElementUID) 
		{
			CCPACSWingConnection& endConnection = segment.GetInnerConnection();
			CCPACSWingProfile& endProfile = endConnection.GetProfile();
			TopoDS_Wire endWire = endProfile.GetWire(true);
			TopoDS_Shape endShape = endConnection.GetSectionElementTransformation().Transform(endWire);
			endShape = endConnection.GetSectionTransformation().Transform(endShape);
			endShape = endConnection.GetPositioningTransformation().Transform(endShape);
			endWire = TopoDS::Wire(endShape);
			generator.AddWire(endWire);
		} else 
		{
			// somethings goes wrong, we could not find the ending Segment.
			throw CTiglError("Error: Could not find toSectionElement in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
		}

		generator.CheckCompatibility(Standard_False);
		generator.Build();
		loft = generator.Shape();

		// transform with wing transformation
		loft = wing->GetWingTransformation().Transform(loft);

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

//	// Gets the upper point in relative wing coordinates for a given eta and xsi
//	gp_Pnt CCPACSWingComponentSegment::GetUpperPoint(double eta, double xsi)
//	{
//        return GetPoint(eta, xsi, true);
//	}
//
//    // Gets the lower point in relative wing coordinates for a given eta and xsi
//    gp_Pnt CCPACSWingComponentSegment::GetLowerPoint(double eta, double xsi)
//	{
//        return GetPoint(eta, xsi, false);
//	}
//

    // Returns the volume of this segment
    double CCPACSWingComponentSegment::GetVolume(void)
    {
        Update();
        return( myVolume );
    }

	// Returns the surface area of this segment
    double CCPACSWingComponentSegment::GetSurfaceArea(void)
    {
        Update();
        return( mySurfaceArea );
    }

//    // Returns an upper or lower point on the segment surface in
//    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
//    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
//    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
//    // edge on the outer wing profile. If fromUpper is true, a point
//    // on the upper surface is returned, otherwise from the lower.
//    gp_Pnt CCPACSWingComponentSegment::GetPoint(double eta, double xsi, bool fromUpper)
//    {
//        if (eta < 0.0 || eta > 1.0)
//        {
//            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
//        }
//
//		CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
//		CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
//
//        // Compute points on wing profiles for the given xsi
//        gp_Pnt innerProfilePoint;
//        gp_Pnt outerProfilePoint;
//        if (fromUpper == true)
//        {
//            innerProfilePoint = innerProfile.GetUpperPoint(xsi);
//            outerProfilePoint = outerProfile.GetUpperPoint(xsi);
//        }
//        else
//        {
//            innerProfilePoint = innerProfile.GetLowerPoint(xsi);
//            outerProfilePoint = outerProfile.GetLowerPoint(xsi);
//        }
//
//        // Do section element transformation on points
//        innerProfilePoint = innerConnection.GetSectionElementTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetSectionElementTransformation().Transform(outerProfilePoint);
//
//        // Do section transformations
//        innerProfilePoint = innerConnection.GetSectionTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetSectionTransformation().Transform(outerProfilePoint);
//
//        // Do positioning transformations
//        innerProfilePoint = innerConnection.GetPositioningTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetPositioningTransformation().Transform(outerProfilePoint);
//
//        // Get point on wing segment in dependence of eta by linear interpolation
//        Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(innerProfilePoint, outerProfilePoint);
//        Standard_Real firstParam = profileLine->FirstParameter();
//        Standard_Real lastParam  = profileLine->LastParameter();
//        Standard_Real param = (lastParam - firstParam) * eta;
//        gp_Pnt profilePoint;
//        profileLine->D0(param, profilePoint);
//
//		return profilePoint;
//    }
//
    // Gets the uid of this segment
    const std::string & CCPACSWingComponentSegment::GetUID(void)
    {
    	return uid;
    }

    // Gets the uid of this segment
    const char* CCPACSWingComponentSegment::GetUIDPtr(void)
    {
    	return uid.c_str();
    }


    // Gets the fromElementUID of this segment
	const std::string & CCPACSWingComponentSegment::GetFromElementUID(void)
	{
		return fromElementUID;
	}

	// Gets the toElementUID of this segment
	const std::string & CCPACSWingComponentSegment::GetToElementUID(void)
	{
		return toElementUID;
	}


} // end namespace tigl
