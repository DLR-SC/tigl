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
#include "CCPACSWingSegment.h"
#include "CTiglError.h"
#include "tiglcommonfunctions.h"

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

    inline double min(double a, double b)
    {
        return a < b? a : b;
    }
    
    TopoDS_Wire transformToWingCoords(const tigl::CTiglTransformation& wingTransform, const tigl::CCPACSWingConnection& wingConnection, const TopoDS_Wire& origWire)
    {
        // Do section element transformations
        tigl::CTiglTransformation trafo = wingConnection.GetSectionElementTransformation();

        // Do section transformations
        trafo.PreMultiply(wingConnection.GetSectionTransformation());

        // Do positioning transformations (positioning of sections)
        trafo.PreMultiply(wingConnection.GetPositioningTransformation());

        trafo.PreMultiply(wingTransform);

        TopoDS_Shape resultWire = trafo.Transform(origWire);

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
    : segments(this)
    , componentSegments(this)
    , configuration(config)
    , rebuildFusedSegments(true)
    , rebuildFusedSegWEdge(true)
    , rebuildShells(true)
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
    transformation.reset();

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Update internal wing data
void CCPACSWing::Update(void)
{
    if (!invalidated) {
        return;
    }

    transformation.updateMatrix();
    invalidated = false;
    rebuildFusedSegments = true;    // forces a rebuild of all segments with regards to the updated translation
    rebuildShells = true;
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
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name          = ptrName;
    }

    // Get attribute "uid"
    char* ptrUID = NULL;
    tempString   = "uID";
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get subelement "parent_uid"
    char* ptrParentUID = NULL;
    tempString         = wingXPath + "/parentUID";
    elementPath        = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS &&
        tixiGetTextElement(tixiHandle, elementPath, &ptrParentUID) == SUCCESS) {

        SetParentUID(ptrParentUID);
    }

    // Get Transformation
    transformation.ReadCPACS(tixiHandle, wingXPath);

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
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS) {
        SetSymmetryAxis(ptrSym);
    }

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


// Gets the loft of the whole wing with modeled leading edge.
TopoDS_Shape & CCPACSWing::GetLoftWithLeadingEdge(void)
{
    if (rebuildFusedSegWEdge) {
        fusedSegmentWithEdge = BuildFusedSegments(true)->Shape();
    }
    rebuildFusedSegWEdge = false;
    return fusedSegmentWithEdge;
}
    
// Gets the loft of the whole wing.
TopoDS_Shape & CCPACSWing::GetUpperShape(void)
{
    if (rebuildShells) {
        BuildUpperLowerShells();
    }
    rebuildShells = false;
    return upperShape;
}
    
// Gets the loft of the whole wing.
TopoDS_Shape & CCPACSWing::GetLowerShape(void)
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

    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        TopoDS_Wire startWire = segments.GetSegment(i).GetInnerWire();
        generator.AddWire(startWire);
    }

    TopoDS_Wire endWire = segments.GetSegment(segments.GetSegmentCount()).GetOuterWire();
    generator.AddWire(endWire);

    generator.CheckCompatibility(Standard_False);
    generator.Build();
        
    TopoDS_Shape loftShape = generator.Shape();
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft, segments.GetSegmentCount());
    return loft;
}
    
// Builds a fused shape of all wing segments
void CCPACSWing::BuildUpperLowerShells()
{
    //@todo: this probably works only if the wings does not split somewere
    BRepOffsetAPI_ThruSections generatorUp(Standard_False, Standard_True, Precision::Confusion() );
    BRepOffsetAPI_ThruSections generatorLow(Standard_False, Standard_True, Precision::Confusion() );

    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        CCPACSWingConnection& startConnection = segments.GetSegment(i).GetInnerConnection();
        CCPACSWingProfile& startProfile = startConnection.GetProfile();
        TopoDS_Wire upperWire, lowerWire;
        upperWire = TopoDS::Wire(transformToWingCoords(GetWingTransformation(), startConnection, BRepBuilderAPI_MakeWire(startProfile.GetUpperWire())));
        lowerWire = TopoDS::Wire(transformToWingCoords(GetWingTransformation(), startConnection, BRepBuilderAPI_MakeWire(startProfile.GetLowerWire())));
        generatorUp.AddWire(upperWire);
        generatorLow.AddWire(lowerWire);
    }

    CCPACSWingConnection& endConnection = segments.GetSegment(segments.GetSegmentCount()).GetOuterConnection();
    CCPACSWingProfile& endProfile = endConnection.GetProfile();
    TopoDS_Wire endUpWire, endLowWire;

    endUpWire  = TopoDS::Wire(transformToWingCoords(GetWingTransformation(), endConnection, BRepBuilderAPI_MakeWire(endProfile.GetUpperWire())));
    endLowWire = TopoDS::Wire(transformToWingCoords(GetWingTransformation(), endConnection, BRepBuilderAPI_MakeWire(endProfile.GetLowerWire())));

    generatorUp.AddWire(endUpWire);
    generatorLow.AddWire(endLowWire);
    generatorLow.Build();
    generatorUp.Build();
    upperShape = generatorUp.Shape();
    lowerShape = generatorLow.Shape();
}


// Gets the wing transformation (original wing implementation, but see GetTransformation)
CTiglTransformation CCPACSWing::GetWingTransformation(void)
{
    return transformation.getTransformationMatrix();
}

// Get the positioning transformation for a given section-uid
CTiglTransformation CCPACSWing::GetPositioningTransformation(std::string sectionUID)
{
    return positionings.GetPositioningTransformation(sectionUID);
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
    const TopoDS_Shape& fusedSegments = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedSegments, System);
    double myVolume = System.Mass();
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

// Returns the surface area of this wing
double CCPACSWing::GetSurfaceArea(void)
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

    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        refArea += segments.GetSegment(i).GetReferenceArea(symPlane);
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
    return segments.GetSegment(index).GetLowerSurface();
}

// Returns the upper Surface of a Segment
Handle(Geom_Surface) CCPACSWing::GetUpperSegmentSurface(int index)
{
    return segments.GetSegment(index).GetUpperSurface();
}

// sets the symmetry plane for all childs, segments and component segments
void CCPACSWing::SetSymmetryAxis(const std::string& axis)
{
    CTiglAbstractGeometricComponent::SetSymmetryAxis(axis);

    for (int i = 1; i <= segments.GetSegmentCount(); ++i) {
        CCPACSWingSegment& segment = segments.GetSegment(i);
        segment.SetSymmetryAxis(axis);
    }

    for (int i = 1; i <= componentSegments.GetComponentSegmentCount(); ++i) {
        CCPACSWingComponentSegment& compSeg = componentSegments.GetComponentSegment(i);
        compSeg.SetSymmetryAxis(axis);
    }
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
            return xmax-xmin;
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

    for (int i = 1; i <= segments.GetSegmentCount(); ++i) {
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
CCPACSGuideCurve& CCPACSWing::GetGuideCurve(std::string uid)
{
    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        CCPACSWingSegment& segment = segments.GetSegment(i);
        if (segment.GuideCurveExists(uid)) {
            return segment.GetGuideCurve(uid);
        }
    }
    throw tigl::CTiglError("Error: Guide Curve with UID " + uid + " does not exists", TIGL_ERROR);
}

} // end namespace tigl
