#include "test.h"

#include "CTiglCurvesToSurface.h"

#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <GeomConvert.hxx>
#include <Precision.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>


#include <sstream>
#include <tiglcommonfunctions.h>
#include <Geom_BSplineCurve.hxx>

namespace tigl
{

TEST(CTiglCurvesToSurface, testSkinnedBSplineSurface)
{
    /*
     * Tests the method skinning_bspline_surface(spline_list, degree_v_direction, parameters) by creating a skinned
     * surface by two B-splines (argument u), Skinning direction is v, so:
     * surface(u, v) = curve1(u) * (1 - v) + curve2(u) * v
     */

    // degree of both B-splines
    unsigned int degree_u = 2;

    // create first B-spline that represents the parabola f(x) = (x - 0.5)^2 in order to being able to test it afterwards
    TColgp_Array1OfPnt controlPoints1(1, 3);
    controlPoints1(1) = gp_Pnt(0., 6., 0.25);
    controlPoints1(2) = gp_Pnt(0.5, 6., - 0.25);
    controlPoints1(3) = gp_Pnt(1., 6., 0.25);

    TColStd_Array1OfReal knots(1, 2);
    knots(1) = 0.;
    knots(2) = 1.;

    TColStd_Array1OfInteger mults(1, 2);
    mults(1) = 3;
    mults(2) = 3;

    Handle(Geom_BSplineCurve) curve1 = new Geom_BSplineCurve(controlPoints1, knots, mults, degree_u);

    // create second B-spline that represents the parabola g(x) = - (x - 0.5)^2 in order to being able to test it afterwards
    TColgp_Array1OfPnt controlPoints2(1, 3);
    controlPoints2(1) = gp_Pnt(0., 0., - 0.25);
    controlPoints2(2) = gp_Pnt(0.5, 0., 0.25);
    controlPoints2(3) = gp_Pnt(1., 0., - 0.25);

    Handle(Geom_BSplineCurve) curve2 = new Geom_BSplineCurve(controlPoints2, knots, mults, degree_u);

    std::vector<Handle(Geom_Curve) > splines_vector;
    splines_vector.push_back(curve1);
    splines_vector.push_back(curve2);

    CTiglCurvesToSurface skinner(splines_vector);
    Handle(Geom_BSplineSurface) skinnedSurface = skinner.Surface();


    // now test the skinned surface
    for (int u_idx = 0; u_idx < 100; ++u_idx) {
        for (int v_idx = 0; v_idx < 100; ++v_idx) {
            double u_value = u_idx / 100.;
            double v_value = v_idx / 100.;

            gp_Pnt surface_point = skinnedSurface->Value(u_value, v_value);
            gp_Pnt point_curve1 = curve1->Value(u_value);
            gp_Pnt point_curve2 = curve2->Value(u_value);
            gp_Pnt right_point(point_curve1.X() * (1 - v_value) + point_curve2.X() * v_value, point_curve1.Y() * (1 - v_value) + point_curve2.Y() * v_value, point_curve1.Z() * (1 - v_value) + point_curve2.Z() * v_value);

            ASSERT_NEAR(surface_point.X(), right_point.X(), 1e-15);
            ASSERT_NEAR(surface_point.Y(), right_point.Y(), 1e-15);
            ASSERT_NEAR(surface_point.Z(), right_point.Z(), 1e-15);
        }
    }
}

TEST(TiglBSplineAlgorithms, curvesToSurfaceContinous)
{
    // Read in nacelle data from BRep
    TopoDS_Shape shape_u;

    BRep_Builder builder_u;

    BRepTools::Read(shape_u, "TestData/CurveNetworks/fuselage1/guides.brep", builder_u);

    // get the splines in u-direction from the Edges
    std::vector<Handle(Geom_Curve)> curves;
    for (TopExp_Explorer exp(shape_u, TopAbs_EDGE); exp.More(); exp.Next()) {
        TopoDS_Edge curve_edge = TopoDS::Edge(exp.Current());
        double beginning = 0;
        double end = 1;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(curve_edge, beginning, end);
        Handle(Geom_BSplineCurve) spline = GeomConvert::CurveToBSplineCurve(curve);
        curves.push_back(spline);
    }

    CTiglCurvesToSurface skinner(curves, true);
    Handle(Geom_BSplineSurface) surface = skinner.Surface();

    double umin, umax, vmin, vmax;
    surface->Bounds(umin, umax, vmin, vmax);
    EXPECT_TRUE(surface->DN(umin, vmin, 0, 0).IsEqual(surface->DN(umin, vmax, 0, 0), 1e-10, 1e-6));
    EXPECT_TRUE(surface->DN(umin, vmin, 0, 1).IsEqual(surface->DN(umin, vmax, 0, 1), 1e-10, 1e-6));

    double umean = 0.5 * (umin + umax);
    EXPECT_TRUE(surface->DN(umean, vmin, 0, 0).IsEqual(surface->DN(umean, vmax, 0, 0), 1e-10, 1e-6));
    EXPECT_TRUE(surface->DN(umean, vmin, 0, 1).IsEqual(surface->DN(umean, vmax, 0, 1), 1e-10, 1e-6));

    EXPECT_TRUE(surface->DN(umax, vmin, 0, 0).IsEqual(surface->DN(umax, vmax, 0, 0), 1e-10, 1e-6));
    EXPECT_TRUE(surface->DN(umax, vmin, 0, 1).IsEqual(surface->DN(umax, vmax, 0, 1), 1e-10, 1e-6));

    // Write surface
    BRepTools::Write(BRepBuilderAPI_MakeFace(surface, Precision::Confusion()).Face(), "TestData/curvesToSurfaceContinous.brep");
}

TEST(TiglBSplineAlgorithms, curvesToSurfaceBug)
{

    std::vector<Handle(Geom_Curve)> profileCurves;
    double xmin=1e6;
    double xmax=-1e6;
    for (int i = 5; i<9; ++i) {

        std::stringstream ss;
        ss << "TestData/bugs/501/edge_1_profile_" << i << ".brep";

        TopoDS_Edge edge;
        BRep_Builder builder;
        BRepTools::Read(edge, ss.str().c_str(), builder);

        Standard_Real umin, umax;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, umin, umax);
        gp_Vec p = curve->DN(umin, 0);
        xmin = p.X() < xmin ? p.X() : xmin;
        xmax = p.X() > xmax ? p.X() : xmax;

        profileCurves.push_back(GetBSplineCurve(edge));
    }

    tigl::CTiglCurvesToSurface surfaceSkinner(profileCurves);
    surfaceSkinner.SetMaxDegree(1);
    Handle(Geom_BSplineSurface) surface = surfaceSkinner.Surface();

    double umin, umax, vmin, vmax;
    surface->Bounds(umin, umax, vmin, vmax);

    double u = umin + 0.999*(umax-umin);
    gp_Vec p = surface->DN(u, vmin + 0.95*(vmax-vmin), 0, 0);
    EXPECT_TRUE(p.X() <= xmax);
    EXPECT_TRUE(p.X() >= xmin);

    BRepBuilderAPI_MakeFace faceMaker(surface, 1e-10);
    TopoDS_Face result = faceMaker.Face();
    BRepTools::Write(result, "TestData/bugs/501/resultShape.brep");


}

}
