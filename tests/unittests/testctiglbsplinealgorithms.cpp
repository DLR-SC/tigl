#include "test.h"
#include "CTiglError.h"

#include "tigl_internal.h"
#include <Geom_BSplineSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include<Convert_ParabolaToBSplineCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <vector>
#include <cmath>

#include <CTiglBSplineAlgorithms.h>

#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Precision.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <GeomConvert.hxx>


namespace tigl
{

TEST(TiglBSplineAlgorithms, testComputeParamsBSplineCurve)
{
    // test for method computeParamsBSplineCurve

    // create B-spline
    TColgp_Array1OfPnt controlPoints(1, 4);
    controlPoints(1) = gp_Pnt(0, 0, 0);
    controlPoints(2) = gp_Pnt(1, 1, 0);
    controlPoints(3) = gp_Pnt(3, -1, 0);
    controlPoints(4) = gp_Pnt(4, 0, 0);

    TColStd_Array1OfReal Weights(1, 4);
    Weights(1) = 1;
    Weights(2) = 1;
    Weights(3) = 1;
    Weights(4) = 1;

    TColStd_Array1OfReal Knots(1, 2);
    Knots(1) = 0.;
    Knots(2) = 1;

    TColStd_Array1OfInteger Multiplicities(1, 2);
    Multiplicities(1) = 4;
    Multiplicities(2) = 4;

    Standard_Integer Degree = 3;

    Handle(Geom_BSplineCurve) bspline = new Geom_BSplineCurve(controlPoints, Weights, Knots, Multiplicities, Degree);

    // compute centripetal parameters by the method that shall be tested here
    double alpha = 0.5;
    Handle(TColStd_HArray1OfReal) parameters = CTiglBSplineAlgorithms::computeParamsBSplineCurve(controlPoints, alpha);

    // compute right parameters by hand
    TColStd_Array1OfReal right_parameters(1, 4);
    double first_to_second = pow(controlPoints(1).SquareDistance(controlPoints(2)), 0.5);
    double second_to_third = pow(controlPoints(2).SquareDistance(controlPoints(3)), 0.5);
    double third_to_fourth = pow(controlPoints(3).SquareDistance(controlPoints(4)), 0.5);
    double polygon_length_alpha = pow(first_to_second, alpha) + pow(second_to_third, alpha) + pow(third_to_fourth, alpha);

    right_parameters(1) = 0;
    right_parameters(2) = pow(first_to_second, alpha) / polygon_length_alpha;
    right_parameters(3) = (pow(first_to_second, alpha) + pow(second_to_third, alpha)) / polygon_length_alpha;
    right_parameters(4) = 1;

    // assure that computed parameters are right
    ASSERT_NEAR(parameters->Value(1), right_parameters(1), 1e-15);
    ASSERT_NEAR(parameters->Value(2), right_parameters(2), 1e-15);
    ASSERT_NEAR(parameters->Value(3), right_parameters(3), 1e-15);
    ASSERT_NEAR(parameters->Value(4), right_parameters(4), 1e-15);
}

TEST(TiglBSplineAlgorithms, testComputeParamsBSplineSurface)
{
    // test for method computeParamsBSplineSurf

    // TODO
}

TEST(TiglBSplineAlgorithms, testCreateCommonKnotsVectorCurve)
{
    // test for method createCommonKnotsVectorCurve

    // TODO B-spline knot insertion doesn't work on the parameter range boundaries
    // create first B-spline
    TColgp_Array1OfPnt controlPoints1(1, 4);
    controlPoints1(1) = gp_Pnt(0, 0, 0);
    controlPoints1(2) = gp_Pnt(1, 1, 0);
    controlPoints1(3) = gp_Pnt(3, -1, 0);
    controlPoints1(4) = gp_Pnt(4, 0, 0);

    TColStd_Array1OfReal Weights1(1, 4);
    Weights1(1) = 1;
    Weights1(2) = 1;
    Weights1(3) = 1;
    Weights1(4) = 1;

    TColStd_Array1OfReal Knots1(1, 2);
    Knots1(1) = 0.;
    Knots1(2) = 1;

    TColStd_Array1OfInteger Multiplicities1(1, 2);
    Multiplicities1(1) = 4;
    Multiplicities1(2) = 4;

    Standard_Integer Degree1 = 3;

    Handle(Geom_BSplineCurve) bspline1 = new Geom_BSplineCurve(controlPoints1, Weights1, Knots1, Multiplicities1, Degree1);

    bspline1->InsertKnot(1. / 3);
    // knot vector of bspline1 is now: [0, 0, 0, 0, 1./3, 1, 1, 1, 1, 1]

    // create second B-spline
    // create B-spline
    TColgp_Array1OfPnt controlPoints2(1, 4);
    controlPoints2(1) = gp_Pnt(-2, -1, 0);
    controlPoints2(2) = gp_Pnt(0, 1, 0);
    controlPoints2(3) = gp_Pnt(3, -1, 0);
    controlPoints2(4) = gp_Pnt(5, 0, 0);

    TColStd_Array1OfReal Weights2(1, 4);
    Weights2(1) = 1;
    Weights2(2) = 1;
    Weights2(3) = 1;
    Weights2(4) = 1;

    TColStd_Array1OfReal Knots2(1, 2);
    Knots2(1) = 0.;
    Knots2(2) = 1;

    TColStd_Array1OfInteger Multiplicities2(1, 2);
    Multiplicities2(1) = 4;
    Multiplicities2(2) = 4;

    Standard_Integer Degree2 = 3;

    Handle(Geom_BSplineCurve) bspline2 = new Geom_BSplineCurve(controlPoints2, Weights2, Knots2, Multiplicities2, Degree2);

    bspline2->InsertKnot(0.5);
    bspline2->InsertKnot(0.7);
    bspline2->InsertKnot(0.9, 2);
    // knot vector of bspline2 is now: [0, 0, 0, 0, 0.5, 0.7, 0.9, 0.9, 1, 1, 1, 1]

    // create third B-spline
    // create B-spline
    TColgp_Array1OfPnt controlPoints3(1, 4);
    controlPoints3(1) = gp_Pnt(1, 0, 0);
    controlPoints3(2) = gp_Pnt(2, 2, 2);
    controlPoints3(3) = gp_Pnt(4, 1, -5);
    controlPoints3(4) = gp_Pnt(8, 0, -1);

    TColStd_Array1OfReal Weights3(1, 4);
    Weights3(1) = 1;
    Weights3(2) = 1;
    Weights3(3) = 1;
    Weights3(4) = 1;

    TColStd_Array1OfReal Knots3(1, 2);
    Knots3(1) = 0.;
    Knots3(2) = 1;

    TColStd_Array1OfInteger Multiplicities3(1, 2);
    Multiplicities3(1) = 4;
    Multiplicities3(2) = 4;

    Standard_Integer Degree3 = 3;

    Handle(Geom_BSplineCurve) bspline3 = new Geom_BSplineCurve(controlPoints3, Weights3, Knots3, Multiplicities3, Degree3);

    bspline3->InsertKnot(1. / 3);
    bspline3->InsertKnot(0.5, 3);
    bspline3->InsertKnot(2. / 3);
    // knot vector of bspline3 is now: [0, 0, 0, 0, 1./3, 0.5, 0.5, 0.5, 2./3, 1, 1, 1, 1]

    TColStd_Array1OfReal right_knot_vector(1, 7);
    right_knot_vector(1) = 0.;
    right_knot_vector(2) = 1. / 3;
    right_knot_vector(3) = 0.5;
    right_knot_vector(4) = 2. / 3;
    right_knot_vector(5) = 0.7;
    right_knot_vector(6) = 0.9;
    right_knot_vector(7) = 1.;

    TColStd_Array1OfInteger right_multiplicities(1, 7);
    right_multiplicities(1) = 4;
    right_multiplicities(2) = 1;
    right_multiplicities(3) = 3;
    right_multiplicities(4) = 1;
    right_multiplicities(5) = 1;
    right_multiplicities(6) = 2;
    right_multiplicities(7) = 4;

    std::vector<Handle(Geom_BSplineCurve)> splines_vector;
    splines_vector.push_back(bspline1);
    splines_vector.push_back(bspline2);
    splines_vector.push_back(bspline3);

    std::vector<Handle(Geom_BSplineCurve)> modified_splines_vector = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(splines_vector);

    TColStd_Array1OfReal computed_knot_vector(1, 7);
    modified_splines_vector[0]->Knots(computed_knot_vector);

    TColStd_Array1OfInteger computed_multiplicities(1, 7);
    modified_splines_vector[0]->Multiplicities(computed_multiplicities);

    ASSERT_NEAR(right_knot_vector(1), computed_knot_vector(1), 1e-15);
    ASSERT_NEAR(right_knot_vector(2), computed_knot_vector(2), 1e-15);
    ASSERT_NEAR(right_knot_vector(3), computed_knot_vector(3), 1e-15);
    ASSERT_NEAR(right_knot_vector(4), computed_knot_vector(4), 1e-15);
    ASSERT_NEAR(right_knot_vector(5), computed_knot_vector(5), 1e-15);
    ASSERT_NEAR(right_knot_vector(6), computed_knot_vector(6), 1e-15);
    ASSERT_NEAR(right_knot_vector(7), computed_knot_vector(7), 1e-15);

    // create B-spline with different degree
//    int wrong_degree = 2;
//    Handle(Geom_BSplineCurve) bspline_ = new Geom_BSplineCurve(controlPoints1, Weights1, Knots1, Multiplicities1, wrong_degree);
//    splines_vector.push_back(bspline_);
//    ASSERT_THROW(std::vector<Handle(Geom_BSplineCurve)> new_spline_vector = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(splines_vector), tigl::CTiglError);

}

TEST(TiglBSplineAlgorithms, testCreateCommonKnotsVectorSurface)
{
    // tests the method createCommonKnotsVectorSurface

    // create the first B-spline surface
    TColgp_Array2OfPnt controlPoints1(1, 4, 1, 3);
    controlPoints1(1, 1) = gp_Pnt(0., 0., 0.);
    controlPoints1(2, 1) = gp_Pnt(1., 1., 0.);
    controlPoints1(3, 1) = gp_Pnt(3., -1., 0.);
    controlPoints1(4, 1) = gp_Pnt(4., 0., 0.);
    controlPoints1(1, 2) = gp_Pnt(0., 1., 0.);
    controlPoints1(2, 2) = gp_Pnt(1., 0., 0.);
    controlPoints1(3, 2) = gp_Pnt(4., -1., 0.);
    controlPoints1(4, 2) = gp_Pnt(5., 0., 0.);
    controlPoints1(1, 3) = gp_Pnt(0., 0., -1.);
    controlPoints1(2, 3) = gp_Pnt(2., 1., 0.);
    controlPoints1(3, 3) = gp_Pnt(3., -2., 0.);
    controlPoints1(4, 3) = gp_Pnt(8., 0., 0.);

    TColStd_Array1OfReal knots_u(1, 2);
    knots_u(1) = 0.;
    knots_u(2) = 1.;

    TColStd_Array1OfInteger mults_u(1, 2);
    mults_u(1) = 4;
    mults_u(2) = 4;

    TColStd_Array1OfReal knots_v(1, 2);
    knots_v(1) = 0.;
    knots_v(2) = 1.;

    TColStd_Array1OfInteger mults_v(1, 2);
    mults_v(1) = 3;
    mults_v(2) = 3;

    unsigned int degree_u = 3;
    unsigned int degree_v = 2;

    Handle(Geom_BSplineSurface) surface1 = new Geom_BSplineSurface(controlPoints1, knots_u, knots_v, mults_u, mults_v, degree_u, degree_v);
    surface1->InsertUKnot(0.5, 2, 1e-15);
    surface1->InsertUKnot(0.7, 1, 1e-15);
    // u knot vector is now: [0, 0, 0, 0, 0.5, 0.5, 0.7, 1, 1, 1, 1]

    surface1->InsertVKnot(0.3, 3, 1e-15);
    // v knot vector is now: [0, 0, 0, 0.3, 0.3, 0.3, 1, 1, 1]


    // create the second B-spline surface
    TColgp_Array2OfPnt controlPoints2(1, 4, 1, 3);
    controlPoints2(1, 1) = gp_Pnt(1, 0, 0);
    controlPoints2(2, 1) = gp_Pnt(2, 1, 0);
    controlPoints2(3, 1) = gp_Pnt(3, -1, 0);
    controlPoints2(4, 1) = gp_Pnt(4, 0, -3);
    controlPoints2(1, 2) = gp_Pnt(0, 1, 2);
    controlPoints2(2, 2) = gp_Pnt(1, 0, 8);
    controlPoints2(3, 2) = gp_Pnt(4, 5, 0);
    controlPoints2(4, 2) = gp_Pnt(5, 1, 0);
    controlPoints2(1, 3) = gp_Pnt(0, 0, 0);
    controlPoints2(2, 3) = gp_Pnt(2, 3, 0);
    controlPoints2(3, 3) = gp_Pnt(3, -2, 0);
    controlPoints2(4, 3) = gp_Pnt(5, 0, 0);

    Handle(Geom_BSplineSurface) surface2 = new Geom_BSplineSurface(controlPoints2, knots_u, knots_v, mults_u, mults_v, degree_u, degree_v);
    surface2->InsertUKnot(1. / 3, 1, 1e-15);
    surface2->InsertUKnot(3. / 5, 1, 1e-15);
    // u knot vector is now: [0, 0, 0, 0, 1./3, 3./5, 1, 1, 1, 1]

    surface2->InsertVKnot(0.6, 2, 1e-15, Standard_True);
    // v knot vector is now: [0, 0, 0, 0.6, 0.6, 1, 1, 1]


    // create the third B-spline surface
    TColgp_Array2OfPnt controlPoints3(1, 4, 1, 3);
    controlPoints3(1, 1) = gp_Pnt(1, 0, 0);
    controlPoints3(2, 1) = gp_Pnt(2, 1, 0);
    controlPoints3(3, 1) = gp_Pnt(3, -1, 0);
    controlPoints3(4, 1) = gp_Pnt(4, 0, -3);
    controlPoints3(1, 2) = gp_Pnt(0, 1, 2);
    controlPoints3(2, 2) = gp_Pnt(1, 0, 8);
    controlPoints3(3, 2) = gp_Pnt(4, 5, 0);
    controlPoints3(4, 2) = gp_Pnt(5, 1, 0);
    controlPoints3(1, 3) = gp_Pnt(0, 0, 0);
    controlPoints3(2, 3) = gp_Pnt(2, 3, 0);
    controlPoints3(3, 3) = gp_Pnt(3, -2, 0);
    controlPoints3(4, 3) = gp_Pnt(5, 0, 0);


    Handle(Geom_BSplineSurface) surface3 = new Geom_BSplineSurface(controlPoints3, knots_u, knots_v, mults_u, mults_v, degree_u, degree_v);
    surface3->InsertUKnot(0.7, 1, 1e-15);
    // u knot vector is now: [0, 0, 0, 0, 0.7, 1, 1, 1, 1]

    surface3->InsertVKnot(0.2, 1, 1e-15);
    surface3->InsertVKnot(0.3, 1, 1e-15);
    surface3->InsertVKnot(0.9, 1, 1e-15);
    // v knot vector is now: [0, 0, 0, 0.2, 0.3, 0.9, 1, 1, 1]

    TColStd_Array1OfReal right_knot_vector_u(1, 6);
    right_knot_vector_u(1) = 0;
    right_knot_vector_u(2) = 1. / 3;
    right_knot_vector_u(3) = 0.5;
    right_knot_vector_u(4) = 3. / 5;
    right_knot_vector_u(5) = 0.7;
    right_knot_vector_u(6) = 1;

    TColStd_Array1OfInteger right_mult_u(1, 6);
    right_mult_u(1) = 4;
    right_mult_u(2) = 1;
    right_mult_u(3) = 2;
    right_mult_u(4) = 1;
    right_mult_u(5) = 1;
    right_mult_u(6) = 4;

    TColStd_Array1OfReal right_knot_vector_v(1, 6);
    right_knot_vector_v(1) = 0;
    right_knot_vector_v(2) = 0.2;
    right_knot_vector_v(3) = 0.3;
    right_knot_vector_v(4) = 0.6;
    right_knot_vector_v(5) = 0.9;
    right_knot_vector_v(6) = 1;

    TColStd_Array1OfInteger right_mult_v(1, 6);
    right_mult_v(1) = 3;
    right_mult_v(2) = 1;
    right_mult_v(3) = 2;  // 3
    right_mult_v(4) = 2;  // 4
    right_mult_v(5) = 1;
    right_mult_v(6) = 3;

    std::vector<Handle(Geom_BSplineSurface) > surfaces_vector;
    surfaces_vector.push_back(surface1);
    surfaces_vector.push_back(surface2);
    surfaces_vector.push_back(surface3);

    std::vector<Handle(Geom_BSplineSurface) > modified_surfaces_vector = CTiglBSplineAlgorithms::createCommonKnotsVectorSurface(surfaces_vector);

    TColStd_Array1OfReal computed_knot_vector_u(1, 6);
    modified_surfaces_vector[0]->UKnots(computed_knot_vector_u);

    TColStd_Array1OfInteger computed_mults_u(1, 6);
    modified_surfaces_vector[0]->UMultiplicities(computed_mults_u);

    TColStd_Array1OfReal computed_knot_vector_v(1, 6);
    modified_surfaces_vector[0]->VKnots(computed_knot_vector_v);

    TColStd_Array1OfInteger computed_mults_v(1, 6);
    modified_surfaces_vector[0]->VMultiplicities(computed_mults_v);

    ASSERT_NEAR(computed_knot_vector_u(1), right_knot_vector_u(1), 1e-15);
    ASSERT_NEAR(computed_knot_vector_u(2), right_knot_vector_u(2), 1e-15);
    ASSERT_NEAR(computed_knot_vector_u(3), right_knot_vector_u(3), 1e-15);
    ASSERT_NEAR(computed_knot_vector_u(4), right_knot_vector_u(4), 1e-15);
    ASSERT_NEAR(computed_knot_vector_u(5), right_knot_vector_u(5), 1e-15);
    ASSERT_NEAR(computed_knot_vector_u(6), right_knot_vector_u(6), 1e-15);

    ASSERT_NEAR(computed_mults_u(1), right_mult_u(1), 1e-15);
    ASSERT_NEAR(computed_mults_u(2), right_mult_u(2), 1e-15);
    ASSERT_NEAR(computed_mults_u(3), right_mult_u(3), 1e-15);
    ASSERT_NEAR(computed_mults_u(4), right_mult_u(4), 1e-15);
    ASSERT_NEAR(computed_mults_u(5), right_mult_u(5), 1e-15);
    ASSERT_NEAR(computed_mults_u(6), right_mult_u(6), 1e-15);

    ASSERT_NEAR(computed_knot_vector_v(1), right_knot_vector_v(1), 1e-15);
    ASSERT_NEAR(computed_knot_vector_v(2), right_knot_vector_v(2), 1e-15);
    ASSERT_NEAR(computed_knot_vector_v(3), right_knot_vector_v(3), 1e-15);
    ASSERT_NEAR(computed_knot_vector_v(4), right_knot_vector_v(4), 1e-15);
    ASSERT_NEAR(computed_knot_vector_v(5), right_knot_vector_v(5), 1e-15);
    ASSERT_NEAR(computed_knot_vector_v(6), right_knot_vector_v(6), 1e-15);

    ASSERT_NEAR(computed_mults_v(1), right_mult_v(1), 1e-15);
    ASSERT_NEAR(computed_mults_v(2), right_mult_v(2), 1e-15);
    ASSERT_NEAR(computed_mults_v(3), right_mult_v(3), 1e-15);

    ASSERT_NEAR(computed_mults_v(4), right_mult_v(4), 1e-15);
    ASSERT_NEAR(computed_mults_v(5), right_mult_v(5), 1e-15);
    ASSERT_NEAR(computed_mults_v(6), right_mult_v(6), 1e-15);
}

TEST(TiglBSplineAlgorithms, testSkinnedBSplineSurface)
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

    std::vector<Handle(Geom_BSplineCurve) > splines_vector;
    splines_vector.push_back(curve1);
    splines_vector.push_back(curve2);

    Handle(Geom_BSplineSurface) skinnedSurface = CTiglBSplineAlgorithms::skinnedBSplineSurface(splines_vector);


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

TEST(TiglBSplineAlgorithms, testReparametrizeBSpline)
{
    /*
     * Tests the method reparametrizeBSpline(spline, old_parameters, new_parameters)
     */

    // create B-spline
    unsigned int degree = 3;

    TColgp_Array1OfPnt controlPoints(1, 8);
    controlPoints(1) = gp_Pnt(0., -1., 0.);
    controlPoints(2) = gp_Pnt(2., 3., 1.);
    controlPoints(3) = gp_Pnt(1., 5., -2.);
    controlPoints(4) = gp_Pnt(2., 8., -1.);
    controlPoints(5) = gp_Pnt(0., 10., 2.);
    controlPoints(6) = gp_Pnt(-1., 12., 4.);
    controlPoints(7) = gp_Pnt(-2., 16., 5.);
    controlPoints(8) = gp_Pnt(0., 17., 0.);

    TColStd_Array1OfReal knots(1, 5);
    knots(1) = 0.;
    knots(2) = 0.1;
    knots(3) = 0.3;
    knots(4) = 0.8;
    knots(5) = 1.;

    TColStd_Array1OfInteger mults(1, 5);
    mults(1) = 4;
    mults(2) = 1;
    mults(3) = 2;
    mults(4) = 1;
    mults(5) = 4;

    Handle(Geom_BSplineCurve) spline = new Geom_BSplineCurve(controlPoints, knots, mults, degree);

    TColStd_Array1OfReal old_parameters(1, 7);
    old_parameters(1) = 0.;
    old_parameters(2) = 0.2;
    old_parameters(3) = 0.4;
    old_parameters(4) = 0.5;
    old_parameters(5) = 0.6;
    old_parameters(6) = 0.8;
    old_parameters(7) = 1.;

    TColStd_Array1OfReal new_parameters(1, 7);
    new_parameters(1) = 0.;
    new_parameters(2) = 0.1;
    new_parameters(3) = 0.2;
    new_parameters(4) = 0.3;
    new_parameters(5) = 0.7;
    new_parameters(6) = 0.95;
    new_parameters(7) = 1.;

    Handle(Geom_BSplineCurve) reparam_spline = CTiglBSplineAlgorithms::reparametrizeBSpline(spline, old_parameters, new_parameters);

    TColStd_Array1OfReal test_point_params(1, 101);
    for (int i = 0; i < 101; ++i) {
        test_point_params(i + 1) = i / 100.;
    }

    TColStd_Array1OfReal test_point_new_params(1, 101);

    for (int i = test_point_params.Lower(); i <= test_point_params.Upper(); ++i) {
        for (int j = old_parameters.Lower(); j <= old_parameters.Upper() - 1; ++j) {
            if (std::abs(test_point_params(i) - old_parameters(j + 1)) < 1e-15) {
                test_point_new_params(i) = new_parameters(j + 1);
            }
            else if (std::abs(test_point_params(i) - old_parameters(1)) < 1e-15) {
                test_point_new_params(i) = new_parameters(1);
            }
            else if (old_parameters(j + 1) > test_point_params(i) && test_point_params(i) > old_parameters(j)) {
                test_point_new_params(i) = test_point_params(i) * (new_parameters(j + 1) - new_parameters(j)) / (old_parameters(j + 1) - old_parameters(j));
            }
        }
    }

    // check that B-spline geometry remains the same
    for (int param_idx = old_parameters.Lower(); param_idx <= old_parameters.Upper(); ++param_idx) {
        gp_Pnt old_point = spline->Value(old_parameters(param_idx));
        gp_Pnt new_point = reparam_spline->Value(new_parameters(param_idx));
        ASSERT_NEAR(old_point.X(), new_point.X(), 1e-14);
        ASSERT_NEAR(old_point.Y(), new_point.Y(), 1e-14);
        ASSERT_NEAR(old_point.Z(), new_point.Z(), 1e-14);
    }

    // check that knots are inserted number_of_degree-times at each new parameter
    ASSERT_NEAR(reparam_spline->Knot(1), 0., 1e-15);
    ASSERT_NEAR(reparam_spline->Knot(2), 0.05, 1e-15);
    ASSERT_NEAR(reparam_spline->Knot(3), 0.1, 1e-15);
    ASSERT_NEAR(reparam_spline->Knot(4), 0.15, 1e-15);
    ASSERT_NEAR(reparam_spline->Knot(5), 0.2, 1e-15);
    ASSERT_NEAR(reparam_spline->Knot(6), 0.3, 1e-15);
    ASSERT_NEAR(reparam_spline->Knot(7), 0.7, 1e-15);
    ASSERT_NEAR(reparam_spline->Knot(8), 0.95, 1e-15);
    ASSERT_NEAR(reparam_spline->Knot(9), 1., 1e-15);

    ASSERT_EQ(reparam_spline->Multiplicity(1), 4);
    ASSERT_EQ(reparam_spline->Multiplicity(2), 1);
    ASSERT_EQ(reparam_spline->Multiplicity(3), 3);
    ASSERT_EQ(reparam_spline->Multiplicity(4), 2);
    ASSERT_EQ(reparam_spline->Multiplicity(5), 3);
    ASSERT_EQ(reparam_spline->Multiplicity(6), 3);
    ASSERT_EQ(reparam_spline->Multiplicity(7), 3);
    ASSERT_EQ(reparam_spline->Multiplicity(8), 3);
    ASSERT_EQ(reparam_spline->Multiplicity(9), 4);
}


TEST(TiglBSplineAlgorithms, testFlipSurface)
{
    /*
     * Test the method flipSurface(surface)
     */

    // create B-spline surface
    TColgp_Array2OfPnt controlPoints(1, 4, 1, 3);
    controlPoints(1, 1) = gp_Pnt(0., 0., 0.);
    controlPoints(2, 1) = gp_Pnt(1., 1., 0.);
    controlPoints(3, 1) = gp_Pnt(3., -1., 0.);
    controlPoints(4, 1) = gp_Pnt(4., 0., 0.);
    controlPoints(1, 2) = gp_Pnt(0., 1., 0.);
    controlPoints(2, 2) = gp_Pnt(1., 0., 0.);
    controlPoints(3, 2) = gp_Pnt(4., -1., 0.);
    controlPoints(4, 2) = gp_Pnt(5., 0., 0.);
    controlPoints(1, 3) = gp_Pnt(0., 0., -1.);
    controlPoints(2, 3) = gp_Pnt(2., 1., 0.);
    controlPoints(3, 3) = gp_Pnt(3., -2., 0.);
    controlPoints(4, 3) = gp_Pnt(8., 0., 0.);

    TColStd_Array1OfReal knots_u(1, 2);
    knots_u(1) = 0.;
    knots_u(2) = 1.;

    TColStd_Array1OfInteger mults_u(1, 2);
    mults_u(1) = 4;
    mults_u(2) = 4;

    TColStd_Array1OfReal knots_v(1, 2);
    knots_v(1) = 0.;
    knots_v(2) = 1.;

    TColStd_Array1OfInteger mults_v(1, 2);
    mults_v(1) = 3;
    mults_v(2) = 3;

    unsigned int degree_u = 3;
    unsigned int degree_v = 2;

    Handle(Geom_BSplineSurface) surface = new Geom_BSplineSurface(controlPoints, knots_u, knots_v, mults_u, mults_v, degree_u, degree_v);

    // flip this surface
    Handle(Geom_BSplineSurface) flippedSurface = CTiglBSplineAlgorithms::flipSurface(surface);

    //now test it
    for (int u_idx = 0; u_idx < 101; ++u_idx) {
        for (int v_idx = 0; v_idx < 101; ++v_idx) {
            double u_value = u_idx / 100.;
            double v_value = v_idx / 100.;

            gp_Pnt point = surface->Value(u_idx, v_idx);
            gp_Pnt same_point = flippedSurface->Value(v_idx, u_idx);

            ASSERT_NEAR(point.X(), same_point.X(), 1e-15);
            ASSERT_NEAR(point.Y(), same_point.Y(), 1e-15);
            ASSERT_NEAR(point.Z(), same_point.Z(), 1e-15);
        }
    }
}

TEST(TiglBSplineAlgorithms, testInterpolatingSurface)
{
    // create exact surface
    // create B-spline surface
    TColgp_Array2OfPnt controlPoints(1, 4, 1, 3);
    controlPoints(1, 1) = gp_Pnt(0., 0., 0.);
    controlPoints(2, 1) = gp_Pnt(1., 1., 0.);
    controlPoints(3, 1) = gp_Pnt(3., -1., 0.);
    controlPoints(4, 1) = gp_Pnt(4., 0., 0.);
    controlPoints(1, 2) = gp_Pnt(0., 1., 0.);
    controlPoints(2, 2) = gp_Pnt(1., 0., 0.);
    controlPoints(3, 2) = gp_Pnt(4., -1., 0.);
    controlPoints(4, 2) = gp_Pnt(5., 0., 0.);
    controlPoints(1, 3) = gp_Pnt(0., 0., -1.);
    controlPoints(2, 3) = gp_Pnt(2., 1., 0.);
    controlPoints(3, 3) = gp_Pnt(3., -2., 0.);
    controlPoints(4, 3) = gp_Pnt(8., 0., 0.);

    TColStd_Array1OfReal knots_u(1, 2);
    knots_u(1) = 0.;
    knots_u(2) = 1.;

    TColStd_Array1OfInteger mults_u(1, 2);
    mults_u(1) = 4;
    mults_u(2) = 4;

    TColStd_Array1OfReal knots_v(1, 2);
    knots_v(1) = 0.;
    knots_v(2) = 1.;

    TColStd_Array1OfInteger mults_v(1, 2);
    mults_v(1) = 3;
    mults_v(2) = 3;

    unsigned int degree_u = 3;
    unsigned int degree_v = 2;

    Handle(Geom_BSplineSurface) surface = new Geom_BSplineSurface(controlPoints, knots_u, knots_v, mults_u, mults_v, degree_u, degree_v);

    TColgp_Array2OfPnt points(1, 100, 1, 100);
    for (unsigned int u_idx = 1; u_idx <= 100; ++u_idx) {
        for (unsigned int v_idx = 1; v_idx <= 100; ++v_idx) {
            points(u_idx, v_idx) = surface->Value(u_idx / 100., v_idx / 100.);
        }
    }

    std::pair<Handle(TColStd_HArray1OfReal), Handle(TColStd_HArray1OfReal)> parameters = CTiglBSplineAlgorithms::computeParamsBSplineSurf(points);

    Handle(Geom_BSplineSurface) interpolatingSurf = CTiglBSplineAlgorithms::interpolatingSurface(points, parameters.first, parameters.second);

    for (unsigned int u_idx = 1; u_idx <= 100; ++u_idx) {
        for (unsigned int v_idx = 1; v_idx <= 100; ++v_idx) {
            gp_Pnt surf_pnt = surface->Value(u_idx / 100., v_idx / 100.);
            gp_Pnt interp_pnt = interpolatingSurf->Value(parameters.first->Value(u_idx), parameters.second->Value(v_idx));
            ASSERT_NEAR(interp_pnt.X(), surf_pnt.X(), 4e-15);
            ASSERT_NEAR(interp_pnt.Y(), surf_pnt.Y(), 4e-15);
            ASSERT_NEAR(interp_pnt.Z(), surf_pnt.Z(), 4e-15);
        }
    }
}

TEST(TiglBSplineAlgorithms, testCreateGordonSurface)
{
    // Tests the method createGordonSurface

    // creating first u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset -1 in x-direction
    unsigned int degree = 3;  // degree of the four u-directional B-splines and the five v-directional B-splines

    TColgp_Array1OfPnt controlPoints_u1(1, 4);
    controlPoints_u1(1) = gp_Pnt(-1., 0.25, 0.);
    controlPoints_u1(2) = gp_Pnt(-1., -1. / 12, 1. / 3);
    controlPoints_u1(3) = gp_Pnt(-1., -1. / 12, 2. / 3);
    controlPoints_u1(4) = gp_Pnt(-1., 0.25, 1.);

    TColStd_Array1OfReal knots(1, 2);
    knots(1) = 0.;
    knots(2) = 1.;

    TColStd_Array1OfInteger mults(1, 2);
    mults(1) = 4;
    mults(2) = 4;

    Handle(Geom_BSplineCurve) spline_u1 = new Geom_BSplineCurve(controlPoints_u1, knots, mults, degree);
    spline_u1->InsertKnot(0.6);

    // creating second u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset 2 in x-direction
    TColgp_Array1OfPnt controlPoints_u2(1, 4);
    controlPoints_u2(1) = gp_Pnt(2., 0.25, 0.);
    controlPoints_u2(2) = gp_Pnt(2., -1. / 12, 1. / 3);
    controlPoints_u2(3) = gp_Pnt(2., -1. / 12, 2. / 3);
    controlPoints_u2(4) = gp_Pnt(2., 0.25, 1.);

    Handle(Geom_BSplineCurve) spline_u2 = new Geom_BSplineCurve(controlPoints_u2, knots, mults, degree);
    spline_u2->InsertKnot(0.6);
    // creating third u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset 3 in x-direction
    TColgp_Array1OfPnt controlPoints_u3(1, 4);
    controlPoints_u3(1) = gp_Pnt(3., 0.25, 0.);
    controlPoints_u3(2) = gp_Pnt(3., -1. / 12, 1. / 3);
    controlPoints_u3(3) = gp_Pnt(3., -1. / 12, 2. / 3);
    controlPoints_u3(4) = gp_Pnt(3., 0.25, 1.);

    Handle(Geom_BSplineCurve) spline_u3 = new Geom_BSplineCurve(controlPoints_u3, knots, mults, degree);
    spline_u3->InsertKnot(0.6);
    // creating fourth u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset 4 in x-direction
    TColgp_Array1OfPnt controlPoints_u4(1, 4);
    controlPoints_u4(1) = gp_Pnt(4., 0.25, 0.);
    controlPoints_u4(2) = gp_Pnt(4., -1. / 12, 1. / 3);
    controlPoints_u4(3) = gp_Pnt(4., -1. / 12, 2. / 3);
    controlPoints_u4(4) = gp_Pnt(4., 0.25, 1.);

    Handle(Geom_BSplineCurve) spline_u4 = new Geom_BSplineCurve(controlPoints_u4, knots, mults, degree);
    spline_u4->InsertKnot(0.6);

    // creating first v-directional B-spline which represents z(x) = 0 at y = 0.25
    TColgp_Array1OfPnt controlPoints_v1(1, 4);
    controlPoints_v1(1) = gp_Pnt(-1., 0.25, 0.);
    controlPoints_v1(2) = gp_Pnt(2. / 3, 0.25, 0.);
    controlPoints_v1(3) = gp_Pnt(7. / 3, 0.25, 0.);
    controlPoints_v1(4) = gp_Pnt(4., 0.25, 0.);

    Handle(Geom_BSplineCurve) spline_v1 = new Geom_BSplineCurve(controlPoints_v1, knots, mults, degree);

    // creating second v-directional B-spline which represents z(x) = 0.5 - sqrt(0.1) at y = 0.1
    TColgp_Array1OfPnt controlPoints_v2(1, 4);
    controlPoints_v2(1) = gp_Pnt(-1., 0.1, 0.5 - std::sqrt(0.1));
    controlPoints_v2(2) = gp_Pnt(2. / 3, 0.1, 0.5 - sqrt(0.1));
    controlPoints_v2(3) = gp_Pnt(7. / 3, 0.1, 0.5 - sqrt(0.1));
    controlPoints_v2(4) = gp_Pnt(4., 0.1, 0.5 - sqrt(0.1));

    Handle(Geom_BSplineCurve) spline_v2 = new Geom_BSplineCurve(controlPoints_v2, knots, mults, degree);

    // creating third v-directional B-spline which represents z(x) = 0.5 - sqrt(0.05) at y = 0.05
    TColgp_Array1OfPnt controlPoints_v3(1, 4);
    controlPoints_v3(1) = gp_Pnt(-1., 0.05, 0.5 - std::sqrt(0.05));
    controlPoints_v3(2) = gp_Pnt(2. / 3, 0.05, 0.5 - sqrt(0.05));
    controlPoints_v3(3) = gp_Pnt(7. / 3, 0.05, 0.5 - sqrt(0.05));
    controlPoints_v3(4) = gp_Pnt(4., 0.05, 0.5 - sqrt(0.05));

    Handle(Geom_BSplineCurve) spline_v3 = new Geom_BSplineCurve(controlPoints_v3, knots, mults, degree);

    // creating fourth v-directional B-spline which represents z(x) = 0.5 + sqrt(0.1) at y = 0.1
    TColgp_Array1OfPnt controlPoints_v4(1, 4);
    controlPoints_v4(1) = gp_Pnt(-1., 0.1, 0.5 + std::sqrt(0.1));
    controlPoints_v4(2) = gp_Pnt(2. / 3, 0.1, 0.5 + sqrt(0.1));
    controlPoints_v4(3) = gp_Pnt(7. / 3, 0.1, 0.5 + sqrt(0.1));
    controlPoints_v4(4) = gp_Pnt(4., 0.1, 0.5 + sqrt(0.1));

    Handle(Geom_BSplineCurve) spline_v4 = new Geom_BSplineCurve(controlPoints_v4, knots, mults, degree);

    // creating fifth v-directional B-spline which represents z(x) = 1 at y = 0.25
    TColgp_Array1OfPnt controlPoints_v5(1, 4);
    controlPoints_v5(1) = gp_Pnt(-1., 0.25, 1.);
    controlPoints_v5(2) = gp_Pnt(2. / 3, 0.25, 1.);
    controlPoints_v5(3) = gp_Pnt(7. / 3, 0.25, 1.);
    controlPoints_v5(4) = gp_Pnt(4., 0.25, 1.);

    Handle(Geom_BSplineCurve) spline_v5 = new Geom_BSplineCurve(controlPoints_v5, knots, mults, degree);

    // u- and v-directional B-splines are already compatible in B-spline sense (common knot vector, same parametrization)
    std::vector<Handle(Geom_BSplineCurve)> compatible_splines_u_vector;
    compatible_splines_u_vector.push_back(spline_u1);
    compatible_splines_u_vector.push_back(spline_u2);
    compatible_splines_u_vector.push_back(spline_u3);
    compatible_splines_u_vector.push_back(spline_u4);

    std::vector<Handle(Geom_BSplineCurve)> compatible_splines_v_vector;
    compatible_splines_v_vector.push_back(spline_v1);
    compatible_splines_v_vector.push_back(spline_v2);
    compatible_splines_v_vector.push_back(spline_v3);
    compatible_splines_v_vector.push_back(spline_v4);
    compatible_splines_v_vector.push_back(spline_v5);

    // intersection point parameters of v-directional curve with u-directional curves:
    Handle(TColStd_HArray1OfReal) intersection_params_v = new TColStd_HArray1OfReal(1, 4);
    intersection_params_v->SetValue(1, 0.);
    intersection_params_v->SetValue(2, 3. / 5);
    intersection_params_v->SetValue(3, 4. / 5);
    intersection_params_v->SetValue(4, 1.);

    // intersection point parameters of u-directional curve with v-directional curves:
    Handle(TColStd_HArray1OfReal) intersection_params_u = new TColStd_HArray1OfReal(1, 5);
    intersection_params_u->SetValue(1, 0.);
    intersection_params_u->SetValue(2, 0.5 - std::sqrt(0.1));
    intersection_params_u->SetValue(3, 0.5 - std::sqrt(0.05));
    intersection_params_u->SetValue(4, 0.5 + std::sqrt(0.1));
    intersection_params_u->SetValue(5, 1.);

    Handle(Geom_BSplineSurface) gordonSurface = CTiglBSplineAlgorithms::createGordonSurface(compatible_splines_u_vector, compatible_splines_v_vector, intersection_params_u, intersection_params_v);

    // after creating the test surface above, now test it:
    for (int u_idx = 0; u_idx <= 100; ++u_idx) {
        for (int v_idx = 0; v_idx <= 100; ++v_idx) {
            double u_value = u_idx / 100.;
            double v_value = v_idx / 100.;

            gp_Pnt surface_point = gordonSurface->Value(v_value, u_value);
            gp_Pnt point_curve1 = spline_u1->Value(u_value);  // represents y(z) = (z - 0.5)^2 with offset -1 in x-direction
            gp_Pnt point_curve2 = spline_u4->Value(u_value);  // represents y(z) = (z - 0.5)^2 with offset 2 in x-direction
            gp_Pnt right_point(point_curve1.X() * (1. - v_value) + point_curve2.X() * v_value, point_curve1.Y() * (1. - v_value) + point_curve2.Y() * v_value, point_curve1.Z() * (1. - v_value) + point_curve2.Z() * v_value);

            ASSERT_NEAR(surface_point.X(), right_point.X(), 2e-15);
            ASSERT_NEAR(surface_point.Y(), right_point.Y(), 2e-15);
            ASSERT_NEAR(surface_point.Z(), right_point.Z(), 2e-15);
        }
    }
}

TEST(TiglBSplineAlgorithms, testIntersectionFinder)
{
    // creating u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset 3 in x-direction
    TColgp_Array1OfPnt controlPoints_u(1, 4);
    controlPoints_u(1) = gp_Pnt(3., 0.25, 0.);
    controlPoints_u(2) = gp_Pnt(3., -1. / 12, 1. / 3);
    controlPoints_u(3) = gp_Pnt(3., -1. / 12, 2. / 3);
    controlPoints_u(4) = gp_Pnt(3., 0.25, 1.);

    TColStd_Array1OfReal knots(1, 2);
    knots(1) = 0.;
    knots(2) = 1.;

    TColStd_Array1OfInteger mults(1, 2);
    mults(1) = 4;
    mults(2) = 4;

    unsigned int degree = 3;

    Handle(Geom_BSplineCurve) spline_u = new Geom_BSplineCurve(controlPoints_u, knots, mults, degree);

    // creating v-directional B-spline which represents z(x) = 0.5 + sqrt(0.1) at y = 0.1
    TColgp_Array1OfPnt controlPoints_v(1, 4);
    controlPoints_v(1) = gp_Pnt(-1., 0.1, 0.5 + std::sqrt(0.1));
    controlPoints_v(2) = gp_Pnt(2. / 3, 0.1, 0.5 + sqrt(0.1));
    controlPoints_v(3) = gp_Pnt(7. / 3, 0.1, 0.5 + sqrt(0.1));
    controlPoints_v(4) = gp_Pnt(4., 0.1, 0.5 + sqrt(0.1));

    Handle(Geom_BSplineCurve) spline_v = new Geom_BSplineCurve(controlPoints_v, knots, mults, degree);

    std::vector<std::pair<double, double> > intersection_vector = CTiglBSplineAlgorithms::intersectionFinder(spline_u, spline_v);

    // splines should intersect at u = 0.5 + std::sqrt(0.1) and v = 4. / 5
    ASSERT_NEAR(intersection_vector[0].first, 0.5 + std::sqrt(0.1), 1e-15);
    ASSERT_NEAR(intersection_vector[0].second, 4. / 5, 1e-15);
}

TEST(TiglBSplineAlgorithms, testCreateGordonSurfaceGeneral)
{
    // Tests the method createGordonSurfaceGeneral

    // creating first u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset -1 in x-direction
    unsigned int degree = 3;  // degree of the four u-directional B-splines and the five v-directional B-splines

    TColgp_Array1OfPnt controlPoints_u1(1, 4);
    controlPoints_u1(1) = gp_Pnt(-1., 0.25, 0.);
    controlPoints_u1(2) = gp_Pnt(-1., -1. / 12, 1. / 3);
    controlPoints_u1(3) = gp_Pnt(-1., -1. / 12, 2. / 3);
    controlPoints_u1(4) = gp_Pnt(-1., 0.25, 1.);

    TColStd_Array1OfReal knots(1, 2);
    knots(1) = 0.;
    knots(2) = 1.;

    TColStd_Array1OfInteger mults(1, 2);
    mults(1) = 4;
    mults(2) = 4;

    Handle(Geom_BSplineCurve) spline_u1 = new Geom_BSplineCurve(controlPoints_u1, knots, mults, degree);
    spline_u1->InsertKnot(0.5);

    // creating second u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset 2 in x-direction
    TColgp_Array1OfPnt controlPoints_u2(1, 4);
    controlPoints_u2(1) = gp_Pnt(2., 0.25, 0.);
    controlPoints_u2(2) = gp_Pnt(2., -1. / 12, 1. / 3);
    controlPoints_u2(3) = gp_Pnt(2., -1. / 12, 2. / 3);
    controlPoints_u2(4) = gp_Pnt(2., 0.25, 1.);

    Handle(Geom_BSplineCurve) spline_u2 = new Geom_BSplineCurve(controlPoints_u2, knots, mults, degree);
    spline_u2->InsertKnot(0.6);
    // creating third u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset 3 in x-direction
    TColgp_Array1OfPnt controlPoints_u3(1, 4);
    controlPoints_u3(1) = gp_Pnt(3., 0.25, 0.);
    controlPoints_u3(2) = gp_Pnt(3., -1. / 12, 1. / 3);
    controlPoints_u3(3) = gp_Pnt(3., -1. / 12, 2. / 3);
    controlPoints_u3(4) = gp_Pnt(3., 0.25, 1.);

    Handle(Geom_BSplineCurve) spline_u3 = new Geom_BSplineCurve(controlPoints_u3, knots, mults, degree);
    spline_u3->InsertKnot(0.6);
    // creating fourth u-directional B-spline which represents y(z) = (z - 0.5)^2 with offset 4 in x-direction
    TColgp_Array1OfPnt controlPoints_u4(1, 4);
    controlPoints_u4(1) = gp_Pnt(4., 0.25, 0.);
    controlPoints_u4(2) = gp_Pnt(4., -1. / 12, 1. / 3);
    controlPoints_u4(3) = gp_Pnt(4., -1. / 12, 2. / 3);
    controlPoints_u4(4) = gp_Pnt(4., 0.25, 1.);

    Handle(Geom_BSplineCurve) spline_u4 = new Geom_BSplineCurve(controlPoints_u4, knots, mults, degree);
    spline_u4->InsertKnot(0.6);

    // creating first v-directional B-spline which represents z(x) = 0 at y = 0.25
    TColgp_Array1OfPnt controlPoints_v1(1, 4);
    controlPoints_v1(1) = gp_Pnt(-1., 0.25, 0.);
    controlPoints_v1(2) = gp_Pnt(2. / 3, 0.25, 0.);
    controlPoints_v1(3) = gp_Pnt(7. / 3, 0.25, 0.);
    controlPoints_v1(4) = gp_Pnt(4., 0.25, 0.);

    Handle(Geom_BSplineCurve) spline_v1 = new Geom_BSplineCurve(controlPoints_v1, knots, mults, degree);

    // creating second v-directional B-spline which represents z(x) = 0.5 - sqrt(0.1) at y = 0.1
    TColgp_Array1OfPnt controlPoints_v2(1, 4);
    controlPoints_v2(1) = gp_Pnt(-1., 0.1, 0.5 - std::sqrt(0.1));
    controlPoints_v2(2) = gp_Pnt(2. / 3, 0.1, 0.5 - sqrt(0.1));
    controlPoints_v2(3) = gp_Pnt(7. / 3, 0.1, 0.5 - sqrt(0.1));
    controlPoints_v2(4) = gp_Pnt(4., 0.1, 0.5 - sqrt(0.1));

    Handle(Geom_BSplineCurve) spline_v2 = new Geom_BSplineCurve(controlPoints_v2, knots, mults, degree);

    // creating third v-directional B-spline which represents z(x) = 0.5 - sqrt(0.05) at y = 0.05
    TColgp_Array1OfPnt controlPoints_v3(1, 4);
    controlPoints_v3(1) = gp_Pnt(-1., 0.05, 0.5 - std::sqrt(0.05));
    controlPoints_v3(2) = gp_Pnt(2. / 3, 0.05, 0.5 - sqrt(0.05));
    controlPoints_v3(3) = gp_Pnt(7. / 3, 0.05, 0.5 - sqrt(0.05));
    controlPoints_v3(4) = gp_Pnt(4., 0.05, 0.5 - sqrt(0.05));

    Handle(Geom_BSplineCurve) spline_v3 = new Geom_BSplineCurve(controlPoints_v3, knots, mults, degree);

    // creating fourth v-directional B-spline which represents z(x) = 0.5 + sqrt(0.1) at y = 0.1
    TColgp_Array1OfPnt controlPoints_v4(1, 4);
    controlPoints_v4(1) = gp_Pnt(-1., 0.1, 0.5 + std::sqrt(0.1));
    controlPoints_v4(2) = gp_Pnt(2. / 3, 0.1, 0.5 + sqrt(0.1));
    controlPoints_v4(3) = gp_Pnt(7. / 3, 0.1, 0.5 + sqrt(0.1));
    controlPoints_v4(4) = gp_Pnt(4., 0.1, 0.5 + sqrt(0.1));

    Handle(Geom_BSplineCurve) spline_v4 = new Geom_BSplineCurve(controlPoints_v4, knots, mults, degree);

    // creating fifth v-directional B-spline which represents z(x) = 1 at y = 0.25
    TColgp_Array1OfPnt controlPoints_v5(1, 4);
    controlPoints_v5(1) = gp_Pnt(-1., 0.25, 1.);
    controlPoints_v5(2) = gp_Pnt(2. / 3, 0.25, 1.);
    controlPoints_v5(3) = gp_Pnt(7. / 3, 0.25, 1.);
    controlPoints_v5(4) = gp_Pnt(4., 0.25, 1.);

    Handle(Geom_BSplineCurve) spline_v5 = new Geom_BSplineCurve(controlPoints_v5, knots, mults, degree);

    // u- and v-directional B-splines are already compatible in B-spline sense (common knot vector, same parametrization)
    std::vector<Handle(Geom_BSplineCurve)> splines_u_vector;
    splines_u_vector.push_back(spline_u3);
    splines_u_vector.push_back(spline_u1);
    splines_u_vector.push_back(spline_u2);
    splines_u_vector.push_back(spline_u4);

    std::vector<Handle(Geom_BSplineCurve)> splines_v_vector;
    splines_v_vector.push_back(spline_v5);
    splines_v_vector.push_back(spline_v4);
    splines_v_vector.push_back(spline_v2);
    splines_v_vector.push_back(spline_v3);
    splines_v_vector.push_back(spline_v1);

    Handle(Geom_BSplineSurface) gordonSurface = CTiglBSplineAlgorithms::createGordonSurfaceGeneral(splines_u_vector, splines_v_vector);

    // after creating the test surface above, now test it:
    for (int u_idx = 0; u_idx <= 100; ++u_idx) {
        for (int v_idx = 0; v_idx <= 100; ++v_idx) {
            double u_value = u_idx / 100.;
            double v_value = v_idx / 100.;

            gp_Pnt surface_point = gordonSurface->Value(v_value, u_value);
            gp_Pnt point_curve1 = spline_u1->Value(u_value);  // represents y(z) = (z - 0.5)^2 with offset -1 in x-direction
            gp_Pnt point_curve2 = spline_u4->Value(u_value);  // represents y(z) = (z - 0.5)^2 with offset 2 in x-direction
            gp_Pnt right_point(point_curve1.X() * (1. - v_value) + point_curve2.X() * v_value, point_curve1.Y() * (1. - v_value) + point_curve2.Y() * v_value, point_curve1.Z() * (1. - v_value) + point_curve2.Z() * v_value);

            ASSERT_NEAR(surface_point.X(), right_point.X(), 1e-14);
            ASSERT_NEAR(surface_point.Y(), right_point.Y(), 1e-14);
            ASSERT_NEAR(surface_point.Z(), right_point.Z(), 1e-14);
        }
    }
}

TEST(TiglBSplineAlgorithms, gordonSurfaceWing2)
{
    // u-directional B-spline curves
    // first read the brep-input file
    TopoDS_Shape shape_u;
    Standard_CString file_u = "TestData/wing2/profiles.brep";
    BRep_Builder builder_u;

    BRepTools::Read(shape_u, file_u, builder_u);

     TopExp_Explorer Explorer;
     // get the splines in u-direction from the Edges
     std::vector<Handle(Geom_BSplineCurve)> splines_u_vector;
     for (Explorer.Init(shape_u, TopAbs_EDGE); Explorer.More(); Explorer.Next()) {
         TopoDS_Edge curve_edge = TopoDS::Edge(Explorer.Current());
         double beginning = 0;
         double end = 1;
         Handle(Geom_Curve) curve = BRep_Tool::Curve(curve_edge, beginning, end);
         Handle(Geom_BSplineCurve) spline = GeomConvert::CurveToBSplineCurve(curve);
         splines_u_vector.push_back(spline);
     }

    // v-directional B-spline curves
    // first read the BRep-input file
    TopoDS_Shape shape_v;
    Standard_CString file_v = "TestData/wing2/guides.brep";
    BRep_Builder builder_v;

    BRepTools::Read(shape_v, file_v, builder_v);

    // now filter out the Edges
    TopTools_IndexedMapOfShape mapEdges_v;
    TopExp::MapShapes(shape_v, TopAbs_EDGE, mapEdges_v);

    // get the splines in v-direction from the Edges
    std::vector<Handle(Geom_BSplineCurve)> splines_v_vector;
    for (Explorer.Init(shape_v, TopAbs_EDGE); Explorer.More(); Explorer.Next()) {
        TopoDS_Edge curve_edge = TopoDS::Edge(Explorer.Current());
        double beginning = 0;
        double end = 1;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(curve_edge, beginning, end);
        Handle(Geom_BSplineCurve) spline = GeomConvert::CurveToBSplineCurve(curve);
        splines_v_vector.push_back(spline);
    }


    Handle(Geom_BSplineSurface) gordonSurface = CTiglBSplineAlgorithms::createGordonSurfaceGeneral(splines_u_vector, splines_v_vector);
    BRepTools::Write(BRepBuilderAPI_MakeFace(gordonSurface, Precision::Confusion()), "TestData/Wing2_result.brep");
}


} // namespace tigl
