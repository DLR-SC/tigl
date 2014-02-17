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
#include "BRepTools_WireExplorer.hxx"
#include "Geom_Curve.hxx"
#include "gp_Pnt.hxx"
#include "gp_Pnt.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CCPACSGuideCurveProfile.h"
#include "CCPACSGuideCurveProfiles.h"
#include "CCPACSFuselageProfileGetPointAlgo.h"

/******************************************************************************/

class FuselageGuideCurve : public ::testing::Test
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

        // read configuration
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
        Standard_Real u1, u2;

        // get guide curve
        //tigl::CCPACSGuideCurve & guideCurve = config.GetGuideCurve("GuideCurveModel_Fuselage_Sec1_El1_Pro");

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

    // save x-y data
    void outputXY(const int& i, const double& x, const double& y, const std::string& filename)
    {
        ofstream out;
        if (i>0) {
            out.open(filename.c_str(), ios::app);
        }
        else {
            out.open(filename.c_str());
        }
        out << setprecision(17) << std::scientific  << x << "\t" << y << endl;
        out.close();
    }
    void outputXYVector(const int& i, const double& x, const double& y, const double& vx, const double& vy, const std::string& filename)
    {
        ofstream out;
        if (i>0) {
            out.open(filename.c_str(), ios::app);
        }
        else {
            out.open(filename.c_str());
        }
        out << setprecision(17) << std::scientific  << x << "\t" << y << "\t" << vx << "\t" << vy << "\t" << endl;
        out.close();
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
* Tests CCPACSGuideCurveProfile class
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSGuideCurveProfile)
{
    tigl::CCPACSGuideCurveProfile guideCurve("/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[2]");
    guideCurve.ReadCPACS(tixiHandle);
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Fuselage_GuideCurveProfile_Middle_NonLinear");
    ASSERT_EQ(guideCurve.GetName(), "NonLinear Middle Guide Curve Profile for GuideCurveModel - Fuselage");
    ASSERT_EQ(guideCurve.GetFileName(), "/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[2]");
}
/**
* Tests CCPACSGuideCurveProfiles class
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSGuideCurveProfiles)
{
    tigl::CCPACSGuideCurveProfiles guideCurves;
    guideCurves.ReadCPACS(tixiHandle);
    ASSERT_EQ(guideCurves.GetGuideCurveProfileCount(), 9);
    tigl::CCPACSGuideCurveProfile& guideCurve = guideCurves.GetGuideCurveProfile("GuideCurveModel_Fuselage_GuideCurveProfile_Middle_NonLinear");
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Fuselage_GuideCurveProfile_Middle_NonLinear");
    ASSERT_EQ(guideCurve.GetName(), "NonLinear Middle Guide Curve Profile for GuideCurveModel - Fuselage");
    ASSERT_EQ(guideCurve.GetFileName(), "/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[2]");
}
/**
* * Tests CCPACSFuselageProfileGetPointAlgo class
*/
TEST_F(FuselageGuideCurve, tiglFuselageGuideCurve_CCPACSFuselageProfileGetPointAlgo)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    // get upper and lower fuselage profile
    tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile("GuideCurveModel_Fuselage_Sec3_El1_Pro");
    TopoDS_Wire wire = profile.GetWire();

    // instantiate getPointAlgo
    tigl::CCPACSFuselageProfileGetPointAlgo getPointAlgo(wire);
    gp_Pnt point;
    gp_Vec tangent;

    // plot points and tangents
    for (int i=0; i<=50; i++) {
        double alpha = i/double(50);
        getPointAlgo.GetPointTangent(alpha, point, tangent);
        outputXY(i, point.Y(), point.Z(), "./TestData/analysis/tiglFuselageGuideCurve_profileSamplePoints_points.dat");
        outputXYVector(i, point.Y(), point.Z(), tangent.Y(), tangent.Z(), "./TestData/analysis/tiglFuselageGuideCurve_profileSamplePoints_tangents.dat");
        // plot points and tangents with gnuplot by:
        // echo "plot 'TestData/analysis/tiglFuselageGuideCurve_profileSamplePoints_tangents.dat' u 1:2:3:4 with vectors filled head lw 2, 'TestData/analysis/tiglFuselageGuideCurve_profileSamplePoints_points.dat' w lines lw 2" | gnuplot -persist
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
}

