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
#include "CCPACSConfigurationManager.h"
#include "BRep_Tool.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "Geom_Curve.hxx"
#include "gp_Pnt.hxx"
#include "gp_Pnt.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CCPACSGuideCurves.h"

/******************************************************************************/

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

        // read configuration
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
        Standard_Real u1, u2;

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

    // save x-y data
    void outputXY(const int & i, const double& x, const double&y, const std::string& filename)
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
TEST_F(WingGuideCurve, tiglWingGuideCurve_readData)
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

/**
* Tests CCPACSGuideCurve class
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSGuideCurve)
{
    tigl::CCPACSGuideCurve guideCurve("/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[7]");
    guideCurve.ReadCPACS(tixiHandle);
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Wing_GuideCurveProfile_LeadingEdge_NonLinear");
    ASSERT_EQ(guideCurve.GetName(), "NonLinear Leading Edge Guide Curve Profile for GuideCurveModel - Wing");
    ASSERT_EQ(guideCurve.GetFileName(), "/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[7]");
}
/**
* Tests CCPACSGuideCurves class
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_CCPACSGuideCurves)
{
    tigl::CCPACSGuideCurves guideCurves;
    guideCurves.ReadCPACS(tixiHandle);
    ASSERT_EQ(guideCurves.GetGuideCurveCount(), 9);
    tigl::CCPACSGuideCurve& guideCurve = guideCurves.GetGuideCurve("GuideCurveModel_Wing_GuideCurveProfile_LeadingEdge_NonLinear");
    ASSERT_EQ(guideCurve.GetUID(), "GuideCurveModel_Wing_GuideCurveProfile_LeadingEdge_NonLinear");
    ASSERT_EQ(guideCurve.GetName(), "NonLinear Leading Edge Guide Curve Profile for GuideCurveModel - Wing");
    ASSERT_EQ(guideCurve.GetFileName(), "/cpacs/vehicles/profiles/guideCurveProfiles/guideCurveProfile[7]");
}
/**
* Tests if B-spline guide curve intersects the sample points
*/
TEST_F(WingGuideCurve, tiglWingGuideCurve_samplePoints)
{
    /*
    Standard_Real u1, u2;
    TopoDS_Wire guideCurveWire = guideCurve.GetWire();
    BRepTools_WireExplorer guideCurveExplorer(guideCurveWire);
    Handle(Geom_Curve) curve =  BRep_Tool::Curve(guideCurveExplorer.Current(), u1, u2);
    // project sample points on curve and check distance
    for (unsigned int i = 0; i < beta.size(); ++i) {
        gp_Pnt samplePoint(Standard_Real(0.0), Standard_Real(beta[i]), Standard_Real(gamma[i]));
        GeomAPI_ProjectPointOnCurve projection(samplePoint, curve);
        gp_Pnt projectedPoint=projection.NearestPoint();
        outputXY(i, samplePoint.X(), samplePoint.Z(), "./TestData/analysis/tiglGuideCurve_samplePoints_cst.dat");
        outputXY(i, projectedPoint.X(), projectedPoint.Z(), "./TestData/analysis/tiglGuideCurve_samplePoints_bspline.dat");
        ASSERT_NEAR(0., samplePoint.Distance(projectedPoint), 1e-10);
    }
    */
}
