/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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

#include <iostream>
#include <sstream>
#include <exception>
#include <vector>
#include <cassert>
#include <cmath>

#include "CTiglControlSurfaceTransformation.h"

#include "gp_Pnt.hxx"
#include "gp_Ax1.hxx"
#include "Standard_Version.hxx"



namespace tigl
{

CTiglControlSurfaceTransformation::CTiglControlSurfaceTransformation(gp_Pnt hingePoint1 ,gp_Pnt hingePoint2,
                                                                     gp_Pnt hingePoint1s, gp_Pnt hingePoint2s,
                                                                     double rotationAlpha )
{

    // four points, defining HingeLine before and after the translation.
    gp_Vec p1(hingePoint1.XYZ());
    gp_Vec p2(hingePoint2.XYZ());
    gp_Vec p1s(hingePoint1s.XYZ());
    gp_Vec p2s(hingePoint2s.XYZ());

    // those three vectors define the local coordinate System.
    gp_Vec exV = (p2 - p1);
    exV.Normalize();
    gp_Vec eyV( 1 ,  - ( exV.X() / exV.Y() ) , 0);
    eyV.Normalize();
    gp_Vec ezV( - ( exV.X()*exV.Z() ) / ( exV.X()*exV.X() + exV.Y() * exV.Y() ) , - ( exV.Y()*exV.Z() ) / ( exV.X()*exV.X() + exV.Y() * exV.Y() )  , 1);
    ezV.Normalize();

    /* testing if local coord System is orthogonal */
    assert(fabs(exV*eyV) <= 1e-7);
    assert(fabs(exV*ezV) <= 1e-7);
    assert(fabs(ezV*eyV) <= 1e-7);

    // translation
    gp_Vec t = p1s-p1;

    gp_Vec outerNewT = p2s - p1s;
    Standard_Real p2Cx = outerNewT * exV;
    Standard_Real p2Cy = outerNewT * eyV;
    Standard_Real p2Cz = outerNewT * ezV;
    gp_Vec transformedOuterOld(p2Cx, p2Cy, p2Cz);

    // calculating needed rotation around the 3 axes.
    double r  = transformedOuterOld.Magnitude();
    double phi     = atan2(p2Cy,p2Cx);
    double theta   = acos(p2Cz/r);

    // testing
    gp_Vec p2c = p1s + r*cos(phi)*sin(theta)*exV + r*sin(phi)*sin(theta)*eyV + r*cos(theta)*ezV;
    assert((p2s-p2c).Magnitude() < 1e-7);

    // transformation
    gp_Trsf wToS;
    wToS.SetValues(exV.X(), exV.Y(), exV.Z(), 0,
                   eyV.X(), eyV.Y(), eyV.Z(), 0,
                   ezV.X(), ezV.Y(), ezV.Z(), 0
#if OCC_VERSION_HEX < VERSION_HEX_CODE(6,8,0)
                   , 1e-7, 1e-7);
#else
                   );
#endif

    gp_Trsf TransP;
    TransP.SetTranslation(-p1);
    transformationToLocal = wToS*TransP;

    // transformation inverted
    transformationFromLocal = transformationToLocal.Inverted();

    // rotation
    double alpha = ( rotationAlpha * M_PI ) / 180;
    rotAlpha.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0)), -alpha);

    // rotation
    double rotThetaValue = - M_PI /2 + theta;
    rotTheta.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,1,0)), rotThetaValue);

    // rotation
    rotPhi.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), phi);

    // translation
    double magnitudeDif = (p1-p2).Magnitude() - (p1s-p2s).Magnitude();
    gp_Vec directionTrans = (p2s-p1s).Normalized().Multiplied(-(magnitudeDif/2));
    /* Has to be adjusted if scaling is reenabled */
    translation.SetTranslation(t.Added(directionTrans));

    /*
     * Scaling disabled due to strange artifacts in openCascade
     * to reenable it change the codeline above this comment:
     * translation.SetTranslation(t.Added(directionTrans));
     * to
     * translation.SetTranslation(t);
     *
     * and delete comment marks around scale by the totalTransformation.
    */
    /*
    double sf =  (p2s-p1s).Magnitude()/(p2-p1).Magnitude();
    // MS: this line is problematic, since OpenCASCADE does not allow non-uniform scaling
    scale.SetValues(sf,0,0,0,
                    0 ,1,0,0,
                    0 ,0,1,0,
                    1e-14,1e-14);
    */

    // calculating and applying the final transformation.
    totalTransformation = translation * transformationFromLocal * rotPhi * rotTheta * rotAlpha /* * scale */ * transformationToLocal;
}


gp_Trsf CTiglControlSurfaceTransformation::getTotalTransformation()
{
    return totalTransformation;
}

gp_Trsf CTiglControlSurfaceTransformation::getTranslationTransformation()
{
    return translation;
}

gp_Trsf CTiglControlSurfaceTransformation::getScaleTransformation()
{
    return scale;
}

gp_Trsf CTiglControlSurfaceTransformation::getRotAlphaTransformation()
{
    return rotAlpha;
}

gp_Trsf CTiglControlSurfaceTransformation::getRotPhiTransformation()
{
    return rotPhi;
}

gp_Trsf CTiglControlSurfaceTransformation::getRotThetaTransformation()
{
    return rotTheta;
}

gp_Trsf CTiglControlSurfaceTransformation::getToLocalTransformation()
{
    return transformationToLocal;
}

gp_Trsf CTiglControlSurfaceTransformation::getFromLocalTransformation()
{
    return transformationFromLocal;
}

CTiglControlSurfaceTransformation::~CTiglControlSurfaceTransformation()
{

}


} // end namespace tigl
