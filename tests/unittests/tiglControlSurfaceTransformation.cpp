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

#include "test.h"
#include "CTiglControlSurfaceTransformation.h"

#include "Standard_Real.hxx"
#include "gp_Ax1.hxx"
#include "gp_Quaternion.hxx"
#include <limits>


TEST(TiglControlSurfaceTransformation, localTransformation)
{
    for ( int i = 0; i < 5; i++ ) {
        gp_XYZ xyz1;
        gp_XYZ xyz2;
        gp_XYZ xyz3;
        gp_XYZ xyz4;

        if ( i == 0) {
            // testing with some random values
            xyz1 = gp_XYZ(1,1,1);
            xyz2 = gp_XYZ(2,2,2);
            xyz3 = gp_XYZ(1,2,4);
            xyz4 = gp_XYZ(3,7,1);
        }
        else if ( i == 1) {
            // testing mixed negative and positive values
            xyz1 = gp_XYZ(-1,-1,-1);
            xyz2 = gp_XYZ(-2,-2,-2);
            xyz3 = gp_XYZ(1,2,4);
            xyz4 = gp_XYZ(3,7,1);
        }
        else if ( i == 2) {
            // testing with no Translation
            xyz1 = gp_XYZ(1,1,0);
            xyz2 = gp_XYZ(1,2,0);
            xyz3 = gp_XYZ(1,1,0);
            xyz4 = gp_XYZ(1,2,0);
        }
        else if ( i == 3) {
            // testing with only negative calues
            xyz1 = gp_XYZ(-1,-1,0);
            xyz2 = gp_XYZ(-1,-2,0);
            xyz3 = gp_XYZ(-1,-1,0);
            xyz4 = gp_XYZ(-1,-2,0);
        }
        else { /* i = 4 */
            // testing with big values
            xyz1 = gp_XYZ(818,1,0);
            xyz2 = gp_XYZ(1,2048,0);
            xyz3 = gp_XYZ(1,-1024,0);
            xyz4 = gp_XYZ(512,2,312);
        }

        gp_Pnt p1(xyz1);
        gp_Pnt p2(xyz2);
        gp_Pnt p1s(xyz3);
        gp_Pnt p2s(xyz4);

        tigl::CTiglControlSurfaceTransformation transformation(p1, p2, p1s, p2s, 90.);
        gp_Vec v1(p1.XYZ());
        gp_Vec v2(p2.XYZ());

        gp_Trsf t = transformation.getToLocalTransformation();
        gp_Pnt p1Trans = p1.Transformed(t);
        /* p1Trans has be be zero, because p1 is the LocalCoord-System´ origin */
        ASSERT_EQ(p1Trans.X(), 0.);
        ASSERT_EQ(p1Trans.Y(), 0.);
        ASSERT_EQ(p1Trans.Z(), 0.);

        gp_Pnt p2Trans = p2.Transformed(t);
        ASSERT_NEAR(p2Trans.Y(), 0., 1e-7);
        ASSERT_NEAR(p2Trans.Z(), 0., 1e-7);
        /* only the x value of p2Trans can vary */
        ASSERT_NEAR(p2Trans.X(), (v1-v2).Magnitude(), 1e-7 );

        gp_Vec v1SameCheck = v1.Transformed(t);
        v1SameCheck.Transform(transformation.getFromLocalTransformation());
        gp_Vec zeroVec = v1 - v1SameCheck;
        /* v1 transformed and reTransformed should still be the same vector */
        ASSERT_NEAR(zeroVec.X(), 0., 1e-7);
        ASSERT_NEAR(zeroVec.Y(), 0., 1e-7);
        ASSERT_NEAR(zeroVec.Z(), 0., 1e-7);
    }
}

TEST(TiglControlSurfaceTransformation, rotAlphaTransformation)
{
    // values here should not have any effect on the test.
    gp_Pnt p1(1,1,1);
    gp_Pnt p2(1,2,1);
    gp_Pnt p1s(4,1,1);
    gp_Pnt p2s(5,2,1);

    for ( int i = 0; i <= 3; i++ ) {
        double rotAlpha;
        if ( i == 0 ) {
            rotAlpha = 45;
        }
        else if ( i == 1 ) {
            rotAlpha = 0;
        }
        else if ( i == 2 ) {
            rotAlpha = 90;
        }
        else /* i == 3 */ {
            rotAlpha = -45;
        }

        tigl::CTiglControlSurfaceTransformation transformation(p1, p2, p1s, p2s, rotAlpha);
        gp_Trsf t = transformation.getRotAlphaTransformation();
        gp_Quaternion quad = t.GetRotation();
        ASSERT_NEAR(quad.GetRotationAngle(), fabs(rotAlpha/180.*M_PI), 1e-8);
    }
}

TEST(TiglControlSurfaceTransformation, rotThetaTransformation)
{
    gp_Pnt p1(1,1,0);
    gp_Pnt p2(1,2,0);
    gp_Pnt p1s(p1.XYZ());
    gp_Pnt p2s(2,1,1);
    tigl::CTiglControlSurfaceTransformation transformation(p1,p2,p1s,p2s, 90.);

    gp_Trsf t = transformation.getRotThetaTransformation();
    gp_Quaternion quad = t.GetRotation();
    ASSERT_NEAR(quad.GetRotationAngle(), M_PI/4, 1e-7);
}

TEST(TiglControlSurfaceTransformation, rotPhiTransformation)
{
    gp_Pnt p1(1,1,0);
    gp_Pnt p2(1,2,0);
    gp_Pnt p1s(p1.XYZ());
    gp_Pnt p2s(2,2,0);
    tigl::CTiglControlSurfaceTransformation transformation(p1, p2, p1s, p2s, 90.);

    gp_Trsf t = transformation.getRotPhiTransformation();
    gp_Quaternion quad = t.GetRotation();
    ASSERT_NEAR(quad.GetRotationAngle(), M_PI/4, 1e-7);
}

TEST(TiglControlSurfaceTransformation, translationTransformation)
{
    gp_Pnt p1(1,1,1);
    gp_Pnt p2(1,2,1);
    gp_Pnt p1s(1+1,1,1);
    gp_Pnt p2s(1+1,2,1);
    tigl::CTiglControlSurfaceTransformation transformation(p1, p2, p1s, p2s, 90.);

    gp_Trsf t = transformation.getTranslationTransformation();
    gp_XYZ translationPart  = t.TranslationPart();
    ASSERT_NEAR(translationPart.X(), 1., 1e-6);
    ASSERT_NEAR(translationPart.Y(), 0., 1e-6);
    ASSERT_NEAR(translationPart.Z(), 0., 1e-6);

    p1s.SetXYZ(gp_XYZ(1,1+1,1));
    p2s.SetXYZ(gp_XYZ(1,2+1,1));
    transformation = tigl::CTiglControlSurfaceTransformation(p1, p2, p1s, p2s, 90.);
    t = transformation.getTranslationTransformation();
    translationPart  = t.TranslationPart();
    ASSERT_NEAR(translationPart.X(), 0., 1e-6);
    ASSERT_NEAR(translationPart.Y(), 1., 1e-6);
    ASSERT_NEAR(translationPart.Z(), 0., 1e-6);

    p1s.SetXYZ(gp_XYZ(1,1,1+1));
    p2s.SetXYZ(gp_XYZ(1,2,1+1));
    transformation = tigl::CTiglControlSurfaceTransformation(p1, p2, p1s, p2s, 90.);
    t = transformation.getTranslationTransformation();
    translationPart  = t.TranslationPart();
    ASSERT_NEAR(translationPart.X(), 0., 1e-6);
    ASSERT_NEAR(translationPart.Y(), 0., 1e-6);
    ASSERT_NEAR(translationPart.Z(), 1., 1e-6);
}

// MS: The test is disabled, since unisotropic scaling is not supported
// by the gp_Trsf class.
TEST(TiglControlSurfaceTransformation, DISABLED_scaleTransformation)
{
    gp_Pnt p1(1,3,1);
    gp_Pnt p2(1,2,112);
    gp_Pnt p1s(8,1,32);
    gp_Pnt p2s(14,2,9);
    tigl::CTiglControlSurfaceTransformation transformation(p1, p2, p1s, p2s, 90.);

    gp_Vec v1(p1.XYZ());
    gp_Vec v2(p2.XYZ());
    gp_Vec v1s(p1s.XYZ());
    gp_Vec v2s(p2s.XYZ());

    gp_Trsf t = transformation.getScaleTransformation();
    ASSERT_NEAR((v1s-v2s).Magnitude() / (v1 - v2).Magnitude(), t.Value(1,1), 1e-7);
    ASSERT_EQ(t.Value(2,2),1.0);
    ASSERT_EQ(t.Value(3,3),1.0);
}
