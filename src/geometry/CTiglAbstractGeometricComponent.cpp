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
* @brief  Partial implementation of the ITiglGeometricComponent interface.
*/

#include "CTiglAbstractGeometricComponent.h"
#include "CTiglError.h"
#include "CTiglLogging.h"

// OCCT defines
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

namespace tigl 
{

// Constructor
CTiglAbstractGeometricComponent::CTiglAbstractGeometricComponent(void)
    : myUID("")
    , mySymmetryAxis(TIGL_NO_SYMMETRY)
{
    Reset();
}

void CTiglAbstractGeometricComponent::Reset()
{
    SetUID("");
    mySymmetryAxis = TIGL_NO_SYMMETRY;
    transformation.SetIdentity();
    backTransformation.SetIdentity();
    translation = CTiglPoint(0.,0.,0.);
    scaling     = CTiglPoint(1.,1.,1.);
    rotation    = CTiglPoint(0.,0.,0.);
    translationType = ABS_LOCAL;
}

// Destructor
CTiglAbstractGeometricComponent::~CTiglAbstractGeometricComponent(void)
{
}

// Gets the component uid
const std::string &CTiglAbstractGeometricComponent::GetUID(void) const
{
    return myUID;
}

// Sets the component uid
void CTiglAbstractGeometricComponent::SetUID(const std::string& uid)
{
    myUID = uid;
}

// Gets symmetry axis
TiglSymmetryAxis CTiglAbstractGeometricComponent::GetSymmetryAxis(void)
{
    return mySymmetryAxis;
}

// [[CAS_AES]] Gets symmetry axis as string
const char* CTiglAbstractGeometricComponent::GetSymmetryAxisString(void) const
{
    if (mySymmetryAxis == TIGL_X_Z_PLANE) {
        return "x-z-plane";
    } else if (mySymmetryAxis == TIGL_X_Y_PLANE) {
        return "x-y-plane";
    } else if (mySymmetryAxis == TIGL_Y_Z_PLANE) {
        return "y-z-plane";
    } else {
        return "";
    }
}

// Gets symmetry axis
void CTiglAbstractGeometricComponent::SetSymmetryAxis(const std::string& axis)
{
    if (axis == "x-z-plane") {
        mySymmetryAxis = TIGL_X_Z_PLANE;
    } else if (axis == "x-y-plane") {
        mySymmetryAxis = TIGL_X_Y_PLANE;
    } else if (axis == "y-z-plane") {
        mySymmetryAxis = TIGL_Y_Z_PLANE;
    } else {
        mySymmetryAxis = TIGL_NO_SYMMETRY;
    }
}

CTiglTransformation CTiglAbstractGeometricComponent::GetTransformation()
{
    return transformation;
}

CTiglPoint CTiglAbstractGeometricComponent::GetTranslation() const
{
    return translation;
}

ECPACSTranslationType CTiglAbstractGeometricComponent::GetTranslationType(void) const
{
    return translationType;
}

// [[CAS_AES]] added getter for rotation
CTiglPoint CTiglAbstractGeometricComponent::GetRotation() const
{
    return rotation;
}

// [[CAS_AES]] added getter for scaling
CTiglPoint CTiglAbstractGeometricComponent::GetScaling() const
{
    return scaling;
}

void CTiglAbstractGeometricComponent::Translate(CTiglPoint trans)
{
    translation.x += trans.x;
    translation.y += trans.y;
    translation.z += trans.z;
}

PNamedShape CTiglAbstractGeometricComponent::GetLoft(void)
{
    if (!(loft)) {
#ifdef DEBUG
        LOG(INFO) << "Building loft " << GetUID();
#endif
        loft = BuildLoft();
    }
    return loft;
}

PNamedShape CTiglAbstractGeometricComponent::GetMirroredLoft(void)
{
    if (mySymmetryAxis == TIGL_NO_SYMMETRY) {
        PNamedShape nullShape;
        nullShape.reset();
        return nullShape;
    }

    gp_Ax2 mirrorPlane;
    if (mySymmetryAxis == TIGL_X_Z_PLANE) {
        mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0.,1.,0.));
    }
    else if (mySymmetryAxis == TIGL_X_Y_PLANE) {
        mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0.,0.,1.));
    }
    else if (mySymmetryAxis == TIGL_Y_Z_PLANE) {
        mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(1.,0.,0.));
    }

    gp_Trsf theTransformation;
    theTransformation.SetMirror(mirrorPlane);
    BRepBuilderAPI_Transform myBRepTransformation(GetLoft()->Shape(), theTransformation);
    std::string mirrorName = GetLoft()->Name();
    mirrorName += "M";
    std::string mirrorShortName = GetLoft()->ShortName();
    mirrorShortName += "M";
    TopoDS_Shape mirroredShape = myBRepTransformation.Shape();
    
    PNamedShape mirroredPNamedShape(new CNamedShape(*GetLoft()));
    mirroredPNamedShape->SetShape(mirroredShape);
    mirroredPNamedShape->SetName(mirrorName.c_str());
    mirroredPNamedShape->SetShortName(mirrorShortName.c_str());
    return mirroredPNamedShape;
}

bool CTiglAbstractGeometricComponent::GetIsOn(const gp_Pnt& pnt) 
{
    const TopoDS_Shape& segmentShape = GetLoft()->Shape();

    // fast check with bounding box
    Bnd_Box boundingBox;
    BRepBndLib::Add(segmentShape, boundingBox);

    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    if (pnt.X() < xmin || pnt.X() > xmax ||
        pnt.Y() < ymin || pnt.Y() > ymax ||
        pnt.Z() < zmin || pnt.Z() > zmax) {

        return false;
    }

    double tolerance = 0.03; // 3cm

    BRepClass3d_SolidClassifier classifier;
    classifier.Load(segmentShape);
    classifier.Perform(pnt, tolerance);
    if ((classifier.State() == TopAbs_IN) || (classifier.State() == TopAbs_ON)) {
        return true;
    }
    else {
        return false;
    }
}

bool CTiglAbstractGeometricComponent::GetIsOnMirrored(const gp_Pnt& pnt) 
{
    if (mySymmetryAxis == TIGL_NO_SYMMETRY) {
        return false;
    }

    gp_Pnt mirroredPnt(pnt);
    if (mySymmetryAxis == TIGL_X_Z_PLANE) {
        mirroredPnt.SetY(-mirroredPnt.Y());
    }
    else if (mySymmetryAxis == TIGL_X_Y_PLANE) {
        mirroredPnt.SetZ(-mirroredPnt.Z());
    }
    else if (mySymmetryAxis == TIGL_Y_Z_PLANE) {
        mirroredPnt.SetX(-mirroredPnt.X());
    }
    
    return GetIsOn(mirroredPnt);
}

} // end namespace tigl
