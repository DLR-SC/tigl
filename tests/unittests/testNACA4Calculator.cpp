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
#include "CTiglNACA4Calculator.h"
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
#include <gp_Vec.hxx>

TEST(CTiglNACA4Calculator, naca2212_le_and_te_points){
    tigl::CTiglNACA4Calculator  NACA4(2,2,12, 0.00252);
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

TEST(CTiglNACA4Calculator, naca2212_le_and_te_points_with_class_lowerCurve){
    tigl::CTiglNACA4Calculator  NACA4(2,2,12, 0.00252); 
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

TEST(CTiglNACA4Calculator, naca0012_random_point){
    tigl::CTiglNACA4Calculator NACA4(0,0,12, 0.00252);
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

TEST(CTiglNACA4Calculator, naca0009_random_point_and_le_point){
    tigl::CTiglNACA4Calculator NACA4(0,0,9, 0.00189);
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

TEST(CTiglNACA4Calculator, Nnaca6509_le_and_te_points){
    tigl::CTiglNACA4Calculator NACA4(6,5,9, 0.00189);
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

TEST(CTiglNACA4Calculator, naca0012_max_profile_thickness){ 
    tigl::CTiglNACA4Calculator NACA4(0,0,12, 0.00126);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); 
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
    EXPECT_NEAR(result1, 0.06001216339, 1e-11); 
}

TEST(CTiglNACA4Calculator, naca0018_max_profile_thickness){ 
    tigl::CTiglNACA4Calculator NACA4(0,0,18, 0.00189);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); 
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
    EXPECT_NEAR(result1, 0.09001824509, 1e-12); 
}

TEST(CTiglNACA4Calculator, naca2212_camberline_at_te_and_le){ 
    tigl::CTiglNACA4Calculator NACA4(2,2,12, 0.00126);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}

TEST(CTiglNACA4Calculator, naca4509_camberline_at_te_and_le){ 
    tigl::CTiglNACA4Calculator NACA4(4,5,9, 0.000945);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}

TEST(CTiglNACA4Calculator, naca0015_camberline_at_te_and_le){ 
    tigl::CTiglNACA4Calculator NACA4(0,0,15, 0.001575);
    double result1 = NACA4.camberline(0.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
    double result2 = NACA4.camberline(1.0); 
    EXPECT_NEAR(result1, 0, 1e-14);
}   

TEST(CTiglNACA4Calculator, naca15030105_assertion_throw_normalization){
    
    EXPECT_THROW(tigl::CTiglNACA4Calculator NACA4(150,30,105, 0.001575), tigl::CTiglError);
}

TEST(CTiglNACA4Calculator, naca03010_assertion_throw_normalization){
    
    EXPECT_THROW(tigl::CTiglNACA4Calculator NACA4(0,30,10, 0.001575), tigl::CTiglError);
}

TEST(CTiglNACA4Calculator, naca03250_assertion_throw_normalization){
    
    EXPECT_THROW(tigl::CTiglNACA4Calculator NACA4(0,3,250, 0.001575), tigl::CTiglError);
}


TEST(CTiglNACA4Calculator, naca0015_trailingedge_length){
    tigl::CTiglNACA4Calculator NACA4(0,0,15, 0.12);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    double half_thickness1_y = result1.Y();
    double half_thickness1_x = result1.X();
    gp_Vec2d result2 = NACA4.lower_curve(1);
    double half_thickness2_y = result2.Y();
    double half_thickness2_x = result2.X();
    double thickness = sqrt((half_thickness1_x - half_thickness2_x)*(half_thickness1_x - half_thickness2_x)+(half_thickness1_y - half_thickness2_y)*(half_thickness1_y - half_thickness2_y));


    EXPECT_NEAR(thickness, 0.12, 1e-14);
}

TEST(CTiglNACA4Calculator, naca001515_trailingedge_length){
    tigl::CTiglNACA4Calculator NACA4(0,0,15, 0.15);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    double half_thickness1_y = result1.Y();
    double half_thickness1_x = result1.X();
    gp_Vec2d result2 = NACA4.lower_curve(1);
    double half_thickness2_y = result2.Y();
    double half_thickness2_x = result2.X();
    double thickness = sqrt((half_thickness1_x - half_thickness2_x)*(half_thickness1_x - half_thickness2_x)+(half_thickness1_y - half_thickness2_y)*(half_thickness1_y - half_thickness2_y));

    EXPECT_NEAR(thickness, 0.15, 1e-14);
}

TEST(CTiglNACA4Calculator, naca2215_trailingedge_length){
    tigl::CTiglNACA4Calculator NACA4(2,2,15, 0.20);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    double half_thickness1_y = result1.Y();
    double half_thickness1_x = result1.X();
    gp_Vec2d result2 = NACA4.lower_curve(1);
    double half_thickness2_y = result2.Y();
    double half_thickness2_x = result2.X();
    double thickness = sqrt((half_thickness1_x - half_thickness2_x)*(half_thickness1_x - half_thickness2_x)+(half_thickness1_y - half_thickness2_y)*(half_thickness1_y - half_thickness2_y));


    EXPECT_NEAR(thickness, 0.20, 1e-14);
}

TEST(CTiglNACA4Calculator, naca6415_trailingedge_length){
    tigl::CTiglNACA4Calculator NACA4(6,4,15, 0.13);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    double half_thickness1_y = result1.Y();
    double half_thickness1_x = result1.X();
    gp_Vec2d result2 = NACA4.lower_curve(1);
    double half_thickness2_y = result2.Y();
    double half_thickness2_x = result2.X();
    double thickness = sqrt((half_thickness1_x - half_thickness2_x)*(half_thickness1_x - half_thickness2_x)+(half_thickness1_y - half_thickness2_y)*(half_thickness1_y - half_thickness2_y));


    EXPECT_NEAR(thickness, 0.13, 1e-14);
}

TEST(CTiglNACA4Calculator, naca2212_upperCurve_ycoord_and_upper_curve_x_and_zcoord){
    tigl::CTiglNACA4Calculator NACA4(2,2,12, 15);
    tigl::CTiglNACA4UpperCurve upperCurve(NACA4);
    ASSERT_EQ(upperCurve.valueY(0.), 0.);
    ASSERT_EQ(upperCurve.valueY(0.5), 0.);
    ASSERT_EQ(upperCurve.valueY(1.), 0.);
    // upperCurve.valueX/valueZ(t) evaluate the analytic curve at x=t*t, not x=t directly
    // (see CTiglNACA4UpperCurve::valueX)
    gp_Vec2d pnt = NACA4.upper_curve(0.5*0.5);
    ASSERT_EQ(upperCurve.valueX(0.5), pnt.X());
    ASSERT_EQ(upperCurve.valueZ(0.5), pnt.Y());
}

TEST(CTiglNACA4Calculator, naca2212_lowerCurve_ycoord_and_lower_curve_x_and_zcoord){
    tigl::CTiglNACA4Calculator NACA4(2,2,12, 15);
    tigl::CTiglNACA4LowerCurve lowerCurve(NACA4);

    ASSERT_EQ(lowerCurve.valueY(0.), 0.);
    ASSERT_EQ(lowerCurve.valueY(0.5), 0.);
    ASSERT_EQ(lowerCurve.valueY(1.), 0.);

    // lowerCurve.valueX/valueZ(t) evaluate the analytic curve at x=t*t, not x=t directly
    // (see CTiglNACA4UpperCurve::valueX)
    gp_Vec2d pnt = NACA4.lower_curve(0.5*0.5);
    ASSERT_EQ(lowerCurve.valueX(0.5), pnt.X());
    ASSERT_EQ(lowerCurve.valueZ(0.5), pnt.Y());
}

TEST(CTiglNACA4Calculator, naca2212_bspline_vs_lower_curve_coord)
{
    tigl::CTiglNACA4Calculator NACA4(2,2,12, 15);
    Handle(Geom_BSplineCurve) lower_spline = NACA4.lower_bspline();

    // the bspline's own parameter u corresponds to x=u*u, not x=u directly (see
    // CTiglNACA4UpperCurve::valueX)
    gp_Vec2d pnt = NACA4.lower_curve(0.5*0.5);
    gp_Pnt pnt2;
    lower_spline->D0(0.5, pnt2);
    ASSERT_NEAR(pnt2.X(), pnt.X(), 1e-4);
    ASSERT_NEAR(pnt2.Z(), pnt.Y(), 1e-4);
}

TEST(CTiglNACA4Calculator, naca2212_export_bsplines){
    tigl::CTiglNACA4Calculator NACA4(2,2,12, .15);
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

TEST(CTiglNACA4Calculator, naca0012_export_bsplines){
    tigl::CTiglNACA4Calculator NACA4(0,0,12, .015);
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

TEST(CTiglNACA4Calculator, naca2412_LePoint_TePoint){

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

    tigl::CTiglNACA4Calculator NACA4(2,4,12, 0.15);
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

TEST(CTiglNACA4Calculator, naca0012_LePoint_TePoint){

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

    tigl::CTiglNACA4Calculator NACA4(0,0,12, 0.015);
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

TEST(CTiglNACA4Calculator, naca0012_trailingEdge_absent_when_zero_thickness){
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

TEST(CTiglNACA4Calculator, naca2412_edge_counter){

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


    tigl::CTiglNACA4Calculator calc(code, te_thickness);
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

TEST(CTiglNACA4Calculator, upper_curve_does_not_throw_for_valid_x_range){
    tigl::CTiglNACA4Calculator naca(2, 4, 12, 0.00252);
    for (double x = 0.0; x <= 1.0; x += 0.05) {
        EXPECT_NO_THROW((void)naca.upper_curve(x));
    }
}



TEST(CTiglNACA4Calculator, naca2412_getUpperLowerWire) {
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

// Regression test for CFunctionToBspline::concatC1 (used internally by
// CTiglNACA4Calculator::upper_bspline/lower_bspline). A strongly cambered profile forces the
// adaptive Chebyshev fit to produce many segments, exercising the multi-segment
// concatenation path. Away from the leading/trailing edge (where the thickness formula's
// sqrt(x) term makes the true tangent direction change very fast / become vertical - an
// inherent feature of the NACA4 shape, not a bug), every internal knot join should now be
// honestly continuous, since concatC1 checks continuity instead of blindly declaring it.
TEST(CTiglNACA4Calculator, naca6415_upper_lower_bspline_c1_continuous_everywhere)
{
    // Strongly cambered profile: exercises the adaptive multi-segment path in
    // CFunctionToBspline. CTiglNACA4UpperCurve/LowerCurve reparametrize with x=t*t, which
    // removes the thickness distribution's sqrt(x) derivative singularity at the leading
    // edge - so unlike before that reparametrization, every internal knot (including right
    // at the leading/trailing edge) should now be honestly C1 continuous, with no margin
    // needed to exclude a "genuinely near-vertical" region.
    tigl::CTiglNACA4Calculator NACA4(6, 4, 15, 0.13);

    auto checkContinuity = [](const Handle(Geom_BSplineCurve)& curve) {
        // make sure this actually exercises the multi-segment concatenation path
        ASSERT_GT(curve->NbKnots(), 2);

        const double eps = 1e-7;
        for (int i = 2; i < curve->NbKnots(); ++i) {
            double u = curve->Knot(i);

            gp_Pnt pLeft, pRight;
            gp_Vec dLeft, dRight;
            curve->D1(u - eps, pLeft, dLeft);
            curve->D1(u + eps, pRight, dRight);

            EXPECT_NEAR(pLeft.Distance(pRight), 0.0, 1e-6) << "position jump at knot " << u;
            EXPECT_NEAR(dLeft.Angle(dRight), 0.0, 1e-3) << "tangent kink at knot " << u;
        }
    };

    checkContinuity(NACA4.upper_bspline());
    checkContinuity(NACA4.lower_bspline());
}
