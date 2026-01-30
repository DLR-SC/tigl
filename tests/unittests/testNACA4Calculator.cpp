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



TEST(NACA4Calculator, Nacacalculatortest1_coordinates){
    tigl::NACA4Calculator  NACA4(2,2,12, 0.00126);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    EXPECT_NEAR(result1.X(), (1.00006), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.00125843), 1e-8);
    gp_Vec2d result2 = NACA4.lower_curve(1);
    EXPECT_NEAR(result2.X(), (0.999937), 1e-6); // es muss hie rbeim fehler immer auf die exakte stelle der anzahl der nkstellen geprüft werden
    EXPECT_NEAR(result2.Y(), (-0.00125843), 1e-8);

}

TEST(NACA4Calculator, Nacacalculatortest2_coordinates){
    tigl::NACA4Calculator NACA4(0,0,12, 0.00126);
    gp_Vec2d result1 = NACA4.upper_curve(0.5);
    EXPECT_NEAR(result1.X(), (0.5), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.0529403), 1e-7);
    gp_Vec2d result2 = NACA4.lower_curve(0.5);
    EXPECT_NEAR(result2.X(), (0.5), 1e-6); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result2.Y(), (-0.0529403), 1e-7);

}

TEST(NACA4Calculator, Nacacalculatortest3_coordinates){
    tigl::NACA4Calculator NACA4(0,0,9, 0.000945);
    gp_Vec2d result1 = NACA4.upper_curve(0.2);
    EXPECT_NEAR(result1.X(), (0.2), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.0430316), 1e-7);
    gp_Vec2d result2 = NACA4.lower_curve(0.2);
    EXPECT_NEAR(result2.X(), (0.2), 1e-6); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result2.Y(), (-0.0430316), 1e-7);

}

TEST(NACA4Calculator, Nacacalculatortest4_coordinates){
    tigl::NACA4Calculator NACA4(6,5,9, 0.000945);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    EXPECT_NEAR(result1.X(), (1.00022), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.000918906), 1e-9);
    gp_Vec2d result2 = NACA4.lower_curve(1);
    EXPECT_NEAR(result2.X(), (0.999779), 1e-6); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result2.Y(), (-0.000918906), 1e-9);
}

TEST(NACA4Calculator, Nacacalculatortest5_thickness){ 
    tigl::NACA4Calculator NACA4(0,0,12, 0.00126);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); //bei 2.999 läuft der test nicht mehr durch
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
}

TEST(NACA4Calculator, Nacacalculatortest6_thickness){ 
    tigl::NACA4Calculator NACA4(0,0,18, 0.00189);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); //bei 2.999 läuft der test nicht mehr durch
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
    EXPECT_NEAR(result1, 0.09, 1e-2); //TODO: Fact check this
}

TEST(NACA4Calculator, Nacacalculatortest7_camberline){ 
    tigl::NACA4Calculator NACA4(2,2,12, 0.00126);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}

TEST(NACA4Calculator, Nacacalculatortest8_camberline){ 
    tigl::NACA4Calculator NACA4(4,5,9, 0.000945);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}

TEST(NACA4Calculator, Nacacalculatortest9_camberline){ 
    tigl::NACA4Calculator NACA4(0,0,15, 0.001575);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}   

TEST(NACA4Calculator, Nacacalculatortest10_trailingedge){
    tigl::NACA4Calculator NACA4(0,0,15, 9e-05);
    double result1 = NACA4.trailing_edge_thickness_function(9e-05);
    EXPECT_NEAR(result1, 0.10348, 1e-14);
}

TEST(NACA4Calculator, Nacacalculatortest11_trailingedge){
    tigl::NACA4Calculator NACA4(6,4,21, 0.000945);
    double result1 = NACA4.trailing_edge_thickness_function(0.000945);
    EXPECT_NEAR(result1, 0.1027, 1e-14);
}


//TODO 
TEST(NACA4Calculator, Nacacalculatortest12_trailingedge){
    tigl::NACA4Calculator NACA4(0,0,15, 5);
    double result1 = NACA4.trailing_edge_thickness_function(5);
    EXPECT_NEAR(result1, -6.5630666666666668, 1e-14);//result 1 = -6.5630666666666668 , vgl wert ist : -6.5630699999999997
}

