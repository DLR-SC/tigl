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
#include "CCPACSWingSection.h"
#include "CCPACSConfiguration.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWings.h"
#include "CCPACSModel.h"
#include "CTiglError.h"
#include "tiglcommonfunctions.h"
#include "TixiSaveExt.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

namespace tigl
{

namespace
{
    inline double max(double a, double b)
    {
        return a > b? a : b;
    }
    
    TopoDS_Wire transformToWingCoords(const tigl::CCPACSWingConnection& wingConnection, const TopoDS_Wire& origWire)
    {
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

    // Set the face traits
    void SetFaceTraits (PNamedShape loft, unsigned int nSegments) 
    { 
        // designated names of the faces
        std::vector<std::string> names(3);
        names[0]="Bottom";
        names[1]="Top";
        names[2]="TrailingEdge";
        std::vector<std::string> endnames(2);
        endnames[0]="Inside";
        endnames[1]="Outside";

        // map of faces
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(loft->Shape(),   TopAbs_FACE, map);

        unsigned int nFaces = map.Extent();
        // check if number of faces without inside and outside surface (nFaces-2) 
        // is a multiple of 2 (without Trailing Edges) or 3 (with Trailing Edges)
        if (!((nFaces-2)/nSegments == 2 || (nFaces-2)/nSegments == 3) || nFaces < 4) {
            LOG(ERROR) << "CCPACSWing: Unable to determine wing face names from wing loft.";
            return;
        }
        // remove trailing edge name if there is no trailing edge
        if ((nFaces-2)/nSegments == 2) {
            names.erase(names.begin()+2);
        }
        // assign "Top" and "Bottom" to face traits
        for (unsigned int i = 0; i < nFaces-2; i++) {
            CFaceTraits traits = loft->GetFaceTraits(i);
            traits.SetName(names[i%names.size()].c_str());
            loft->SetFaceTraits(i, traits);
        }
        // assign "Inside" and "Outside" to face traits
        for (unsigned int i = nFaces-2; i < nFaces; i++) {
            CFaceTraits traits = loft->GetFaceTraits(i);
            traits.SetName(endnames[i-nFaces+2].c_str());
            loft->SetFaceTraits(i, traits);
        }
    }
}


// Constructor
CCPACSWing::CCPACSWing(CCPACSConfiguration* config)
    : generated::CPACSWing(config->GetWings())
    , CTiglAbstractPhysicalComponent(m_transformation)
    , configuration(config)
    , rebuildFusedSegments(true)
    , rebuildFusedSegWEdge(true)
    , rebuildShells(true)
{
    Cleanup();
}

CCPACSWing::CCPACSWing(CCPACSWings* parent)
    : generated::CPACSWing(parent)
    , CTiglAbstractPhysicalComponent(m_transformation)
    , configuration(&parent->GetParent<CCPACSModel>()->GetConfiguration())
    , rebuildFusedSegments(true)
    , rebuildFusedSegWEdge(true)
    , rebuildShells(true) {
    Cleanup();
}
CCPACSWing::CCPACSWing(generated::CPACSRotorBlades* parent)
    : generated::CPACSWing(parent)
    , CTiglAbstractPhysicalComponent(m_transformation) {
    throw std::logic_error("Instantiating CCPACSWing with CPACSRotorBlades as parent is not implemented");
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
    m_segments.Invalidate();
    if (m_positionings)
        m_positionings->Invalidate();
    if (m_componentSegments)
        m_componentSegments->Invalidate();
}

// Cleanup routine
void CCPACSWing::Cleanup()
{
    m_name = "";
    m_description = "";

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

//// Builds transformation matrix for the wing
//void CCPACSWing::BuildMatrix()
//{
//    transformation.SetIdentity();
//
//    // Step 1: scale the wing around the orign
//    transformation.AddScaling(scaling.x, scaling.y, scaling.z);
//
//    // Step 2: rotate the wing
//    // Step 2a: rotate the wing around z (yaw   += right tip forward)
//    transformation.AddRotationZ(rotation.z);
//    // Step 2b: rotate the wing around y (pitch += nose up)
//    transformation.AddRotationY(rotation.y);
//    // Step 2c: rotate the wing around x (roll  += right tip up)
//    transformation.AddRotationX(rotation.x);
//
//    // Step 3: translate the rotated wing into its position
//    transformation.AddTranslation(translation.x, translation.y, translation.z);
//
//    //backTransformation = transformation.Inverted();
//}

// Update internal wing data
void CCPACSWing::Update()
{
    if (!invalidated) {
        return;
    }

    //BuildMatrix();
    invalidated = false;
    rebuildFusedSegments = true;    // forces a rebuild of all segments with regards to the updated translation
    rebuildShells = true;
}

// Read CPACS wing element
void CCPACSWing::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath)
{
    Cleanup();
    generated::CPACSWing::ReadCPACS(tixiHandle, wingXPath);

    // Register ourself at the unique id manager
    configuration->GetUIDManager().AddUID(m_uID, this);

    Update();
}

const std::string& CCPACSWing::GetUID() const {
    return generated::CPACSWing::GetUID();
}

void CCPACSWing::SetUID(const std::string& uid) {
    generated::CPACSWing::SetUID(uid);
}

TiglSymmetryAxis CCPACSWing::GetSymmetryAxis() {
    return *m_symmetry;
}

void CCPACSWing::SetSymmetryAxis(const TiglSymmetryAxis& axis) {
    m_symmetry = axis;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSWing::GetConfiguration() const
{
    return *configuration;
}

// Get section count
int CCPACSWing::GetSectionCount() const
{
    return static_cast<int>(m_sections.GetSection().size());
}

// Returns the section for a given index
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

// Returns the segment for a given uid
CCPACSWingSegment& CCPACSWing::GetSegment(std::string uid)
{
    return m_segments.GetSegment(uid);
}

// Get componentSegment count
int CCPACSWing::GetComponentSegmentCount()
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

// Returns the segment for a given uid
CCPACSWingComponentSegment& CCPACSWing::GetComponentSegment(std::string uid)
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
std::string CCPACSWing::GetShortShapeName() 
{
    unsigned int windex = 0;
    for (int i = 1; i <= GetConfiguration().GetWingCount(); ++i) {
        tigl::CCPACSWing& w = GetConfiguration().GetWing(i);
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
PNamedShape CCPACSWing::BuildLoft()
{
    return BuildFusedSegments(true);
}

// Builds a fused shape of all wing segments
PNamedShape CCPACSWing::BuildFusedSegments(bool splitWingInUpperAndLower)
{
    //@todo: this probably works only if the wings does not split somewere
    BRepOffsetAPI_ThruSections generator(Standard_True, Standard_True, Precision::Confusion() );

    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        TopoDS_Wire startWire = m_segments.GetSegment(i).GetInnerWire();
        generator.AddWire(startWire);
    }

    TopoDS_Wire endWire = m_segments.GetSegment(m_segments.GetSegmentCount()).GetOuterWire();
    generator.AddWire(endWire);

    generator.CheckCompatibility(Standard_False);
    generator.Build();
        
    TopoDS_Shape loftShape = generator.Shape();
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft, m_segments.GetSegmentCount());
    return loft;
}
    
// Builds a fused shape of all wing segments
void CCPACSWing::BuildUpperLowerShells()
{
    //@todo: this probably works only if the wings does not split somewere
    BRepOffsetAPI_ThruSections generatorUp(Standard_False, Standard_True, Precision::Confusion() );
    BRepOffsetAPI_ThruSections generatorLow(Standard_False, Standard_True, Precision::Confusion() );

    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        CCPACSWingConnection& startConnection = m_segments.GetSegment(i).GetInnerConnection();
        CCPACSWingProfile& startProfile = startConnection.GetProfile();
        TopoDS_Wire upperWire, lowerWire;
        upperWire = TopoDS::Wire(transformToWingCoords(startConnection, BRepBuilderAPI_MakeWire(startProfile.GetUpperWire())));
        lowerWire = TopoDS::Wire(transformToWingCoords(startConnection, BRepBuilderAPI_MakeWire(startProfile.GetLowerWire())));
        generatorUp.AddWire(upperWire);
        generatorLow.AddWire(lowerWire);
    }

    CCPACSWingConnection& endConnection = m_segments.GetSegment(m_segments.GetSegmentCount()).GetOuterConnection();
    CCPACSWingProfile& endProfile = endConnection.GetProfile();
    TopoDS_Wire endUpWire, endLowWire;

    endUpWire  = TopoDS::Wire(transformToWingCoords(endConnection, BRepBuilderAPI_MakeWire(endProfile.GetUpperWire())));
    endLowWire = TopoDS::Wire(transformToWingCoords(endConnection, BRepBuilderAPI_MakeWire(endProfile.GetLowerWire())));

    generatorUp.AddWire(endUpWire);
    generatorLow.AddWire(endLowWire);
    generatorLow.Build();
    generatorUp.Build();
    upperShape = GetWingTransformation().Transform(generatorUp.Shape());
    lowerShape = GetWingTransformation().Transform(generatorLow.Shape());
}


// Gets the wing transformation (original wing implementation, but see GetTransformation)
CTiglTransformation CCPACSWing::GetWingTransformation()
{
    Update();   // create new transformation matrix if scaling, rotation or translation was changed
    return m_transformation.AsTransformation();
}

// Get the positioning transformation for a given section-uid
CTiglTransformation CCPACSWing::GetPositioningTransformation(std::string sectionUID)
{
    return m_positionings->GetPositioningTransformation(sectionUID);
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
double CCPACSWing::GetVolume()
{
    const TopoDS_Shape& fusedSegments = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedSegments, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Get the Transformation object (general interface implementation)
CTiglTransformation CCPACSWing::GetTransformation()
{
    return GetWingTransformation();
}

// Sets the Transformation object
void CCPACSWing::Translate(CTiglPoint trans)
{
    CTiglAbstractGeometricComponent::Translate(trans);
    invalidated = true;
    m_segments.Invalidate();
    if(m_componentSegments)
        m_componentSegments->Invalidate();
    Update();
}

// Setter for translation
void CCPACSWing::SetTranslation(const CTiglPoint& translation)
{
    m_transformation.SetTranslation(translation);
    invalidated = true;
    // TODO: check whether we have to invalidate segments and componentsegments
    Update();
}

// Setter for rotation
void CCPACSWing::SetRotation(const CTiglPoint& rotation)
{
    m_transformation.SetRotation(rotation);
    invalidated = true;
    // TODO: check whether we have to invalidate segments and componentsegments
    Update();
}

// Setter for scaling
void CCPACSWing::SetScaling(const CTiglPoint& scaling)
{
    m_transformation.SetScaling(scaling);
    invalidated = true;
    // TODO: check whether we have to invalidate segments and componentsegments
    Update();
}

// Returns the surface area of this wing
double CCPACSWing::GetSurfaceArea()
{
    const TopoDS_Shape& fusedSegments = GetLoft()->Shape();

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
    const TopoDS_Shape& loft = GetLoft()->Shape();

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
        for (int i = 1; i <= GetSegmentCount(); ++i) {
            const TopoDS_Shape& segmentShape = GetSegment(i).GetLoft()->Shape();
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
            return xmax-ymin;
            break;
        default:
            return ymax-ymin;
        }
    }
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
    for (int iSeg = 1; iSeg <= GetSegmentCount(); ++iSeg) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&) GetSegment(iSeg);
        if (segment.GetIsOn(point) == true) {
            segmentFound = iSeg;
            break;
        }
    }

    if (segmentFound <= 0) {
        return -1;
    }

    CCPACSWingSegment& segment = (CCPACSWingSegment&) GetSegment(segmentFound);
    segment.GetEtaXsi(point, eta, xsi);

    // TODO: do we need that here?
    onTop = segment.GetIsOnTop(point);

    return segmentFound;
}

// Get the guide curve with a given UID
const CCPACSGuideCurve& CCPACSWing::GetGuideCurve(std::string uid)
{
    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        CCPACSWingSegment& segment = m_segments.GetSegment(i);
        if (segment.GuideCurveExists(uid)) {
            return segment.GetGuideCurve(uid);
        }
    }
    throw tigl::CTiglError("Error: Guide Curve with UID " + uid + " does not exists", TIGL_ERROR);
}

// Getter for positionings
const CCPACSPositionings& CCPACSWing::GetPositionings()
{
    return *m_positionings;
}


} // end namespace tigl
