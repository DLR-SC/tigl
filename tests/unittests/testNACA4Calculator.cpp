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
#include "NACA4Calculator.h"
#include "generated/CPACSNacaProfile.h"
#include "CTiglUIDManager.h"
#include "Debugging.h"
#include "Cache.h"
#include <BRepCheck_Analyzer.hxx>
#include <gp_Vec2d.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>



TEST(NACA4Calculator, Nacacalculatortest1_coordinates){
    tigl::NACA4Calculator  NACA4(2,2,12, 0.00126);
    gp_Vec2d result1 = NACA4.upper_curve(1);
    EXPECT_NEAR(result1.X(), (1.00006), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.00125843), 1e-8);
    gp_Vec2d result2 = NACA4.lower_curve(1);
    EXPECT_NEAR(result2.X(), (0.999937), 1e-6); // es muss hie rbeim fehler immer auf die exakte stelle der anzahl der nkstellen geprüft werden
    EXPECT_NEAR(result2.Y(), (-0.00125843), 1e-8);

}


TEST(NACA4Calculator, naca4lowercurve_coordinates){
    tigl::NACA4Calculator  NACA4(2,2,12, 0.00126);
    tigl::NACA4LowerCurve lowerCurve(NACA4);
    EXPECT_NEAR(lowerCurve.valueX(1), (0.999937), 1e-6); // es muss hie rbeim fehler immer auf die exakte stelle der anzahl der nkstellen geprüft werden
    EXPECT_NEAR(lowerCurve.valueY(1), 0.0, 1e-8);
    EXPECT_NEAR(lowerCurve.valueZ(1), (-0.00125843), 1e-8);
}

TEST(NACA4Calculator, Nacacalculatortest2_coordinates){
    tigl::NACA4Calculator NACA4(0,0,12, 0.00126);
    gp_Vec2d result1 = NACA4.upper_curve(0.5);
    EXPECT_NEAR(result1.X(), (0.5), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result1.Y(), (0.0529403), 1e-7);
    gp_Vec2d result2 = NACA4.lower_curve(0.5);
    EXPECT_NEAR(result2.X(), (0.5), 1e-6); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result2.Y(), (-0.0529403), 1e-7);
    gp_Vec2d result3 = NACA4.upper_curve(0.);
    EXPECT_NEAR(result3.X(), (0.0), 1e-5); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result3.Y(), (0.0), 1e-7);
    gp_Vec2d result4 = NACA4.lower_curve(0.);
    EXPECT_NEAR(result4.X(), (0.0), 1e-6); // hier muss jz ein vektor das ergebnis sein
    EXPECT_NEAR(result4.Y(), (0.0), 1e-7);
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
    EXPECT_NEAR(result1.X(), (1.00022), 1e-5); 
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

TEST(NACA4Calculator, Nacacalculatortest12_trailingedge){
    tigl::NACA4Calculator NACA4(0,0,15, 5);
    double result1 = NACA4.trailing_edge_thickness_function(5);
    EXPECT_NEAR(result1, -6.5630666666666668, 1e-14);
}


TEST(NACA4Calculator, Nacacalculatortest13_trailingedge){
    tigl::NACA4Calculator NACA4(2,2,12, 15);
    double result1 = NACA4.trailing_edge_thickness_function(15);
    EXPECT_NEAR(result1, -24.8964, 1e-14);
}

TEST(NACA4Calculator, Nacacalculatortest14){
    tigl::NACA4Calculator NACA4(2,2,12, 15);
    tigl::NACA4UpperCurve upperCurve(NACA4);
    ASSERT_EQ(upperCurve.valueY(0.), 0.);
    ASSERT_EQ(upperCurve.valueY(0.5), 0.);
    ASSERT_EQ(upperCurve.valueY(1.), 0.);
    gp_Vec2d pnt = NACA4.upper_curve(0.5);
    ASSERT_EQ(upperCurve.valueX(0.5), pnt.X());
    ASSERT_EQ(upperCurve.valueZ(0.5), pnt.Y());
}

TEST(NACA4Calculator, Nacacalculatortest15){
    tigl::NACA4Calculator NACA4(2,2,12, 15);
    tigl::NACA4LowerCurve lowerCurve(NACA4);
    
    ASSERT_EQ(lowerCurve.valueY(0.), 0.);
    ASSERT_EQ(lowerCurve.valueY(0.5), 0.);
    ASSERT_EQ(lowerCurve.valueY(1.), 0.);

    gp_Vec2d pnt = NACA4.lower_curve(0.5);
    ASSERT_EQ(lowerCurve.valueX(0.5), pnt.X());
    ASSERT_EQ(lowerCurve.valueZ(0.5), pnt.Y());
}

TEST(NACA4Calculator, Nacacalculatortest16)
{
    tigl::NACA4Calculator NACA4(2,2,12, 15);
    //tigl::NACA4LowerCurve lowerCurve(NACA4);
    Handle(Geom_BSplineCurve) lower_spline = NACA4.lower_bspline();

    gp_Vec2d pnt = NACA4.lower_curve(0.5);
    gp_Pnt pnt2;
    lower_spline->D0(0.5, pnt2);
    ASSERT_NEAR(pnt2.X(), pnt.X(), 1e-4);
    ASSERT_NEAR(pnt2.Z(), pnt.Y(), 1e-4);
}

TEST(NACA4Calculator, Nacacalculatortest17){
    tigl::NACA4Calculator NACA4(2,2,12, .15);
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
/*
TEST(NACA4Calculator, Nacacalculatortest18){
    const char* filename = "TestData/naca_test.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // read configuration
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
        Standard_Real u1, u2;

        // get profile curves of 1st airfoil
        tigl::CCPACSWingProfile & profile = config.GetWingProfile("NACA0012");
        TopoDS_Edge upperWire = profile.GetUpperWire();
}
*/
TEST(NACA4Calculator, Nacacalculatortest19){

    // Create a UID manager and use it for the profile. Parent container is not needed for this unit test.
    tigl::CTiglUIDManager uidMgr;
    // disambiguate nullptr for overloaded constructors by casting to the intended parent type
    tigl::CCPACSWingProfile cpacsProfile(static_cast<tigl::CCPACSWingProfiles*>(nullptr), &uidMgr);
    tigl::generated::CPACSNacaProfile nacadef(&cpacsProfile);


    nacadef.SetNaca4DigitCode_choice1(boost::optional<std::string>(std::string("2412")));
    nacadef.SetTrailingEdgeThickness(boost::optional<double>(0.15));

    tigl::CTiglWingProfileNACA profile(cpacsProfile, nacadef);

    tigl::dumpShape(profile.GetLowerWire(), "TestData/export", "lower_wire", 0);

    TopoDS_Edge upper = profile.GetUpperWire();
    TopoDS_Edge lower = profile.GetLowerWire();

    EXPECT_FALSE(upper.IsNull());
    EXPECT_FALSE(lower.IsNull());
}

TEST(NACA4Calculator, Nacacalculatortest21_trailingEdge_absent_when_zero_thickness){
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
TEST(NACA4Calculator, Nacacalculatortest22){

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


    tigl::NACA4Calculator calc(code, te_thickness);
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

    tigl::dumpShape(wire, "TestData/export", "naca_profile1_wire", 0);
}





//trailing_edge
//upperlowercurve erstmal weglassen (nur eine exception werfen)

