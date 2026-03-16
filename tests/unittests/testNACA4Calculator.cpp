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
#include "NACA4Calculator.h"
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




TEST(NACA4Calculator, Nacacalculatortest1_coordinates){
    tigl::NACA4Calculator  NACA4(2,2,12, 0.00252);
    if(NACA4.profile_thickness(1) > 0){ //tests if yt > 0 is weil ich den nd im calculator machn darf
    gp_Vec2d result1 = NACA4.upper_curve(1);
    EXPECT_NEAR(result1.X(), (1.00006), 1e-5); 
    EXPECT_NEAR(result1.Y(), (0.00125843), 1e-8);
    gp_Vec2d result2 = NACA4.lower_curve(1);
    EXPECT_NEAR(result2.X(), (0.999937), 1e-6); 
    EXPECT_NEAR(result2.Y(), (-0.00125843), 1e-8);
    }
    if(NACA4.profile_thickness(0) > 0){
    gp_Vec2d result3 = NACA4.upper_curve(0.);
    EXPECT_NEAR(result3.X(), (0.0), 1e-5); 
    EXPECT_NEAR(result3.Y(), (0.0), 1e-7);
    gp_Vec2d result4 = NACA4.lower_curve(0.);
    EXPECT_NEAR(result4.X(), (0.0), 1e-6); 
    EXPECT_NEAR(result4.Y(), (0.0), 1e-7);
    }
}

TEST(NACA4Calculator, Nacacalculatortestnaca4lowercurve_coordinates){
    tigl::NACA4Calculator  NACA4(2,2,12, 0.00252); //davor wars 0.00126
    tigl::NACA4LowerCurve lowerCurve(NACA4);
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

TEST(NACA4Calculator, Nacacalculatortest2_coordinates){
    tigl::NACA4Calculator NACA4(0,0,12, 0.00252);
    if(NACA4.profile_thickness(0.5) > 0){
    gp_Vec2d result1 = NACA4.upper_curve(0.5);
    EXPECT_NEAR(result1.X(), (0.5), 1e-5); 
    EXPECT_NEAR(result1.Y(), (0.0529403), 1e-7);
    gp_Vec2d result2 = NACA4.lower_curve(0.5);
    EXPECT_NEAR(result2.X(), (0.5), 1e-6); 
    EXPECT_NEAR(result2.Y(), (-0.0529403), 1e-7);
    }
    if(NACA4.profile_thickness(0.)){
    gp_Vec2d result3 = NACA4.upper_curve(0.);
    EXPECT_NEAR(result3.X(), (0.0), 1e-5);  
    EXPECT_NEAR(result3.Y(), (0.0), 1e-7);
    gp_Vec2d result4 = NACA4.lower_curve(0.);
    EXPECT_NEAR(result4.X(), (0.0), 1e-6); 
    EXPECT_NEAR(result4.Y(), (0.0), 1e-7);
    }
}

TEST(NACA4Calculator, Nacacalculatortest3_coordinates){
    tigl::NACA4Calculator NACA4(0,0,9, 0.00189);
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

TEST(NACA4Calculator, Nacacalculatortest4_coordinates){
    tigl::NACA4Calculator NACA4(6,5,9, 0.00189);
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

TEST(NACA4Calculator, Nacacalculatortest5_thickness){ 
    tigl::NACA4Calculator NACA4(0,0,12, 0.00126);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); 
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
}

TEST(NACA4Calculator, Nacacalculatortest6_thickness){ 
    tigl::NACA4Calculator NACA4(0,0,18, 0.00189);
    double result1 = NACA4.profile_thickness(0.3); 
    double left_result = NACA4.profile_thickness(0.299); 
    double right_result = NACA4.profile_thickness(0.311);
    EXPECT_GT(result1, left_result);
    EXPECT_GT(result1, right_result);
    EXPECT_NEAR(result1, 0.09, 1e-2); //TODO: Fact check this 1e-2
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


/*
TEST(NACA4Calculator, Nacacalculatortest10_trailingedge){
    tigl::NACA4Calculator NACA4(0,0,15, 9e-05);
    double result1 = NACA4.trailing_edge_thickness_function(9e-05);
    EXPECT_NEAR(result1, 0.10348, 1e-14);
}

TEST(NACA4Calculator, Nacacalculatortest11_trailingedge_thickness){
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
    */

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

TEST(NACA4Calculator, Nacacalculatortest18){
    tigl::NACA4Calculator NACA4(0,0,12, .015);
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

TEST(NACA4Calculator, Nacacalculatortest19_LePoint_TePoint){

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

    tigl::NACA4Calculator NACA4(2,4,12, 0.15);
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

}

TEST(NACA4Calculator, Nacacalculatortest20_LePoint_TePoint){

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

    tigl::NACA4Calculator NACA4(0,0,12, 0.015);
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

    int edgeCount = 0;
    for (TopExp_Explorer ex(wire, TopAbs_EDGE); ex.More(); ex.Next()) ++edgeCount;
    if (te.IsNull()) {
        EXPECT_EQ(edgeCount, 2);
    } else {
        EXPECT_EQ(edgeCount, 3);
    }
}


TEST(NACA4Calculator, Nacacalculatortest23){
    const char* filename = "TestData/naca_test.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        TixiDocumentHandle           tixiHandle;
        TiglCPACSConfigurationHandle tiglHandle;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "Cpacs2Test", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // read configuration
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
        Standard_Real u1, u2;

        // get profile curves of airfoil
        tigl::CCPACSWingProfile & profile = config.GetWingProfile("NACA0012");
        TopoDS_Edge upperWire = profile.GetUpperWire();
        EXPECT_TRUE(!upperWire.IsNull());
        Handle(Geom_Curve) upperCurve = BRep_Tool::Curve(upperWire, u1, u2);
        ASSERT_TRUE(!upperCurve.IsNull());
        //dumpshape dazufügen
}

TEST(NACA4Calculator, Nacacalculatortest24)
{
    TiglHandleWrapper tiglHandle("TestData/naca_test.cpacs.xml", "");

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

      Standard_Real u1, u2;
    tigl::CCPACSWingProfile& profile = config.GetWingProfile("NACA0012");
    TopoDS_Edge upperWire = profile.GetUpperWire();
        EXPECT_TRUE(!upperWire.IsNull());
        Handle(Geom_Curve) upperCurve = BRep_Tool::Curve(upperWire, u1, u2);
        ASSERT_TRUE(!upperCurve.IsNull());
}

//negative werte mögl?


