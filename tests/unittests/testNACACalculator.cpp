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
#include "CTiglNACACalculator.h"
#include "generated/CPACSNacaProfile.h"
#include "CTiglUIDManager.h"
#include "Debugging.h" 
#include <BRepCheck_Analyzer.hxx>
#include <gp_Vec2d.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx> 
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx> 
#include "Geom_Curve.hxx"
#include "Geom_BSplineCurve.hxx"
#include "CTiglError.h"
#include <tixi.h>


TEST(CTiglNACACalculator, naca2212_le_and_te_points){
    //tigl::CTiglNACACalculator  NACA4(2,2,12, 0.00252);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2212"), 0.00252);
    ASSERT_TRUE(NACA4.profile_thickness(1) >= 0);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    EXPECT_NEAR(result1.X(), (1.00006), 1e-5);
    EXPECT_NEAR(result1.Y(), (0.00125843), 1e-8);
    gp_Vec2d result2 = NACA4.lower_curve(1);
    EXPECT_NEAR(result2.X(), (0.999937), 1e-6); 
    EXPECT_NEAR(result2.Y(), (-0.00125843), 1e-8);
    
    ASSERT_TRUE(NACA4.profile_thickness(0) >= 0);
    gp_Vec2d result3 = NACA4.upper_curve(0.);
    EXPECT_NEAR(result3.X(), (0.0), 1e-5); 
    EXPECT_NEAR(result3.Y(), (0.0), 1e-7);
    gp_Vec2d result4 = NACA4.lower_curve(0.);
    EXPECT_NEAR(result4.X(), (0.0), 1e-6); 
    EXPECT_NEAR(result4.Y(), (0.0), 1e-7);
}

TEST(CTiglNACACalculator, naca2212_le_and_te_points_with_class_lowerCurve){
    tigl::CTiglNACACalculator  NACA4(tigl::NACA4DigitCode("2212"), 0.00252); 
    //tigl::CTiglNACACalculator  NACA4(2,2,12, 0.00252); 
    tigl::CTiglNACA4LowerCurve lowerCurve(NACA4);
    EXPECT_NEAR(lowerCurve.valueX(1), (0.999937), 1e-6); 
    EXPECT_NEAR(lowerCurve.valueY(1), 0.0, 1e-8);
    EXPECT_NEAR(lowerCurve.valueZ(1), (-0.00125843), 1e-8);
    gp_Vec2d result3 = NACA4.upper_curve(0.);
    EXPECT_NEAR(result3.X(), (0.0), 1e-5); 
    EXPECT_NEAR(result3.Y(), (0.0), 1e-7);
    gp_Vec2d result4 = NACA4.lower_curve(0.);
    EXPECT_NEAR(result4.X(), (0.0), 1e-6); 
    EXPECT_NEAR(result4.Y(), (0.0), 1e-7);
}

TEST(CTiglNACACalculator, naca0012_random_point){
    //tigl::CTiglNACACalculator NACA4(0,0,12, 0.00252);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0012"), 0.00252);
    ASSERT_TRUE(NACA4.profile_thickness(0.5) >= 0);
    gp_Vec2d result1 = NACA4.upper_curve(0.5);
    EXPECT_NEAR(result1.X(), (0.5), 1e-5); 
    EXPECT_NEAR(result1.Y(), (0.0529403), 1e-7);
    gp_Vec2d result2 = NACA4.lower_curve(0.5);
    EXPECT_NEAR(result2.X(), (0.5), 1e-6); 
    EXPECT_NEAR(result2.Y(), (-0.0529403), 1e-7);

    ASSERT_TRUE(NACA4.profile_thickness(0.) >= 0);
    gp_Vec2d result3 = NACA4.upper_curve(0.);
    EXPECT_NEAR(result3.X(), (0.0), 1e-5);  
    EXPECT_NEAR(result3.Y(), (0.0), 1e-7);
    gp_Vec2d result4 = NACA4.lower_curve(0.);
    EXPECT_NEAR(result4.X(), (0.0), 1e-6); 
    EXPECT_NEAR(result4.Y(), (0.0), 1e-7);
}

TEST(CTiglNACACalculator, naca0009_random_point_and_le_point){
    //tigl::CTiglNACACalculator NACA4(0,0,9, 0.00189);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0009"), 0.00189);
    gp_Vec2d result1 = NACA4.upper_curve(0.2);
    EXPECT_NEAR(result1.X(), (0.2), 1e-5); 
    EXPECT_NEAR(result1.Y(), (0.0430316), 1e-7);
    gp_Vec2d result2 = NACA4.lower_curve(0.2);
    EXPECT_NEAR(result2.X(), (0.2), 1e-6); 
    EXPECT_NEAR(result2.Y(), (-0.0430316), 1e-7);
    gp_Vec2d result3 = NACA4.upper_curve(0.);
    EXPECT_NEAR(result3.X(), (0.0), 1e-5); 
    EXPECT_NEAR(result3.Y(), (0.0), 1e-7);
    gp_Vec2d result4 = NACA4.lower_curve(0.);
    EXPECT_NEAR(result4.X(), (0.0), 1e-6);
    EXPECT_NEAR(result4.Y(), (0.0), 1e-7);
}

TEST(CTiglNACACalculator, Nnaca6509_le_and_te_points){
    //tigl::CTiglNACACalculator NACA4(6,5,9, 0.00189);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("6509"), 0.00189);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    EXPECT_NEAR(result1.X(), (1.00022), 1e-5); 
    EXPECT_NEAR(result1.Y(), (0.000918906), 1e-9);
    gp_Vec2d result2 = NACA4.lower_curve(1);
    EXPECT_NEAR(result2.X(), (0.999779), 1e-6); 
    EXPECT_NEAR(result2.Y(), (-0.000918906), 1e-9);
    gp_Vec2d result3 = NACA4.upper_curve(0.);
    EXPECT_NEAR(result3.X(), (0.0), 1e-5); 
    EXPECT_NEAR(result3.Y(), (0.0), 1e-7);
    gp_Vec2d result4 = NACA4.lower_curve(0.);
    EXPECT_NEAR(result4.X(), (0.0), 1e-6); 
    EXPECT_NEAR(result4.Y(), (0.0), 1e-7);
}

TEST(CTiglNACACalculator, naca0012_max_profile_thickness){ 
    //tigl::CTiglNACACalculator NACA4(0,0,12, 0.00126);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0012"), 0.00126);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); 
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
    EXPECT_NEAR(result1, 0.06001216339, 1e-11); 
}

TEST(CTiglNACACalculator, naca0018_max_profile_thickness){ 
    //tigl::CTiglNACACalculator NACA4(0,0,18, 0.00189);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0018"), 0.00189);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); 
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
    EXPECT_NEAR(result1, 0.09001824509, 1e-12); 
}

TEST(CTiglNACACalculator, naca2212_camberline_at_te_and_le){ 
    //tigl::CTiglNACACalculator NACA4(2,2,12, 0.00126);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2212"), 0.00126);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}

TEST(CTiglNACACalculator, naca4509_camberline_at_te_and_le){ 
    //tigl::CTiglNACACalculator NACA4(4,5,9, 0.000945);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("4509"), 0.000945);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}

TEST(CTiglNACACalculator, naca0015_camberline_at_te_and_le){ 
    //tigl::CTiglNACACalculator NACA4(0,0,15, 0.001575);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0015"), 0.001575);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}   

TEST(CTiglNACACalculator, naca15030105_assertion_throw_normalization){
    
    //EXPECT_THROW(tigl::CTiglNACACalculator NACA4(150,30,105, 0.001575), tigl::CTiglError);
    EXPECT_THROW(tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("15030105"), 0.001575), tigl::CTiglError);
}

TEST(CTiglNACACalculator, naca030810_assertion_throw_normalization){
    
    //EXPECT_THROW(tigl::CTiglNACACalculator NACA4(0,30,10, 0.001575), tigl::CTiglError);
    EXPECT_THROW(tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("030810"), 0.001575), tigl::CTiglError);
}

TEST(CTiglNACACalculator, naca0f3250_assertion_throw_normalization){
    
    //EXPECT_THROW(tigl::CTiglNACACalculator NACA4(0,3,250, 0.001575), tigl::CTiglError);
    EXPECT_THROW(tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0f3250"), 0.001575), tigl::CTiglError);
}


TEST(CTiglNACACalculator, naca0015_trailingedge_length){
    //tigl::CTiglNACACalculator NACA4(0,0,15, 0.12);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0015"), 0.12);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    double half_thickness1_y = result1.Y();
    double half_thickness1_x = result1.X();
    gp_Vec2d result2 = NACA4.lower_curve(1);
    double half_thickness2_y = result2.Y();
    double half_thickness2_x = result2.X();
    double thickness = sqrt((half_thickness1_x - half_thickness2_x)*(half_thickness1_x - half_thickness2_x)+(half_thickness1_y - half_thickness2_y)*(half_thickness1_y - half_thickness2_y));


    EXPECT_NEAR(thickness, 0.12, 1e-14);
}

TEST(CTiglNACACalculator, naca001515_trailingedge_length){
    //tigl::CTiglNACACalculator NACA4(0,0,15, 0.15);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0015"), 0.15);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    double half_thickness1_y = result1.Y();
    double half_thickness1_x = result1.X();
    gp_Vec2d result2 = NACA4.lower_curve(1);
    double half_thickness2_y = result2.Y();                                                                                                                                                                         
    double half_thickness2_x = result2.X();
    double thickness = sqrt((half_thickness1_x - half_thickness2_x)*(half_thickness1_x - half_thickness2_x)+(half_thickness1_y - half_thickness2_y)*(half_thickness1_y - half_thickness2_y));

    EXPECT_NEAR(thickness, 0.15, 1e-14);
}

TEST(CTiglNACACalculator, naca2215_trailingedge_length){
    //tigl::CTiglNACACalculator NACA4(2,2,15, 0.20);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2215"), 0.20);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    double half_thickness1_y = result1.Y();
    double half_thickness1_x = result1.X();
    gp_Vec2d result2 = NACA4.lower_curve(1);
    double half_thickness2_y = result2.Y();
    double half_thickness2_x = result2.X();
    double thickness = sqrt((half_thickness1_x - half_thickness2_x)*(half_thickness1_x - half_thickness2_x)+(half_thickness1_y - half_thickness2_y)*(half_thickness1_y - half_thickness2_y));


    EXPECT_NEAR(thickness, 0.20, 1e-14);
}

TEST(CTiglNACACalculator, naca6415_trailingedge_length){
    //tigl::CTiglNACACalculator NACA4(6,4,15, 0.13);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("6415"), 0.13);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    double half_thickness1_y = result1.Y();
    double half_thickness1_x = result1.X();
    gp_Vec2d result2 = NACA4.lower_curve(1);
    double half_thickness2_y = result2.Y();
    double half_thickness2_x = result2.X();
    double thickness = sqrt((half_thickness1_x - half_thickness2_x)*(half_thickness1_x - half_thickness2_x)+(half_thickness1_y - half_thickness2_y)*(half_thickness1_y - half_thickness2_y));


    EXPECT_NEAR(thickness, 0.13, 1e-14);
}

TEST(CTiglNACACalculator, naca2212_upperCurve_ycoord_and_upper_curve_x_and_zcoord){
    //tigl::CTiglNACACalculator NACA4(2,2,12, 15);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2212"), 15);
    tigl::CTiglNACA4UpperCurve upperCurve(NACA4);
    ASSERT_EQ(upperCurve.valueY(0.), 0.);
    ASSERT_EQ(upperCurve.valueY(0.5), 0.);
    ASSERT_EQ(upperCurve.valueY(1.), 0.);
    gp_Vec2d pnt = NACA4.upper_curve(0.5);
    ASSERT_EQ(upperCurve.valueX(0.5), pnt.X());
    ASSERT_EQ(upperCurve.valueZ(0.5), pnt.Y());
}

TEST(CTiglNACACalculator, naca2212_lowerCurve_ycoord_and_lower_curve_x_and_zcoord){
    //tigl::CTiglNACACalculator NACA4(2,2,12, 15);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2212"), 15);
    tigl::CTiglNACA4LowerCurve lowerCurve(NACA4);
    
    ASSERT_EQ(lowerCurve.valueY(0.), 0.);
    ASSERT_EQ(lowerCurve.valueY(0.5), 0.);
    ASSERT_EQ(lowerCurve.valueY(1.), 0.);

    gp_Vec2d pnt = NACA4.lower_curve(0.5);
    ASSERT_EQ(lowerCurve.valueX(0.5), pnt.X());
    ASSERT_EQ(lowerCurve.valueZ(0.5), pnt.Y());
}

TEST(CTiglNACACalculator, naca2212_bspline_vs_lower_curve_coord)
{
    //tigl::CTiglNACACalculator NACA4(2,2,12, 15);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2212"), 15);
    Handle(Geom_BSplineCurve) lower_spline = NACA4.lower_bspline();

    gp_Vec2d pnt = NACA4.lower_curve(0.5);
    gp_Pnt pnt2;
    lower_spline->D0(0.5, pnt2);
    ASSERT_NEAR(pnt2.X(), pnt.X(), 1e-2); //is 1e-2 too small?
    ASSERT_NEAR(pnt2.Z(), pnt.Y(), 1e-2);
}

TEST(CTiglNACACalculator, naca2212_export_bsplines){
    //tigl::CTiglNACACalculator NACA4(2,2,12, .15);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2212"), .15);
    Handle(Geom_BSplineCurve) upperCurve = NACA4.upper_bspline(); 
    Handle(Geom_BSplineCurve) lowerCurve = NACA4.lower_bspline(); 
    ASSERT_FALSE(upperCurve.IsNull());
    ASSERT_FALSE(lowerCurve.IsNull());
    auto lowerEdge = BRepBuilderAPI_MakeEdge(lowerCurve).Edge();
    ASSERT_FALSE(lowerEdge.IsNull());
    BRepTools::Write(lowerEdge, "TestData/export/lowerEdgeTest.brep");

    auto upperEdge = BRepBuilderAPI_MakeEdge(upperCurve).Edge();
    ASSERT_FALSE(upperEdge.IsNull());
    BRepTools::Write(upperEdge, "TestData/export/upperEdgeTest.brep");
}

TEST(CTiglNACACalculator, naca0012_export_bsplines){
    //tigl::CTiglNACACalculator NACA4(0,0,12, .015);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0012"), .015);
    Handle(Geom_BSplineCurve) upperCurvesym = NACA4.upper_bspline(); 
    Handle(Geom_BSplineCurve) lowerCurvesym = NACA4.lower_bspline(); 
    ASSERT_FALSE(upperCurvesym.IsNull());
    ASSERT_FALSE(lowerCurvesym.IsNull());
    auto lowerEdge = BRepBuilderAPI_MakeEdge(lowerCurvesym).Edge();
    ASSERT_FALSE(lowerEdge.IsNull());
    BRepTools::Write(lowerEdge, "TestData/export/lowerEdgeTest_symetric.brep");

    auto upperEdge = BRepBuilderAPI_MakeEdge(upperCurvesym).Edge();
    ASSERT_FALSE(upperEdge.IsNull());
    BRepTools::Write(upperEdge, "TestData/export/upperEdgeTest_symetric.brep");
}

TEST(CTiglNACACalculator, naca2412_LePoint_TePoint){

    // Create a UID manager and use it for the profile. Parent container is not needed for this unit test.
    tigl::CTiglUIDManager uidMgr;
    // disambiguate nullptr for overloaded constructors by casting to the intended parent type
    tigl::CCPACSWingProfile cpacsProfile(static_cast<tigl::CCPACSWingProfiles*>(nullptr), &uidMgr);
    tigl::generated::CPACSNacaProfile nacadef(&cpacsProfile);

    nacadef.SetNaca4DigitCode_choice1(boost::optional<std::string>(std::string("2412")));
    nacadef.SetTrailingEdgeThickness(boost::optional<double>(0.15));

    tigl::CTiglWingProfileNACA profile(cpacsProfile, nacadef);

    TopoDS_Edge upper = profile.GetUpperWire();
    TopoDS_Edge lower = profile.GetLowerWire();
    TopoDS_Edge te    = profile.GetTrailingEdge();
    gp_Pnt lePoint = profile.GetLEPoint();
    gp_Pnt tePoint = profile.GetTEPoint();

    //tigl::CTiglNACACalculator NACA4(2,4,12, 0.15);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2412"), 0.15);
    gp_Pnt result1 = profile.GetLEPoint();
    double result2 = NACA4.camberline(0.0); 
    gp_Vec2d result3 = NACA4.upper_curve(0.0);
    gp_Vec2d result4 = NACA4.lower_curve(0.0);
    EXPECT_EQ(result2, result1.Z());    
    EXPECT_EQ(result1.Z(), result3.Y());
    EXPECT_EQ(result1.Z(), result4.Y());

    EXPECT_FALSE(upper.IsNull());
    EXPECT_FALSE(lower.IsNull());
    EXPECT_FALSE(te.IsNull());

    TopoDS_Edge ul = profile.GetUpperLowerWire();
    EXPECT_FALSE(ul.IsNull());
    TopoDS_Edge ulSharp = profile.GetUpperLowerWire(SHARP_TRAILINGEDGE);
    EXPECT_FALSE(ulSharp.IsNull());
    TopoDS_Edge ulBlunt = profile.GetUpperLowerWire(BLUNT_TRAILINGEDGE);
    EXPECT_FALSE(ulBlunt.IsNull());

}

TEST(CTiglNACACalculator, naca0012_LePoint_TePoint){

    // Create a UID manager and use it for the profile. Parent container is not needed for this unit test.
    tigl::CTiglUIDManager uidMgr;
    // disambiguate nullptr for overloaded constructors by casting to the intended parent type
    tigl::CCPACSWingProfile cpacsProfile(static_cast<tigl::CCPACSWingProfiles*>(nullptr), &uidMgr);
    tigl::generated::CPACSNacaProfile nacadef(&cpacsProfile);

    nacadef.SetNaca4DigitCode_choice1(boost::optional<std::string>(std::string("0012")));
    nacadef.SetTrailingEdgeThickness(boost::optional<double>(0.015));

    tigl::CTiglWingProfileNACA profile(cpacsProfile, nacadef);

    TopoDS_Edge upper = profile.GetUpperWire();
    TopoDS_Edge lower = profile.GetLowerWire();
    TopoDS_Edge te    = profile.GetTrailingEdge();
    gp_Pnt lePoint = profile.GetLEPoint();
    gp_Pnt tePoint = profile.GetTEPoint();

    //tigl::CTiglNACACalculator NACA4(0,0,12, 0.015);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("0012"), 0.015);
    gp_Pnt result1 = profile.GetLEPoint();
    double result2 = NACA4.camberline(0.0); 
    gp_Vec2d result3 = NACA4.upper_curve(0.0);
    gp_Vec2d result4 = NACA4.lower_curve(0.0);
    EXPECT_EQ(result2, result1.Z());    
    EXPECT_EQ(result1.Z(), result3.Y());
    EXPECT_EQ(result1.Z(), result4.Y());

    EXPECT_FALSE(upper.IsNull());
    EXPECT_FALSE(lower.IsNull());
    EXPECT_FALSE(te.IsNull());

    TopoDS_Edge ul = profile.GetUpperLowerWire();
    EXPECT_FALSE(ul.IsNull());
    TopoDS_Edge ulSharp = profile.GetUpperLowerWire(SHARP_TRAILINGEDGE);
    EXPECT_FALSE(ulSharp.IsNull());
    TopoDS_Edge ulBlunt = profile.GetUpperLowerWire(BLUNT_TRAILINGEDGE);
    EXPECT_FALSE(ulBlunt.IsNull());

}

TEST(CTiglNACACalculator, naca0012_trailingEdge_absent_when_zero_thickness){
    tigl::CTiglUIDManager uidMgr;
    tigl::CCPACSWingProfile cpacsProfile(static_cast<tigl::CCPACSWingProfiles*>(nullptr), &uidMgr);
    tigl::generated::CPACSNacaProfile nacadef(&cpacsProfile);
    const std::string code = "0012";
    nacadef.SetNaca4DigitCode_choice1(boost::optional<std::string>(code));
    nacadef.SetTrailingEdgeThickness(boost::optional<double>(0.0));

    tigl::CTiglWingProfileNACA profile(cpacsProfile, nacadef);

    TopoDS_Edge te = profile.GetTrailingEdge();
    EXPECT_TRUE(te.IsNull());
}

TEST(CTiglNACACalculator, naca2412_edge_counter){

    tigl::CTiglUIDManager uidMgr;
    tigl::CCPACSWingProfile cpacsProfile(static_cast<tigl::CCPACSWingProfiles*>(nullptr), &uidMgr);
    tigl::generated::CPACSNacaProfile nacadef(&cpacsProfile);
    const std::string code = "2412";
    const double te_thickness = 0.15;
    nacadef.SetNaca4DigitCode_choice1(boost::optional<std::string>(code));
    nacadef.SetTrailingEdgeThickness(boost::optional<double>(te_thickness));

    tigl::CTiglWingProfileNACA profile(cpacsProfile, nacadef);

    
    TopoDS_Edge upper = profile.GetUpperWire();
    TopoDS_Edge lower = profile.GetLowerWire();
    TopoDS_Edge te    = profile.GetTrailingEdge();


    ASSERT_TRUE(BRepCheck_Analyzer(upper).IsValid());
    ASSERT_TRUE(BRepCheck_Analyzer(lower).IsValid());


    tigl::CTiglNACACalculator calc(tigl::NACA4DigitCode(code), te_thickness);
    EXPECT_EQ(profile.HasBluntTE(), calc.get_trailing_edge_thickness() > 0.0);

    if (!te.IsNull()) {
        ASSERT_TRUE(BRepCheck_Analyzer(te).IsValid());
    }
   
    BRepBuilderAPI_MakeWire closedWireBuilder;
    closedWireBuilder.Add(lower);
    closedWireBuilder.Add(upper);
    if (!te.IsNull()) {
        closedWireBuilder.Add(te);
    }
    closedWireBuilder.Build();
    ASSERT_TRUE(closedWireBuilder.IsDone());
    TopoDS_Wire wire = closedWireBuilder.Wire();
    ASSERT_TRUE(BRepCheck_Analyzer(wire).IsValid());

    int edgeCount = 0;
    for (TopExp_Explorer ex(wire, TopAbs_EDGE); ex.More(); ex.Next()) ++edgeCount;
    if (te.IsNull()) {
        EXPECT_EQ(edgeCount, 2);
    } else {
        EXPECT_EQ(edgeCount, 3);
    }
}

TEST(CTiglNACACalculator, naca22012_le_and_te_points){
    //tigl::CTiglNACACalculator  NACA4(2,2,1, 12, 0.00252);
    tigl::CTiglNACACalculator  NACA5(tigl::NACA5DigitCode("22112"), 0.00252);
    gp_Vec2d result1 = NACA5.upper_curve(1);
    //EXPECT_NEAR(result1.X(), (0.30103296264314128), 1e-3); 
    EXPECT_NEAR(result1.Y(), (0.00125923), 1e-6);
    
}

TEST(CTiglNACACalculator, naca23012_export_bsplines){
    //tigl::CTiglNACACalculator NACA4(2,2,0,18, 0.0);
    tigl::CTiglNACACalculator NACA5(tigl::NACA5DigitCode("22018"), 0.0);
    Handle(Geom_BSplineCurve) upperCurve = NACA5.upper_bspline(); 
    Handle(Geom_BSplineCurve) lowerCurve = NACA5.lower_bspline(); 
    ASSERT_FALSE(upperCurve.IsNull());
    ASSERT_FALSE(lowerCurve.IsNull());
    auto lowerEdge = BRepBuilderAPI_MakeEdge(lowerCurve).Edge();
    ASSERT_FALSE(lowerEdge.IsNull());
    BRepTools::Write(lowerEdge, "TestData/export/lowerEdgeTest5_22012.brep");

    auto upperEdge = BRepBuilderAPI_MakeEdge(upperCurve).Edge();
    ASSERT_FALSE(upperEdge.IsNull());
    BRepTools::Write(upperEdge, "TestData/export/upperEdgeTest5_22012.brep");
}


TEST(CTiglNACACalculator, naca23012_python){
    //tigl::CTiglNACACalculator NACA4(2,2,0,18, 0.0);
    tigl::CTiglNACACalculator NACA5(tigl::NACA5DigitCode("22018"), 0.0);
    Handle(Geom_BSplineCurve) upperCurve = NACA5.upper_bspline(); 
    Handle(Geom_BSplineCurve) lowerCurve = NACA5.lower_bspline(); 
    ASSERT_FALSE(upperCurve.IsNull());
    ASSERT_FALSE(lowerCurve.IsNull());
    auto lowerEdge = BRepBuilderAPI_MakeEdge(lowerCurve).Edge();
    ASSERT_FALSE(lowerEdge.IsNull());
    BRepTools::Write(lowerEdge, "TestData/export/lowerEdgeTest5_22012.brep");

    auto upperEdge = BRepBuilderAPI_MakeEdge(upperCurve).Edge();
    ASSERT_FALSE(upperEdge.IsNull());
    BRepTools::Write(upperEdge, "TestData/export/upperEdgeTest5_22012.brep");
}

TEST(CTiglNACACalculator, naca22112_le_and_te_points_with_class_lowerCurve){
    //tigl::CTiglNACACalculator  NACA4(2,2,1,12, 0.00252); 
    tigl::CTiglNACACalculator  NACA5(tigl::NACA5DigitCode("22112"), 0.00252); 
    tigl::CTiglNACA4LowerCurve lowerCurve(NACA5);
    EXPECT_NEAR(lowerCurve.valueX(1), (0.99999099), 1e-5); 
    EXPECT_NEAR(lowerCurve.valueY(1), 0.0, 1e-8);
    EXPECT_NEAR(lowerCurve.valueZ(1), (-0.00125997), 1e-7);//den wert hab ich von airfooilttols.com
}

TEST(CTiglNACACalculator, naca24112_le_and_te_points_with_class_lowerCurve){
    //tigl::CTiglNACACalculator  NACA4(2,4,1,12, 0.00252); 
    tigl::CTiglNACACalculator  NACA5(tigl::NACA5DigitCode("24112"), 0.00252); 
    tigl::CTiglNACA4LowerCurve lowerCurve(NACA5);
    EXPECT_NEAR(lowerCurve.valueX(1), (0.99999999), 1e-6); 
    EXPECT_NEAR(lowerCurve.valueY(1), 0.0, 1e-8);
    EXPECT_NEAR(lowerCurve.valueZ(1), (-0.00126000), 1e-6);
    //EXPECT_NEAR(lowerCurve.valueZ(1), (-6785.00126000), 1e-7);
}

TEST(CTiglNACACalculator, naca22112_export_bsplines){
    //tigl::CTiglNACACalculator NACA4(2,2,1,12, 0.00252);
    tigl::CTiglNACACalculator NACA5(tigl::NACA5DigitCode("22112"), 0.00252);
    Handle(Geom_BSplineCurve) upperCurve = NACA5.upper_bspline(); 
    Handle(Geom_BSplineCurve) lowerCurve = NACA5.lower_bspline(); 
    ASSERT_FALSE(upperCurve.IsNull());
    ASSERT_FALSE(lowerCurve.IsNull());
    auto lowerEdge = BRepBuilderAPI_MakeEdge(lowerCurve).Edge();
    ASSERT_FALSE(lowerEdge.IsNull());
    BRepTools::Write(lowerEdge, "TestData/export/lowerEdgeTest5.brep");

    auto upperEdge = BRepBuilderAPI_MakeEdge(upperCurve).Edge();
    ASSERT_FALSE(upperEdge.IsNull());
    BRepTools::Write(upperEdge, "TestData/export/upperEdgeTest5.brep");
}

TEST(CTiglNACACalculator, naca24112_export_bsplines){
    //tigl::CTiglNACACalculator NACA4(2,4,1,12, 0.00252);
    tigl::CTiglNACACalculator NACA5(tigl::NACA5DigitCode("24112"), 0.00252);
    Handle(Geom_BSplineCurve) upperCurve = NACA5.upper_bspline(); 
    Handle(Geom_BSplineCurve) lowerCurve = NACA5.lower_bspline(); 
    ASSERT_FALSE(upperCurve.IsNull());
    ASSERT_FALSE(lowerCurve.IsNull());
    auto lowerEdge = BRepBuilderAPI_MakeEdge(lowerCurve).Edge();
    ASSERT_FALSE(lowerEdge.IsNull());
    BRepTools::Write(lowerEdge, "TestData/export/lowerEdgeTest5_24112.brep");

    auto upperEdge = BRepBuilderAPI_MakeEdge(upperCurve).Edge();
    ASSERT_FALSE(upperEdge.IsNull());
    BRepTools::Write(upperEdge, "TestData/export/upperEdgeTest5_24112.brep");
}
TEST(CTiglNACACalculator, naca24112_export_bsplines2){
    tigl::CTiglNACACalculator NACA5(tigl::NACA5DigitCode("24112"), 0.00252);
    //tigl::CTiglNACACalculator NACA4(2,4,1,12, 0.00252);
    for(double x =0; x<1; x+=0.05){
        auto z = NACA5.upper_curve(x);
    }
}

TEST(CTiglNACACalculator, upper_curve_does_not_throw_for_valid_x_range){
    //tigl::CTiglNACACalculator naca(2, 4, 12, 0.00252);
    tigl::CTiglNACACalculator NACA4(tigl::NACA4DigitCode("2412"), 0.00252);
    for (double x = 0.0; x <= 1.0; x += 0.05) {
        EXPECT_NO_THROW((void)NACA4.upper_curve(x));
    }
}

TEST(CTiglNACACalculator, naca2412_getUpperLowerWire) {
    tigl::CTiglUIDManager uidMgr;
    tigl::CCPACSWingProfile cpacsProfile(static_cast<tigl::CCPACSWingProfiles*>(nullptr), &uidMgr);
    tigl::generated::CPACSNacaProfile nacadef(&cpacsProfile);

    nacadef.SetNaca4DigitCode_choice1(boost::optional<std::string>(std::string("2412")));
    nacadef.SetTrailingEdgeThickness(boost::optional<double>(0.15));

    tigl::CTiglWingProfileNACA profile(cpacsProfile, nacadef);

    TopoDS_Edge ul = profile.GetUpperLowerWire();
    EXPECT_FALSE(ul.IsNull());

    TopoDS_Edge ulSharp = profile.GetUpperLowerWire(SHARP_TRAILINGEDGE);
    EXPECT_FALSE(ulSharp.IsNull());

    TopoDS_Edge ulBlunt = profile.GetUpperLowerWire(BLUNT_TRAILINGEDGE);
    EXPECT_FALSE(ulBlunt.IsNull());

    EXPECT_TRUE(profile.HasBluntTE());

    Standard_Real u1, u2;
    Handle(Geom_Curve) ulCurve = BRep_Tool::Curve(ul, u1, u2);
    ASSERT_FALSE(ulCurve.IsNull());
}

TEST(CTiglNACACalculator, naca23012_getUpperLowerWire) {
    tigl::CTiglUIDManager uidMgr;
    tigl::CCPACSWingProfile cpacsProfile(static_cast<tigl::CCPACSWingProfiles*>(nullptr), &uidMgr);
    tigl::generated::CPACSNacaProfile nacadef(&cpacsProfile);

    nacadef.SetNaca5DigitCode_choice2(boost::optional<std::string>(std::string("23012")));
    nacadef.SetTrailingEdgeThickness(boost::optional<double>(0.15));

    tigl::CTiglWingProfileNACA profile(cpacsProfile, nacadef);

    TopoDS_Edge ul = profile.GetUpperLowerWire();
    EXPECT_FALSE(ul.IsNull());

    TopoDS_Edge ulSharp = profile.GetUpperLowerWire(SHARP_TRAILINGEDGE);
    EXPECT_FALSE(ulSharp.IsNull());

    TopoDS_Edge ulBlunt = profile.GetUpperLowerWire(BLUNT_TRAILINGEDGE);
    EXPECT_FALSE(ulBlunt.IsNull());

    EXPECT_TRUE(profile.HasBluntTE());

    Standard_Real u1, u2;
    Handle(Geom_Curve) ulCurve = BRep_Tool::Curve(ul, u1, u2);
    ASSERT_FALSE(ulCurve.IsNull());
}


TEST(CTiglNACACalculator, naca5digit_22018_bsplinePoles_fromXML) {
    const char* xmlfile = "/localdata2/gedl_ha/code/tigl/tests/TestData/naca_5_test.xml";

    TixiDocumentHandle tixiHandle = -1;
    TiglCPACSConfigurationHandle tiglHandle = -1;

    EXPECT_EQ(tixiOpenDocument(xmlfile, &tixiHandle), SUCCESS);
    ASSERT_TRUE(tixiHandle >= 0);

    EXPECT_EQ(tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle), TIGL_SUCCESS);

    char* nacaCodeStr = NULL;
    std::string xpath = "//wingAirfoil[@uID='NACA0009']/nacaProfile/naca5DigitCode";
    EXPECT_EQ(tixiGetTextElement(tixiHandle, xpath.c_str(), &nacaCodeStr), SUCCESS);
    ASSERT_STREQ(nacaCodeStr, "22018");

    //tigl::NACA5DigitCode naca5code(std::string(nacaCodeStr));
    tigl::CTiglNACACalculator nacaCalc(tigl::NACA5DigitCode(nacaCodeStr), 0.0);

    Handle(Geom_BSplineCurve) upperCurve = nacaCalc.upper_bspline();
    Handle(Geom_BSplineCurve) lowerCurve = nacaCalc.lower_bspline();

    ASSERT_FALSE(upperCurve.IsNull());
    ASSERT_FALSE(lowerCurve.IsNull());
    /*
    int nbPoles = upperCurve->NbPoles();
    ASSERT_GE(nbPoles, 1);

    std::vector<double> upperPoleX, upperPoleZ;
    for (int i = 1; i <= nbPoles; ++i) {
        gp_Pnt pole = upperCurve->Pole(i);
        upperPoleX.push_back(pole.X());
        upperPoleZ.push_back(pole.Z());

        EXPECT_GE(pole.X(), 0.0);
        EXPECT_LE(pole.X(), 1.05);
        EXPECT_GE(pole.Z(), -0.01);
    }

    for (int i = 1; i <= lowerCurve->NbPoles(); ++i) {
        gp_Pnt pole = lowerCurve->Pole(i);
        EXPECT_GE(pole.X(), 0.0);
        EXPECT_LE(pole.X(), 1.05);
        EXPECT_LE(pole.Z(), 0.01);
    }

    EXPECT_GE(upperPoleZ.front(), 0.0);
    EXPECT_LE(upperPoleZ.back(), 0.01);
    EXPECT_GE(upperPoleX.front(), 0.0);
    EXPECT_GE(upperPoleX.back(), 0.99);

    EXPECT_NO_THROW((void)nacaCalc.upper_curve(0.3));
    EXPECT_NO_THROW((void)nacaCalc.lower_curve(0.3));
    gp_Vec2d upperPt = nacaCalc.upper_curve(0.3);
    gp_Vec2d lowerPt = nacaCalc.lower_curve(0.3);
    EXPECT_GT(upperPt.Y(), lowerPt.Y());
    */
    auto upperEdge = BRepBuilderAPI_MakeEdge(upperCurve).Edge();
    auto lowerEdge = BRepBuilderAPI_MakeEdge(lowerCurve).Edge();
    ASSERT_FALSE(upperEdge.IsNull());
    ASSERT_FALSE(lowerEdge.IsNull());
    BRepTools::Write(upperEdge, "TestData/export/upperEdgeTest5_22018_fromXML.brep");
    BRepTools::Write(lowerEdge, "TestData/export/lowerEdgeTest5_22018_fromXML.brep");

    EXPECT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
    EXPECT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
}
