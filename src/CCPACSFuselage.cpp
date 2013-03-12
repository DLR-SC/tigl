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
* @brief  Implementation of CPACS fuselage handling routines.
*/

#include <iostream>

#include "CCPACSFuselage.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSConfiguration.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRep_Tool.hxx"
#include "BRepGProp.hxx"
#include "BRepBuilderAPI_Transform.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "TopoDS_Edge.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "GC_MakeSegment.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "XCAFDoc_ShapeTool.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "TDataStd_Name.hxx"
#include "TDataXtd_Shape.hxx"

#define _USE_MATH_DEFINES
#include <math.h>


namespace tigl {

    // Constructor
    CCPACSFuselage::CCPACSFuselage(CCPACSConfiguration* config)
        : segments(this)
        , configuration(config)
        , rebuildFusedSegments(true)
    {
        Cleanup();
    }

    // Destructor
    CCPACSFuselage::~CCPACSFuselage(void)
    {
        Cleanup();
    }

    // Invalidates internal state
    void CCPACSFuselage::Invalidate(void)
    {
        invalidated = true;
        segments.Invalidate();
        positionings.Invalidate();
    }

    // Cleanup routine
    void CCPACSFuselage::Cleanup(void)
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

    // Builds transformation matrix for the fuselage
    void CCPACSFuselage::BuildMatrix(void)
    {
        transformation.SetIdentity();

        // Step 1: scale the fuselage around the orign
        transformation.AddScaling(scaling.x, scaling.y, scaling.z);

        // Step 2: rotate the fuselage
        // Step 2a: rotate the fuselage around z (yaw   += right tip forward)
        transformation.AddRotationZ(rotation.z);
        // Step 2b: rotate the fuselage around y (pitch += nose up)
        transformation.AddRotationY(rotation.y);
        // Step 2c: rotate the fuselage around x (roll  += right tip up)
        transformation.AddRotationX(rotation.x);

        // Step 3: translate the rotated fuselage into its position
        transformation.AddTranslation(translation.x, translation.y, translation.z);
    }

    // Update internal data
    void CCPACSFuselage::Update(void)
    {
        if (!invalidated)
            return;

        BuildMatrix();
        invalidated = false;
        rebuildFusedSegments = true;
    }

    // Read CPACS fuselage element
    void CCPACSFuselage::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath)
    {
        Cleanup();

        char*       elementPath;
        std::string tempString;

        // Get subelement "name"
        char* ptrName = NULL;
        tempString    = fuselageXPath + "/name";
        elementPath   = const_cast<char*>(tempString.c_str());
        if(tixiGetTextElement(tixiHandle, elementPath, &ptrName)==SUCCESS)
            name = ptrName;

        // Get attribue "uID"
		char* ptrUID = NULL;
        tempString   = "uID";
        elementPath  = const_cast<char*>(tempString.c_str());
        if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(fuselageXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS)
            SetUID(ptrUID);

        // Get subelement "parent_uid"
        char* ptrParentUID = NULL;
        tempString         = fuselageXPath + "/parentUID";
        elementPath        = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS  && 
            tixiGetTextElement(tixiHandle, elementPath, &ptrParentUID) == SUCCESS ) 
        {
            SetParentUID(ptrParentUID);
        }



        // Get subelement "/transformation/translation"
        tempString  = fuselageXPath + "/transformation/translation";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(translation.x), &(translation.y), &(translation.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <translation/> in CCPACSFuselage::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "/transformation/scaling"
        tempString  = fuselageXPath + "/transformation/scaling";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(scaling.x), &(scaling.y), &(scaling.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <scaling/> in CCPACSFuselage::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "/transformation/rotation"
        tempString  = fuselageXPath + "/transformation/rotation";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(rotation.x), &(rotation.y), &(rotation.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <rotation/> in CCPACSFuselage::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "sections"
        sections.ReadCPACS(tixiHandle, fuselageXPath);

        // Get subelement "positionings"
        positionings.ReadCPACS(tixiHandle, fuselageXPath);

        // Get subelement "segments"
        segments.ReadCPACS(tixiHandle, fuselageXPath);

        // Register ourself at the unique id manager
        configuration->GetUIDManager().AddUID(ptrUID, this);

        // Get symmetry axis attribute
        char* ptrSym = NULL;
        tempString   = "symmetry";
        elementPath  = const_cast<char*>(tempString.c_str());
        if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(fuselageXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS)
            SetSymmetryAxis(ptrSym);

        Update();
    }

    // Returns the name of the fuselage
    std::string CCPACSFuselage::GetName(void) const
    {
        return name;
    }

    // Returns the parent configuration
    CCPACSConfiguration& CCPACSFuselage::GetConfiguration(void) const
    {
        return *configuration;
    }

    // Get section count
    int CCPACSFuselage::GetSectionCount(void) const
    {
        return sections.GetSectionCount();
    }

    // Returns the section for a given index
    CCPACSFuselageSection& CCPACSFuselage::GetSection(int index) const
    {
        return sections.GetSection(index);
    }

    // Get segment count
    int CCPACSFuselage::GetSegmentCount(void) const
    {
        return segments.GetSegmentCount();
    }

    TDF_Label CCPACSFuselage::ExportDataStructure(Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label)
    {
        TDF_Label fuselageLabel = CTiglAbstractPhysicalComponent::ExportDataStructure(myAssembly, label);

//        // Export all segments
//        gp_Trsf t0;
//        TopLoc_Location location0(t0);
//        Handle(TDocStd_Document) doc;
//        Handle ( XCAFApp_Application ) anApp = XCAFApp_Application::GetApplication();
//        anApp->GetDocument(1, doc);
//        TDF_Label aLabel = myAssembly->AddShape(segments.GetSegment(12).GetLoft(), false);
//        TDataStd_Name::Set (aLabel, GetUID().c_str());
//        TDF_Label labelA02 = myAssembly->NewShape();
//        TDataStd_Name::Set(labelA02, "ASSEMBLY02");
//        TDF_Label component05 = myAssembly->AddComponent(labelA02, aLabel, location0);

        // All Segments

        // Other (sub)-components
        for (int i=1; i <= segments.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = segments.GetSegment(i);
            TDF_Label fuselageSegmentLabel = myAssembly->AddShape(segment.GetLoft(), false);
            TDataStd_Name::Set (fuselageSegmentLabel, segment.GetUID().c_str());
            //TDF_Label& subSegmentLabel = segment.ExportDataStructure(myAssembly, fuselageSegmentLabel);
        }

        return fuselageLabel;
    }

    // Returns the segment for a given index
    CTiglAbstractSegment & CCPACSFuselage::GetSegment(const int index)
    {
        return (CTiglAbstractSegment &) segments.GetSegment(index);
    }

    // Gets the loft of the whole fuselage.
    TopoDS_Shape& CCPACSFuselage::GetLoft(void)
    {
        if (rebuildFusedSegments) {
            BuildFusedSegments();
            // Transform by fuselage transformation
           fusedSegments = GetFuselageTransformation().Transform(fusedSegments);
        }
        rebuildFusedSegments = false;
        return fusedSegments;
    }

	// Builds a fused shape of all fuselage segments
	void CCPACSFuselage::BuildFusedSegments(void)
	{
		// Ne need a smooth fuselage by default
		// @TODO: OpenCascade::ThruSections is currently buggy and crashes, if smooth lofting
		// is performed. Therefore we swicth the 2. parameter to Standard_True (non smooth lofting).
		// This has to be reverted, as soon as the bug is fixed!!!
		BRepOffsetAPI_ThruSections generator(Standard_True, Standard_False, Precision::Confusion() );

		for (int i=1; i <= segments.GetSegmentCount(); i++) {
			CCPACSFuselageConnection& startConnection = segments.GetSegment(i).GetStartConnection();

			CCPACSFuselageProfile& startProfile = startConnection.GetProfile();

			TopoDS_Wire startWire = startProfile.GetWire(true);

			// Do section element transformations
			TopoDS_Shape startShape = startConnection.GetSectionElementTransformation().Transform(startWire);

			// Do section transformations
			startShape = startConnection.GetSectionTransformation().Transform(startShape);

			// Do positioning transformations (positioning of sections)
			startShape = startConnection.GetPositioningTransformation().Transform(startShape);

			// Cast shapes to wires, see OpenCascade documentation
			if (startShape.ShapeType() != TopAbs_WIRE) {
				throw CTiglError("Error: Wrong shape type in CCPACSFuselage::BuildFusedSegments", TIGL_ERROR);
			}
			startWire = TopoDS::Wire(startShape);

			generator.AddWire(startWire);
		}

		CCPACSFuselageConnection& endConnection = segments.GetSegment(segments.GetSegmentCount()).GetEndConnection();
		CCPACSFuselageProfile& endProfile = endConnection.GetProfile();
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


    // Gets the fuselage transformation
    CTiglTransformation CCPACSFuselage::GetFuselageTransformation(void)
    {
        Update();
        return transformation;
    }

    // Get the positioning transformation for a given section index
    CTiglTransformation CCPACSFuselage::GetPositioningTransformation(std::string index)
    {
        return positionings.GetPositioningTransformation(index);
    }

    // Gets a point on the given fuselage segment in dependence of a parameters eta and zeta with
    // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
    // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
    // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
    gp_Pnt CCPACSFuselage::GetPoint(int segmentIndex, double eta, double zeta)
    {
        gp_Pnt pnt = ((CCPACSFuselageSegment &) GetSegment(segmentIndex)).GetPoint(eta, zeta);
        return GetFuselageTransformation().Transform(pnt);
    }


    // Returns the volume of this fuselage
    double CCPACSFuselage::GetVolume(void)
    {
        double myVolume = 0.0;
        GetLoft();

        // Calculate volume
        GProp_GProps System;
        BRepGProp::VolumeProperties(fusedSegments, System);
        myVolume = System.Mass();
        return myVolume;
    }

    // Get the Transformation object
    CTiglTransformation CCPACSFuselage::GetTransformation(void)
    {
        return GetFuselageTransformation();
    }

	// Sets the Transformation object
    void CCPACSFuselage::Translate(CTiglPoint trans)
    {
        CTiglAbstractGeometricComponent::Translate(trans);
    	invalidated = true;
		Update();
    }

    // Returns the circumference of the segment "segmentIndex" at a given eta
    double CCPACSFuselage::GetCircumference(const int segmentIndex, const double eta)
    {
        return ((CCPACSFuselageSegment &) GetSegment(segmentIndex)).GetCircumference(eta);
    }
	
	// Returns the surface area of this fuselage
	double CCPACSFuselage::GetSurfaceArea(void)
    {
        double myArea = 0.0;
        GetLoft();
        // Calculate surface area
        GProp_GProps System;
		BRepGProp::SurfaceProperties(fusedSegments, System);
        myArea = System.Mass();
        return myArea;
    }

	// Returns the point where the distance between the selected fuselage and the ground is at minimum.
	// The Fuselage could be turned with a given angle at at given axis, specified by a point and a direction.
	gp_Pnt CCPACSFuselage::GetMinumumDistanceToGround(gp_Ax1 RAxis, double angle)
	{

		TopoDS_Shape fusedFuselage = GetLoft();

		// now rotate the fuselage
		gp_Trsf myTrsf;
		myTrsf.SetRotation(RAxis, angle * M_PI / 180.);
		BRepBuilderAPI_Transform xform(fusedFuselage, myTrsf);
		fusedFuselage = xform.Shape();

		// build cutting plane for intersection
		// We move the "ground" to "-1000" to be sure it is _under_ the fuselage
		gp_Pnt p1(-1.0e7, -1.0e7, -1000);
		gp_Pnt p2( 1.0e7, -1.0e7, -1000);
		gp_Pnt p3( 1.0e7,  1.0e7, -1000);
		gp_Pnt p4(-1.0e7,  1.0e7, -1000);

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

		// calculate extrema
		BRepExtrema_DistShapeShape extrema(fusedFuselage, shaft_face);
		extrema.Perform();

		return extrema.PointOnShape1(1);
	}

	// sets the symmetry plane for all childs, segments and component segments
	void CCPACSFuselage::SetSymmetryAxis(const std::string& axis){
		CTiglAbstractGeometricComponent::SetSymmetryAxis(axis);

		for(int i = 1; i <= segments.GetSegmentCount(); ++i){
			CCPACSFuselageSegment& segment = segments.GetSegment(i);
			segment.SetSymmetryAxis(axis);
		}
	}

} // end namespace tigl
