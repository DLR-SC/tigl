/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#include "CTiglSectionElement.h"

#include "tiglmathfunctions.h"
#include "tiglcommonfunctions.h"
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Surface.hxx>
#include <GeomLProp_SLProps.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

tigl::CTiglTransformation tigl::CTiglSectionElement::GetTotalTransformation(TiglCoordinateSystem referenceCS) const
{
    // Do section element transformation on points
    tigl::CTiglTransformation totalTransformation = GetElementTransformation();

    // Do section transformations
    totalTransformation.PreMultiply(GetSectionTransformation());

    // Do positioning transformations
    totalTransformation.PreMultiply(GetPositioningTransformation());

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return totalTransformation;
    case FUSELAGE_COORDINATE_SYSTEM:
        return totalTransformation;
    case GLOBAL_COORDINATE_SYSTEM:
        totalTransformation.PreMultiply(GetParentTransformation());
        return totalTransformation;
    default:
        throw CTiglError("CTiglSectionElement::GetTotalTransformation: Invalid coordinate system passed.");
    }
}

tigl::CTiglPoint tigl::CTiglSectionElement::GetOrigin(TiglCoordinateSystem referenceCS) const
{

    CTiglPoint origin(0, 0, 0);
    return GetTotalTransformation(referenceCS) * origin;
}

tigl::CTiglPoint tigl::CTiglSectionElement::GetCenter(TiglCoordinateSystem referenceCS) const
{
    TopoDS_Wire wire = GetWire(referenceCS);
    return CTiglPoint(GetCenterOfMass(wire).XYZ());
}

double tigl::CTiglSectionElement::GetCircumference(TiglCoordinateSystem referenceCS) const
{
    TopoDS_Wire wire = GetWire(referenceCS);
    GProp_GProps System;
    BRepGProp::LinearProperties(wire, System);
    return System.Mass();
}

double tigl::CTiglSectionElement::GetArea(TiglCoordinateSystem referenceCS) const
{
    TopoDS_Wire wire = GetWire(referenceCS);
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire); // is it to slow?
    GProp_GProps System;
    BRepGProp::SurfaceProperties(face, System);
    return System.Mass();
}

double tigl::CTiglSectionElement::GetHeight(TiglCoordinateSystem referenceCS) const
{

    TopoDS_Wire wire  = GetWire(referenceCS);
    CTiglTransformation toXZPlane = GetRotationToXZPlane(referenceCS);

    wire                    = TopoDS::Wire(toXZPlane.Transform(wire));

    BRepMesh_IncrementalMesh mesh(wire, 0.00001); // tessellate the wire to have a more accurate bounding box.
    Bnd_Box boundingBox;
    BRepBndLib::Add(wire, boundingBox);
    CTiglPoint min, max;
    boundingBox.Get(min.x, min.y, min.z, max.x, max.y, max.z);

    return (max.z - min.z);
}

double tigl::CTiglSectionElement::GetWidth(TiglCoordinateSystem referenceCS) const
{

    TopoDS_Wire wire  = GetWire(referenceCS);
    CTiglTransformation toXZPlane = GetRotationToXZPlane(referenceCS);

    wire                    = TopoDS::Wire(toXZPlane.Transform(wire));

    BRepMesh_IncrementalMesh mesh(wire, 0.00001); // tessellate the wire to have a more accurate bounding box.
    Bnd_Box boundingBox;
    BRepBndLib::Add(wire, boundingBox);
    CTiglPoint min, max;
    boundingBox.Get(min.x, min.y, min.z, max.x, max.y, max.z);

    return (max.x - min.x);
}

void tigl::CTiglSectionElement::SetOrigin(const CTiglPoint& newO, TiglCoordinateSystem referenceCS)
{
    CTiglTransformation total = GetTotalTransformation(referenceCS);
    total.SetTranslation(newO);
    SetTotalTransformation(total,referenceCS);
}

void tigl::CTiglSectionElement::SetCenter(const tigl::CTiglPoint& newCenter, TiglCoordinateSystem referenceCS)
{
    // The center is the center of the airfoil.
    CTiglPoint oldCenter = GetCenter(referenceCS);
    CTiglPoint delta = newCenter - oldCenter;
    tigl::CTiglTransformation  total = GetTotalTransformation(referenceCS);
    total.AddTranslation(delta.x, delta.y, delta.z);
    SetTotalTransformation(total, referenceCS);
}

void tigl::CTiglSectionElement::ScaleCircumference(double scaleFactor, TiglCoordinateSystem referenceCS)
{
    ScaleUniformly(scaleFactor, referenceCS);
}

void tigl::CTiglSectionElement::ScaleUniformly(double scaleFactor, TiglCoordinateSystem referenceCS)
{
    /*
  *  The idea is to bring the element to its center and scale it in the appropriate coordinate system.
  *
  *  For world coordinate we get:
  *
  *  FPSE' = Ti*S*T*FPSE   where T bring the center to world origin and S scale uniformly the profile, i stand for inverse
  *  E' =  Si*Pi*Fi*Ti*S*T*FPSE
  *
  *  For fuselage coordinate we get:
  *  PSE' =  Ti*S*T*PSE   where T bring the center to fuselage origin and S scale uniformly the profile
  *  E' =  Si*Pi*Ti*S*T*PSE
  *
  */

    CTiglTransformation total = GetTotalTransformation(referenceCS);

    CTiglTransformation centerToOriginM, scaleM, newTotal;

    scaleM.SetIdentity();
    scaleM.AddScaling(scaleFactor, scaleFactor, scaleFactor);

    CTiglPoint center = GetCenter(referenceCS);
    centerToOriginM.SetIdentity();
    centerToOriginM.AddTranslation(-center.x, -center.y, -center.z);

    newTotal = centerToOriginM.Inverted() * scaleM * centerToOriginM * total;

    SetTotalTransformation(newTotal, referenceCS);
}

void tigl::CTiglSectionElement::SetWidth(double newWidth, TiglCoordinateSystem referenceCS)
{
    // if we want to set the width to 0
    // we set directly the scaling in the CCPACSTransformation to not loose other information
    if (isNear(newWidth, 0, 0.0001)) {
        CCPACSTransformation& storedElementTransformation = GetElementCCPACSTransformation();
        CTiglPoint elementScaling = storedElementTransformation.getScaling();
        elementScaling.x = 0;
        elementScaling.y = 0;
        storedElementTransformation.setScaling(elementScaling);
        InvalidateParent();
        return;

    }

    double oldWidth = GetWidth(referenceCS);

    // if the current height is zero -> we can not scale it, so we first "reset" the value to a valid state
    if (isNear(oldWidth, 0, 0.0001)) {
        SetElementAndSectionScalingToNoneZero();
        oldWidth = GetWidth(referenceCS);
        if (isNear(oldWidth, 0, 0.0001)) { // desperate case
            throw CTiglError("Impossible to set the scaling to obtain a none zero width!");
        }
    }

    double scaleFactor = newWidth / oldWidth;

    CTiglTransformation scaleX, centerToOrigin;
    scaleX.AddScaling(scaleFactor, 1, 1);
    CTiglPoint center = GetCenter(referenceCS);
    centerToOrigin.AddTranslation(-center.x, -center.y, -center.z);
    CTiglTransformation toXZPlane = GetRotationToXZPlane(referenceCS);

    CTiglTransformation newGlobal = centerToOrigin.Inverted()
                                    * toXZPlane.Inverted()
                                    * scaleX
                                    * toXZPlane
                                    * centerToOrigin
                                    * GetTotalTransformation(referenceCS);

    SetTotalTransformation(newGlobal, referenceCS);
}

void tigl::CTiglSectionElement::SetHeight(double newHeight, TiglCoordinateSystem referenceCS)
{
    // if we want to set the height to 0
    // we set directly the scaling in the CCPACSTransformation to not loose other information
    if (isNear(newHeight, 0, 0.0001)) {
        CCPACSTransformation& storedElementTransformation = GetElementCCPACSTransformation();
        CTiglPoint elementScaling                         = storedElementTransformation.getScaling();
        elementScaling.z                                  = 0;
        storedElementTransformation.setScaling(elementScaling);
        InvalidateParent();
        return;
    }

    double oldHeight = GetHeight(referenceCS);
    CTiglPoint centerPre = GetCenter(referenceCS);
    // if the current height is zero -> we can not scale it, so we first "reset" the value to a valid state
    if (isNear(oldHeight, 0, 0.0001)) {
        SetElementAndSectionScalingToNoneZero();
        oldHeight = GetHeight(referenceCS);
        if (isNear(oldHeight, 0, 0.0001) ) { // desperate case
            throw CTiglError("Impossible to set the scaling to obtain a none zero height!");
        }
    }

    double scaleFactor = newHeight / oldHeight;
    CTiglTransformation scaleZ, centerToOrigin;
    scaleZ.AddScaling(1, 1, scaleFactor);
    CTiglPoint center = GetCenter(referenceCS);
    centerToOrigin.AddTranslation(-center.x, -center.y, -center.z);
    CTiglTransformation toXZPlane = GetRotationToXZPlane(referenceCS);

    CTiglTransformation newGlobal = centerToOrigin.Inverted()
                                    * toXZPlane.Inverted()
                                    * scaleZ
                                    * toXZPlane
                                    * centerToOrigin
                                    * GetTotalTransformation(referenceCS);

    SetTotalTransformation(newGlobal, referenceCS);
}

void tigl::CTiglSectionElement::SetArea(double newArea, TiglCoordinateSystem referenceCS)
{
    // if we want to set the area to 0
    // we set directly the scaling in the CCPACSTransformation to not loose other information
    if (isNear(newArea, 0, 0.0001)) {
        CCPACSTransformation& storedElementTransformation = GetElementCCPACSTransformation();
        storedElementTransformation.setScaling(CTiglPoint(0, 0, 0));
        InvalidateParent();
        return;
    }

    // standard case (we scale uniformly to obtain the wanted area)
    double oldArea = GetArea(referenceCS);

    // if the area is 0 we first "reset" the 0 scaling value to trivial value
    if (isNear(oldArea, 0, 0.0001)) {
        SetElementAndSectionScalingToNoneZero();
        oldArea = GetArea(referenceCS);
        if (isNear(oldArea, 0, 0.0001)) { // desperate case
            throw CTiglError("Impossible to set the scaling to obtain a none zero area!");
        }
    }

    double scaleFactor = sqrt(newArea / oldArea);
    ScaleUniformly(scaleFactor, referenceCS);
}

void tigl::CTiglSectionElement::SetTotalTransformation(const tigl::CTiglTransformation& newTotalTransformation,
                                                       TiglCoordinateSystem referenceCS)
{

    // We have
    // FP'S'E' = G        where E' is the new element transformation,
    //                    and S is the new section transforamtion,
    //                    and P' is the new positioning transforamtion
    //                    and G is the given new total transformation
    //                    and F is the parent transformation
    // P'S'E' = F⁻¹G

    CTiglTransformation newPSE ;

    if ( referenceCS == GLOBAL_COORDINATE_SYSTEM ) {
        newPSE =  GetParentTransformation().Inverted()
                * newTotalTransformation;

    } else if (referenceCS == FUSELAGE_COORDINATE_SYSTEM || referenceCS == WING_COORDINATE_SYSTEM ) {
        newPSE = newTotalTransformation;
    };

    SetPositioningSectionElementTransformation(newPSE);
}

void tigl::CTiglSectionElement::SetPositioningSectionElementTransformation(const tigl::CTiglTransformation &newTransformation)
{

    CTiglPoint scal1, rot1, scal2, rot2, trans;

    newTransformation.DecomposeTRSRS(scal1,rot1,scal2,rot2,trans);

    // set the new transformation matrix in the element
    CCPACSTransformation& storedElementTransformation = GetElementCCPACSTransformation();

    storedElementTransformation.setScaling(scal1);
    storedElementTransformation.setRotation(rot1);
    storedElementTransformation.setTranslation(CTiglPoint(0,0,0));

    CCPACSTransformation& storedSectionTransformation = GetSectionCCPACSTransformation();
    storedSectionTransformation.setScaling(scal2);
    storedSectionTransformation.setRotation(rot2);

    storedSectionTransformation.setTranslation(CTiglPoint(0,0,0));
    CCPACSPositionings& positionings = GetPositionings();
    positionings.SetPositioningTransformation(GetSectionUID(), trans, false );

    InvalidateParent();

}

void tigl::CTiglSectionElement::SetElementAndSectionScalingToNoneZero()
{
    double tolerance = 0.001;
    CCPACSTransformation& storedElementTransformation = GetElementCCPACSTransformation();
    CTiglPoint elementScaling = storedElementTransformation.getScaling();
    if ( isNear(elementScaling.x, 0, tolerance) )  {
        elementScaling.x = 1;
    }
    if ( isNear(elementScaling.y, 0, tolerance) )  {
        elementScaling.y = 1;
    }
    if ( isNear(elementScaling.z, 0, tolerance) )  {
        elementScaling.z = 1;
    }
    storedElementTransformation.setScaling(elementScaling);

    CCPACSTransformation& storedSectionTransformation = GetSectionCCPACSTransformation();
    CTiglPoint sectionScaling = storedSectionTransformation.getScaling();
    if ( isNear(sectionScaling.x, 0, tolerance) )  {
        sectionScaling.x = 1;
    }
    if ( isNear(sectionScaling.y, 0, tolerance) )  {
        sectionScaling.y = 1;
    }
    if ( isNear(sectionScaling.z, 0, tolerance) )  {
        sectionScaling.z = 1;
    }
    storedSectionTransformation.setScaling(sectionScaling);

    InvalidateParent();
}


tigl::CTiglTransformation tigl::CTiglSectionElement::GetRotationToXZPlane(TiglCoordinateSystem referenceCS) const
{
    CTiglPoint normal = GetNormal(referenceCS);
    // get the rotation to have the wire on the XZ plan
    CTiglPoint uY(0,1,0);
    CTiglPoint uZ(0,0,1);
    CTiglPoint o(0,0,0);

    CTiglTransformation rot = CTiglTransformation::GetRotationToAlignAToB(normal, uY);
    CTiglTransformation global = GetTotalTransformation(referenceCS);

    CTiglPoint zP = (rot * global * uZ) - (rot * global * o) ;
    zP.normalize();

    // determine the axis rotation angle to bring zP on (0,0,1)
    CTiglPoint crossProduct = CTiglPoint::cross_prod(zP, uZ);
    double tripleProduct = CTiglPoint::inner_prod(uZ, crossProduct) ;
    double angle = Degrees(acos( CTiglPoint::inner_prod(uZ, zP)) );
    if (tripleProduct < 0 && (!isNear(angle, 0)) ) {
        angle = 360 - angle;
    }
    CTiglTransformation setZP = CTiglTransformation::GetRotationFromAxisRotation(uY,-angle);

    return setZP*rot;
}


void tigl::CTiglSectionElement::SetNormal(CTiglPoint newNormal, TiglCoordinateSystem referenceCS)
{

    newNormal.normalize();

    CTiglPoint normal = GetNormal(referenceCS);
    CTiglPoint center = GetCenter(referenceCS);
    CTiglTransformation toOrigin;
    toOrigin.AddTranslation(-center.x, -center.y, -center.z);
    CTiglTransformation global = GetTotalTransformation(referenceCS);

    double oldAngle = GetRotationAroundNormal(referenceCS);

    CTiglTransformation setN = CTiglTransformation::GetRotationToAlignAToB( normal ,newNormal);
    SetTotalTransformation( toOrigin.Inverted() * setN * toOrigin * global, referenceCS);

    // Call Set rotation around normal to reset the angle to its initial position
    SetRotationAroundNormal(oldAngle, referenceCS);

}

void tigl::CTiglSectionElement::SetRotationAroundNormal(double angle, TiglCoordinateSystem referenceCS)
{

    double currentAngle = GetRotationAroundNormal(referenceCS);

    CTiglPoint normal = GetNormal(referenceCS);

    CTiglTransformation setAngle =  CTiglTransformation::GetRotationFromAxisRotation(normal, angle - currentAngle);

    CTiglPoint center = GetCenter(referenceCS);
    CTiglTransformation toOrigin;
    toOrigin.AddTranslation(-center.x, -center.y, -center.z);

    CTiglTransformation global = GetTotalTransformation(referenceCS);
    SetTotalTransformation(  toOrigin.Inverted() * setAngle * toOrigin * global, referenceCS );

}

double tigl::CTiglSectionElement::GetRotationAroundNormal(TiglCoordinateSystem referenceCS) const
{

    CTiglPoint stdUZDir = GetStdDirForProfileUnitZ(referenceCS);
    CTiglPoint currentUZDir = GetCurrentUnitZDirectionOfProfile(referenceCS);

    // to determine the sens of the rotation  (we always rotate conter-clockwise around the normal)
    CTiglPoint normal = GetNormal(referenceCS);
    CTiglPoint crossProduct = CTiglPoint::cross_prod(currentUZDir, normal);
    double tripleProduct = CTiglPoint::inner_prod(stdUZDir, crossProduct) ;

    double innerProduct  = CTiglPoint::inner_prod(stdUZDir, currentUZDir);
    // perform somme check to be sure that the value is between 1 and  -1
    // some approximation error can be introduced by the inner product
    if ( innerProduct > 1 ) {
        innerProduct = 1.0;
    }
    else if (innerProduct < -1 ) {
        innerProduct = -1.0;
    }

    double angle = Degrees(acos(innerProduct));

    if (tripleProduct < 0 && (!isNear(angle, 0)) ) {
        angle = 360 - angle;
    }

    return angle;
}


tigl::CTiglPoint tigl::CTiglSectionElement::GetCurrentUnitZDirectionOfProfile(TiglCoordinateSystem referenceCS) const
{
    CTiglTransformation global = GetTotalTransformation(referenceCS);
    CTiglPoint uZ = (global * CTiglPoint(0,0,1)) - (global * CTiglPoint(0,0,0)) ;
    uZ.normalize();
    return uZ;
}


