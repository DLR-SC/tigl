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

#include "tigl_config.h"

#include "CCPACSFuselage.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSConfiguration.h"
#include "tiglcommonfunctions.h"

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

#define _USE_MATH_DEFINES
#include <math.h>


namespace tigl
{

// Constructor
CCPACSFuselage::CCPACSFuselage(CCPACSConfiguration* config)
    : segments(this)
    , configuration(config)
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
    loft.reset();
    segments.Invalidate();
    positionings.Invalidate();
}

// Cleanup routine
void CCPACSFuselage::Cleanup(void)
{
    name = "";
    transformation.reset();

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
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
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName)==SUCCESS) {
        name = ptrName;
    }

    // Get attribue "uID"
    char* ptrUID = NULL;
    tempString   = "uID";
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(fuselageXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get subelement "parent_uid"
    char* ptrParentUID = NULL;
    tempString         = fuselageXPath + "/parentUID";
    elementPath        = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS  && 
        tixiGetTextElement(tixiHandle, elementPath, &ptrParentUID) == SUCCESS ) {

        SetParentUID(ptrParentUID);
    }

    transformation.ReadCPACS(tixiHandle, fuselageXPath);

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
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(fuselageXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS) {
        SetSymmetryAxis(ptrSym);
    }
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

// Returns the segment for a given index
CTiglAbstractSegment & CCPACSFuselage::GetSegment(const int index)
{
    return (CTiglAbstractSegment &) segments.GetSegment(index);
}

// Returns the segment for a given uid
CTiglAbstractSegment & CCPACSFuselage::GetSegment(std::string uid)
{
    return (CTiglAbstractSegment &) segments.GetSegment(uid);
}

// get short name for loft
std::string CCPACSFuselage::GetShortShapeName ()
{
    unsigned int findex = 0;
    for (int i = 1; i <= GetConfiguration().GetFuselageCount(); ++i) {
        tigl::CCPACSFuselage& f = GetConfiguration().GetFuselage(i);
        if (GetUID() == f.GetUID()) {
            findex = i;
            std::stringstream shortName;
            shortName << "F" << findex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

// Builds a fused shape of all fuselage segments
PNamedShape CCPACSFuselage::BuildLoft(void)
{
    // Get Continuity of first segment
    // TODO: adapt lofting to have multiple different continuities
    TiglContinuity cont = segments.GetSegment(1).GetContinuity();
    Standard_Boolean ruled = (cont == C0? true : false);


    // Ne need a smooth fuselage by default
    // @TODO: OpenCascade::ThruSections is currently buggy and crashes, if smooth lofting
    // is performed. Therefore we swicth the 2. parameter to Standard_True (non smooth lofting).
    // This has to be reverted, as soon as the bug is fixed!!!
    BRepOffsetAPI_ThruSections generator(Standard_True, ruled, Precision::Confusion() );

    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        generator.AddWire(segments.GetSegment(i).GetStartWire());
    }
    generator.AddWire(segments.GetSegment(segments.GetSegmentCount()).GetEndWire());
        
    generator.SetMaxDegree(2); //surfaces will be C1-continuous
    generator.SetParType(Approx_Centripetal);
    generator.CheckCompatibility(Standard_False);
    generator.Build();
    TopoDS_Shape loftShape =  generator.Shape();
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    return loft;
}


// Gets the fuselage transformation
CTiglTransformation CCPACSFuselage::GetFuselageTransformation(void)
{
    return transformation.getTransformationMatrix();
}

// Get the positioning transformation for a given section index
CTiglTransformation CCPACSFuselage::GetPositioningTransformation(const std::string &sectionUID)
{
    return positionings.GetPositioningTransformation(sectionUID);
}

// Gets a point on the given fuselage segment in dependence of a parameters eta and zeta with
// 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
// profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
// the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
gp_Pnt CCPACSFuselage::GetPoint(int segmentIndex, double eta, double zeta)
{
    return ((CCPACSFuselageSegment &) GetSegment(segmentIndex)).GetPoint(eta, zeta);
}


// Returns the volume of this fuselage
double CCPACSFuselage::GetVolume(void)
{
    const TopoDS_Shape& fusedSegments = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedSegments, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Get the Transformation object
CTiglTransformation CCPACSFuselage::GetTransformation(void)
{
    return GetFuselageTransformation();
}

// Returns the circumference of the segment "segmentIndex" at a given eta
double CCPACSFuselage::GetCircumference(const int segmentIndex, const double eta)
{
    return ((CCPACSFuselageSegment &) GetSegment(segmentIndex)).GetCircumference(eta);
}
    
// Returns the surface area of this fuselage
double CCPACSFuselage::GetSurfaceArea(void)
{
    const TopoDS_Shape& fusedSegments = GetLoft()->Shape();
    // Calculate surface area
    GProp_GProps System;
    BRepGProp::SurfaceProperties(fusedSegments, System);
    double myArea = System.Mass();
    return myArea;
}

// Returns the point where the distance between the selected fuselage and the ground is at minimum.
// The Fuselage could be turned with a given angle at at given axis, specified by a point and a direction.
gp_Pnt CCPACSFuselage::GetMinumumDistanceToGround(gp_Ax1 RAxis, double angle)
{

    TopoDS_Shape fusedFuselage = GetLoft()->Shape();

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
void CCPACSFuselage::SetSymmetryAxis(const std::string& axis)
{
    CTiglAbstractGeometricComponent::SetSymmetryAxis(axis);

    for (int i = 1; i <= segments.GetSegmentCount(); ++i) {
        CCPACSFuselageSegment& segment = segments.GetSegment(i);
        segment.SetSymmetryAxis(axis);
    }
}

// Get the guide curve with a given UID
CCPACSGuideCurve& CCPACSFuselage::GetGuideCurve(std::string uid)
{
    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        CCPACSFuselageSegment& segment = segments.GetSegment(i);
        if (segment.GuideCurveExists(uid)) {
            return segment.GetGuideCurve(uid);
        }
    }
    throw tigl::CTiglError("Error: Guide Curve with UID " + uid + " does not exists", TIGL_ERROR);
}

} // end namespace tigl
