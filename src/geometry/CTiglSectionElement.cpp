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

#include "tiglcommonfunctions.h"
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Surface.hxx>
#include <GeomLProp_SLProps.hxx>

tigl::CTiglTransformation tigl::CTiglSectionElement::GetTotalTransformation(TiglCoordinateSystem referenceCS) const
{
    // Do section element transformation on points
    tigl::CTiglTransformation totalTransformation = GetSectionElementTransformation();

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

tigl::CTiglTransformation
tigl::CTiglSectionElement::GetElementTrasformationToTranslatePoint(const CTiglPoint& newP, const CTiglPoint& oldP,
                                                                   TiglCoordinateSystem referenceCS)
{
    // We want that:
    // w = G'p      where   w is the new Point Position
    //                      G' the new Global transformation that is apply on the point
    //                      p is the point position in element coordinate system
    // w = T*G*p    Where   T is the translation needed to bring the old point (oldP) to it position (newP)
    //                      G is the old global transformation
    // w = G*Tl*p   Where Tl = augmented( affinePart(G)^-1 * t) where t is the translation vector
    // w = (F)P*S*E*Tl*0
    // w = (F)P*S*E'*0
    // -> So we simply need to compute Tl, then we can obtaint the transformation of the element = E*Tl

    CTiglPoint tLeft = newP - oldP;

    CTiglTransformation G = GetTotalTransformation(referenceCS);
    // remove translation of G;
    G.SetValue(0, 3, 0);
    G.SetValue(1, 3, 0);
    G.SetValue(2, 3, 0);
    G.SetValue(3, 3, 1);

    CTiglPoint tRight = G.Inverted() * tLeft;
    CTiglTransformation Tl;
    Tl.AddTranslation(tRight.x, tRight.y, tRight.z);

    CTiglTransformation newE = GetSectionElementTransformation() * Tl;

    return newE;
}

tigl::CTiglTransformation tigl::CTiglSectionElement::GetElementTransformationToScaleCircumference(double scaleFactor,
                                                                                                  TiglCoordinateSystem referenceCS) {
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

    CTiglTransformation centerToOriginM, scaleM, newE;

    scaleM.SetIdentity();
    scaleM.AddScaling(scaleFactor, scaleFactor, scaleFactor);

    CTiglPoint center = GetCenter(referenceCS);
    centerToOriginM.SetIdentity();
    centerToOriginM.AddTranslation(-center.x, -center.y, -center.z);

    if (referenceCS == TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM) {
        newE = GetSectionTransformation().Inverted() *
               GetPositioningTransformation().Inverted() *
               GetParentTransformation().Inverted() *
               centerToOriginM.Inverted() *
               scaleM *
               centerToOriginM *
               GetTotalTransformation(referenceCS);
    }
    else if (referenceCS == TiglCoordinateSystem::FUSELAGE_COORDINATE_SYSTEM) {
        newE = GetSectionTransformation().Inverted() *
               GetPositioningTransformation().Inverted() *
               centerToOriginM.Inverted() *
               scaleM *
               centerToOriginM *
               GetTotalTransformation(referenceCS);

    }

    return newE;


}

double tigl::CTiglSectionElement::GetArea(TiglCoordinateSystem referenceCS) const
{
    TopoDS_Wire wire = GetWire(referenceCS);
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire); // is it to slow?
    GProp_GProps System;
    BRepGProp::SurfaceProperties(face, System);
    return System.Mass();
}
