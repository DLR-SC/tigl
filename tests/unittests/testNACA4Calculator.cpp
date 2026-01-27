/* 
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-14 Hannah Gedler <hannah.gedler@dlr.de>
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
* @brief Tests for NACA wing profiles
*/

#include "test.h" // Brings in the GTest framework
#include "testUtils.h"
#include "tigl.h"
#include "math/tiglmathfunctions.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWingProfile.h"
#include "gp_Pnt.hxx"
#include "gp_Pnt.hxx"
#include "NACA4Calculator.cpp" 
#include "NACA4Calculator.h"
#include <gp_Vec2d.hxx>



TEST(NACA4Calculator, Nacacalculatortest1){
    tigl::NACA4Calculator  NACA4(2,2,12);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    EXPECT_NEAR(result1.X(), (1.00006), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.00125843), 1e-8);
    gp_Vec2d result2 = NACA4.lower_curve(1);
    EXPECT_NEAR(result2.X(), (0.999937), 1e-6); // es muss hie rbeim fehler immer auf die exakte stelle der anzahl der nkstellen geprüft werden
    EXPECT_NEAR(result2.Y(), (-0.00125843), 1e-8);

}

TEST(NACA4Calculator, Nacacalculatortest2){
    tigl::NACA4Calculator NACA4(0,0,12);
    gp_Vec2d result1 = NACA4.upper_curve(0.5);
    EXPECT_NEAR(result1.X(), (0.5), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.0529403), 1e-7);
    gp_Vec2d result2 = NACA4.lower_curve(0.5);
    EXPECT_NEAR(result2.X(), (0.5), 1e-6); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result2.Y(), (-0.0529403), 1e-7);

}

TEST(NACA4Calculator, Nacacalculatortest3){
    tigl::NACA4Calculator NACA4(0,0,9);
    gp_Vec2d result1 = NACA4.upper_curve(0.2);
    EXPECT_NEAR(result1.X(), (0.2), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.0430316), 1e-7);
    gp_Vec2d result2 = NACA4.lower_curve(0.2);
    EXPECT_NEAR(result2.X(), (0.2), 1e-6); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result2.Y(), (-0.0430316), 1e-7);

}

TEST(NACA4Calculator, Nacacalculatortest4){
    tigl::NACA4Calculator NACA4(6,5,9);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    EXPECT_NEAR(result1.X(), (1.00022), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.000918906), 1e-9);
    gp_Vec2d result2 = NACA4.lower_curve(1);
    EXPECT_NEAR(result2.X(), (0.999779), 1e-6); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result2.Y(), (-0.000918906), 1e-9);
}

TEST(NACA4Calculator, Nacacalculatortest5_thickness){ 
    tigl::NACA4Calculator NACA4(0,0,12);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); //bei 2.999 läuft der test nicht mehr durch
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
}


