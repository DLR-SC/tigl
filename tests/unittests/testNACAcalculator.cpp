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
    EXPECT_NEAR(NACA.upper_y_fct(1), 0.00125843, 1e-14);
    EXPECT_NEAR(NACA.lower_y_fct(1), -0.00125843, 1e-14);
}