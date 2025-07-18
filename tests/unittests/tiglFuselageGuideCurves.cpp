/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief Tests for fuselage guide curves
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include "testUtils.h"
#include "CCPACSConfigurationManager.h"
#include "BRep_Tool.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "Geom_Curve.hxx"
#include "Geom_Plane.hxx"
#include "Geom_Circle.hxx"
#include "gp_Pnt.hxx"
#include "gp_Pnt.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "GeomAPI_IntCS.hxx"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CCPACSFuselage.h"
#include "CCPACSGuideCurveProfile.h"
#include "generated/CPACSGuideCurveProfiles.h"
#include "CCPACSFuselageProfileGetPointAlgo.h"
#include "CCPACSGuideCurveAlgo.h"
#include "CCPACSFuselageSegment.h"
#include "CTiglLogging.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglFuselageSegmentGuidecurveBuilder.h"

using namespace std;

/******************************************************************************/

class FuselageGuideCurve : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/simple_test_guide_curves.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE(tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "GuideCurveModel", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // constant values for the guide curve points
        const double tempy[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
        beta                 = std::vector<double>(tempy, tempy + sizeof(tempy) / sizeof(tempy[0]));
        const double tempz[] = {0.0, 0.001, 0.003, 0.009, 0.008, 0.007, 0.006, 0.002, 0.0};
        gamma                = std::vector<double>(tempz, tempz + sizeof(tempz) / sizeof(tempz[0]));
    }

    void TearDown() override
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    // save x-y data
    void outputXY(const int& i, const double& x, const double& y, const std::string& filename)
    {
        ofstream out;
        if (i > 0) {
            out.open(filename.c_str(), ios::app);
        }
        else {
            out.open(filename.c_str());
        }
        out << setprecision(17) << std::scientific << x << "\t" << y << endl;
        out.close();
    }
    void outputXYVector(const int& i, const double& x, const double& y, const double& vx, const double& vy,
                        const std::string& filename)
    {
        ofstream out;
        if (i > 0) {
            out.open(filename.c_str(), ios::app);
        }
        else {
            out.open(filename.c_str());
        }
        out << setprecision(17) << std::scientific << x << "\t" << y << "\t" << vx << "\t" << vy << "\t" << endl;
        out.close();
    }

    TixiDocumentHandle tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    //tigl::CCPACSGuideCurve guideCurve;
    std::vector<double> alpha;
    std::vector<double> beta;
    std::vector<double> gamma;
};

class FuselageGuideCurve2 : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/bugs/747/simpletest_fuselage_guides.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE(tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "CpacsTest", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    void TearDown() override
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};

class FuselageGuideCurveAtKink : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/kinks_withGC_ParameterDef.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE(tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    void TearDown() override
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};

/******************************************************************************/

/**
* Tests CCPACSGuideCurveProfile class
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSGuideCurveProfile)
{
    tigl::CCPACSGuideCurveProfile guideCurve(NULL, NULL);
    guideCurve.ReadCPACS(tixiHandle, "/cpacs/vehicles/profiles/guideCurves/guideCurveProfile[2]");
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Fuselage_GuideCurveProfile_Middle_NonLinear");
    ASSERT_EQ(guideCurve.GetName(), "NonLinear Middle Guide Curve Profile for GuideCurveModel - Fuselage");
}
/**
* Tests CCPACSGuideCurveProfiles class
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSGuideCurveProfiles)
{
    tigl::CCPACSGuideCurveProfiles guideCurves(NULL, NULL);
    guideCurves.ReadCPACS(tixiHandle, "/cpacs/vehicles/profiles/guideCurves");
    ASSERT_EQ(guideCurves.GetGuideCurveProfileCount(), 6);
    tigl::CCPACSGuideCurveProfile& guideCurve =
        guideCurves.GetGuideCurveProfile("GuideCurveModel_Fuselage_GuideCurveProfile_Middle_NonLinear");
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Fuselage_GuideCurveProfile_Middle_NonLinear");
    ASSERT_EQ(guideCurve.GetName(), "NonLinear Middle Guide Curve Profile for GuideCurveModel - Fuselage");
}

/**
* Tests CCPACSFuselageProfileGetPointAlgo class
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSFuselageProfileGetPointAlgoOnCircle)
{
    double radius1 = 1.0;
    gp_Pnt location1(radius1, 0.0, 0.0);
    gp_Ax2 circlePosition1(location1, gp::DY(), gp::DX());
    Handle(Geom_Circle) circle1 = new Geom_Circle(circlePosition1, radius1);

    // convert to edge
    double start          = 0.0;
    double end            = 2 * M_PI;
    TopoDS_Edge innerEdge = BRepBuilderAPI_MakeEdge(circle1, start, end);

    // convert to wires
    TopoDS_Wire innerWire = BRepBuilderAPI_MakeWire(innerEdge);

    // put wire into container for getPointAlgo
    TopTools_SequenceOfShape innerWireContainer;
    innerWireContainer.Append(innerWire);

    // instantiate getPointAlgo
    tigl::CCPACSFuselageProfileGetPointAlgo getPointAlgo(innerWireContainer);
    gp_Pnt point;
    gp_Vec tangent;

    // plot points and tangents
    int N = 20;
    int M = 2;
    for (int i = 0; i <= N + 2 * M; i++) {
        double da    = 1.0 / double(N);
        double alpha = -M * da + da * i;
        getPointAlgo.GetPointTangent(alpha, point, tangent);
        outputXY(i, point.X(), point.Z(), "./TestData/analysis/tiglFuselageGuideCurve_circleSamplePoints_points.dat");
        outputXYVector(i, point.X(), point.Z(), tangent.X(), tangent.Z(),
                       "./TestData/analysis/tiglFuselageGuideCurve_circleSamplePoints_tangents.dat");
        // plot points and tangents with gnuplot by:
        // echo "plot 'TestData/analysis/tiglFuselageGuideCurve_circleSamplePoints_tangents.dat' u 1:2:3:4 with vectors filled head lw 2, 'TestData/analysis/tiglFuselageGuideCurve_circleSamplePoints_points.dat' w linespoints lw 2" | gnuplot -persist
    }

    // start: point must be zero and tangent must be in negative z-direction and has to be of length pi
    getPointAlgo.GetPointTangent(0.0, point, tangent);
    ASSERT_NEAR(point.X(), 2.0, 1E-10);
    ASSERT_NEAR(point.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point.Z(), 0.0, 1E-10);
    ASSERT_NEAR(tangent.X(), 0.0, 1E-10);
    ASSERT_NEAR(tangent.Y(), 0.0, 1E-10);
    ASSERT_NEAR(tangent.Z(), -2 * M_PI, 1E-10);

    // end: Tangent must be in negative z-direction has to be of length pi
    getPointAlgo.GetPointTangent(-1.0, point, tangent);
    ASSERT_NEAR(point.X(), 2.0, 1E-10);
    ASSERT_NEAR(point.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point.Z(), 2 * M_PI, 1E-10);
    ASSERT_NEAR(tangent.X(), 0.0, 1E-10);
    ASSERT_NEAR(tangent.Y(), 0.0, 1E-10);
    ASSERT_NEAR(tangent.Z(), -2 * M_PI, 1E-10);

    // check points and tangents for alpha > 1
    gp_Pnt point2;
    gp_Vec tangent2;
    getPointAlgo.GetPointTangent(1.0, point, tangent);
    getPointAlgo.GetPointTangent(2.0, point2, tangent2);
    ASSERT_NEAR(point2.X(), 2.0, 1E-10);
    ASSERT_NEAR(point2.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point2.Z(), -2 * M_PI, 1E-10);
    ASSERT_NEAR(tangent2.X(), 0.0, 1E-10);
    ASSERT_NEAR(tangent2.Y(), 0.0, 1E-10);
    ASSERT_NEAR(tangent2.Z(), -2 * M_PI, 1E-10);
    ASSERT_EQ(tangent.X(), tangent2.X());
    ASSERT_EQ(tangent.Y(), tangent2.Y());
    ASSERT_EQ(tangent.Z(), tangent2.Z());
    ASSERT_NEAR(point.Distance(point2), 2 * M_PI, 1E-10);

    // check if tangent is constant for alpha < 0
    getPointAlgo.GetPointTangent(0.0, point, tangent);
    getPointAlgo.GetPointTangent(-1.0, point2, tangent2);
    ASSERT_NEAR(point2.X(), 2.0, 1E-10);
    ASSERT_NEAR(point2.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point2.Z(), 2 * M_PI, 1E-10);
    ASSERT_NEAR(tangent2.X(), 0.0, 1E-10);
    ASSERT_NEAR(tangent2.Y(), 0.0, 1E-10);
    ASSERT_NEAR(tangent2.Z(), -2 * M_PI, 1E-10);
    ASSERT_EQ(tangent.X(), tangent2.X());
    ASSERT_EQ(tangent.Y(), tangent2.Y());
    ASSERT_EQ(tangent.Z(), tangent2.Z());
    ASSERT_NEAR(point.Distance(point2), 2 * M_PI, 1E-10);
}
/**
* * Tests CCPACSFuselageProfileGetPointAlgo class
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSFuselageProfileGetPointAlgo)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    // get upper and lower fuselage profile
    tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile("GuideCurveModel_Fuselage_Sec3_El1_Pro");
    TopoDS_Wire wire                     = profile.GetWire();

    // pack wire container for get point algo
    TopTools_SequenceOfShape wireContainer;
    wireContainer.Append(wire);
    // instantiate getPointAlgo
    tigl::CCPACSFuselageProfileGetPointAlgo getPointAlgo(wireContainer);
    gp_Pnt point;
    gp_Vec tangent;

    // plot points and tangents
    int N = 20;
    int M = 2;
    for (int i = 0; i <= N + 2 * M; i++) {
        double da    = 1.0 / double(N);
        double alpha = -M * da + da * i;
        getPointAlgo.GetPointTangent(alpha, point, tangent);
        outputXY(i, point.Y(), point.Z(), "./TestData/analysis/tiglFuselageGuideCurve_profileSamplePoints_points.dat");
        outputXYVector(i, point.Y(), point.Z(), tangent.Y(), tangent.Z(),
                       "./TestData/analysis/tiglFuselageGuideCurve_profileSamplePoints_tangents.dat");
        // plot points and tangents with gnuplot by:
        // echo "plot 'TestData/analysis/tiglFuselageGuideCurve_profileSamplePoints_tangents.dat' u 1:2:3:4 with vectors filled head lw 2, 'TestData/analysis/tiglFuselageGuideCurve_profileSamplePoints_points.dat' w linespoints lw 2" | gnuplot -persist
    }

    // lower point
    getPointAlgo.GetPointTangent(0.0, point, tangent);
    ASSERT_NEAR(point.X(), 0.0, 1E-10);
    ASSERT_NEAR(point.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point.Z(), -0.5, 1E-10);

    // upper point
    getPointAlgo.GetPointTangent(1.0, point, tangent);
    ASSERT_NEAR(point.X(), 0.0, 1E-10);
    ASSERT_NEAR(point.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point.Z(), 0.487497455, 1E-10);

    // check if tangent is constant for alpha > 1
    gp_Vec tangent2;
    getPointAlgo.GetPointTangent(1.0, point, tangent);
    getPointAlgo.GetPointTangent(2.0, point, tangent2);
    ASSERT_EQ(tangent.X(), tangent2.X());
    ASSERT_EQ(tangent.Y(), tangent2.Y());
    ASSERT_EQ(tangent.Z(), tangent2.Z());

    // check if tangent is constant for alpha < 0
    getPointAlgo.GetPointTangent(0.0, point, tangent);
    getPointAlgo.GetPointTangent(-1.0, point, tangent2);
    ASSERT_EQ(tangent.X(), tangent2.X());
    ASSERT_EQ(tangent.Y(), tangent2.Y());
    ASSERT_EQ(tangent.Z(), tangent2.Z());
}

/**
* Tests CCPACSGuideCurveAlgo class
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSGuideCurveAlgo)
{
    // create two circles
    double radius1  = 1.0;
    double radius2  = 2.0;
    double distance = 4.0;
    gp_Pnt location1(0.0, -radius1, 0.0);
    gp_Ax2 circlePosition1(location1, gp::DX(), gp::DZ());
    Handle(Geom_Circle) circle1 = new Geom_Circle(circlePosition1, radius1);
    gp_Pnt location2(distance, -radius2, 0.0);
    gp_Ax2 circlePosition2(location2, gp::DX(), gp::DZ());
    Handle(Geom_Circle) circle2 = new Geom_Circle(circlePosition2, radius2);

    // convert to wires and consider only half circles starting at the bottom
    double start      = M_PI;
    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(circle1, start, start + M_PI);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(circle2, start, start + M_PI);
    TopoDS_Wire wire1 = BRepBuilderAPI_MakeWire(edge1);
    TopoDS_Wire wire2 = BRepBuilderAPI_MakeWire(edge2);

    // container for guide curve algo
    TopTools_SequenceOfShape wireContainer1;
    wireContainer1.Append(wire1);
    TopTools_SequenceOfShape wireContainer2;
    wireContainer2.Append(wire2);

    // get guide curve profile
    tigl::CCPACSGuideCurveProfile guideCurveProfile(NULL, NULL);
    guideCurveProfile.ReadCPACS(tixiHandle, "/cpacs/vehicles/profiles/guideCurves/guideCurveProfile[2]");

    std::vector<gp_Pnt> guideCurvePnts;
    // instantiate guideCurveAlgo
    guideCurvePnts = tigl::CCPACSGuideCurveAlgo<tigl::CCPACSFuselageProfileGetPointAlgo>(
        wireContainer1, wireContainer2, 0.5, 0.5, 2 * radius1, 2 * radius2, gp_Dir(0.0, 0.0, 1.0), guideCurveProfile);
    TopoDS_Edge guideCurveEdge = EdgeSplineFromPoints(guideCurvePnts);

    // check if guide curve runs through sample points
    // get curve
    Standard_Real u1, u2;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(guideCurveEdge, u1, u2);
    // set predicted sample points from cpacs file
    const double temp[] = {0, 0, 0.012, 0.037, 0.110, 0.098, 0.086, 0.073, 0.024, 0, 0};
    std::vector<double> predictedSamplePointsY(temp, temp + sizeof(temp) / sizeof(temp[0]));
    for (unsigned int i = 0; i <= 10; ++i) {
        // get intersection point of the guide curve with planes parallel to the y-z plane located at a
        double a                 = i / double(10);
        Handle(Geom_Plane) plane = new Geom_Plane(gp_Pnt(a * distance, 0.0, 0.0), gp_Dir(1.0, 0.0, 0.0));
        GeomAPI_IntCS intersection(curve, plane);
        ASSERT_EQ(Standard_True, intersection.IsDone());
        ASSERT_EQ(intersection.NbPoints(), 1);
        gp_Pnt point = intersection.Point(1);

        // scale sample points since 2nd profile is scaled by a factor 2
        predictedSamplePointsY[i] *= (2 * radius1 + (2 * radius2 - 2 * radius1) * a);
        // check is guide curve runs through the predicted sample points
        ASSERT_NEAR(a * distance, point.X(), 1E-14);
        ASSERT_NEAR(predictedSamplePointsY[i], point.Y(), 1E-14);
        ASSERT_NEAR(0.0, point.Z(), 1E-14);
    }
}

/**
* Tests fuselage segment guide curve routines
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSFuselageSegment)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSFuselage& fuselage            = config.GetFuselage(1);

    ASSERT_EQ(fuselage.GetSegmentCount(), 2);
    tigl::CCPACSFuselageSegment& segment1 = (tigl::CCPACSFuselageSegment&)fuselage.GetSegment(1);

    ASSERT_TRUE(segment1.GetGuideCurves().get_ptr() != NULL);
    tigl::CCPACSGuideCurves& guides = *segment1.GetGuideCurves();
    ASSERT_EQ(guides.GetGuideCurveCount(), 3);

    // obtain leading edge guide curve
    TopoDS_Edge guideCurveWire = guides.GetGuideCurve(2).GetCurve();

    // check if guide curve runs through sample points
    // get curve
    Standard_Real u1, u2;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(guideCurveWire, u1, u2);
    // gamma values of cpacs data points
    const double temp[] = {0, 0, 0.012, 0.037, 0.110, 0.098, 0.086, 0.073, 0.024, 0, 0};
    std::vector<double> gammaDeviation(temp, temp + sizeof(temp) / sizeof(temp[0]));
    // number of sample points
    unsigned int N = 10;
    // segment length
    double length = 10.0;
    // segment position
    double position = -10.0;
    // start profile scale factor
    double startScale = 1.0;
    // end profile scale factor
    double endScale = 2.0;
    for (unsigned int i = 0; i <= N; ++i) {
        // get intersection point of the guide curve with planes parallel to the y-z-direction
        // located at a
        double a             = length * i / double(N);
        gp_Pnt planeLocation = gp_Pnt(a + position, 0.5 * startScale + 0.5 * (endScale - startScale) / length * a, 0.0);
        gp_Vec dirVec(gp_Pnt(position, 0.5 * startScale, 0.0), gp_Pnt(0.0, 0.5 * endScale, 0.0));
        Handle(Geom_Plane) plane = new Geom_Plane(planeLocation, gp_Dir(dirVec));
        GeomAPI_IntCS intersection(curve, plane);
        ASSERT_EQ(intersection.NbPoints(), 1);
        gp_Pnt point = intersection.Point(1);

        // start at segment minimal x position
        gp_Vec predictedPoint(position, 0.0, 0.0);
        // go along the fuselage segment maximal y edge
        predictedPoint += gp_Vec(0.0, 0.5 * startScale + 0.5 * (endScale - startScale) / length * a, 0.0);
        predictedPoint += gp_Vec(a, 0.0, 0.0);
        // scale sample points since outer profile's diameter is greater by a factor of 2
        double s = (startScale + (endScale - startScale) * i / double(N));
        // go along direction perpendicular to the leading edge in the x-y plane
        double angle = atan2(0.5 * (endScale - startScale), length);
        predictedPoint += gp_Vec(-sin(angle) * gammaDeviation[i] * s, cos(angle) * gammaDeviation[i] * s, 0.0);

        // check is guide curve runs through the predicted sample points
        ASSERT_NEAR(predictedPoint.X(), point.X(), 1E-10);
        ASSERT_NEAR(predictedPoint.Y(), point.Y(), 1E-10);
        ASSERT_NEAR(predictedPoint.Z(), point.Z(), 1E-14);
    }
}

TEST_F(FuselageGuideCurve2, bug747)
{
    // https://github.com/DLR-SC/tigl/issues/747

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSFuselage& fuselage            = config.GetFuselage(1);

    tigl::CCPACSFuselageProfile& startProfile = fuselage.GetSegment(1).GetStartConnection().GetProfile();
    TopoDS_Wire diameter                      = startProfile.GetDiameterWire();

    // Can be calculate the correct diameter?
    EXPECT_NEAR(GetLength(diameter), 2., 1e-3);

    // Can we build the loft? Is the result reasonable?
    PNamedShape loft = fuselage.GetLoft();
    double minx, maxx, miny, maxy, minz, maxz;
    GetShapeExtension(loft->Shape(), minx, maxx, miny, maxy, minz, maxz);
    EXPECT_TRUE(minx > -1.5);
    EXPECT_TRUE(maxx < 2.);
    EXPECT_TRUE(miny > -1.);
    EXPECT_TRUE(maxy < 1.);
    EXPECT_TRUE(minz > -1.);
    EXPECT_TRUE(maxz < 1.);
}


TEST(FuselageGuideCurve_bug, 766)
{
    // https://github.com/DLR-SC/tigl/issues/766
    const char* filename = "TestData/bugs/766/simple_test_guide_curves.xml";
    ReturnCode tixiRet;
    TiglReturnCode tiglRet;

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;

    tixiRet = tixiOpenDocument(filename, &tixiHandle);
    ASSERT_TRUE(tixiRet == SUCCESS);
    tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "GuideCurveModel", &tiglHandle);
    ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    tigl::CCPACSFuselage& fuselage = config.GetFuselage(1);
    EXPECT_THROW(fuselage.GetLoft()->Shape(), tigl::CTiglError);

    tigl::CCPACSWing& wing = config.GetWing(1);
    EXPECT_THROW(wing.GetLoft()->Shape(), tigl::CTiglError);

}

TEST_F(FuselageGuideCurveAtKink, kinksGuideCurvesParameterDef)
{
    double x,y,z;
    double x2,y2,z2;
    // Get the points' coordinates and compare to guide curve start and end
    tiglFuselageGetPoint(tiglHandle, 1, 1, 0, 0.28, &x, &y, &z);
    tiglFuselageGetPoint(tiglHandle, 1, 1, 1, 0.28, &x2, &y2, &z2);

    auto& uid_mgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    auto& segment = uid_mgr.ResolveObject<tigl::CCPACSFuselageSegment>("segmentD150_Fuselage_1Segment2ID");
    auto& guideCurves = *segment.GetGuideCurves();
    tigl::CCPACSGuideCurve& guideCurve = guideCurves.GetGuideCurve(2);
    auto gcBuilder = tigl::CTiglFuselageSegmentGuidecurveBuilder(segment);

    gp_Pnt firstPointGC = gcBuilder.BuildGuideCurvePnts(&guideCurve)[0];
    gp_Pnt lastPointGC = gcBuilder.BuildGuideCurvePnts(&guideCurve).back();
    double tolerance = 1e-12;

    // Compare guide curve's starting point with expected value on profile
    ASSERT_NEAR(x, firstPointGC.X(), tolerance);
    ASSERT_NEAR(y, firstPointGC.Y(), tolerance);
    ASSERT_NEAR(z, firstPointGC.Z(), tolerance);

    // Compare guide curve's end point with expected value on profile
    ASSERT_NEAR(x2, lastPointGC.X(), tolerance);
    ASSERT_NEAR(y2, lastPointGC.Y(), tolerance);
    ASSERT_NEAR(z2, lastPointGC.Z(), tolerance);
}

TEST_F(FuselageGuideCurveAtKink, getFromDefinition_checkArgs)
{
    auto& uid_mgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    auto& segment = uid_mgr.ResolveObject<tigl::CCPACSFuselageSegment>("segmentD150_Fuselage_1Segment2ID");
    auto& guideCurves = *segment.GetGuideCurves();
    tigl::CCPACSGuideCurve& guideCurve = guideCurves.GetGuideCurve(2);

    guideCurve.SetFromGuideCurveUID_choice1(boost::none);
    guideCurve.SetFromRelativeCircumference_choice2_1(boost::none);
    guideCurve.SetFromParameter_choice2_2(boost::none);
    EXPECT_THROW(guideCurve.GetFromDefinition(), tigl::CTiglError);
    EXPECT_THROW(guideCurve.GetRootCurve(), tigl::CTiglError);
}

TEST_F(FuselageGuideCurveAtKink, getToDefinition_checkArgs)
{
    auto& uid_mgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    auto& segment = uid_mgr.ResolveObject<tigl::CCPACSFuselageSegment>("segmentD150_Fuselage_1Segment2ID");
    auto& guideCurves = *segment.GetGuideCurves();
    tigl::CCPACSGuideCurve& guideCurve = guideCurves.GetGuideCurve(2);

    guideCurve.SetToRelativeCircumference_choice1(boost::none);
    guideCurve.SetToParameter_choice2(boost::none);
    EXPECT_THROW(guideCurve.GetToDefinition(), tigl::CTiglError);
}

TEST_F(FuselageGuideCurveAtKink, GuideCurveAlgo_checkArgs)
{
    auto& uid_mgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    auto& segment = uid_mgr.ResolveObject<tigl::CCPACSFuselageSegment>("segmentD150_Fuselage_1Segment2ID");
    auto& guideCurves = *segment.GetGuideCurves();
    tigl::CCPACSGuideCurve& guideCurve = guideCurves.GetGuideCurve(2);

    tigl::CTiglFuselageConnection& startConnection = segment.GetStartConnection();
    tigl::CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
    TopoDS_Wire startWire = startProfile.GetWire(!startProfile.GetMirrorSymmetry());
    startWire = TopoDS::Wire(transformFuselageProfileGeometry(segment.GetParent()->GetParentComponent()->GetTransformationMatrix(), startConnection, startWire));
    TopTools_SequenceOfShape startWireContainer;
    startWireContainer.Append(startWire);

    tigl::CCPACSConfiguration const& config = segment.GetParent()->GetConfiguration();
    tigl::CCPACSGuideCurveProfile const& guideCurveProfile = config.GetGuideCurveProfile("leftFuselageCurve");

    // Testing the catch of wrong argument UID as toParameter (using a lot of other dummy arguments)
    EXPECT_THROW(
                std::vector<gp_Pnt> guideCurvePnts = tigl::CCPACSGuideCurveAlgo<tigl::CCPACSFuselageProfileGetPointAlgo> (startWireContainer,
                                                                                                                  startWireContainer,
                                                                                                                  0.,
                                                                                                                  0.,
                                                                                                                  0.,
                                                                                                                  0.,
                                                                                                                  gp_Dir(0.,0.,1.),
                                                                                                                  guideCurveProfile,
                                                                                                                  tigl::CCPACSGuideCurve::FromOrToDefinition::PARAMETER,
                                                                                                                  tigl::CCPACSGuideCurve::FromOrToDefinition::UID),
                tigl::CTiglError);
}
