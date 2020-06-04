/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
#include <algorithm>
#include <map>

#include "generated/CPACSRotorBlades.h"
#include "CCPACSWing.h"
#include "CCPACSWingSection.h"
#include "CTiglWingBuilder.h"
#include "CCPACSConfiguration.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWings.h"
#include "CCPACSAircraftModel.h"
#include "CCPACSRotorcraftModel.h"
#include "CTiglError.h"
#include "tiglcommonfunctions.h"
#include "tiglmathfunctions.h"
#include "CNamedShape.h"
#include "CTiglCurveConnector.h"
#include "CTiglBSplineAlgorithms.h"
#include "CCPACSControlSurfaces.h"
#include "CCPACSTrailingEdgeDevice.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepTools.hxx"
#include "ShapeFix_Wire.hxx"
#include "CTiglMakeLoft.h"
#include "CCutShape.h"
#include "CFuseShapes.h"
#include "CGroupShapes.h"
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS.hxx>


namespace tigl
{

namespace
{
    inline double max(double a, double b)
    {
        return a > b? a : b;
    }

    inline double min(double a, double b)
    {
        return a < b? a : b;
    }

    // Returns the index of the maximum value
    int maxIndex(double x, double y, double z)
    {
        if (x >= y && x >= z) {
            return 0;
        }
        else if (y >= x && y >= z) {
            return 1;
        }
        else {
            return 2;
        }
    }

    size_t NumberOfControlSurfaces(const CCPACSWing& wing);
}

CCPACSWing::CCPACSWing(CCPACSWings* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWing(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
    , guideCurves(*this, &CCPACSWing::BuildGuideCurveWires)
    , wingShapeWithCutouts(*this, &CCPACSWing::BuildWingWithCutouts)
    , wingCleanShape(*this, &CCPACSWing::BuildFusedSegments)
    , rebuildFusedSegWEdge(true)
    , rebuildShells(true)
    , buildFlaps(false)
{
    if (parent->IsParent<CCPACSAircraftModel>())
        configuration = &parent->GetParent<CCPACSAircraftModel>()->GetConfiguration();
    else if (parent->IsParent<CCPACSRotorcraftModel>())
        configuration = &parent->GetParent<CCPACSRotorcraftModel>()->GetConfiguration();
    else
        throw CTiglError("Unknown parent");

    Cleanup();
}
CCPACSWing::CCPACSWing(CCPACSRotorBlades* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWing(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
    , configuration(&parent->GetConfiguration())
    , guideCurves(*this, &CCPACSWing::BuildGuideCurveWires)
    , wingShapeWithCutouts(*this, &CCPACSWing::BuildWingWithCutouts)
    , wingCleanShape(*this, &CCPACSWing::BuildFusedSegments)
    , rebuildFusedSegWEdge(true)
    , rebuildShells(true)
    , buildFlaps(false)
{
    Cleanup();
}

// Destructor
CCPACSWing::~CCPACSWing()
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWing::InvalidateImpl(const boost::optional<std::string>& source) const
{
    // TODO(rlandert): replace by caches
    rebuildShells = true;
    rebuildFusedSegWEdge = true;

    Reset();
    loft.clear();
    guideCurves.clear();
    wingCleanShape.clear();
    wingShapeWithCutouts.clear();

    // Invalidate segments, since these get their shapes from the wing
    m_segments.Invalidate(GetUID());
    // Invalidate component segments, since these use the wing loft geometry
    if (m_componentSegments) {
        m_componentSegments->Invalidate(GetUID());
    }
}

// Cleanup routine
void CCPACSWing::Cleanup()
{
    m_name = "";
    m_description = boost::none;
    isRotorBlade = false;
    m_transformation.reset();

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

void CCPACSWing::ConnectGuideCurveSegments(void)
{
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        CCPACSWingSegment& segment = GetSegment(isegment);

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
                //TODO: also call curve->connect already
            }
        }
    }
}

// Update internal wing data
void CCPACSWing::Update()
{
}

// Read CPACS wing element
void CCPACSWing::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& wingXPath)
{
    Cleanup();
    generated::CPACSWing::ReadCPACS(tixiHandle, wingXPath);

    if (wingXPath.find("rotorBlade") != std::string::npos) {
        isRotorBlade = true;

        // WORKAROUND
        // The rotor blade is attached implicitly to the rotor and should not have an additional parent
        // See issue #509
        if (m_parentUID) {
            LOG(WARNING) << "Parent of rotor blade '" << GetUID() << "' is removed since it will be parented by a rotor. Consider to remove the parentUID node.";
            m_parentUID = boost::none;
        }
    }

    ConnectGuideCurveSegments();

    Update();
}

std::string CCPACSWing::GetDefaultedUID() const {
    return generated::CPACSWing::GetUID();
}

void CCPACSWing::SetSymmetryAxis(const TiglSymmetryAxis& axis)
{
    CTiglRelativelyPositionedComponent::SetSymmetryAxis(axis);
    Invalidate();
}

void CCPACSWing::SetParentUID(const boost::optional<std::string>& value)
{
    generated::CPACSWing::SetParentUID(value);
    // invalidate in case parent transformation is relevant
    if (GetTranslationType() == ABS_LOCAL) {
        Invalidate();
    }
}

// Returns whether this wing is a rotor blade
bool CCPACSWing::IsRotorBlade() const
{
    return isRotorBlade;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSWing::GetConfiguration() const
{
    return *configuration;
}

// Get section count
int CCPACSWing::GetSectionCount() const
{
    return static_cast<int>(m_sections.GetSections().size());
}

// Returns the section for a given index
CCPACSWingSection& CCPACSWing::GetSection(int index)
{
    return m_sections.GetSection(index);
}
const CCPACSWingSection& CCPACSWing::GetSection(int index) const
{
    return m_sections.GetSection(index);
}

// Get segment count
int CCPACSWing::GetSegmentCount() const
{
    return m_segments.GetSegmentCount();
}

// Returns the segment for a given index
CCPACSWingSegment& CCPACSWing::GetSegment(const int index)
{
    return m_segments.GetSegment(index);
}

const CCPACSWingSegment& CCPACSWing::GetSegment(const int index) const
{
    return m_segments.GetSegment(index);
}

// Returns the segment for a given uid
CCPACSWingSegment& CCPACSWing::GetSegment(std::string uid)
{
    return m_segments.GetSegment(uid);
}

const CCPACSWingSegment& CCPACSWing::GetSegment(std::string uid) const
{
    return m_segments.GetSegment(uid);
}

// Get componentSegment count
int CCPACSWing::GetComponentSegmentCount() const
{
    if (m_componentSegments)
        return m_componentSegments->GetComponentSegmentCount();
    else
        return 0;
}

// Returns the segment for a given index
CCPACSWingComponentSegment& CCPACSWing::GetComponentSegment(const int index)
{
    return m_componentSegments->GetComponentSegment(index);
}

const CCPACSWingComponentSegment& CCPACSWing::GetComponentSegment(const int index) const {
    return m_componentSegments->GetComponentSegment(index);
}

// Returns the segment for a given uid
CCPACSWingComponentSegment& CCPACSWing::GetComponentSegment(const std::string& uid)
{
    return m_componentSegments->GetComponentSegment(uid);
}

// Gets the loft of the whole wing with modeled leading edge.
TopoDS_Shape & CCPACSWing::GetLoftWithLeadingEdge()
{
    if (rebuildFusedSegWEdge) {
        fusedSegmentWithEdge = (*wingCleanShape)->Shape();
    }
    rebuildFusedSegWEdge = false;
    return fusedSegmentWithEdge;
}
    
// Gets the loft of the whole wing.
TopoDS_Shape & CCPACSWing::GetUpperShape()
{
    if (rebuildShells) {
        BuildUpperLowerShells();
    }
    rebuildShells = false;
    return upperShape;
}
    
// Gets the loft of the whole wing.
TopoDS_Shape & CCPACSWing::GetLowerShape()
{
    if (rebuildShells) {
        BuildUpperLowerShells();
    }
    rebuildShells = false;
    return lowerShape;
}
    

// get short name for loft
std::string CCPACSWing::GetShortShapeName() const
{
    unsigned int windex = 0;
    for (int i = 1; i <= GetConfiguration().GetWingCount(); ++i) {
        const CCPACSWing& w = GetConfiguration().GetWing(i);
        if (GetUID() == w.GetUID()) {
            windex = i;
            std::stringstream shortName;
            shortName << "W" << windex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

// build loft
PNamedShape CCPACSWing::BuildLoft() const
{
    if (buildFlaps) {
        return GroupedFlapsAndWingShapes();
    } else {
        return *wingCleanShape;
    }
}

TopoDS_Shape CCPACSWing::GetLoftWithCutouts()
{
    return (*wingShapeWithCutouts)->Shape();
}

// Builds a fused shape of all wing segments
void CCPACSWing::BuildFusedSegments(PNamedShape& shape) const
{
    shape = CTiglWingBuilder(*this);
}
    
// Builds a fused shape of all wing segments
void CCPACSWing::BuildUpperLowerShells()
{
    //@todo: this probably works only if the wings does not split somewere
    BRepOffsetAPI_ThruSections generatorUp(Standard_False, Standard_True, Precision::Confusion() );
    BRepOffsetAPI_ThruSections generatorLow(Standard_False, Standard_True, Precision::Confusion() );

    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        CTiglWingConnection& startConnection = m_segments.GetSegment(i).GetInnerConnection();
        CCPACSWingProfile& startProfile = startConnection.GetProfile();
        TopoDS_Wire upperWire, lowerWire;
        upperWire = TopoDS::Wire(transformWingProfileGeometry(GetTransformationMatrix(), startConnection, BRepBuilderAPI_MakeWire(startProfile.GetUpperWire())));
        lowerWire = TopoDS::Wire(transformWingProfileGeometry(GetTransformationMatrix(), startConnection, BRepBuilderAPI_MakeWire(startProfile.GetLowerWire())));
        generatorUp.AddWire(upperWire);
        generatorLow.AddWire(lowerWire);
    }

    CTiglWingConnection& endConnection = m_segments.GetSegment(m_segments.GetSegmentCount()).GetOuterConnection();
    CCPACSWingProfile& endProfile = endConnection.GetProfile();
    TopoDS_Wire endUpWire, endLowWire;

    endUpWire  = TopoDS::Wire(transformWingProfileGeometry(GetTransformationMatrix(), endConnection, BRepBuilderAPI_MakeWire(endProfile.GetUpperWire())));
    endLowWire = TopoDS::Wire(transformWingProfileGeometry(GetTransformationMatrix(), endConnection, BRepBuilderAPI_MakeWire(endProfile.GetLowerWire())));

    generatorUp.AddWire(endUpWire);
    generatorLow.AddWire(endLowWire);
    generatorLow.Build();
    generatorUp.Build();
    upperShape = generatorUp.Shape();
    lowerShape = generatorLow.Shape();
}


void CCPACSWing::BuildWingWithCutouts(PNamedShape& result) const
{


    if (NumberOfControlSurfaces(*this) == 0) {
        return;
    }


    TopoDS_Compound allFlapPrisms;
    BRep_Builder compoundBuilderFlaps;
    compoundBuilderFlaps.MakeCompound (allFlapPrisms);

    PNamedShape fusedBoxes;
    bool first = true;
    for ( int i = 1; i <= GetComponentSegmentCount(); i++ ) {

       const CCPACSWingComponentSegment &componentSegment = GetComponentSegment(i);
       if (!componentSegment.GetControlSurfaces().is_initialized()) continue;

       const CCPACSControlSurfaces& controlSurfs = componentSegment.GetControlSurfaces().value();
       if (!controlSurfs.GetTrailingEdgeDevices().is_initialized()) continue;
       const CCPACSTrailingEdgeDevices& controlSurfaceDevices = controlSurfs.GetTrailingEdgeDevices().value();

        for ( size_t j = controlSurfaceDevices.GetTrailingEdgeDevices().size(); j > 0 ; j-- ) {
            CCPACSTrailingEdgeDevice& controlSurfaceDevice = *controlSurfaceDevices.GetTrailingEdgeDevices().at(j-1);

            PNamedShape controlSurfacePrism = controlSurfaceDevice.GetCutOutShape();
            if (controlSurfaceDevice.GetType() != SPOILER) {
                if (!first) {
                    ListPNamedShape childs;
                    childs.push_back(controlSurfacePrism);
                    fusedBoxes = CFuseShapes(fusedBoxes, childs);
                }
                else {
                    first = false;
                    fusedBoxes = controlSurfacePrism;
                }
            }

            // trigger build of the flap
            controlSurfaceDevice.GetLoft();
        }
    }

    CCutShape cutter(*wingCleanShape, fusedBoxes);
    cutter.Perform();
    result = cutter.NamedShape();
    for (int iFace = 0; iFace < static_cast<int>(result->GetFaceCount()); ++iFace) {
        CFaceTraits ft = result->GetFaceTraits(iFace);
        ft.SetOrigin(*wingCleanShape);
        result->SetFaceTraits(iFace, ft);
    }
}

// Builds a fuse shape of all wing segments with flaps
PNamedShape CCPACSWing::GroupedFlapsAndWingShapes() const
{
    // check whether there are control surfaces
    if (!GetComponentSegments() || NumberOfControlSurfaces(*this) == 0) {
        return PNamedShape();
    }

    ListPNamedShape flapsAndWingShapes;

    for (const auto& componentSegment : GetComponentSegments()->GetComponentSegments()) {

       if (!componentSegment->GetControlSurfaces().is_initialized()) continue;

       const auto& controlSurfs = componentSegment->GetControlSurfaces().value();
       if (!controlSurfs.GetTrailingEdgeDevices().is_initialized()) continue;

       const auto& controlSurfaceDevices = controlSurfs.GetTrailingEdgeDevices().value();
       const auto& devices = controlSurfaceDevices.GetTrailingEdgeDevices();
       for ( size_t j = devices.size(); j > 0 ; j-- ) {

            const auto& controlSurfaceDevice = *devices.at(j-1);
            auto deviceShape = controlSurfaceDevice.GetTransformedFlapShape();
            flapsAndWingShapes.push_back(deviceShape);
       }
    }

    flapsAndWingShapes.push_back(*wingShapeWithCutouts);
    auto loft = CGroupShapes(flapsAndWingShapes);

    return loft;
}

// Get the positioning transformation for a given section-uid
CTiglTransformation CCPACSWing::GetPositioningTransformation(std::string sectionUID)
{
    if (m_positionings)
        return m_positionings->GetPositioningTransformation(sectionUID);
    else
        return CTiglTransformation(); // return identity if no positioning transformation is given
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

// Gets a point on the chord surface in absolute (world) coordinates for a given segment, eta, xsi
gp_Pnt CCPACSWing::GetChordPoint(int segmentIndex, double eta, double xsi, TiglCoordinateSystem referenceCS)
{
    return  ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetChordPoint(eta, xsi, referenceCS);
}

// Returns the volume of this wing
double CCPACSWing::GetVolume()
{
    const TopoDS_Shape fusedSegments = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedSegments, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Returns the surface area of this wing
double CCPACSWing::GetSurfaceArea()
{
    const TopoDS_Shape fusedSegments = GetLoft()->Shape();

    // Calculate surface area
    GProp_GProps System;
    BRepGProp::SurfaceProperties(fusedSegments, System);
    double myArea = System.Mass();
    return myArea;
}

// Returns the reference area of the wing by taking account the quadrilateral portions
// of each wing segment by projecting the wing segments into the plane defined by the user
double CCPACSWing::GetReferenceArea(TiglSymmetryAxis symPlane)
{
    double refArea = 0.0;

    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        refArea += m_segments.GetSegment(i).GetReferenceArea(symPlane);
    }
    return refArea;
}


double CCPACSWing::GetWettedArea(TopoDS_Shape parent)
{
    const TopoDS_Shape loft = GetLoft()->Shape();

    TopoDS_Shape wettedLoft = BRepAlgoAPI_Cut(loft, parent); 

    GProp_GProps System;
    BRepGProp::SurfaceProperties(wettedLoft, System);
    double wetArea = System.Mass();
    return wetArea;
}

    
// Returns the lower Surface of a Segment
Handle(Geom_Surface) CCPACSWing::GetLowerSegmentSurface(int index)
{
    return m_segments.GetSegment(index).GetLowerSurface();
}

// Returns the upper Surface of a Segment
Handle(Geom_Surface) CCPACSWing::GetUpperSegmentSurface(int index)
{
    return m_segments.GetSegment(index).GetUpperSurface();
}

double CCPACSWing::GetWingspan()
{
    Bnd_Box boundingBox;
    if (GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
        // As we have no symmetry information
        // we have to find out the major direction
        // of the wing.
        // This is not so trivial, as e.g. the VTP can
        // be longer in depth than the actual span.
        // Boxwings have to be treated as well.
        // Here, we apply a heuristic that finds out
        // The major depth direction and the major
        // spanning direction. The depth direction
        // is then discarded in the span evaluation.

        gp_XYZ cumulatedSpanDirection(0, 0, 0);
        gp_XYZ cumulatedDepthDirection(0, 0, 0);
        for (int i = 1; i <= GetSegmentCount(); ++i) {
            CCPACSWingSegment& segment = m_segments.GetSegment(i);
            const TopoDS_Shape segmentShape = segment.GetLoft()->Shape();
            BRepBndLib::Add(segmentShape, boundingBox);

            gp_XYZ dirSpan  = segment.GetChordPoint(1,0).XYZ() - segment.GetChordPoint(0,0).XYZ();
            gp_XYZ dirDepth = segment.GetChordPoint(0,1).XYZ() - segment.GetChordPoint(0,0).XYZ();
            dirSpan  = gp_XYZ(fabs(dirSpan.X()), fabs(dirSpan.Y()), fabs(dirSpan.Z()));
            dirDepth = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
            cumulatedSpanDirection += dirSpan;
            cumulatedDepthDirection += dirDepth;
        }
        CCPACSWingSegment& outerSegment = m_segments.GetSegment(GetSegmentCount());
        gp_XYZ dirDepth = outerSegment.GetChordPoint(1,1).XYZ() - outerSegment.GetChordPoint(1,0).XYZ();
        dirDepth = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
        cumulatedDepthDirection += dirDepth;
        
        int depthIndex = maxIndex(cumulatedDepthDirection.X(),
                                  cumulatedDepthDirection.Y(),
                                  cumulatedDepthDirection.Z());

        // Get the extension of the wing in all
        // directions of the world coordinate system 
        Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
        boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        double xw = xmax - xmin;
        double yw = ymax - ymin;
        double zw = zmax - zmin;

        // The direction of depth should not be included in the span evaluation
        switch (depthIndex) {
        default:
        case 0:
            return cumulatedSpanDirection.Y() >= cumulatedSpanDirection.Z() ? yw : zw;
        case 1:
            return cumulatedSpanDirection.X() >= cumulatedSpanDirection.Z() ? xw : zw;
        case 2:
            return cumulatedSpanDirection.X() >= cumulatedSpanDirection.Y() ? xw : yw;
        }
    }
    else {
        for (int i = 1; i <= GetSegmentCount(); ++i) {
            CCPACSWingSegment& segment = GetSegment(i);
            TopoDS_Shape segmentShape = segment.GetLoft()->Shape();
            BRepBndLib::Add(segmentShape, boundingBox);
            TopoDS_Shape segmentMirroredShape = segment.GetMirroredLoft()->Shape();
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
            return xmax-xmin;
            break;
        default:
            return ymax-ymin;
        }
    }
}

// Returns the aspect ratio of a wing: AR=b**2/A=((2s)**2)/(2A_half)
//     b: full span; A: Reference area of full wing (wing + symmetrical wing)
//     s: half span; A_half: Reference area of wing without symmetrical wing
double CCPACSWing::GetAspectRatio()
{
    return 2.0*(pow_int(GetWingspan(),2)/GetReferenceArea(GetSymmetryAxis()));
}

/**
    * This function calculates location of the quarter of mean aerodynamic chord,
    * and gives the chord lenght as well. It uses the classical method that can
    * be applied to trapozaidal wings. This method is used for each segment.
    * The values are found by taking into account of sweep and dihedral.
    * But the effect of insidance angle is neglected. These values should coincide
    * with the values found with tornado tool.
    */
void  CCPACSWing::GetWingMAC(double& mac_chord, double& mac_x, double& mac_y, double& mac_z)
{
    double A_sum = 0.;
    double cc_mac_sum=0.;
    gp_XYZ cc_mac_sum_p(0., 0., 0.);

    for (int i = 1; i <= m_segments.GetSegmentCount(); ++i) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&) GetSegment(i);
        gp_Pnt innerLeadingPoint   = segment.GetChordPoint(0, 0.);
        gp_Pnt innerTrailingPoint  = segment.GetChordPoint(0, 1.);
        gp_Pnt outerLeadingPoint   = segment.GetChordPoint(1, 0.);
        gp_Pnt innterTrailingPoint = segment.GetChordPoint(1, 1.);

        double distance  = innerLeadingPoint.Distance(innerTrailingPoint);
        double distance2 = outerLeadingPoint.Distance(innterTrailingPoint);

        // points projected to the x == 0 plane
        gp_Pnt point1= gp_Pnt(0.0, innerLeadingPoint.Y(), innerLeadingPoint.Z());
        gp_Pnt point2= gp_Pnt(0.0, outerLeadingPoint.Y(), outerLeadingPoint.Z());
        gp_Pnt point3= gp_Pnt(0.0, innerTrailingPoint.Y(), innerTrailingPoint.Z());
        gp_Pnt point4= gp_Pnt(0.0, innterTrailingPoint.Y(), innterTrailingPoint.Z());

        double len1 = point1.Distance(point2);
        double len2 = point3.Distance(point4);
        double len3 = outerLeadingPoint.Y()  - innerLeadingPoint.Y();
        double len4 = innterTrailingPoint.Y()- innerTrailingPoint.Y();

        double lenght  =(len1+len2)/2.;
        double lenght2 =(len3+len4)/2.;

        double T = distance2/distance;

        double b_mac =lenght*(2*distance2+distance)/(3*(distance2+distance));
        double c_mac =distance-(distance-distance2)/lenght*b_mac;

        gp_Pnt quarterchord  = segment.GetChordPoint(0, 0.25);
        gp_Pnt quarterchord2 = segment.GetChordPoint(1, 0.25);

        double sw_tan   = (quarterchord2.X()-quarterchord.X())/lenght;
        double dihe_sin = (quarterchord2.Z()-quarterchord.Z())/lenght;
        double dihe_cos = lenght2/lenght;

        gp_XYZ seg_mac_p;
        seg_mac_p.SetX(0.25*distance - 0.25*c_mac + b_mac*sw_tan);
        seg_mac_p.SetY(dihe_cos*b_mac);
        seg_mac_p.SetZ(dihe_sin*b_mac);
        seg_mac_p.Add(innerLeadingPoint.XYZ());

        double A =((1. + T)*distance*lenght/2.);

        A_sum += A;
        cc_mac_sum_p += seg_mac_p.Multiplied(A);
        cc_mac_sum   += c_mac*A;
    }

    // compute mac position and chord
    mac_x     = cc_mac_sum_p.X()/A_sum;
    mac_y     = cc_mac_sum_p.Y()/A_sum;
    mac_z     = cc_mac_sum_p.Z()/A_sum;
    mac_chord = cc_mac_sum/A_sum;
}


// Calculates the segment coordinates from global (x,y,z) coordinates
// Returns the segment index of the according segment
// If x,y,z does not belong to any segment, -1 is returned
int CCPACSWing::GetSegmentEtaXsi(const gp_Pnt& point, double& eta, double& xsi, bool& onTop)
{
    // search the segment
    int segmentFound = -1;
    bool onFollowing = false;
    for (int iSeg = 1; iSeg <= GetSegmentCount(); ++iSeg) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&) GetSegment(iSeg);
        if (segment.GetIsOn(point) == true) {
            segmentFound = iSeg;

            // check if also on following segment
            if (iSeg < GetSegmentCount() && GetSegment(iSeg+1).GetIsOn(point)) {
                onFollowing = true;
            }

            break;
        }
    }

    if (segmentFound <= 0) {
        return -1;
    }

    if (onFollowing) {
        // check wich of both segments is the correct one
        CCPACSWingSegment& segment1 = (CCPACSWingSegment&) GetSegment(segmentFound);
        CCPACSWingSegment& segment2 = (CCPACSWingSegment&) GetSegment(segmentFound+1);

        double eta1, eta2, xsi1, xsi2;
        segment1.GetEtaXsi(point, eta1, xsi1);
        segment2.GetEtaXsi(point, eta2, xsi2);

        // Get the points on the chord face
        double eta1p = max(0.0, min(1.0, eta1));
        double eta2p = max(0.0, min(1.0, eta2));
        double xsi1p = max(0.0, min(1.0, xsi1));
        double xsi2p = max(0.0, min(1.0, xsi2));

        gp_Pnt p1 = segment1.GetChordPoint(eta1p, xsi1p);
        gp_Pnt p2 = segment2.GetChordPoint(eta2p, xsi2p);

        double d1 = p1.Distance(point);
        double d2 = p2.Distance(point);

        if (d1 <= d2) {
            eta = eta1;
            xsi = xsi1;

            // TODO: do we need that here?
            onTop = segment1.GetIsOnTop(point);

            return segmentFound;
        }
        else {
            eta = eta2;
            xsi = xsi2;

            // TODO: do we need that here?
            onTop = segment2.GetIsOnTop(point);

            return segmentFound + 1;
        }

    }
    else {

        CCPACSWingSegment& segment = (CCPACSWingSegment&) GetSegment(segmentFound);
        segment.GetEtaXsi(point, eta, xsi);

        // TODO: do we need that here?
        onTop = segment.GetIsOnTop(point);

        return segmentFound;
    }
}

// Get the guide curve with a given UID
CCPACSGuideCurve& CCPACSWing::GetGuideCurveSegment(std::string uid)
{
    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        CCPACSWingSegment& segment = m_segments.GetSegment(i);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        if (segment.GetGuideCurves()->GuideCurveExists(uid)) {
            return segment.GetGuideCurves()->GetGuideCurve(uid);
        }
    }
    throw tigl::CTiglError("Guide Curve with UID " + uid + " does not exists", TIGL_ERROR);
}

TopoDS_Compound CCPACSWing::GetGuideCurveWires() const
{
    return guideCurves->wiresAsCompound;
}

std::vector<double> CCPACSWing::GetGuideCurveStartParameters() const
{
    std::vector<double> res;
    for (const auto& curve : guideCurves->curves) {
        res.push_back(curve.fromRelCircumference);
    }
    return res;
}


std::vector<gp_Pnt> CCPACSWing::GetGuideCurvePoints()
{
    std::vector<gp_Pnt> points;

    // connect the belonging guide curve segments
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        CCPACSWingSegment& segment = m_segments.GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        CCPACSGuideCurves& segmentCurves = *segment.GetGuideCurves();
        for (int iguide = 1; iguide <=  segmentCurves.GetGuideCurveCount(); ++iguide) {
            CCPACSGuideCurve& curve = segmentCurves.GetGuideCurve(iguide);
            std::vector<gp_Pnt> curPoints = curve.GetCurvePoints();
            points.insert(points.end(), curPoints.begin(), curPoints.end());
        }
    }
    return points;
}


void CCPACSWing::BuildGuideCurveWires(LocatedGuideCurves& cache) const
{
    // check, if the wing has a blunt trailing edge
    bool hasBluntTE = true;
    if (GetSectionCount() > 0) {
        const CCPACSWingSegment& segment = m_segments.GetSegment(1);
        hasBluntTE = !segment.GetInnerConnection().GetProfile().GetTrailingEdge().IsNull();
    }
    
    // the guide curves will be sorted according to the inner
    // from relativeCircumference
    std::map<double, const CCPACSGuideCurve*> roots;

    // get chord centers on each section for the centripetal parametrization
    std::vector<gp_Pnt> sectionCenters(GetSegmentCount()+1);

    // get inner chord face center of first segment
    const CCPACSWingSegment& innerSegment = m_segments.GetSegment(1);
    sectionCenters[0] = innerSegment.GetInnerProfilePoint(0.);  // inner front
    gp_Pnt back       = innerSegment.GetInnerProfilePoint(1.);  // inner back
    back.BaryCenter(0.5, sectionCenters[0], 0.5);               // inner chord center
    
    // connect the belonging guide curve segments
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        const CCPACSWingSegment& segment = m_segments.GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        // get outer chord face center of current segment
        sectionCenters[isegment] = segment.GetOuterProfilePoint(0.);   // outer front
        back                     = segment.GetOuterProfilePoint(1.);   // outer back
        back.BaryCenter(0.5, sectionCenters[isegment], 0.5);           // outer chord center

        const CCPACSGuideCurves& segmentCurves = *segment.GetGuideCurves();
        for (int iguide = 1; iguide <=  segmentCurves.GetGuideCurveCount(); ++iguide) {
            const CCPACSGuideCurve& curve = segmentCurves.GetGuideCurve(iguide);
            if (!curve.GetFromGuideCurveUID_choice1()) {
                // this is a root curve
                double fromRef = *curve.GetFromRelativeCircumference_choice2();
                if (fromRef >= 1. && !hasBluntTE) {
                    fromRef = -1.;
                }
                roots.insert(std::make_pair(fromRef, &curve));
            }
        }
    }

    // get the parameters at the section centers
    std::vector<double> sectionParams = CTiglBSplineAlgorithms::computeParamsBSplineCurve(OccArray(sectionCenters), 0., 1., 0.5);

    // connect guide curve segments to a spline with given continuity conditions and tangents
    CTiglCurveConnector connector(roots, sectionParams);
    auto wires = connector.GetConnectedGuideCurves();

    //  collect the from_ref_circumference values
    assert(roots.size() == GetNumberOfSubshapes(wires));

    cache.wiresAsCompound = wires;
    cache.curves.clear();

    auto rootIt = roots.begin();
    for (TopoDS_Iterator anIter(wires); anIter.More(); anIter.Next(), rootIt++) {
        cache.curves.push_back(LocatedGuideCurves::LocatedGuideCurve(TopoDS::Wire(anIter.Value()), rootIt->first));
    }
}

TopoDS_Shape transformWingProfileGeometry(const CTiglTransformation& wingTransform, const CTiglWingConnection& connection, const TopoDS_Shape& wire)
{
    TopoDS_Shape transformedWire(wire);

    // Do section element transformation on points
    CTiglTransformation trafo = connection.GetSectionElementTransformation();

    // Do section transformations
    trafo.PreMultiply(connection.GetSectionTransformation());

    // Do positioning transformations
    trafo.PreMultiply(connection.GetPositioningTransformation());

    trafo.PreMultiply(wingTransform);

    transformedWire = trafo.Transform(transformedWire);

    return transformedWire;
}

void CCPACSWing::SetBuildFlaps(bool build)
{
    buildFlaps = build;
    // Reset the loft
    CTiglAbstractGeometricComponent::Reset();
}

PNamedShape CCPACSWing::GetWingCleanShape() const
{
    return *wingCleanShape;
}

namespace
{

    size_t NumberOfControlSurfaces(const CCPACSWing& wing)
    {
        size_t nControlSurfaces = 0;
        for ( const auto& componentSegment : wing.GetComponentSegments()->GetComponentSegments() ) {
           if (!componentSegment->GetControlSurfaces() || !componentSegment->GetControlSurfaces()->GetTrailingEdgeDevices()) {
               continue;
           }
           const CCPACSTrailingEdgeDevices& teds = componentSegment->GetControlSurfaces()->GetTrailingEdgeDevices().value();
           nControlSurfaces += teds.GetTrailingEdgeDevices().size();
        }
        return nControlSurfaces;
    }

}

} // end namespace tigl
