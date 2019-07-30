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
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include "CTiglLogging.h"

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

}

CCPACSWing::CCPACSWing(CCPACSWings* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWing(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
    , rebuildFusedSegments(true)
    , rebuildFusedSegWEdge(true)
    , rebuildShells(true)
    , guideCurves(*this, &CCPACSWing::BuildGuideCurveWires)
    , wingHelper(*this, &CCPACSWing::SetWingHelper)
{
    if (parent->IsParent<CCPACSAircraftModel>()) {
        configuration = &parent->GetParent<CCPACSAircraftModel>()->GetConfiguration();
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
    , rebuildFusedSegments(true)
    , rebuildFusedSegWEdge(true)
    , rebuildShells(true)
    , guideCurves(*this, &CCPACSWing::BuildGuideCurveWires)
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
void CCPACSWing::Invalidate()
{
    invalidated = true;
    loft.clear();
    guideCurves.clear();
    m_segments.Invalidate();
    if (m_positionings) {
        m_positionings->Invalidate();
    }
    if (m_componentSegments) {
        m_componentSegments->Invalidate();
    }
    wingHelper.clear();
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
    if (!invalidated) {
        return;
    }

    invalidated = false;
    rebuildFusedSegments = true;    // forces a rebuild of all segments with regards to the updated translation
    rebuildShells = true;
}

// Read CPACS wing element
void CCPACSWing::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& wingXPath)
{
    Cleanup();
    generated::CPACSWing::ReadCPACS(tixiHandle, wingXPath);

    if (wingXPath.find("rotorBlade") != std::string::npos) {
        isRotorBlade = true;
    }

    ConnectGuideCurveSegments();

    Update();
}

std::string CCPACSWing::GetDefaultedUID() const
{
    return generated::CPACSWing::GetUID();
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
        fusedSegmentWithEdge = BuildFusedSegments(true)->Shape();
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
    return BuildFusedSegments(true);
}

// Builds a fused shape of all wing segments
PNamedShape CCPACSWing::BuildFusedSegments(bool splitWingInUpperAndLower) const
{
    PNamedShape loft = CTiglWingBuilder(*this);
    return loft;
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
    return ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetUpperPoint(eta, xsi);
}

// Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
gp_Pnt CCPACSWing::GetLowerPoint(int segmentIndex, double eta, double xsi)
{
    return  ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetLowerPoint(eta, xsi);
}

// Gets a point on the chord surface in absolute (world) coordinates for a given segment, eta, xsi
gp_Pnt CCPACSWing::GetChordPoint(int segmentIndex, double eta, double xsi)
{
    return  ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetChordPoint(eta, xsi);
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

double CCPACSWing::GetReferenceArea()
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
    }
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
    if (!wingHelper->HasShape()) {
        LOG(WARNING) << "The wing seems empty.";
        return 0;
    }

    Bnd_Box boundingBox;

    for (int i = 1; i <= GetSegmentCount(); ++i) {
        CCPACSWingSegment& segment = GetSegment(i);
        TopoDS_Shape segmentShape  = segment.GetLoft()->Shape();
        BRepBndLib::Add(segmentShape, boundingBox);
        if (GetSymmetryAxis() != TIGL_NO_SYMMETRY) {
            TopoDS_Shape segmentMirroredShape = segment.GetMirroredLoft()->Shape();
            BRepBndLib::Add(segmentMirroredShape, boundingBox);
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

    for (int i = 1; i <= GetSegmentCount(); ++i) {
        CCPACSWingSegment& segment = GetSegment(i);
        TopoDS_Shape segmentShape  = segment.GetLoft()->Shape();
        BRepBndLib::Add(segmentShape, boundingBox);
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
double CCPACSWing::GetAspectRatio()
{
    if ( isNear(GetReferenceArea(),0) ) {
        LOG(WARNING) << "Wing area is close to zero, thus the AR is not computed and 0 is returned.";
        return 0;
    }
    return 2.0*pow_int(GetWingHalfSpan(),2)/GetReferenceArea();
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
    return *guideCurves;
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


void CCPACSWing::BuildGuideCurveWires(TopoDS_Compound& cache) const
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
    cache = connector.GetConnectedGuideCurves();
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
    for (int e = 1; e < orderedUIDs.size(); e++) { // start 1 -> the root section should not change
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
    for (int e = 1; e < orderedUIDs.size(); e++) { // start 1 -> the root section should not change
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

    for (int i = 0; i < orderedUIDs.size(); i++) {
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


void CCPACSWing::SetHalfSpanKeepAR(double newHalfSpan)
{

    if ( newHalfSpan <= 0 ) {
        throw  CTiglError("Invalid input! The given span must be higher than 0.");
    }

    double oldSpan = GetWingHalfSpan();
    double scaleF = newHalfSpan/oldSpan;
    Scale(scaleF);
}

void CCPACSWing::CreateNewConnectedElementBetween(std::string startElementUID, std::string endElementUID)
{

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

            GetSegments().Invalidate(); // to reorder the segment if needed.

        }
}


std::vector<std::string> CCPACSWing::GetOrderedConnectedElement()
{
    return wingHelper->GetElementUIDsInOrder();
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

} // end namespace tigl
