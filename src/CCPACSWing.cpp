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
* @brief  Implementation of CPACS wing handling routines.
*/

#include <iostream>

#include "CCPACSWing.h"
#include "CCPACSConfiguration.h"
#include "CTiglAbstractSegment.h"
#include "CTiglError.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "XCAFDoc_ShapeTool.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "TDataStd_Name.hxx"
#include "TDataXtd_Shape.hxx"

namespace {
	inline double max(double a, double b){
		return a > b? a : b;
	}
    
    TopoDS_Wire transformToWingCoords(const tigl::CCPACSWingConnection& wingConnection, const TopoDS_Wire& origWire) {
        TopoDS_Shape resultWire(origWire);

        // Do section element transformations
        resultWire = wingConnection.GetSectionElementTransformation().Transform(resultWire);

        // Do section transformations
        resultWire = wingConnection.GetSectionTransformation().Transform(resultWire);

        // Do positioning transformations (positioning of sections)
        resultWire = wingConnection.GetPositioningTransformation().Transform(resultWire);

        // Cast shapes to wires, see OpenCascade documentation
        if (resultWire.ShapeType() != TopAbs_WIRE) {
            throw tigl::CTiglError("Error: Wrong shape type in CCPACSWing::transformToAbsCoords", TIGL_ERROR);
        }
        
        return TopoDS::Wire(resultWire);
    }
}


namespace tigl {

    // Constructor
    CCPACSWing::CCPACSWing(CCPACSConfiguration* config)
        : segments(this)
    	, componentSegments(this)
        , configuration(config)
        , rebuildFusedSegments(true)
        , rebuildFusedSegWEdge(true)
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

        backTransformation = transformation.Inverted();
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
        char* ptrName = NULL;
        tempString    = wingXPath + "/name";
        elementPath   = const_cast<char*>(tempString.c_str());
        if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS)
            name          = ptrName;

        // Get attribute "uid"
        char* ptrUID = NULL;
        tempString   = "uID";
        elementPath  = const_cast<char*>(tempString.c_str());
        if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS)
            SetUID(ptrUID);

        // Get subelement "parent_uid"
        char* ptrParentUID = NULL;
        tempString         = wingXPath + "/parentUID";
        elementPath        = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS &&
            tixiGetTextElement(tixiHandle, elementPath, &ptrParentUID) == SUCCESS)
        {
            SetParentUID(ptrParentUID);
        }


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

        // Get symmetry axis attribute, has to be done, when segments are build
        char* ptrSym = NULL;
        tempString   = "symmetry";
        elementPath  = const_cast<char*>(tempString.c_str());
        if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS)
            SetSymmetryAxis(ptrSym);

        Update();
    }

    // Returns the name of the wing
    const std::string& CCPACSWing::GetName(void) const
    {
        return name;
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
    int CCPACSWing::GetSegmentCount(void) const
    {
        return segments.GetSegmentCount();
    }

    // Get segment count
    TDF_Label CCPACSWing::ExportDataStructure(Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label)
    {
        TDF_Label wingLabel = CTiglAbstractPhysicalComponent::ExportDataStructure(myAssembly, label);

        // Other (sub)-components
        for (int i=1; i <= segments.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = segments.GetSegment(i);
            TDF_Label wingSegmentLabel = myAssembly->AddShape(segment.GetLoft(), false);
            TDataStd_Name::Set (wingSegmentLabel, segment.GetUID().c_str());
            //TDF_Label& subSegmentLabel = segment.ExportDataStructure(myAssembly, wingSegmentLabel);
        }

        return wingLabel;
    }

    // Returns the segment for a given index
    CTiglAbstractSegment & CCPACSWing::GetSegment(const int index)
    {
        return (CTiglAbstractSegment &) segments.GetSegment(index);
    }

	// Returns the segment for a given uid
    CTiglAbstractSegment & CCPACSWing::GetSegment(std::string uid)
    {
        return (CTiglAbstractSegment &) segments.GetSegment(uid);
    }

	 // Get componentSegment count
    int CCPACSWing::GetComponentSegmentCount(void)
    {
		return componentSegments.GetComponentSegmentCount();
    }

    // Returns the segment for a given index
    CTiglAbstractSegment & CCPACSWing::GetComponentSegment(const int index)
    {
        return (CTiglAbstractSegment &) componentSegments.GetComponentSegment(index);
    }

    // Returns the segment for a given uid
    CTiglAbstractSegment & CCPACSWing::GetComponentSegment(std::string uid)
    {
        return (CTiglAbstractSegment &) componentSegments.GetComponentSegment(uid);
    }


    // Gets the loft of the whole wing.
	TopoDS_Shape & CCPACSWing::GetLoft(void)
	{
		if(rebuildFusedSegments) {
			fusedSegments = BuildFusedSegments(false);
			// Transform by wing transformation
		    fusedSegments = GetWingTransformation().Transform(fusedSegments);
		}
		rebuildFusedSegments = false;
		return fusedSegments;
	}

    // Gets the loft of the whole wing with modeled leading edge.
	TopoDS_Shape & CCPACSWing::GetLoftWithLeadingEdge(void)
	{
		if(rebuildFusedSegWEdge) {
			fusedSegmentWithEdge = BuildFusedSegments(true);
			// Transform by wing transformation
			fusedSegmentWithEdge = GetWingTransformation().Transform(fusedSegmentWithEdge);
		}
		rebuildFusedSegWEdge = false;
		return fusedSegmentWithEdge;
	}

	// Builds a fused shape of all wing segments
	TopoDS_Shape CCPACSWing::BuildFusedSegments(bool splitWingInUpperAndLower)
	{
		//@todo: this probably works only if the wings does not split somewere
		BRepOffsetAPI_ThruSections generator(Standard_True, Standard_True, Precision::Confusion() );

		for (int i=1; i <= segments.GetSegmentCount(); i++) {
			CCPACSWingConnection& startConnection = segments.GetSegment(i).GetInnerConnection();
			CCPACSWingProfile& startProfile = startConnection.GetProfile();
			TopoDS_Wire startWire;
			if(!splitWingInUpperAndLower)
				startWire = transformToWingCoords(startConnection, startProfile.GetWire(true));
			else
				startWire = transformToWingCoords(startConnection, startProfile.GetFusedUpperLowerWire());
			generator.AddWire(startWire);
		}

		CCPACSWingConnection& endConnection = segments.GetSegment(segments.GetSegmentCount()).GetOuterConnection();
		CCPACSWingProfile& endProfile = endConnection.GetProfile();
		TopoDS_Wire endWire;
		if(!splitWingInUpperAndLower)
			endWire = transformToWingCoords(endConnection,endProfile.GetWire(true));
		else
			endWire = transformToWingCoords(endConnection,endProfile.GetFusedUpperLowerWire());
		generator.AddWire(endWire);

		generator.CheckCompatibility(Standard_False);
		generator.Build();
		return generator.Shape();
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
        return ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetUpperPoint(eta, xsi);
    }

    // Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
    gp_Pnt CCPACSWing::GetLowerPoint(int segmentIndex, double eta, double xsi)
    {
        return  ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetLowerPoint(eta, xsi);
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
        CTiglAbstractGeometricComponent::Translate(trans);
    	invalidated = true;
    	segments.Invalidate();
    	componentSegments.Invalidate();
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

	// sets the symmetry plane for all childs, segments and component segments
	void CCPACSWing::SetSymmetryAxis(const std::string& axis){
		CTiglAbstractGeometricComponent::SetSymmetryAxis(axis);

		for(int i = 1; i <= segments.GetSegmentCount(); ++i){
			CCPACSWingSegment& segment = segments.GetSegment(i);
			segment.SetSymmetryAxis(axis);
		}

		for(int i = 1; i <= componentSegments.GetComponentSegmentCount(); ++i){
			CCPACSWingComponentSegment& compSeg = componentSegments.GetComponentSegment(i);
			compSeg.SetSymmetryAxis(axis);
		}
	}

	double CCPACSWing::GetWingspan() {
		Bnd_Box boundingBox;
		if (GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
			for (int i = 1; i <= GetSegmentCount(); ++i) {
				TopoDS_Shape& segmentShape = GetSegment(i).GetLoft();
				BRepBndLib::Add(segmentShape, boundingBox);
			}

			Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
			boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
			double xw = xmax - xmin;
			double yw = ymax - ymin;
			double zw = zmax - zmin;

			return max(xw, max(yw, zw));
		}
		else {
			for (int i = 1; i <= GetSegmentCount(); ++i) {
				CTiglAbstractSegment& segment = GetSegment(i);
				TopoDS_Shape& segmentShape = segment.GetLoft();
				BRepBndLib::Add(segmentShape, boundingBox);
				TopoDS_Shape segmentMirroredShape = segment.GetMirroredLoft();
				BRepBndLib::Add(segmentMirroredShape, boundingBox);
			}

			Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
			boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

			switch (GetSymmetryAxis()){
			case TIGL_X_Y_PLANE:
				return zmax-zmin;
				break;
			case TIGL_X_Z_PLANE:
				return ymax-ymin;
				break;
			case TIGL_Y_Z_PLANE:
				return xmax-ymin;
				break;
			default:
				return ymax-ymin;
			}
		}
	}



} // end namespace tigl
