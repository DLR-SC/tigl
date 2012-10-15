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
* @brief  Implementation of CPACS wing handling routines.
*/

#include <iostream>

#include "CCPACSWing.h"
#include "CCPACSConfiguration.h"
#include "ITiglSegment.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepAlgoAPI_Cut.hxx"


namespace tigl {

    // Constructor
    CCPACSWing::CCPACSWing(CCPACSConfiguration* config)
        : configuration(config)
        , segments(this)
    	, componentSegments(this)
		, rebuildFusedSegments(true)
    {
        Cleanup();
    }

    // Destructor
    CCPACSWing::~CCPACSWing(void)
    {
        Cleanup();
    }

    // Invalidates internal state
    void CCPACSWing::Invalidate(void)
    {
        invalidated = true;
        segments.Invalidate();
        positionings.Invalidate();
    }

    // Cleanup routine
    void CCPACSWing::Cleanup(void)
    {
        name = "";
        transformation.SetIdentity();
        translation = CTiglPoint(0.0, 0.0, 0.0);
        scaling     = CTiglPoint(1.0, 1.0, 1.0);
        rotation    = CTiglPoint(0.0, 0.0, 0.0);

        // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
        Reset();

        Invalidate();
    }

    // Builds transformation matrix for the wing
    void CCPACSWing::BuildMatrix(void)
    {
        transformation.SetIdentity();

        // Step 1: scale the wing around the orign
        transformation.AddScaling(scaling.x, scaling.y, scaling.z);

        // Step 2: rotate the wing
        // Step 2a: rotate the wing around z (yaw   += right tip forward)
        transformation.AddRotationZ(rotation.z);
        // Step 2b: rotate the wing around y (pitch += nose up)
        transformation.AddRotationY(rotation.y);
        // Step 2c: rotate the wing around x (roll  += right tip up)
        transformation.AddRotationX(rotation.x);

        // Step 3: translate the rotated wing into its position
        transformation.AddTranslation(translation.x, translation.y, translation.z);
    }

    // Update internal wing data
    void CCPACSWing::Update(void)
    {
        if (!invalidated)
            return;

        BuildMatrix();
        invalidated = false;
		rebuildFusedSegments = true;    // forces a rebuild of all segments with regards to the updated translation
    }

    // Read CPACS wing element
    void CCPACSWing::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath)
    {
        Cleanup();

        char*       elementPath;
        std::string tempString;

        // Get subelement "name"
        char* ptrName = "";
        tempString    = wingXPath + "/name";
        elementPath   = const_cast<char*>(tempString.c_str());
        tixiGetTextElement(tixiHandle, elementPath, &ptrName);
        name          = ptrName;

        // Get attribute "uid"
        char* ptrUID = "";
        tempString   = "uID";
        elementPath  = const_cast<char*>(tempString.c_str());
        tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID);
        SetUID(ptrUID);


        // Get symmetry axis attribute
        char* ptrSym = "";
        tempString   = "symmetry";
        elementPath  = const_cast<char*>(tempString.c_str());
        tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym);
        SetSymmetryAxis(ptrSym);

        // Get subelement "parent_uid"
        char* ptrParentUID = "";
        tempString         = wingXPath + "/parentUID";
        elementPath        = const_cast<char*>(tempString.c_str());
		if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
	        tixiGetTextElement(tixiHandle, elementPath, &ptrParentUID);
		}
        SetParentUID(ptrParentUID);

        // Get subelement "/transformation/translation"
        tempString  = wingXPath + "/transformation/translation";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(translation.x), &(translation.y), &(translation.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <translation/> in CCPACSWing::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "/transformation/scaling"
        tempString  = wingXPath + "/transformation/scaling";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(scaling.x), &(scaling.y), &(scaling.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <scaling/> in CCPACSWing::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "/transformation/rotation"
        tempString  = wingXPath + "/transformation/rotation";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(rotation.x), &(rotation.y), &(rotation.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <rotation/> in CCPACSWing::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "sections"
        sections.ReadCPACS(tixiHandle, wingXPath);

        // Get subelement "positionings"
        positionings.ReadCPACS(tixiHandle, wingXPath);

        // Get subelement "segments"
        segments.ReadCPACS(tixiHandle, wingXPath);

        // Get subelement "componentSegments"
		componentSegments.ReadCPACS(tixiHandle, wingXPath);

        // Register ourself at the unique id manager
        configuration->GetUIDManager().AddUID(ptrUID, this);

        Update();
    }

    // Returns the name of the wing
    const std::string& CCPACSWing::GetName(void) const
    {
        return name;
    }

    // Returns the uid of the wing
    const char* CCPACSWing::GetUIDPtr(void)
    {
		//std::string uid = GetUID();
        uid = GetUID();
        return uid.c_str();
    }

    // Returns the parent configuration
    CCPACSConfiguration& CCPACSWing::GetConfiguration(void) const
    {
        return *configuration;
    }

    // Get section count
    int CCPACSWing::GetSectionCount(void) const
    {
        return sections.GetSectionCount();
    }

    // Returns the section for a given index
    CCPACSWingSection& CCPACSWing::GetSection(int index) const
    {
        return sections.GetSection(index);
    }

    // Get segment count
    int CCPACSWing::GetSegmentCount(void)
    {
        return segments.GetSegmentCount();
    }

    // Returns the segment for a given index
    ITiglSegment & CCPACSWing::GetSegment(const int index)
    {
        return (ITiglSegment &) segments.GetSegment(index);
    }

	// Returns the segment for a given uid
    ITiglSegment & CCPACSWing::GetSegment(std::string uid)
    {
        return (ITiglSegment &) segments.GetSegment(uid);
    }

	 // Get componentSegment count
    int CCPACSWing::GetComponentSegmentCount(void)
    {
		return componentSegments.GetComponentSegmentCount();
    }

    // Returns the segment for a given index
    ITiglSegment & CCPACSWing::GetComponentSegment(const int index)
    {
		return (ITiglSegment &) componentSegments.GetComponentSegment(index);
    }

    // Returns the segment for a given uid
    ITiglSegment & CCPACSWing::GetComponentSegment(std::string uid)
    {
		return (ITiglSegment &) componentSegments.GetComponentSegment(uid);
    }


    // Gets the loft of the whole wing.
	TopoDS_Shape & CCPACSWing::GetLoft(void)
	{
		if(rebuildFusedSegments) {
			BuildFusedSegments();
			// Transform by wing transformation
		    fusedSegments = GetWingTransformation().Transform(fusedSegments);
		}
		rebuildFusedSegments = false;
		return fusedSegments;
	}


	// Builds a fused shape of all wing segments
	void CCPACSWing::BuildFusedSegments(void)
	{
		//@todo: this probably works only if the wings does not split somewere
		BRepOffsetAPI_ThruSections generator(Standard_True, Standard_True, Precision::Confusion() );

		for (int i=1; i <= segments.GetSegmentCount(); i++) {
			CCPACSWingConnection& startConnection = segments.GetSegment(i).GetInnerConnection();

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
				throw CTiglError("Error: Wrong shape type in CCPACSWing::BuildFusedSegments", TIGL_ERROR);
			}
			startWire = TopoDS::Wire(startShape);

			generator.AddWire(startWire);
		}

		CCPACSWingConnection& endConnection = segments.GetSegment(segments.GetSegmentCount()).GetOuterConnection();
		CCPACSWingProfile& endProfile = endConnection.GetProfile();
		TopoDS_Wire endWire = endProfile.GetWire(true);
		TopoDS_Shape endShape = endConnection.GetSectionElementTransformation().Transform(endWire);
		endShape = endConnection.GetSectionTransformation().Transform(endShape);
		endShape = endConnection.GetPositioningTransformation().Transform(endShape);
		endWire = TopoDS::Wire(endShape);
		generator.AddWire(endWire);

		generator.CheckCompatibility(Standard_False);
		generator.Build();
		fusedSegments = generator.Shape();
	}


    // Gets the wing transformation (original wing implementation, but see GetTransformation)
    CTiglTransformation CCPACSWing::GetWingTransformation(void)
    {
        Update();   // create new transformation matrix if scaling, rotation or translation was changed
        return transformation;
    }

    // Get the positioning transformation for a given section-uid
    CTiglTransformation CCPACSWing::GetPositioningTransformation(std::string index)
    {
        return positionings.GetPositioningTransformation(index);
    }

    // Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
    gp_Pnt CCPACSWing::GetUpperPoint(int segmentIndex, double eta, double xsi)
    {
        gp_Pnt pnt = ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetUpperPoint(eta, xsi);
        return GetWingTransformation().Transform(pnt);
    }

    // Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
    gp_Pnt CCPACSWing::GetLowerPoint(int segmentIndex, double eta, double xsi)
    {
        gp_Pnt pnt = ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetLowerPoint(eta, xsi);
        return GetWingTransformation().Transform(pnt);
    }

    // Returns the volume of this wing
    double CCPACSWing::GetVolume(void)
    {
        double myVolume = 0.0;
        GetLoft();

        // Calculate volume
        GProp_GProps System;
        BRepGProp::VolumeProperties(fusedSegments, System);
        myVolume = System.Mass();
        return myVolume;
    }

    // Get the Transformation object (general interface implementation)
    CTiglTransformation CCPACSWing::GetTransformation(void)
    {
        return GetWingTransformation();
    }

	// Sets the Transformation object
    void CCPACSWing::Translate(CTiglPoint trans)
    {
    	translation.x += trans.x;
    	translation.y += trans.y;
    	translation.z += trans.z;
    	invalidated = true;
    	segments.Invalidate();
    	Update();
    }
    
    // Get Translation
    CTiglPoint CCPACSWing::GetTranslation(void)
    {
    	return translation;
    }

	// Returns the surface area of this wing
	double CCPACSWing::GetSurfaceArea(void)
    {
        double myArea = 0.0;
        GetLoft();

        // Calculate surface area
        GProp_GProps System;
		BRepGProp::SurfaceProperties(fusedSegments, System);
        myArea = System.Mass();
        return myArea;
	}

	// Returns the reference area of this wing.
	// Here, we always take the reference wing area to be that of the trapezoidal portion of the wing projected into the centerline.
	// The leading and trailing edge chord extensions are not included in this definition and for some airplanes, such as Boeing's Blended
	// Wing Body, the difference can be almost a factor of two between the "real" wing area and the "trap area". Some companies use reference
	// wing areas that include portions of the chord extensions, and in some studies, even tail area is included as part of the reference area.
	// For simplicity, we use the trapezoidal area here.
	double CCPACSWing::GetReferenceArea()
	{
		double refArea = 0.0;

		for (int i=1; i <= segments.GetSegmentCount(); i++) {
			refArea += segments.GetSegment(i).GetReferenceArea();
		}
		return refArea;
	}


	double CCPACSWing::GetWettedArea(TopoDS_Shape parent)
	{
		double wetArea = 0.0;
		TopoDS_Shape loft = GetLoft();

		TopoDS_Shape wettedLoft = BRepAlgoAPI_Cut(loft, parent); 

		GProp_GProps System;
		BRepGProp::SurfaceProperties(wettedLoft, System);
        wetArea = System.Mass();
		return wetArea;
	}


	// Returns a unique Hashcode for a specific geometric component
	int CCPACSWing::GetComponentHashCode(void)
	{
		GetLoft();
		return fusedSegments.HashCode(2294967295);
	}
	
	// Returns the lower Surface of a Segment
	Handle(Geom_Surface) CCPACSWing::GetLowerSegmentSurface(int index)
	{
		return segments.GetSegment(index).GetLowerSurface();
	}

	// Returns the upper Surface of a Segment
	Handle(Geom_Surface) CCPACSWing::GetUpperSegmentSurface(int index)
	{
		return segments.GetSegment(index).GetUpperSurface();
	}



} // end namespace tigl
