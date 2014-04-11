/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-11 Philipp Kunze <Philipp.Kunze@dlr.de>
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
* @brief  Implementation of CPACS rotor handling routines.
*/

#include <iostream>

#include "CCPACSRotor.h"
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
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "BRepPrimAPI_MakeRevol.hxx"

#ifdef TIGL_USE_XCAF
#include "XCAFDoc_ShapeTool.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "TDataStd_Name.hxx"
#include "TDataXtd_Shape.hxx"
#endif

namespace
{
inline double max(double a, double b)
{
    return a > b? a : b;
}

TopoDS_Wire transformToRotorCoords(const tigl::CCPACSWingConnection& wingConnection, const TopoDS_Wire& origWire)
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
        throw tigl::CTiglError("Error: Wrong shape type in CCPACSRotor::transformToAbsCoords", TIGL_ERROR);
    }

    return TopoDS::Wire(resultWire);
}
}


namespace tigl
{

// Constructor
CCPACSRotor::CCPACSRotor(CCPACSConfiguration* config)
    : configuration(config)
    , rotorHub(config)
//TODO:        , rotorBlades(this)
    , rebuildGeometry(true)
{
    Cleanup();
}

// Destructor
CCPACSRotor::~CCPACSRotor(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSRotor::Invalidate(void)
{
    invalidated = true;
    rotorHub.Invalidate();
//TODO:        rotorBlades.Invalidate();
}

// Cleanup routine
void CCPACSRotor::Cleanup(void)
{
    name = "";
    description = "";
    transformation.SetIdentity();
    translation = CTiglPoint(0.0, 0.0, 0.0);
    scaling     = CTiglPoint(1.0, 1.0, 1.0);
    rotation    = CTiglPoint(0.0, 0.0, 0.0);
    type = TIGLROTOR_MAIN_ROTOR;
    nominalRotationsPerMinute = 0.;

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Builds transformation matrix for the rotor
void CCPACSRotor::BuildMatrix(void)
{
    transformation.SetIdentity();

    // Step 1: scale the rotor around the orign
    transformation.AddScaling(scaling.x, scaling.y, scaling.z);

    // Step 2: rotate the rotor
    // Step 2a: rotate the rotor around z (yaw   += right tip forward)
    transformation.AddRotationZ(rotation.z);
    // Step 2b: rotate the rotor around y (pitch += nose up)
    transformation.AddRotationY(rotation.y);
    // Step 2c: rotate the rotor around x (roll  += right tip up)
    transformation.AddRotationX(rotation.x);

    // Step 3: translate the rotated rotor into its position
    transformation.AddTranslation(translation.x, translation.y, translation.z);

    backTransformation = transformation.Inverted();
}

// Update internal rotor data
void CCPACSRotor::Update(void)
{
    if (!invalidated) {
        return;
    }

    BuildMatrix();
    invalidated = false;
    rebuildGeometry = true;
}

// Read CPACS rotor element
void CCPACSRotor::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get attribute "uid"
    char* ptrUID = NULL;
    tempString   = "uID";
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(rotorXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = rotorXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name      = ptrName;
    }

    // Get subelement "description"
    char* ptrDescription = NULL;
    tempString           = rotorXPath + "/description";
    elementPath          = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrDescription) == SUCCESS) {
        description      = ptrDescription;
    }

    // Get subelement "parent_uid"
    char* ptrParentUID = NULL;
    tempString         = rotorXPath + "/parentUID";
    elementPath        = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS &&
            tixiGetTextElement(tixiHandle, elementPath, &ptrParentUID) == SUCCESS) {
        SetParentUID(ptrParentUID);
    }


    // Get subelement "/transformation/translation"
    tempString  = rotorXPath + "/transformation/translation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(translation.x), &(translation.y), &(translation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <translation/> in CCPACSRotor::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/scaling"
    tempString  = rotorXPath + "/transformation/scaling";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(scaling.x), &(scaling.y), &(scaling.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <scaling/> in CCPACSRotor::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/rotation"
    tempString  = rotorXPath + "/transformation/rotation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(rotation.x), &(rotation.y), &(rotation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <rotation/> in CCPACSRotor::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "type"
    char* ptrType = NULL;
    tempString    = rotorXPath + "/type";
    if (tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrType) == SUCCESS) {
        std::string strType(ptrType);
        if (strType == "mainRotor") {
            type = TIGLROTOR_MAIN_ROTOR;
        }
        else if (strType == "tailRotor") {
            type = TIGLROTOR_TAIL_ROTOR;
        }
        else if (strType == "fenestron") {
            type = TIGLROTOR_FENESTRON;
        }
        else if (strType == "propeller") {
            type = TIGLROTOR_PROPELLER;
        }
        else {
            throw CTiglError("Error: XML error while reading <type> in CCPACSRotor::ReadCPACS: illegal value", TIGL_XML_ERROR);
        }
    }
    else {   // default value: MAIN_ROTOR
        type = TIGLROTOR_MAIN_ROTOR;
    }

    // Get subelement "nominalRotationsPerMinute"
    double tmpDouble = 0.;
    tempString       = rotorXPath + "/nominalRotationsPerMinute";
    elementPath      = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle,elementPath)== SUCCESS)
        if (tixiGetDoubleElement(tixiHandle, elementPath, &tmpDouble) == SUCCESS) {
            nominalRotationsPerMinute = tmpDouble;
        }

    // Get subelement "rotorHub"
    rotorHub.ReadCPACS(tixiHandle, rotorXPath + "/rotorHub");

    // Register ourself at the unique id manager
    configuration->GetUIDManager().AddUID(ptrUID, this);

    // Get symmetry axis attribute, has to be done, when rotor blades are build
    char* ptrSym = NULL;
    tempString   = "symmetry";
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(rotorXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS) {
        SetSymmetryAxis(ptrSym);
    }

    Update();
}

// Returns the name of the rotor
const std::string& CCPACSRotor::GetName(void) const
{
    return name;
}

// Returns the description of the rotor
const std::string& CCPACSRotor::GetDescription(void) const
{
    return description;
}

// Get the Transformation object
CTiglTransformation CCPACSRotor::GetTransformation(void)
{
    Update();   // create new transformation matrix if scaling, rotation or translation was changed
    return transformation;
}

// Sets the Transformation object
void CCPACSRotor::Translate(CTiglPoint trans)
{
    CTiglAbstractGeometricComponent::Translate(trans);
    Invalidate();
    Update();
}

// Get Translation
CTiglPoint CCPACSRotor::GetTranslation(void)
{
    return translation;
}

// Returns the type of the rotor
const TiglRotorType& CCPACSRotor::GetType(void) const
{
    return type;
}

// Returns the nominal rotations per minute (rpm) of the rotor
const double& CCPACSRotor::GetNominalRotationsPerMinute(void) const
{
    return nominalRotationsPerMinute;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotor::GetConfiguration(void) const
{
    return *configuration;
}

// Returns the rotor hub object
const CCPACSRotorHub& CCPACSRotor::GetRotorHub(void) const
{
    return rotorHub;
}

// Returns the rotor disk geometry
TopoDS_Shape CCPACSRotor::GetRotorDisk(void)
{
    return BuildRotorDisk();
}

/*TODO:
    // Get rotor blade count
    int CCPACSRotor::GetRotorBladeCount(void) const
    {
        return rotorBlades.GetRotorBladeCount();
    }

    // Returns the rotor blade for a given index
    CCPACSRotorBlade& CCPACSRotor::GetRotorBlade(int index) const
    {
        return rotorBlades.GetRotorBlade(index);
    }

#ifdef TIGL_USE_XCAF
    // Get data structure for geometry export
    TDF_Label CCPACSRotor::ExportDataStructure(CCPACSConfiguration &config, Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label)
    {
        TDF_Label rotorLabel = CTiglAbstractPhysicalComponent::ExportDataStructure(config, myAssembly, label);

        // Other (sub)-components
        for (int i=1; i <= rotorBlades.GetRotorBladeCount(); i++) {
            CCPACSRotorBlade& rotorBlade = RotorBlades.GetSegment(i);
            TDF_Label rotorBladeLabel = myAssembly->AddShape(rotorBlade.GetLoft(), false);
            TDataStd_Name::Set (rotorBladeLabel, rotorBlade.GetUID().c_str());
            //TDF_Label& subSegmentLabel = segment.ExportDataStructure(myAssembly, rotorSegmentLabel);
        }

        return rotorLabel;
    }
#endif
*/
// Gets the geometry of the whole rotor. (implementation for abstract base class CTiglAbstractGeometricComponent)
TopoDS_Shape CCPACSRotor::BuildLoft()
{

    double thetaDeg = 0.; // current azimuthal position of the rotor in degrees
    double rotDir = (nominalRotationsPerMinute < 0. ? -1. : 1.); // rotation direction (+1: anti-clockwise, -1: clockwise/french)

    TopoDS_Compound rotorGeometry;
    BRep_Builder aBuilder;

    aBuilder.MakeCompound(rotorGeometry);

    for (int i=0; i<rotorHub.GetRotorBladeAttachmentCount(); ++i) {

        TopoDS_Shape origRotorBlade = configuration->GetWing(rotorHub.GetRotorBladeAttachment(i+1).GetRotorBladeUID()).GetLoft();

        for (int j=0; j<rotorHub.GetRotorBladeAttachment(i+1).GetNumberOfBlades(); ++j) {

            // Create a new instance of the referenced rotor blade
            TopoDS_Shape curRotorBlade = origRotorBlade;

            // Rotor blade transformation chain:
            CTiglTransformation curRotorBladeTransformation;
            // 0. Initialize matrix
            curRotorBladeTransformation.SetIdentity();
            // 1. Rotation around hinges, beginning with the last
            for (int k=rotorHub.GetRotorBladeAttachment(i+1).GetHingeCount()-1; k>=0; --k) {
                CTiglPoint curHingePosition = rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetTranslation();
                TiglRotorHingeType curHingeType = rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetType();
                // a. move to origin
                curRotorBladeTransformation.AddTranslation(-curHingePosition.x, -curHingePosition.y, -curHingePosition.z);
                // b. rotate around hinge axis
                if (curHingeType == TIGLROTORHINGE_PITCH) {
                    curRotorBladeTransformation.AddRotationX( (rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetHingeAngle(thetaDeg)));
                }
                else if (curHingeType == TIGLROTORHINGE_FLAP) {
                    curRotorBladeTransformation.AddRotationY(-(rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetHingeAngle(thetaDeg)));
                }
                else if (curHingeType == TIGLROTORHINGE_LEAD_LAG) {
                    curRotorBladeTransformation.AddRotationZ( (rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetHingeAngle(thetaDeg)));
                }
                // c. move back to origin
                curRotorBladeTransformation.AddTranslation(curHingePosition.x, curHingePosition.y, curHingePosition.z);
                //TODO: account for rotation and scaling of hinge?
            }
            // 2. If the rotation direction is clockwise (e.g. french rotor): mirror rotor blade in x direction
            if (rotDir<0) {
                curRotorBladeTransformation.AddMirroringAtYZPlane();
            }
            // 3. Rotate the rotor blade around z to its azimuth position
            curRotorBladeTransformation.AddRotationZ(rotDir * (thetaDeg + rotorHub.GetRotorBladeAttachment(i+1).GetAzimuthAngle(j+1)));
            // 4. Add rotor transformation
            curRotorBladeTransformation.PreMultiply(transformation);
            // Apply transformations
            curRotorBlade = curRotorBladeTransformation.Transform(curRotorBlade);

            // Add the transformed rotor blade to the rotor assembly
            aBuilder.Add(rotorGeometry,curRotorBlade);
        }
    }

    // Return the generated geometry
    return rotorGeometry;
}

// Draws the rotor disk (only using information of the first attached blade)
TopoDS_Shape CCPACSRotor::BuildRotorDisk()
{

    double thetaDeg = 0.; // current azimuthal position of the rotor in degrees
    double rotDir = (nominalRotationsPerMinute < 0. ? -1. : 1.); // rotation direction (+1: anti-clockwise, -1: clockwise/french)

    TopoDS_Shape rotorDisk;

    // rotorBladeAttachment 1
    int i = 0;

    // Make PolyLine quarterChordLine: BRepBuilderAPI_MakePolygon;
    if (rotorHub.GetRotorBladeAttachmentCount() < 1) {
        return rotorDisk;
    }
    if (rotorHub.GetRotorBladeAttachment(i+1).GetNumberOfBlades() < 1) {
        return rotorDisk;
    }
    CCPACSWing* rotorBlade = &(configuration->GetWing(rotorHub.GetRotorBladeAttachment(i+1).GetRotorBladeUID()));
    if (rotorBlade->GetSegmentCount() < 1) {
        return rotorDisk;
    }
    BRepBuilderAPI_MakePolygon P;
    gp_Pnt upperLePoint = rotorBlade->GetUpperPoint(1, 0., 0.);
    gp_Pnt lowerLePoint = rotorBlade->GetLowerPoint(1, 0., 0.);
    gp_Pnt upperTePoint = rotorBlade->GetUpperPoint(1, 0., 1.);
    gp_Pnt lowerTePoint = rotorBlade->GetLowerPoint(1, 0., 1.);
    P.Add(gp_Pnt(0.5*(upperLePoint.X()+lowerLePoint.X()) + 0.125 * (upperTePoint.X()+lowerTePoint.X()-upperLePoint.X()-lowerLePoint.X()),
                 0.5*(upperLePoint.Y()+lowerLePoint.Y()) + 0.125 * (upperTePoint.Y()+lowerTePoint.Y()-upperLePoint.Y()-lowerLePoint.Y()),
                 0.5*(upperLePoint.Z()+lowerLePoint.Z()) + 0.125 * (upperTePoint.Z()+lowerTePoint.Z()-upperLePoint.Z()-lowerLePoint.Z())));
    for (int i=0; i<rotorBlade->GetSegmentCount(); ++i) {
        upperLePoint = rotorBlade->GetUpperPoint(1, 1., 0.);
        lowerLePoint = rotorBlade->GetLowerPoint(1, 1., 0.);
        upperTePoint = rotorBlade->GetUpperPoint(1, 1., 1.);
        lowerTePoint = rotorBlade->GetLowerPoint(1, 1., 1.);
        P.Add(gp_Pnt(0.5*(upperLePoint.X()+lowerLePoint.X()) + 0.125 * (upperTePoint.X()+lowerTePoint.X()-upperLePoint.X()-lowerLePoint.X()),
                     0.5*(upperLePoint.Y()+lowerLePoint.Y()) + 0.125 * (upperTePoint.Y()+lowerTePoint.Y()-upperLePoint.Y()-lowerLePoint.Y()),
                     0.5*(upperLePoint.Z()+lowerLePoint.Z()) + 0.125 * (upperTePoint.Z()+lowerTePoint.Z()-upperLePoint.Z()-lowerLePoint.Z())));
    }
    TopoDS_Shape quarterChordLine = P.Shape();

    // rotorBlade 1
    int j=0;

    // Rotor blade transformation chain:
    CTiglTransformation curRotorBladeTransformation;
    // 0. Initialize matrix
    curRotorBladeTransformation.SetIdentity();
    // 1. Rotation around hinges, beginning with the last
    for (int k=rotorHub.GetRotorBladeAttachment(i+1).GetHingeCount()-1; k>=0; --k) {
        CTiglPoint curHingePosition = rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetTranslation();
        TiglRotorHingeType curHingeType = rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetType();
        // a. move to origin
        curRotorBladeTransformation.AddTranslation(-curHingePosition.x, -curHingePosition.y, -curHingePosition.z);
        // b. rotate around hinge axis
        if (curHingeType == TIGLROTORHINGE_PITCH) {
            curRotorBladeTransformation.AddRotationX( (rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetHingeAngle(thetaDeg)));
        }
        else if (curHingeType == TIGLROTORHINGE_FLAP) {
            curRotorBladeTransformation.AddRotationY(-(rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetHingeAngle(thetaDeg)));
        }
        else if (curHingeType == TIGLROTORHINGE_LEAD_LAG) {
            curRotorBladeTransformation.AddRotationZ( (rotorHub.GetRotorBladeAttachment(i+1).GetHinge(k+1).GetHingeAngle(thetaDeg)));
        }
        // c. move back to origin
        curRotorBladeTransformation.AddTranslation(curHingePosition.x, curHingePosition.y, curHingePosition.z);
        //TODO: account for rotation and scaling of hinge?
    }
    // 2. If the rotation direction is clockwise (e.g. french rotor): mirror rotor blade in x direction
    if (rotDir<0) {
        curRotorBladeTransformation.AddMirroringAtYZPlane();
    }
    // 3. Rotate the rotor blade around z to its azimuth position
    curRotorBladeTransformation.AddRotationZ(rotDir * (thetaDeg + rotorHub.GetRotorBladeAttachment(i+1).GetAzimuthAngle(j+1)));
    // Apply transformations
    quarterChordLine = curRotorBladeTransformation.Transform(quarterChordLine);

    // Make surface of Revolution from PolyLine
    gp_Ax1 axis(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.));
    rotorDisk = BRepPrimAPI_MakeRevol(quarterChordLine, axis);

    // Apply rotor transformation
    rotorDisk = transformation.Transform(rotorDisk);

    // Return the generated geometry
    return rotorDisk;
}

/*TODO:
    // Returns the volume of this rotor
    double CCPACSRotor::GetVolume(void)
    {
        TopoDS_Shape& fusedSegments = GetLoft();

        // Calculate volume
        GProp_GProps System;
        BRepGProp::VolumeProperties(fusedSegments, System);
        double myVolume = System.Mass();
        return myVolume;
    }

    // Returns the surface area of this rotor
    double CCPACSRotor::GetSurfaceArea(void)
    {
        TopoDS_Shape& fusedSegments = GetLoft();

        // Calculate surface area
        GProp_GProps System;
        BRepGProp::SurfaceProperties(fusedSegments, System);
        double myArea = System.Mass();
        return myArea;
    }

    // Returns the reference area of this rotor.
    // Here, we always take the reference rotor disk area projected to a plane normal to the rotor hub direction
    double CCPACSRotor::GetReferenceArea()
    {
        double refArea = 0.0;

        for (int i=1; i <= rotorBlades.GetRotorBladeCount(); i++) {
            refArea += rotorBlades.GetSegment(i).GetReferenceArea();
        }
        return refArea;
    }
*/
// sets the symmetry plane for all childs
void CCPACSRotor::SetSymmetryAxis(const std::string& axis)
{
    CTiglAbstractGeometricComponent::SetSymmetryAxis(axis);
    /*TODO?:
            for(int i = 1; i <= rotorBlades.GetSegmentCount(); ++i){
                CCPACSRotorBlade& rotorBlade = rotorBlades.GetSegment(i);
                rotorBlades.SetSymmetryAxis(axis);
            }
    */
}
/*TODO:
    double CCPACSRotor::GetRotorRadius() {
        Bnd_Box boundingBox;
        if (GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
            for (int i = 1; i <= GetRotorBladeCount(); ++i) {
                TopoDS_Shape& segmentShape = GetRotorBlade(i).GetLoft();
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
            for (int i = 1; i <= GetRotorBladeCount(); ++i) {
                CTiglAbstractSegment& segment = GetRotorBlade(i);
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
*/

} // end namespace tigl
