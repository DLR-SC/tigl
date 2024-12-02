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
#include "TiglWingHelperFunctions.h"
#include "ListFunctions.h"

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
#include "CTiglLogging.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepBuilderAPI_Transform.hxx>

namespace tigl
{

namespace
{
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
    , wingHelper(*this, &CCPACSWing::SetWingHelper)
{
    if (parent->IsParent<CCPACSAircraftModel>()) {
        configuration = &parent->GetParent<CCPACSAircraftModel>()->GetConfiguration();

        // register invalidation in CCPACSDucts
        if (configuration->GetDucts()) {
            configuration->GetDucts()->RegisterInvalidationCallback([&](){ this->Invalidate(); });
        }
    }
    else if (parent->IsParent<CCPACSRotorcraftModel>()) {
        configuration = &parent->GetParent<CCPACSRotorcraftModel>()->GetConfiguration();
    }
    else {
        throw CTiglError("Unknown parent");
    }

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
    , wingHelper(*this, &CCPACSWing::SetWingHelper)
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

    loft.clear();
    guideCurves.clear();
    wingCleanShape.clear();
    wingShapeWithCutouts.clear();
    wingHelper.clear();

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

    Update();
}

std::string CCPACSWing::GetDefaultedUID() const
{
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
    if (m_componentSegments) {
        return m_componentSegments->GetComponentSegmentCount();
    }
    else {
        return 0;

    }
}

// Returns the segment for a given index
CCPACSWingComponentSegment& CCPACSWing::GetComponentSegment(const int index)
{
    return m_componentSegments->GetComponentSegment(index);
}

const CCPACSWingComponentSegment& CCPACSWing::GetComponentSegment(const int index) const
{
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
    
// Gets the upper loft of the wing
TopoDS_Shape & CCPACSWing::GetUpperShape()
{
    if (rebuildShells) {
        BuildUpperLowerShells();
    }
    rebuildShells = false;
    return upperShape;
}
    
// Gets the lower loft of the wing.
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
    PNamedShape ret;
    if (buildFlaps) {

        // note: ducts are removed in BuildWingWithCutouts
        return GroupedFlapsAndWingShapes();
    } else {

        if (GetConfiguration().HasDucts()) {
            return  GetConfiguration().GetDucts()->LoftWithDuctCutouts(*wingCleanShape, GetUID());
        }

        return *wingCleanShape;
    }

    return ret;
}

TopoDS_Shape CCPACSWing::GetLoftWithCutouts()
{
    if (NumberOfControlSurfaces(*this) == 0) {
        LOG(WARNING) << "No control devices defined, GetLoftWithCutOuts() will return a clean shape.";
        return (*wingCleanShape)->Shape();
    }
    else {
        return (*wingShapeWithCutouts)->Shape();
    }
}

// Builds a fused shape of all wing segments
void CCPACSWing::BuildFusedSegments(PNamedShape& shape) const
{
    shape = CTiglWingBuilder(*this);
}
    
// Builds a fused shape of all wing segments
void CCPACSWing::BuildUpperLowerShells()
{
    //@todo: this probably works only if the wings does not split somewhere
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

    // cutout ducts
    if (GetConfiguration().HasDucts()) {
         result = GetConfiguration().GetDucts()->LoftWithDuctCutouts(result, GetUID());
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
    if (m_positionings) {
        return m_positionings->GetPositioningTransformation(sectionUID);
    }
    else {
        return CTiglTransformation(); // return identity if no positioning transformation is given
    }
}

// Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
gp_Pnt CCPACSWing::GetUpperPoint(int segmentIndex, double eta, double xsi)
{
    return ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetUpperPoint(eta, xsi, getPointBehavior);
}

// Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
gp_Pnt CCPACSWing::GetLowerPoint(int segmentIndex, double eta, double xsi)
{
    return  ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetLowerPoint(eta, xsi, getPointBehavior);
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
double CCPACSWing::GetReferenceArea(TiglSymmetryAxis symPlane) const
{
    double refArea = 0.0;

    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        refArea += m_segments.GetSegment(i).GetReferenceArea(symPlane);
    }
    return refArea;
}

double CCPACSWing::GetReferenceArea() const
{
    TiglAxis spanDir = wingHelper->GetMajorDirection();
    TiglAxis deepDir = wingHelper->GetDeepDirection();

    if (spanDir == TIGL_Y_AXIS && deepDir == TIGL_X_AXIS) {
        return GetReferenceArea(TIGL_X_Y_PLANE);
    }
    else if (spanDir == TIGL_Y_AXIS && deepDir == TIGL_Z_AXIS) {
        return GetReferenceArea(TIGL_Y_Z_PLANE);
    }
    else if (spanDir == TIGL_Z_AXIS && deepDir == TIGL_X_AXIS) {
        return GetReferenceArea(TIGL_X_Z_PLANE);
    }
    else if (spanDir == TIGL_Z_AXIS && deepDir == TIGL_Y_AXIS) {
        return GetReferenceArea(TIGL_Y_Z_PLANE);
    }
    else if (spanDir == TIGL_X_AXIS && deepDir == TIGL_Z_AXIS) {
        return GetReferenceArea(TIGL_X_Z_PLANE);
    }
    else if (spanDir == TIGL_X_AXIS && deepDir == TIGL_Y_AXIS) {
        return GetReferenceArea(TIGL_X_Y_PLANE);
    }
    else {
       LOG(ERROR) << "CCPACSWing::GetReferenceArea: Unexpected pair of major direction and deep direction.";
       return 0.0;
    }
}


double CCPACSWing::GetWettedArea(TopoDS_Shape parent) const
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

double CCPACSWing::GetWingspan() const
{
    if (!wingHelper->HasShape()) {
        LOG(WARNING) << "The wing seems empty.";
        return 0;
    }

    // prepare mirror transformation
    TiglSymmetryAxis sym = GetSymmetryAxis();
    gp_Trsf mirrorTransform;
    if (sym != TIGL_NO_SYMMETRY) {
        gp_Ax2 mirrorPlane;
        if (sym == TIGL_X_Z_PLANE) {
            mirrorPlane = gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0., 1., 0.));
        } else if (sym == TIGL_X_Y_PLANE) {
            mirrorPlane = gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0., 0., 1.));
        } else if (sym == TIGL_Y_Z_PLANE) {
            mirrorPlane = gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(1., 0., 0.));
        }
        mirrorTransform.SetMirror(mirrorPlane);
    }


    Bnd_Box boundingBox;
    Standard_Real aDeflection = 0.0001;
    std::vector<CTiglSectionElement* >  cElements = GetCTiglElements();
    for ( size_t i = 0; i < cElements.size(); i++ ) {
        TopoDS_Wire tipWire = cElements.at(i)->GetWire(GLOBAL_COORDINATE_SYSTEM);
        BRepMesh_IncrementalMesh Inc(tipWire, aDeflection); // tessellation for accuracy
        BRepBndLib::Add(tipWire, boundingBox);
        if (sym != TIGL_NO_SYMMETRY ) {
            BRepBuilderAPI_Transform myBRepTransformation(tipWire, mirrorTransform);
            TopoDS_Shape mirroredTipWire = myBRepTransformation.Shape();
            BRepBndLib::Add(mirroredTipWire, boundingBox);
        }
    }

    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    switch (wingHelper->GetMajorDirection()) {
    case TIGL_Z_AXIS:
        return zmax - zmin;
        break;
    case TIGL_Y_AXIS:
        return ymax - ymin;
        break;
    case TIGL_X_AXIS:
        return xmax - xmin;
        break;
    default:
        return ymax - ymin;
    }
}

double CCPACSWing::GetWingHalfSpan()
{
    if (!wingHelper->HasShape()) {
        LOG(WARNING) << "The wing seems empty.";
        return 0;
    }

    Bnd_Box boundingBox;
    Standard_Real aDeflection = 0.0001;

    std::vector<CTiglSectionElement* >  cElements = GetCTiglElements();
    for ( size_t i = 0; i < cElements.size(); i++ ) {
      TopoDS_Wire wire = cElements.at(i)->GetWire(GLOBAL_COORDINATE_SYSTEM);
      BRepMesh_IncrementalMesh Inc(wire, aDeflection); // tessellation for accuracy
      BRepBndLib::Add(wire, boundingBox);
    }

    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    switch (wingHelper->GetMajorDirection()) {
    case TIGL_Z_AXIS:
        return zmax - zmin;
    case TIGL_Y_AXIS:
        return ymax - ymin;
    case TIGL_X_AXIS:
        return xmax - xmin;
    default:
        return ymax - ymin;
    }
}

// Returns the aspect ratio of a wing: AR=b**2/A = ((2s)**2)/(2A_half)
//     b: full span; A: Reference area of full wing (wing + symmetrical wing)
//     s: half span; A_half: Reference area of wing without symmetrical wing
double CCPACSWing::GetAspectRatio() const
{
    auto refArea = GetReferenceArea();

    if ( isNear(refArea, 0.) ) {
        LOG(WARNING) << "Wing area is close to zero, thus the AR is not computed and 0 is returned.";
        return 0;
    }

    auto halfSpan = GetWingspan();

    // If the wing has symmetry defined, the full span was returned
    if (GetSymmetryAxis() != TIGL_NO_SYMMETRY) {
        halfSpan *= 0.5;
    }

    return 2.0*halfSpan*halfSpan/refArea;
}

/**
    * This function calculates location of the quarter of mean aerodynamic chord,
    * and gives the chord length as well. It uses the classical method that can
    * be applied to trapozaidal wings. This method is used for each segment.
    * The values are found by taking into account of sweep and dihedral.
    * But the effect of insidance angle is neglected. These values should coincide
    * with the values found with tornado tool.
    */
void  CCPACSWing::GetWingMAC(double& mac_chord, double& mac_x, double& mac_y, double& mac_z) const
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

        double length  =(len1+len2)/2.;
        double length2 =(len3+len4)/2.;

        double T = distance2/distance;

        double b_mac =length*(2*distance2+distance)/(3*(distance2+distance));
        double c_mac =distance-(distance-distance2)/length*b_mac;

        gp_Pnt quarterchord  = segment.GetChordPoint(0, 0.25);
        gp_Pnt quarterchord2 = segment.GetChordPoint(1, 0.25);

        double sw_tan   = (quarterchord2.X()-quarterchord.X())/length;
        double dihe_sin = (quarterchord2.Z()-quarterchord.Z())/length;
        double dihe_cos = length2/length;

        gp_XYZ seg_mac_p;
        seg_mac_p.SetX(0.25*distance - 0.25*c_mac + b_mac*sw_tan);
        seg_mac_p.SetY(dihe_cos*b_mac);
        seg_mac_p.SetZ(dihe_sin*b_mac);
        seg_mac_p.Add(innerLeadingPoint.XYZ());

        double A =((1. + T)*distance*length/2.);

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
        gp_Pnt p1, p2;
        segment1.GetEtaXsi(point, eta1, xsi1, p1, getPointBehavior);
        segment2.GetEtaXsi(point, eta2, xsi2, p2, getPointBehavior);

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
        gp_Pnt pTmp;
        segment.GetEtaXsi(point, eta, xsi, pTmp, getPointBehavior);

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
    if (GetSegmentCount() == 0) {
        return {};
    }

    const auto& segment = GetSegment(1);
    const auto& guides = segment.GetGuideCurves();
    if (!guides) {
        return {};
    }

    return guides->GetRelativeCircumferenceParameters();
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
                double fromRef;
                if (curve.GetFromRelativeCircumference_choice2_1()) {
                    fromRef = *curve.GetFromRelativeCircumference_choice2_1();
                }
                else if (curve.GetFromParameter_choice2_2()) {
                    fromRef = *curve.GetFromParameter_choice2_2();
                }
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
        cache.curves.push_back({TopoDS::Wire(anIter.Value()), rootIt->first});
    }

    // sort according to from location parameter
    using LocCurve = LocatedGuideCurves::LocatedGuideCurve;
    std::sort(std::begin(cache.curves), std::end(cache.curves), [](const LocCurve& c1, const LocCurve& c2) {
        return c1.fromRelCircumference < c2.fromRelCircumference;
    });
}


double CCPACSWing::GetSweep(double chordPercentage) const
{

    if (!wingHelper->HasShape()) {
        LOG(WARNING) << "The wing seems empty.";
        return 0;
    }
    /*
     * 1) get the segment between root and tip
     * 2) project on the plane formed by the majorDir and the deepDir
     * 3) mirror the vector if they are not in the same direction has the major dir
     * 3) compute the angle between the projected vector and the major axis as a oriented rotation
     */

    CTiglPoint rootChord = wingHelper->GetCTiglElementOfWing(wingHelper->GetRootUID())->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
    CTiglPoint tipChord  =  wingHelper->GetCTiglElementOfWing(wingHelper->GetTipUID())->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
    CTiglPoint rootToTip = tipChord - rootChord;

    TiglAxis majorDir = wingHelper->GetMajorDirection();
    TiglAxis deepDir  = wingHelper->GetDeepDirection();

    // since all coordinate of the majorDir vector are zero except the one that represent the axis
    // the dot product is similar to project the vector on the axis
    double majorProj = CTiglPoint::inner_prod(TiglAxisToCTiglPoint(majorDir), rootToTip);
    double deepProj  = CTiglPoint::inner_prod(TiglAxisToCTiglPoint(deepDir), rootToTip);

    double angleRad = atan2(deepProj, fabs(majorProj)); // fabs (majorProj) mirror the coordinate if needed

    return Degrees(angleRad);
}

double CCPACSWing::GetDihedral(double chordPercentage) const
{

    if (!wingHelper->HasShape()) {
        LOG(WARNING) << "The wing seems empty.";
        return 0;
    }

    /*
     * 1) get the segment between root and tip
     * 2) project on the plane formed by the major Dir and the third Dir
     * 3) mirror the vector if they are not in the same direction has the major dir
     * 3) compute the angle between the projected vector and the major axis as a oriented rotation
     */

    CTiglPoint rootChord = wingHelper->GetCTiglElementOfWing(wingHelper->GetRootUID())->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
    CTiglPoint tipChord  = wingHelper->GetCTiglElementOfWing(wingHelper->GetTipUID())->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
    CTiglPoint rootToTip = tipChord - rootChord;
    TiglAxis majorDir = wingHelper->GetMajorDirection();
    TiglAxis thirdDir = wingHelper->GetThirdDirection();

    // since all coordinate of the majorDir vector are zero except the one that represent the axis
    // the dot product is similar to project the vector on the axis
    double majorProj = CTiglPoint::inner_prod(TiglAxisToCTiglPoint(majorDir), rootToTip);
    double thirdProj = CTiglPoint::inner_prod(TiglAxisToCTiglPoint(thirdDir), rootToTip);

    double angleRad = atan2(thirdProj, fabs(majorProj)); // fabs (majorProj) mirror the coordinate if needed

    return Degrees(angleRad);
}

void CCPACSWing::SetWingHelper(CTiglWingHelper& cache) const
{
    cache.SetWing(const_cast<CCPACSWing*>(this));
}

CTiglPoint CCPACSWing::GetRootLEPosition() const
{
    if (!wingHelper->HasShape()) {
        LOG(WARNING) << "The wing seems empty.";
        return CTiglPoint(0, 0, 0);
    }

    CTiglWingSectionElement* root = wingHelper->GetCTiglElementOfWing(wingHelper->GetRootUID());
    return root->GetChordPoint(0, GLOBAL_COORDINATE_SYSTEM);
}

void CCPACSWing::SetRootLEPosition(tigl::CTiglPoint newRootPosition)
{
    CTiglPoint oldPosition               = GetRootLEPosition();
    CTiglPoint delta                     = newRootPosition - oldPosition;
    CCPACSTransformation& transformation = GetTransformation();
    CTiglPoint currentTranslation        = transformation.getTranslationVector();
    transformation.setTranslation(currentTranslation + delta);
    Invalidate();
}

void CCPACSWing::SetRotation(CTiglPoint newRot)
{
    CCPACSTransformation& transformation = GetTransformation();
    transformation.setRotation(newRot);
    Invalidate();
}

void CCPACSWing::SetSweep(double newAngle, double chordPercentage)
{
    // check inputs
    if (newAngle > 89 || newAngle < -89 || chordPercentage < 0 || chordPercentage > 1) {
        throw CTiglError("Invalid input! required: angle [-89,89], chordPrecentage [0,1] ");
    }

    // The idea is:
    // 1) Compute the new chord point of the tip such that the wing has the wanted origin
    // 2) Find out the shearing in the third direction that bring the chord point to the wanted spot
    // 3) For all chord point:
    //      a) apply that shearing
    //      b) find out the new origin that bring the chord point to new spot
    //      c) set the origin
    //
    // Remark: * To perform the shearing, we first want that the root chord point is on the origin to not move the root.
    //         * The origin is moved with translation instead on applying directly the shearing on the transformation,
    //           because we do not want to modify the shape of each section by the shearing.

    CTiglPoint rootChord = wingHelper->GetCTiglElementOfWing(wingHelper->GetRootUID())
            ->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
    CTiglPoint tipChord = wingHelper->GetCTiglElementOfWing(wingHelper->GetTipUID())
            ->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
    ;

    // compute the value fot the translation operation

    CTiglTransformation translation;
    translation.AddTranslation(-rootChord.x, -rootChord.y, -rootChord.z);
    CTiglTransformation translationI;
    translationI = translation.Inverted();

    // Compute the value for the shear operation

    CTiglPoint majorDir = TiglAxisToCTiglPoint(wingHelper->GetMajorDirection());
    CTiglPoint deepDir  = TiglAxisToCTiglPoint(wingHelper->GetDeepDirection());
    // since all coordinate of the majorDir vector are zero except the one that represent the axis
    // the dot product is similar to project the vector on the axis
    double majorProj =  CTiglPoint::inner_prod(majorDir, translation * tipChord);
    double deepProj = CTiglPoint::inner_prod(deepDir, translation * tipChord);
    // computed the need deepProj to have the wanted sweep angle
    double neededDeepProj = tan(Radians(newAngle)) * fabs(majorProj);

    double lambda = (neededDeepProj - deepProj) / majorProj;
    int lambdaRow = maxIndex(deepDir.x, deepDir.y, deepDir.z);
    int lambdaCol = maxIndex(majorDir.x, majorDir.y, majorDir.z);

    CTiglTransformation shear;
    shear.SetValue(lambdaRow, lambdaCol, lambda);

    std::vector<std::string> orderedUIDs = m_segments.GetElementUIDsInOrder();

    CTiglPoint oldOrigin, oldChordPoint, newOrigin, newChordPoint, originToChordPoint;
    CTiglWingSectionElement* cElement;
    for (size_t e = 1; e < orderedUIDs.size(); e++) { // start 1 -> the root section should not change
        cElement                = wingHelper->GetCTiglElementOfWing(orderedUIDs.at(e));
        oldChordPoint = cElement->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
        oldOrigin = cElement->GetOrigin(GLOBAL_COORDINATE_SYSTEM);
        originToChordPoint = oldChordPoint - oldOrigin;
        newChordPoint = translationI * shear * translation * oldChordPoint;
        newOrigin = newChordPoint - originToChordPoint;
        cElement->SetOrigin(newOrigin);
    }
}

void CCPACSWing::SetDihedral(double newDihedral, double chordPercentage)
{
    // check inputs
    if (newDihedral > 89 || newDihedral < -89 || chordPercentage < 0 || chordPercentage > 1) {
        throw CTiglError("Invalid input! required angle [-89,89], chordPrecentage [0,1] ");
    }
    // The idea is:
    // 1) Compute the new chord point of the tip such that the wing has the wanted origin
    // 2) Find out the shearing in the deep direction that bring the chord point to the wanted spot
    // 3) For all chord point:
    //      a) apply that shearing
    //      b) find out the new origin that bring the chord point to new spot
    //      c) set the origin
    //
    // Remark: * To perform the shearing, we first want that the root chord point is on the origin to not move the root.
    //         * The origin is moved with translation instead on applying directly the shearing on the transformation,
    //           because we do not want to modify the shape of each section by the shearing.

    CTiglPoint rootChord = wingHelper->GetCTiglElementOfWing(wingHelper->GetRootUID())
                               ->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
    CTiglPoint tipChord = wingHelper->GetCTiglElementOfWing(wingHelper->GetTipUID())
                              ->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
    ;

    // compute the value fot the translation operation

    CTiglTransformation translation;
    translation.AddTranslation(-rootChord.x, -rootChord.y, -rootChord.z);
    CTiglTransformation translationI;
    translationI = translation.Inverted();

    // Compute the value for the shear operation

    CTiglPoint majorDir = TiglAxisToCTiglPoint(wingHelper->GetMajorDirection());
    CTiglPoint thirdDir = TiglAxisToCTiglPoint(wingHelper->GetThirdDirection());
    // since all coordinate of the majorDir vector are zero except the one that represent the axis
    // the dot product is similar to project the vector on the axis
    double majorProj = CTiglPoint::inner_prod(majorDir, translation * tipChord);
    double thirdProj = CTiglPoint::inner_prod(thirdDir, translation * tipChord);
    // computed the need thirdProj to have the wanted sweep angle
    double neededThirdProj = tan(Radians(newDihedral)) * fabs(majorProj);

    double lambda = (neededThirdProj - thirdProj) / majorProj;
    int lambdaRow = maxIndex(thirdDir.x, thirdDir.y, thirdDir.z);
    int lambdaCol = maxIndex(majorDir.x, majorDir.y, majorDir.z);

    CTiglTransformation shear;
    shear.SetValue(lambdaRow, lambdaCol, lambda);

    std::vector<std::string> orderedUIDs = m_segments.GetElementUIDsInOrder();

    CTiglPoint oldOrigin, oldChordPoint, newOrigin, newChordPoint, originToChordPoint;
    CTiglWingSectionElement* cElement;
    for (size_t e = 1; e < orderedUIDs.size(); e++) { // start 1 -> the root section should not change
        cElement           = wingHelper->GetCTiglElementOfWing(orderedUIDs.at(e));
        oldChordPoint      = cElement->GetChordPoint(chordPercentage, GLOBAL_COORDINATE_SYSTEM);
        oldOrigin          = cElement->GetOrigin(GLOBAL_COORDINATE_SYSTEM);
        originToChordPoint = oldChordPoint - oldOrigin;
        newChordPoint      = translationI * shear * translation * oldChordPoint;
        newOrigin          = newChordPoint - originToChordPoint;
        cElement->SetOrigin(newOrigin);
    }
}

void CCPACSWing::Scale(double scaleF)
{

    // We do not want to change the root leading edge position while scaling,
    // so we first translate the root leading position to the origin,
    // the we scale it and finally we translate the wing back to its initial position

    CTiglPoint lEPosition = GetRootLEPosition();
    CTiglTransformation translationToOrigin, translationToOriginI;
    translationToOrigin.AddTranslation(-lEPosition.x, -lEPosition.y, -lEPosition.z);
    translationToOriginI = translationToOrigin.Inverted();
    CTiglTransformation scaling;
    scaling.AddScaling(scaleF, scaleF, scaleF);

    std::vector<std::string> orderedUIDs = m_segments.GetElementUIDsInOrder();

    for (size_t i = 0; i < orderedUIDs.size(); i++) {
        CTiglWingSectionElement* cElement = wingHelper->GetCTiglElementOfWing(orderedUIDs.at(i));
        CTiglTransformation global        = cElement->GetTotalTransformation();
        CTiglTransformation newGlobal;
        newGlobal = translationToOriginI * scaling * translationToOrigin * global;
        cElement->SetTotalTransformation(newGlobal);
    }
}

void CCPACSWing::SetAreaKeepAR(double newArea)
{
    // newA = scaleF² * oldA
    // scaleF = sqrt(newA/oldA)

    if ( newArea <= 0 ) {
        throw  CTiglError("Invalid input! The given area must be higher than 0.");
    }

    double oldA = GetReferenceArea();
    double scaleF = sqrt(newArea/oldA);
    Scale(scaleF);

}


void CCPACSWing::SetAreaKeepSpan(double newArea)
{

    double v = 0;
    double w = 0;

    CTiglTransformation projection;

    switch ( wingHelper->GetThirdDirection() ) {
    case TIGL_Z_AXIS:
        projection.AddProjectionOnXYPlane();
        break;
    case TIGL_Y_AXIS:
        projection.AddProjectionOnXZPlane();
        break;
    case TIGL_X_AXIS:
        projection.AddProjectionOnYZPlane();
        break;
    }

    double vInc, wInc;
    CTiglPoint a , b, c;
    for ( int i = 0; i < m_segments.GetSegmentCount(); i++) {
        CCPACSWingSegment& seg = m_segments.GetSegment( i + 1 );
        b = CTiglPoint(seg.GetChordPoint(0,1).XYZ()) - CTiglPoint(seg.GetChordPoint(0,0).XYZ());
        b = projection * b;
        a = CTiglPoint(seg.GetChordPoint(1,0).XYZ()) - CTiglPoint(seg.GetChordPoint(0,0).XYZ());
        a = projection * a;
        c = CTiglPoint(seg.GetChordPoint(1,1).XYZ()) - CTiglPoint(seg.GetChordPoint(1,0).XYZ());
        c = projection * c;
        vInc = CTiglPoint::cross_prod(b + c, a).norm2();
        wInc = CTiglPoint::cross_prod(b , c).norm2();
        v += vInc;
        w += wInc;
    }

    double det = pow(v, 2) - (4 * -2*newArea * w);
    if ( det < 0) {
        throw CTiglError("Impossible to find a valid scale factor to set the area.");
    }

    // we use muller's method to support the case when w = 0
    double scale =  ( -2*2*newArea ) / ( -v - sqrt(det) )   ; // the second solution is always bigger or negative


    if ( scale <  0 ) {
        throw CTiglError("Impossible to find a valid scale factor to set the area.");
    }

    std::vector<std::string> uidOrders = m_segments.GetElementUIDsInOrder();



    for ( size_t i = 0 ; i < uidOrders.size(); i++ ) {
        CTiglWingSectionElement* element = wingHelper->GetCTiglElementOfWing( uidOrders.at(i) );
        // we store the leading edge to be able to reset it after scaling
        CTiglPoint lEPosition = element->GetChordPoint(0);
        // scaling is done from the center of the airfoil, so it will change its leading edge postion
        element->ScaleUniformly(scale);
        // reset the le position (the center will move)
        element->SetChordPoint(0, lEPosition);
    }

}



void CCPACSWing::SetHalfSpanKeepAR(double newHalfSpan)
{

    if ( newHalfSpan <= 0 ) {
        throw  CTiglError("Invalid input! The given span must be larger than 0.");
    }

    double oldSpan = GetWingHalfSpan();
    double scaleF = newHalfSpan/oldSpan;
    Scale(scaleF);
}


void CCPACSWing::SetHalfSpanKeepArea(double newHalfSpan)
{

    double area = GetReferenceArea();
    SetHalfSpanKeepAR(newHalfSpan);
    SetAreaKeepSpan(area);
}



void CCPACSWing::SetARKeepSpan(double newAR)
{

    // remember we define:
    // AR = pow(2*halfSpan,2)/2*A
    // AR = 2*pow(halfSpan,2)/A
    // -> A = 2*pow(halfSpan,2)/AR
    double span = GetWingHalfSpan();
    double newArea = (2 * pow(span, 2)) / newAR;
    SetAreaKeepSpan(newArea);

}


void CCPACSWing::SetARKeepArea(double newAR)
{
    double area = GetReferenceArea();
    double newSpan = sqrt( newAR * area * 0.5);
    SetHalfSpanKeepArea(newSpan);
}

void CCPACSWing::CreateNewConnectedElementBetween(std::string startElementUID, std::string endElementUID)
{
        if(GetSegments().GetSegmentFromTo(startElementUID, endElementUID).GetGuideCurves())
        {
            throw tigl::CTiglError("Adding sections in wing segments containing guide curves is currently not supported.\n"
                                   "In general, guide curves should only be added when all sections are already defined, since the guide curves depend on them.", TIGL_ERROR);
        }

        std::string segmentToSplit = GetSegments().GetSegmentFromTo(startElementUID, endElementUID).GetUID();
        CTiglWingSectionElement *startElement = wingHelper->GetCTiglElementOfWing(startElementUID);
        CTiglWingSectionElement *endElement = wingHelper->GetCTiglElementOfWing(endElementUID);

        // compute the new parameters for the new element
        CTiglPoint center = (startElement->GetCenter() + endElement->GetCenter()) * 0.5;
        CTiglPoint normal = (startElement->GetNormal() + endElement->GetNormal());
        if (isNear(normal.norm2(), 0)) {
            normal = startElement->GetNormal();
        }
        normal.normalize();
        double angleN = (startElement->GetRotationAroundNormal() + endElement->GetRotationAroundNormal()) * 0.5;
        double area = (startElement->GetArea() + endElement->GetArea()) * 0.5;

        // create new section and element
        CTiglUIDManager &uidManager = GetUIDManager();
        std::string baseUID = uidManager.MakeUIDUnique(startElement->GetSectionUID() + "Bis");
        CCPACSWingSection &newSection = GetSections().CreateSection(baseUID, startElement->GetProfileUID());
        CTiglWingSectionElement *newElement = newSection.GetSectionElement(1).GetCTiglSectionElement();

        // set the new parameters
        newElement->SetCenter(center);
        newElement->SetArea(area);
        newElement->SetNormal(normal);
        newElement->SetRotationAroundNormal(angleN);


        // connect the element with segment and update old segment
        GetSegments().SplitSegment(segmentToSplit, newElement->GetSectionElementUID());


}

void CCPACSWing::CreateNewConnectedElementAfter(std::string startElementUID)
{

        std::vector<std::string>  elementsAfter = ListFunctions::GetElementsAfter(wingHelper->GetElementUIDsInOrder(), startElementUID);
        if ( elementsAfter.size() > 0 ) {
            // In this case we insert the element between the start element and the next one
            this->CreateNewConnectedElementBetween(startElementUID, elementsAfter[0] );
        }
        else {
            std::vector<std::string>  elementsBefore = ListFunctions::GetElementsInBetween(wingHelper->GetElementUIDsInOrder(), wingHelper->GetRootUID(),startElementUID);
            if ( elementsBefore.size() < 2) {
                throw  CTiglError("Impossible to add a element after if there is no previous element");
            }

            // Iterate over segments to find the one ending in startElementUID
            // If the corresponding segment contains guide curves -> Throw error, since adding elements after gc-segments is not supported
            for (int i=1; i <= GetSegmentCount(); i++)
            {
                if(GetSegment(i).GetGuideCurves())
                {
                    throw tigl::CTiglError("Adding sections after wing segments containing guide curves is currently not supported.\n"
                                           "In general, guide curves should only be added when all sections are already defined, since the guide curves depend on them.", TIGL_ERROR);
                }
            }

            std::string  previousElementUID = elementsBefore[elementsBefore.size()-2];

            CTiglWingSectionElement* previousElement = wingHelper->GetCTiglElementOfWing(previousElementUID);
            CTiglWingSectionElement* startElement = wingHelper->GetCTiglElementOfWing(startElementUID);

            // Compute the parameters for the new section base on the start element and the previous element.
            // We try to create a continuous fuselage
            CTiglPoint normal  =  startElement->GetNormal() + (startElement->GetNormal() - previousElement->GetNormal() ) ;
            CTiglPoint center = startElement->GetCenter() + (startElement->GetCenter() - previousElement->GetCenter() );
            double angleN = startElement->GetRotationAroundNormal() + (startElement->GetRotationAroundNormal() -previousElement->GetRotationAroundNormal());
            double area = startElement->GetArea();
            if (previousElement->GetArea() > 0) {
                double scaleF = startElement->GetArea() / previousElement->GetArea();
                area = scaleF * area;
            }
            std::string profileUID = startElement->GetProfileUID();
            std::string sectionUID = startElement->GetSectionUID() + "After";


            CCPACSWingSection& newSection = GetSections().CreateSection(sectionUID, profileUID);
            CTiglWingSectionElement* newElement = newSection.GetSectionElement(1).GetCTiglSectionElement();

            newElement->SetNormal(normal);
            newElement->SetRotationAroundNormal(angleN);
            newElement->SetCenter(center);
            newElement->SetArea(area);

            // Connect the element with the segment
            CCPACSWingSegment&  newSegment = GetSegments().AddSegment();
            std::string newSegmentUID = GetUIDManager().MakeUIDUnique("SegGenerated");

            newSegment.SetUID(newSegmentUID);
            newSegment.SetName(newSegmentUID);
            newSegment.SetFromElementUID(startElementUID);
            newSegment.SetToElementUID(newElement->GetSectionElementUID());

        }

}


void CCPACSWing::CreateNewConnectedElementBefore(std::string startElementUID)
{
        std::vector<std::string> elementsBefore = ListFunctions::GetElementsInBetween(wingHelper->GetElementUIDsInOrder(), wingHelper->GetRootUID(),startElementUID);
        if ( elementsBefore.size() > 1 ) {
            this->CreateNewConnectedElementBetween(elementsBefore[elementsBefore.size()-2], startElementUID);
        }
        else {
            std::vector<std::string> elementsAfter  =  ListFunctions::GetElementsAfter(wingHelper->GetElementUIDsInOrder(), startElementUID);
            if (elementsAfter.size() < 1 ) {
                throw  CTiglError("Impossible to add a element before if there is no previous element");
            }

            // Iterate over segments to find the one starting in startElementUID
            // If the corresponding segment contains guide curves -> Throw error, since adding elements after gc-segments is not supported
            for (int i=1; i <= GetSegmentCount(); i++)
            {
                if(GetSegment(i).GetGuideCurves())
                {
                    throw tigl::CTiglError("Adding sections before wing segments containing guide curves is currently not supported.\n"
                                           "In general, guide curves should only be added when all sections are already defined, since the guide curves depend on them.", TIGL_ERROR);
                }
            }

            std::string  previousElementUID = elementsAfter[0];

            CTiglWingSectionElement* previousElement = wingHelper->GetCTiglElementOfWing(previousElementUID);
            CTiglWingSectionElement* startElement = wingHelper->GetCTiglElementOfWing(startElementUID);

            // Compute the parameters for the new section base on the start element and the previous element.
            // We try to create a continuous fuselage
            CTiglPoint normal  =  startElement->GetNormal() + (startElement->GetNormal() - previousElement->GetNormal() ) ;
            CTiglPoint center = startElement->GetCenter() + (startElement->GetCenter() - previousElement->GetCenter() );
            double angleN = startElement->GetRotationAroundNormal() + (startElement->GetRotationAroundNormal() -previousElement->GetRotationAroundNormal());
            double area = startElement->GetArea();
            if (previousElement->GetArea() > 0) {
                double scaleF = startElement->GetArea() / previousElement->GetArea();
                area = scaleF * area;
            }
            std::string profileUID = startElement->GetProfileUID();
            std::string sectionUID = startElement->GetSectionUID() + "Before";


            CCPACSWingSection& newSection = GetSections().CreateSection(sectionUID, profileUID);
            CTiglWingSectionElement* newElement = newSection.GetSectionElement(1).GetCTiglSectionElement();

            newElement->SetNormal(normal);
            newElement->SetRotationAroundNormal(angleN);
            newElement->SetCenter(center);
            newElement->SetArea(area);

            // Connect the element with the segment
            CCPACSWingSegment&  newSegment = GetSegments().AddSegment();
            std::string newSegmentUID = GetUIDManager().MakeUIDUnique("SegGenerated");

            newSegment.SetUID(newSegmentUID);
            newSegment.SetName(newSegmentUID);
            newSegment.SetFromElementUID(newElement->GetSectionElementUID());
            newSegment.SetToElementUID(startElementUID);

            GetSegments().Invalidate();
            // to reorder the segment if needed.
            if ( m_segments.NeedReordering() ){
                try { // we use a try-catch to not rise two time a exception if the reordering occurs during the first cpacs parsing
                    m_segments.ReorderSegments();
                }
                catch (  const CTiglError& err) {
                    LOG(ERROR) << err.what();
                }
            }
        }
}


std::vector<std::string> CCPACSWing::GetOrderedConnectedElement()
{
    return wingHelper->GetElementUIDsInOrder();
}




void CCPACSWing::DeleteConnectedElement(std::string elementUID)
{
    std::vector<std::string> orderedUIDs = wingHelper->GetElementUIDsInOrder();

    if (!ListFunctions::Contains(orderedUIDs, elementUID)) {
        throw CTiglError("Invalid uid, the given element is not a connected element ");
    }
    // section to delete
    CCPACSWingSection& sec = GetSections().GetSection(wingHelper->GetCTiglElementOfWing(elementUID)->GetSectionUID());

    std::vector<std::string> previouss = ListFunctions::GetElementsBefore(orderedUIDs, elementUID);
    std::vector<std::string> nexts = ListFunctions::GetElementsAfter(orderedUIDs, elementUID);

    if (previouss.size() > 0 && nexts.size() == 0) { // section is the last one
        std::string previous = previouss[previouss.size() - 1 ];
        CCPACSWingSegment& seg = GetSegments().GetSegmentFromTo(previous, elementUID);
        GetSegments().RemoveSegment(seg);
        GetSections().RemoveSection(sec);
    }
    else if (previouss.size() == 0 && nexts.size() > 0) { // section is the first one
        std::string next =  nexts.at(0);
        CCPACSWingSegment& seg = GetSegments().GetSegmentFromTo(elementUID, next);
        GetSegments().RemoveSegment(seg);
        GetSections().RemoveSection(sec);
    }
    else if (previouss.size() > 0 && nexts.size() > 0) { // section is in between two other section
        std::string previous = previouss[previouss.size() - 1 ];
        std::string next =  nexts.at(0);
        CCPACSWingSegment& seg1 = GetSegments().GetSegmentFromTo(previous, elementUID);
        CCPACSWingSegment& seg2 = GetSegments().GetSegmentFromTo(elementUID, next);
        GetSegments().RemoveSegment(seg2);
        seg1.SetToElementUID(next);
        GetSections().RemoveSection(sec);
    } else {
        throw CTiglError("Unexpected case: wing structure seems unusual.");
    }
    Invalidate();

}

std::vector<CTiglSectionElement* > CCPACSWing::GetCTiglElements() const
{
    std::vector<std::string> elements =  wingHelper->GetElementUIDsInOrder();
    std::vector<tigl::CTiglSectionElement*> cElements;
    for (size_t i = 0; i < elements.size(); i++ ) {
        cElements.push_back(wingHelper->GetCTiglElementOfWing(elements[i]));
    }
    return cElements;
}


std::vector<std::string> CCPACSWing::GetAllUsedAirfoils()
{
    std::vector<std::string> airfoils;
    std::vector<CTiglSectionElement*> cElements = GetCTiglElements();
    std::string uid;
    for (size_t i = 0 ; i < cElements.size(); i++) {
        uid = cElements.at(i)->GetProfileUID();
        if ( ! ListFunctions::Contains(airfoils, uid) ) {
            airfoils.push_back(uid);
        }
    }
    return airfoils;
}



void CCPACSWing::SetAllAirfoils(const std::string &profileUID)
{
    std::vector<CTiglSectionElement*> cElements = GetCTiglElements();
    for (size_t i = 0 ; i < cElements.size(); i++) {
        cElements.at(i)->SetProfileUID(profileUID);
    }
}



TopoDS_Shape transformWingProfileGeometry(const CTiglTransformation& wingTransform,
                                          const CTiglWingConnection& connection, const TopoDS_Shape& wire)
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
