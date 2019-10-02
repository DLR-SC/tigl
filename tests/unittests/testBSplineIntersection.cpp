#include "test.h"

#include <Geom_BSplineCurve.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TColStd_HArray1OfInteger.hxx>
#include <TColgp_HArray1OfPnt.hxx>

#include "CTiglIntersectBSplines.h"
#include "tiglcommonfunctions.h"

TEST(BSplineIntersection, ex1)
{
    auto knots = OccFArray({0., 1.25, 2.5, 3.75, 5.});
    auto mults = OccIArray({3, 1, 1, 1, 3});

    auto cp = OccArray({
        gp_Pnt(0., 0., 0.),
        gp_Pnt(1., 5., 0.),
        gp_Pnt(2., 0., 0.),
        gp_Pnt(3., 5., 0.),
        gp_Pnt(4., 0., 0.),
        gp_Pnt(5., 5., 0.)
    });

    auto cp2 = OccArray({
        gp_Pnt(0., 0., 0.),
        gp_Pnt(10., 1., 0.),
        gp_Pnt(-1.62, 2., 0.),
        gp_Pnt(4.78, 3., 0.),
        gp_Pnt(0., 4., 0.),
        gp_Pnt(5., 5., 0.)
    });

    Handle(Geom_BSplineCurve) c1 = new Geom_BSplineCurve(cp->Array1(), knots->Array1(), mults->Array1(), 2);
    Handle(Geom_BSplineCurve) c2 = new Geom_BSplineCurve(cp2->Array1(), knots->Array1(), mults->Array1(), 2);

    const double tolerance = 0.03;
    auto results = tigl::IntersectBSplines(c1, c2, tolerance);

    EXPECT_EQ(11, results.size());

    for (auto result : results) {
        gp_Pnt p1 = c1->Value(result.parmOnCurve1);
        gp_Pnt p2 = c2->Value(result.parmOnCurve2);
        EXPECT_LE(p1.Distance(p2), tolerance);
    }

    // Values from Python Code / Manually verified
    EXPECT_NEAR(0.0, tigl::CTiglPoint(9.025569161817309e-09, 3.7549883362060295e-09, 0.).distance2(results[0].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(0.4822782514496766, 2.0792221838260083, 0.).distance2(results[1].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(0.5299902542589878, 2.2428238892925423, 0.).distance2(results[2].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(1.6759669326608981, 1.7749871436459341, 0.).distance2(results[3].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(1.5024716597735064, 2.4876722466426440, 0.).distance2(results[4].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(2.2970753765027037, 1.6912688966211170, 0.).distance2(results[5].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(2.5367033831340510, 2.6767812240028290, 0.).distance2(results[6].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(2.7425055392358964, 3.4184830133790154, 0.).distance2(results[7].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(3.3861085187441030, 3.0046010587661650, 0.).distance2(results[8].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(3.3346825588859490, 3.1899379238877654, 0.).distance2(results[9].point), 1e-8);
    EXPECT_NEAR(0.0, tigl::CTiglPoint(4.9999999788784610, 4.9999999788908305, 0.).distance2(results[10].point), 1e-8);
}

TEST(BSplineIntersection, ex2)
{
    auto knots = OccFArray({0., 5.});
    auto mults = OccIArray({2, 2});

    auto cp = OccArray({
        gp_Pnt(0., 0., 0.),
        gp_Pnt(0.95, 0., 0.)
    });

    auto cp2 = OccArray({
        gp_Pnt(1., 1., 0.),
        gp_Pnt(1., .05, 0.)
    });

    Handle(Geom_BSplineCurve) c1 = new Geom_BSplineCurve(cp->Array1(), knots->Array1(), mults->Array1(), 1);
    Handle(Geom_BSplineCurve) c2 = new Geom_BSplineCurve(cp2->Array1(), knots->Array1(), mults->Array1(), 1);

    const double tolerance = 0.1;
    auto results = tigl::IntersectBSplines(c1, c2, tolerance);

    EXPECT_EQ(1, results.size());

    for (auto result : results) {
        gp_Pnt p1 = c1->Value(result.parmOnCurve1);
        gp_Pnt p2 = c2->Value(result.parmOnCurve2);
        EXPECT_LE(p1.Distance(p2), tolerance);
    }

    // Values from Python Code / Manually verified
    EXPECT_NEAR(0.0, tigl::CTiglPoint(0.975, 0.025, 0.).distance2(results[0].point), 1e-8);
}
