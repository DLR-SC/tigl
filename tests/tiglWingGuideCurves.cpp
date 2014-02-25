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
* @brief Tests for wing guide curves
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include "testUtils.h"
#include "tiglcommonfunctions.h"
#include "CSharedPtr.h"
#include "CCPACSConfigurationManager.h"
#include "BRep_Tool.hxx"
#include "TopoDS_Shape.hxx"
#include "TopTools_SequenceOfShape.hxx"
#include "TopExp_Explorer.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "Geom_Curve.hxx"
#include "Geom_Plane.hxx"
#include "Geom_Circle.hxx"
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"
#include "BRep_Builder.hxx"
#include "GeomAPI_IntCS.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "CTiglError.h"
#include "CTiglTransformation.h"
#include "CCPACSGuideCurveProfile.h"
#include "CCPACSGuideCurveProfiles.h"
#include "CCPACSGuideCurve.h"
#include "CCPACSGuideCurves.h"
#include "CCPACSWingProfileGetPointAlgo.h"
#include "CCPACSGuideCurveAlgo.h"
#include "CCPACSWingSegment.h"

/******************************************************************************/

typedef class CSharedPtr<tigl::CTiglPoint> PCTiglPoint;

class WingGuideCurve : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        const char* filename = "TestData/simple_test_guide_curves.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "GuideCurveModel", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // get guide curve
        //tigl::CCPACSGuideCurve & guideCurve = config.GetGuideCurve("GuideCurveModel_Wing_Sec1_El1_Pro");

        // constant values for the guide curve points
        const double tempy[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
        beta=std::vector<double>(tempy, tempy + sizeof(tempy) / sizeof(tempy[0]) );
        const double tempz[] = {0.0, 0.001, 0.003, 0.009, 0.008, 0.007, 0.006, 0.002, 0.0};
        gamma=std::vector<double>(tempz, tempz + sizeof(tempz) / sizeof(tempz[0]) );
    }

    void TearDown()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    //tigl::CCPACSGuideCurve guideCurve;
    std::vector<double> alpha;
    std::vector<double> beta;
    std::vector<double> gamma;
};


/******************************************************************************/

/**
* Tests if data was read in correctly
*/
/*
TEST_F(WingGuideCurve, tiglWingGuideCurveProfile_readData)
{
    double x;
    double y;
    double z;
    for (int i=0; i!=10; i++) {
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.1*i, 0.0, &x, &y, &z) == TIGL_SUCCESS);
        ASSERT_EQ(x, 0.0);
        ASSERT_EQ(z, 0.0);
        ASSERT_NEAR(y, 5*0.1*i, 1E-10);
    }
}
*/

/**
* Tests CCPACSGuideCurveProfile class
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSGuideCurveProfile)
{
    tigl::CCPACSGuideCurveProfile guideCurve("/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[7]");
    guideCurve.ReadCPACS(tixiHandle);
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Wing_GuideCurveProfile_LeadingEdge_NonLinear");
    ASSERT_EQ(guideCurve.GetName(), "NonLinear Leading Edge Guide Curve Profile for GuideCurveModel - Wing");
    ASSERT_EQ(guideCurve.GetFileName(), "/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[7]");
}

/**
* Tests CCPACSGuideCurveProfiles class
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSGuideCurveProfiles)
{
    tigl::CCPACSGuideCurveProfiles guideCurves;
    guideCurves.ReadCPACS(tixiHandle);
    ASSERT_EQ(guideCurves.GetGuideCurveProfileCount(), 11);
    tigl::CCPACSGuideCurveProfile& guideCurve = guideCurves.GetGuideCurveProfile("GuideCurveModel_Wing_GuideCurveProfile_LeadingEdge_NonLinear");
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Wing_GuideCurveProfile_LeadingEdge_NonLinear");
    ASSERT_EQ(guideCurve.GetName(), "NonLinear Leading Edge Guide Curve Profile for GuideCurveModel - Wing");
    ASSERT_EQ(guideCurve.GetFileName(), "/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[7]");
}

/**
* Tests CCPACSGuideCurve class
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSGuideCurve)
{
    tigl::CCPACSGuideCurve guideCurve("/cpacs/vehicles/aircraft/model/wings/wing/segments/segment[1]/guideCurves/guideCurve[1]");
    guideCurve.ReadCPACS(tixiHandle);
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Wing_Seg_1_2_GuideCurve_TrailingEdgeLower");
    ASSERT_EQ(guideCurve.GetName(), "Lower Trailing Edge GuideCurve from GuideCurveModel - Wing Section 1 Main Element to GuideCurveModel - Wing Section 2 Main Element ");
    ASSERT_EQ(guideCurve.GetGuideCurveProfileUID(), "GuideCurveModel_Wing_GuideCurveProfile_TrailingEdgeLower_NonLinear");
    ASSERT_TRUE(guideCurve.GetFromRelativeCircumferenceIsSet());
    ASSERT_EQ(guideCurve.GetFromRelativeCircumference(), -1.0);
    ASSERT_EQ(guideCurve.GetToRelativeCircumference(), -1.0);
}

/**
* Tests CCPACSGuideCurves class
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSGuideCurves)
{
    tigl::CCPACSGuideCurves guideCurves;
    guideCurves.ReadCPACS(tixiHandle, "/cpacs/vehicles/aircraft/model/wings/wing/segments/segment[2]");
    ASSERT_EQ(guideCurves.GetGuideCurveCount(), 5);
    tigl::CCPACSGuideCurve& guideCurve = guideCurves.GetGuideCurve("GuideCurveModel_Wing_Seg_2_3_GuideCurve_LeadingEdge");
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Wing_Seg_2_3_GuideCurve_LeadingEdge");
    ASSERT_EQ(guideCurve.GetName(), "Leading Edge GuideCurve from GuideCurveModel - Wing Section 2 Main Element to GuideCurveModel - Wing Section 3 Main Element ");
    ASSERT_EQ(guideCurve.GetGuideCurveProfileUID(), "GuideCurveModel_Wing_GuideCurveProfile_LeadingEdge_NonLinear");
    ASSERT_TRUE(!guideCurve.GetFromRelativeCircumferenceIsSet());
    ASSERT_EQ(guideCurve.GetFromGuideCurveUID(), "GuideCurveModel_Wing_Seg_1_2_GuideCurve_LeadingEdge_NonLinear" );
    ASSERT_EQ(guideCurve.GetToRelativeCircumference(), 0.0);
}

/**
* Tests CCPACSWingProfileGetPointAlgo class
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSWingProfileGetPointAlgo)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    // get upper and lower wing profile
    tigl::CCPACSWingProfile& profile = config.GetWingProfile("GuideCurveModel_Wing_Sec3_El1_Pro");
    TopoDS_Wire upperWire = profile.GetUpperWire();
    TopoDS_Wire lowerWire = profile.GetLowerWire();

    // concatenate wires
    TopTools_SequenceOfShape wireContainer;
    wireContainer.Append(lowerWire);
    wireContainer.Append(upperWire);

    // instantiate getPointAlgo
    tigl::CCPACSWingProfileGetPointAlgo getPointAlgo(wireContainer);
    gp_Pnt point;
    gp_Vec tangent;

    // plot points and tangents
    int N = 20;
    int M = 2;
    for (int i=0; i<=N+2*M; i++) {
        double da = 2.0/double(N);
        double alpha = -1.0 -M*da + da*i;
        getPointAlgo.GetPointTangent(alpha, point, tangent);
        outputXY(i, point.X(), point.Z(), "./TestData/analysis/tiglWingGuideCurve_profileSamplePoints_points.dat");
        outputXYVector(i, point.X(), point.Z(), tangent.X(), tangent.Z(), "./TestData/analysis/tiglWingGuideCurve_profileSamplePoints_tangents.dat");
        // plot points and tangents with gnuplot by:
        // echo "plot 'TestData/analysis/tiglWingGuideCurve_profileSamplePoints_tangents.dat' u 1:2:3:4 with vectors filled head lw 2, 'TestData/analysis/tiglWingGuideCurve_profileSamplePoints_points.dat' w linespoints lw 2" | gnuplot -persist
    }

    // leading edge: point must be zero and tangent must be in z-direction
    getPointAlgo.GetPointTangent(0.0, point, tangent);
    ASSERT_NEAR(point.X(), 0.0, 1E-10);
    ASSERT_NEAR(point.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point.Z(), 0.0, 1E-10);
    ASSERT_NEAR(tangent.X(), 0.0, 1E-10);
    ASSERT_NEAR(tangent.Y(), 0.0, 1E-10);

    // lower trailing edge
    getPointAlgo.GetPointTangent(-1.0, point, tangent);
    ASSERT_NEAR(point.X(), 1.0, 1E-5);
    ASSERT_NEAR(point.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point.Z(), -0.003, 1E-5);

    // upper trailing edge
    getPointAlgo.GetPointTangent(1.0, point, tangent);
    ASSERT_NEAR(point.X(), 1.0, 1E-10);
    ASSERT_NEAR(point.Y(), 0.0, 1E-10);
    ASSERT_NEAR(point.Z(), 0.00126, 1E-10);

    // check if tangent is constant for alpha > 1
    gp_Vec tangent2;
    getPointAlgo.GetPointTangent(1.0, point, tangent);
    getPointAlgo.GetPointTangent(2.0, point, tangent2);
    ASSERT_EQ(tangent.X(), tangent2.X());
    ASSERT_EQ(tangent.Y(), tangent2.Y());
    ASSERT_EQ(tangent.Z(), tangent2.Z());

    // check if tangent is constant for alpha < 1
    getPointAlgo.GetPointTangent(-1.0, point, tangent);
    getPointAlgo.GetPointTangent(-2.0, point, tangent2);
    ASSERT_EQ(tangent.X(), tangent2.X());
    ASSERT_EQ(tangent.Y(), tangent2.Y());
    ASSERT_EQ(tangent.Z(), tangent2.Z());
}

/**
* Tests CCPACSGuideCurveAlgo class
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSGuideCurveAlgo)
{

    // create two circles parallel to the x-z plane going through the y-axis at alpha=0 
    // the sense of rotation is such that the negative z-values come first
    double radius1=1.0;
    double radius2=1.0;
    double distance=1.0;
    gp_Pnt location1(radius1, 0.0,  0.0);
    gp_Ax2 circlePosition1(location1, gp::DY(), gp::DX());
    Handle(Geom_Circle) circle1 = new Geom_Circle(circlePosition1, radius1);
    gp_Pnt location2(radius2, distance, 0.0);
    gp_Ax2 circlePosition2(location2, gp::DY(), gp::DX());
    Handle(Geom_Circle) circle2 = new Geom_Circle(circlePosition2, radius2);

    // cut into lower and upper half circle
    double start=0.0;
    double mid=start+M_PI;
    double end=mid+2*M_PI;
    TopoDS_Edge innerLowerEdge = BRepBuilderAPI_MakeEdge(circle1, start, mid);
    TopoDS_Edge innerUpperEdge = BRepBuilderAPI_MakeEdge(circle1,   mid, end);
    TopoDS_Edge outerLowerEdge = BRepBuilderAPI_MakeEdge(circle2, start, mid);
    TopoDS_Edge outerUpperEdge = BRepBuilderAPI_MakeEdge(circle2,   mid, end);

    // convert to wires 
    TopoDS_Wire innerLowerWire = BRepBuilderAPI_MakeWire(innerLowerEdge);
    TopoDS_Wire innerUpperWire = BRepBuilderAPI_MakeWire(innerUpperEdge);
    TopoDS_Wire outerLowerWire = BRepBuilderAPI_MakeWire(outerLowerEdge);
    TopoDS_Wire outerUpperWire = BRepBuilderAPI_MakeWire(outerUpperEdge);

    // concatenate wires for guide curve algo
    TopTools_SequenceOfShape innerWireContainer;
    innerWireContainer.Append(innerLowerWire);
    innerWireContainer.Append(innerUpperWire);
    TopTools_SequenceOfShape outerWireContainer;
    outerWireContainer.Append(outerLowerWire);
    outerWireContainer.Append(outerUpperWire);

    // get guide curve profile
    tigl::CCPACSGuideCurveProfile guideCurveProfile("/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[7]");
    guideCurveProfile.ReadCPACS(tixiHandle);

    TopoDS_Wire guideCurveWire;
    // instantiate guideCurveAlgo
    guideCurveWire = tigl::CCPACSGuideCurveAlgo<tigl::CCPACSWingProfileGetPointAlgo> (innerWireContainer, outerWireContainer, 0.0, 0.0, 2*radius1, 2*radius2, guideCurveProfile);

    // check if guide curve runs through sample points
    // get curve
    Standard_Real u1, u2;
    BRepTools_WireExplorer guideCurveExplorer(guideCurveWire);
    Handle(Geom_Curve) curve =  BRep_Tool::Curve(guideCurveExplorer.Current(), u1, u2);
    // set predicted sample points from cpacs file
    const double temp[] = {0.0, 0.0, 0.01, 0.03, 0.09, 0.08, 0.07, 0.06, 0.02, 0.0, 0.0};
    std::vector<double> predictedSamplePointsX (temp, temp + sizeof(temp) / sizeof(temp[0]) );
    for (unsigned int i = 0; i <= 10; ++i) {
        // get intersection point of the guide curve with planes parallel to the x-z plane located at b
        double b = i/double(10);
        Handle(Geom_Plane) plane = new Geom_Plane(gp_Pnt(0.0, b*distance, 0.0), gp_Dir(0.0, 1.0, 0.0));
        GeomAPI_IntCS intersection (curve, plane);
        ASSERT_TRUE(intersection.IsDone());
        ASSERT_EQ(intersection.NbPoints(), 1);
        gp_Pnt point = intersection.Point(1);

        // scale sample points since 2nd profile is scaled by a factor 2
        predictedSamplePointsX[i]*=(2*radius1+(2*radius2-2*radius1)*b);
        // minus sign since gamma direction is negative x-direction for alpha=0
        predictedSamplePointsX[i]*=-1.0;
        // check is guide curve runs through the predicted sample points
        ASSERT_NEAR(predictedSamplePointsX[i], point.X(), 1E-14);
        ASSERT_NEAR(b*distance, point.Y(), 1E-14);
        ASSERT_NEAR(0.0, point.Z(), 1E-14);
    }
}

/**
* Tests wing segment guide curve routines
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSWingSegment)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    ASSERT_EQ(wing.GetSegmentCount(),3);
    tigl::CCPACSWingSegment& segment1 = (tigl::CCPACSWingSegment&) wing.GetSegment(1);
    tigl::CCPACSWingSegment& segment2 = (tigl::CCPACSWingSegment&) wing.GetSegment(2);
    tigl::CCPACSWingSegment& segment3 = (tigl::CCPACSWingSegment&) wing.GetSegment(3);
    TopTools_SequenceOfShape& guideCurveContainer1 = segment1.BuildGuideCurves();
    TopTools_SequenceOfShape& guideCurveContainer2 = segment2.BuildGuideCurves();
    TopTools_SequenceOfShape& guideCurveContainer3 = segment3.BuildGuideCurves();

    ASSERT_EQ(guideCurveContainer3.Length(), 3);

    // obtain leading edge guide curve 
    TopoDS_Wire guideCurveWire = TopoDS::Wire(guideCurveContainer3(1));

    // check if guide curve runs through sample points
    // get curve
    Standard_Real u1, u2;
    BRepTools_WireExplorer guideCurveExplorer(guideCurveWire);
    Handle(Geom_Curve) curve =  BRep_Tool::Curve(guideCurveExplorer.Current(), u1, u2);
    // gamma values of cpacs data points
    const double temp[] = {0.0, 0.0, 0.01, 0.03, 0.09, 0.08, 0.07, 0.06, 0.02, 0.0, 0.0};
    std::vector<double> gammaDeviation (temp, temp + sizeof(temp) / sizeof(temp[0]) );
    // number of sample points
    int N=10;
    // segement width
    double width=2.0;
    // segment position
    double position=12.0;
    // inner profile scale factor
    double innerScale=1.0;
    // outer profile scale factor
    double outerScale=0.5;
    // outer profile has a sweep angle of -30 degrees)
    double angle=-M_PI/6.0;
    for (unsigned int i = 0; i <= N; ++i) {
        // get intersection point of the guide curve with planes in direction n located at b
        // n is the y direction rotated pi/6 (30 degrees) inside the x-y plane
        double b = width*i/double(N);
        gp_Pnt planeLocation = gp_Pnt(b*sin(angle), b*cos(angle)+position, 0.0);
        Handle(Geom_Plane) plane = new Geom_Plane(planeLocation, gp_Dir(sin(angle), cos(angle), 0.0));
        GeomAPI_IntCS intersection (curve, plane);
        ASSERT_EQ(intersection.NbPoints(), 1);
        gp_Pnt point = intersection.Point(1);

        // start at segment leading edge
        gp_Vec predictedPoint(0.0, position, 0.0);
        // go along the leading edge
        predictedPoint += gp_Vec(b*sin(angle), b*cos(angle), 0.0);
        // scale sample points since outer profile's chordline smaller by a factor of 0.5
        double s=(innerScale+(outerScale-innerScale)*i/double(N));
        // go along direction perpendicular to the leading edge in the x-y plane
        predictedPoint += gp_Vec(-cos(angle)*gammaDeviation[i]*s, sin(angle)*gammaDeviation[i]*s, 0.0);

        // check is guide curve runs through the predicted sample points
        ASSERT_NEAR(predictedPoint.X(), point.X(), 1E-5);
        ASSERT_NEAR(predictedPoint.Y(), point.Y(), 1E-5);
        ASSERT_NEAR(predictedPoint.Z(), point.Z(), 1E-14);
    }
}
