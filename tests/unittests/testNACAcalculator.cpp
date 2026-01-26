#include "test.h" // Brings in the GTest framework
#include "testUtils.h"
#include "tigl.h"
#include "math/tiglmathfunctions.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWingProfile.h"
#include "BRep_Tool.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepTools.hxx"
#include "Geom_Curve.hxx"
#include "gp_Pnt.hxx"
#include "gp_Pnt.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "Geom_BSplineCurve.hxx"
#include "CCSTCurveBuilder.h"
#include "NACAcalculator.cpp" //TODO: Include header file



TEST(NACAcalculator, Nacacalculatortest1){
    NACAcalculator  NACA(2,2,12);
    EXPECT_NEAR(NACA.upper_y_fct(1), 0.00125843, 1e-5);
    EXPECT_NEAR(NACA.lower_y_fct(1), -0.00125843, 1e-5);
}

TEST(NACAcalculator, Nacacalculatortest2){
    NACAcalculator NACA(0,0,12);
    EXPECT_NEAR(NACA.upper_y_fct(0.5), 0.0529403, 1e-5);
    EXPECT_NEAR(NACA.lower_y_fct(0.5), -0.0529403, 1e-5);
}

TEST(NACAcalculator, Nacacalculatortest3){
    NACAcalculator NACA(0,0,9);
    EXPECT_NEAR(NACA.upper_y_fct(0.2), 0.0430316, 1e-5);
    EXPECT_NEAR(NACA.lower_y_fct(0.2), -0.0430316, 1e-5);
}

TEST(NACAcalculator, Nacacalculatortest4){
    NACAcalculator NACA(6,5,9);
    EXPECT_NEAR(NACA.upper_y_fct(1), 0.000918906, 1e-4);
    EXPECT_NEAR(NACA.lower_y_fct(1), -0.000918906, 1e-4);
}