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
#include <cmath>
#include <iostream>

#include "tigl_config.h"

#include "CCPACSFuselage.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWingSegment.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools.hxx"
#include "BRepGProp.hxx"
#include "BRepBuilderAPI_Transform.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "TopoDS_Edge.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "GC_MakeSegment.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "ShapeFix_Wire.hxx"
#include "CTiglMakeLoft.h"
#include "TopExp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"

namespace tigl
{

CCPACSFuselage::CCPACSFuselage(CCPACSFuselages* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselage(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry) {
    Cleanup();
    if (parent->IsParent<CCPACSAircraftModel>())
        configuration = &parent->GetParent<CCPACSAircraftModel>()->GetConfiguration();
    else
        configuration = &parent->GetParent<CCPACSRotorcraftModel>()->GetConfiguration();
}

// Destructor
CCPACSFuselage::~CCPACSFuselage()
{
    Cleanup();
}

// Invalidates internal state
void CCPACSFuselage::Invalidate()
{
    loft.reset();
    m_segments.Invalidate();
    if (m_positionings)
        m_positionings->Invalidate();
}

// Cleanup routine
void CCPACSFuselage::Cleanup()
{
    m_name = "";

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Read CPACS fuselage element
void CCPACSFuselage::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath)
{
    Cleanup();

    generated::CPACSFuselage::ReadCPACS(tixiHandle, fuselageXPath);

    ConnectGuideCurveSegments();

    // Register ourself at the unique id manager
    configuration->GetUIDManager().AddGeometricComponent(m_uID, this);
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSFuselage::GetConfiguration() const
{
    return *configuration;
}

std::string CCPACSFuselage::GetDefaultedUID() const
{
    return generated::CPACSFuselage::GetUID();
}

// Get section count
int CCPACSFuselage::GetSectionCount() const
{
    return m_sections.GetSectionCount();
}

// Returns the section for a given index
CCPACSFuselageSection& CCPACSFuselage::GetSection(int index) const
{
    return m_sections.GetSection(index);
}

// Get segment count
int CCPACSFuselage::GetSegmentCount() const
{
    return m_segments.GetSegmentCount();
}

// Returns the segment for a given index
CCPACSFuselageSegment& CCPACSFuselage::GetSegment(const int index)
{
    return m_segments.GetSegment(index);
}

// Returns the segment for a given uid
CCPACSFuselageSegment& CCPACSFuselage::GetSegment(std::string uid)
{
    return m_segments.GetSegment(uid);
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

void CCPACSFuselage::SetFaceTraits (PNamedShape loft, bool hasSymmetryPlane, bool smoothSurface)
{
    // TODO: Face traits with guides must be made
    // this is currently only valid without guides

    int nFaces = GetNumberOfFaces(loft->Shape());

    std::vector<std::string> names;
    names.push_back(loft->Name());
    names.push_back("symmetry");
    names.push_back("Front");
    names.push_back("Rear");

    // if we have a smooth surface, the whole fuslage is treatet as one segment
    int nSegments = smoothSurface ? 1 : this->GetSegmentCount();

    int facesPerSegment = hasSymmetryPlane ? 2 : 1;
    int remainingFaces = nFaces - facesPerSegment * nSegments;
    if (remainingFaces < 0 || remainingFaces > 2) {
        LOG(WARNING) << "Fuselage faces cannot be names properly (maybe due to Guide Curves?)";
        return;
    }

    int iFaceTotal = 0;
    for (int iSegment = 0; iSegment < nSegments; ++iSegment) {
        for (int iFace = 0; iFace < facesPerSegment; ++iFace) {
            loft->FaceTraits(iFaceTotal++).SetName(names[iFace].c_str());
        }
    }

    // set the caps
    int iFace = 2;
    for (;iFaceTotal < nFaces; ++iFaceTotal) {
        loft->FaceTraits(iFaceTotal).SetName(names[iFace++].c_str());
    }
}

// Builds a fused shape of all fuselage segments
PNamedShape CCPACSFuselage::BuildLoft()
{
    // Get Continuity of first segment
    // TODO: adapt lofting to have multiple different continuities

    TiglContinuity cont = m_segments.GetSegment(1).GetContinuity();
    Standard_Boolean smooth = (cont == ::C0? false : true);

    CTiglMakeLoft lofter;
    // add profiles
    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        lofter.addProfiles(m_segments.GetSegment(i).GetStartWire());
    }
    lofter.addProfiles(m_segments.GetSegment(m_segments.GetSegmentCount()).GetEndWire());

    // add guides
    lofter.addGuides(GetGuideCurveWires());

    lofter.setMakeSmooth(smooth);
    lofter.setMakeSolid(true);

    TopoDS_Shape loftShape =  lofter.Shape();

    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));

    bool hasSymmetryPlane = GetNumberOfEdges(m_segments.GetSegment(1).GetEndWire()) > 1;

    SetFaceTraits(loft, hasSymmetryPlane, smooth);

    return loft;
}

// Get the positioning transformation for a given section index
boost::optional<CTiglTransformation> CCPACSFuselage::GetPositioningTransformation(const std::string &sectionUID)
{
    boost::optional<CTiglTransformation> ret;
    if (m_positionings)
        ret = m_positionings->GetPositioningTransformation(sectionUID);
    return ret;
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
double CCPACSFuselage::GetVolume()
{
    const TopoDS_Shape& fusedSegments = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedSegments, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Returns the circumference of the segment "segmentIndex" at a given eta
double CCPACSFuselage::GetCircumference(const int segmentIndex, const double eta)
{
    return static_cast<CCPACSFuselageSegment&>(GetSegment(segmentIndex)).GetCircumference(eta);
}
    
// Returns the surface area of this fuselage
double CCPACSFuselage::GetSurfaceArea()
{
    const PNamedShape& fusedSegments = GetLoft();

    // loop over all faces that are not symmetry, front or rear
    double myArea = 0.;

    TopTools_IndexedMapOfShape shapeMap;
    TopExp::MapShapes(fusedSegments->Shape(), TopAbs_FACE, shapeMap);
    for (int i = 1; i <= shapeMap.Extent(); ++i) {
        if (GetUID() == fusedSegments->GetFaceTraits(i-1).Name()) {
            const TopoDS_Face& f = TopoDS::Face(shapeMap(i));
            GProp_GProps System;
            BRepGProp::SurfaceProperties(f, System);
            myArea += System.Mass();
        }
    }

    // Calculate surface area
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

// Get the guide curve with a given UID
CCPACSGuideCurve& CCPACSFuselage::GetGuideCurveSegment(std::string uid)
{
    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        CCPACSFuselageSegment& segment = m_segments.GetSegment(i);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        if (segment.GetGuideCurves()->GuideCurveExists(uid)) {
            return segment.GetGuideCurves()->GetGuideCurve(uid);
        }
    }
    throw tigl::CTiglError("Guide Curve with UID " + uid + " does not exists", TIGL_ERROR);
}

TopoDS_Compound &CCPACSFuselage::GetGuideCurveWires()
{
    BuildGuideCurves();
    return guideCurves;
}

void CCPACSFuselage::BuildGuideCurves()
{
    if (!guideCurves.IsNull()) {
        return;
    }
    
    guideCurves.Nullify();
    BRep_Builder b;
    b.MakeCompound(guideCurves);
    std::multimap<double, CCPACSGuideCurve*> roots;
    
    // find roots and connect the belonging guide curve segments
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        CCPACSFuselageSegment& segment = m_segments.GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        CCPACSGuideCurves& segmentCurves = *segment.GetGuideCurves();
        for (int iguide = 1; iguide <=  segmentCurves.GetGuideCurveCount(); ++iguide) {
            CCPACSGuideCurve& curve = segmentCurves.GetGuideCurve(iguide);
            if (!curve.GetFromGuideCurveUID_choice1()) {
                // this is a root curve
                double relCirc= *curve.GetFromRelativeCircumference_choice2();
                //TODO: determine if half fuselage or not. If not
                //the guide curve at relCirc=1 should be inserted at relCirc=0
                roots.insert(std::make_pair(relCirc, &curve));
            }
            else {
                CCPACSGuideCurve& fromCurve = GetGuideCurveSegment(*curve.GetFromGuideCurveUID_choice1());
                fromCurve.ConnectToCurve(&curve);
            }
        }
    }
    
    // connect belonging guide curves to wires
    std::multimap<double, CCPACSGuideCurve*>::iterator it;
    for (it = roots.begin(); it != roots.end(); ++it) {
        CCPACSGuideCurve* curCurve = it->second;
        BRepBuilderAPI_MakeWire wireMaker;
        while (curCurve) {
            const TopoDS_Edge& edge = curCurve->GetCurve();
            wireMaker.Add(edge);
            curCurve = curCurve->GetConnectedCurve();
        }
        TopoDS_Wire result = wireMaker.Wire();
        // Fix Shape, might be necessary since the order of edges could be wrong
        ShapeFix_Wire wireFixer;
        wireFixer.Load(result);
        wireFixer.FixReorder();
        wireFixer.Perform();
        result = wireFixer.Wire();
        b.Add(guideCurves, result);
    }
}

void CCPACSFuselage::ConnectGuideCurveSegments(void)
{
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        CCPACSFuselageSegment& segment = GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        CCPACSGuideCurves& curves = *segment.GetGuideCurves();
        for (int icurve = 1; icurve <= curves.GetGuideCurveCount(); ++icurve) {
            CCPACSGuideCurve& curve = curves.GetGuideCurve(icurve);
            if (!curve.GetFromRelativeCircumference_choice2()) {
                std::string fromUID = *curve.GetFromGuideCurveUID_choice1();
                CCPACSGuideCurve& fromCurve = GetGuideCurveSegment(fromUID);
                curve.SetFromRelativeCircumference_choice2(fromCurve.GetToRelativeCircumference());
            }
        }
    }
}

TopoDS_Shape transformFuselageProfileGeometry(const CTiglTransformation& fuselTransform, const CTiglFuselageConnection& connection, const TopoDS_Shape& shape)
{
    // Do section element transformation on points
    tigl::CTiglTransformation trafo = connection.GetSectionElementTransformation();

    // Do section transformations
    trafo.PreMultiply(connection.GetSectionTransformation());

    // Do positioning transformations
    boost::optional<CTiglTransformation> posTrans = connection.GetPositioningTransformation();
    if (posTrans) {
        trafo.PreMultiply(*posTrans);
    }

    trafo.PreMultiply(fuselTransform);

    return trafo.Transform(shape);

}

} // end namespace tigl
