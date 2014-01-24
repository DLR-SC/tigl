/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-01-13 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief Tests for CST wing profiles
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include "math/tiglmathfunctions.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWingProfile.h"
#include "BRep_Tool.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "Geom_Curve.hxx"
#include "gp_Pnt.hxx"
#include "gp_Pnt.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"

// for testing
#include <fstream>
#include <iomanip>

/******************************************************************************/

class WingCSTProfile : public ::testing::Test 
{
protected:
    virtual void SetUp() 
    {
        const char* filename = "TestData/CPACS_21_CST_simple.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "CSTExampleModel", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // read configuration
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
        Standard_Real u1, u2;

        // get profile curves of 1st airfoil
        tigl::CCPACSWingProfile & profile = config.GetWingProfile("CSTExampleAirfoil1");
        TopoDS_Wire upperWire = profile.GetUpperWire();
        BRepTools_WireExplorer upperExplorer(upperWire);
        upperCurve = BRep_Tool::Curve(upperExplorer.Current(), u1, u2);

        // set CST parameters for 1st airfoil (corresponding to the data file)
        const double temp1[] = {0, 0.005, 0.010, 0.015, 0.020, 0.025, 0.030, 0.035, 0.040, 0.045, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0};
        psi=std::vector<double>(temp1, temp1 + sizeof(temp1) / sizeof(temp1[0]) );
        upperN1=0.5;
        upperN2=1.0;
        const double temp2[] = {0.2, 0.5, 0.2, 0.1};
        upperB=std::vector<double> (temp2, temp2 + sizeof(temp2) / sizeof(temp2[0]) );

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
    Handle(Geom_Curve) upperCurve;
    std::vector<double> psi;
    double upperN1;
    double upperN2;
    std::vector<double> upperB;
};


/******************************************************************************/

/**
* Tests if CST B-spline curve intersects the sample points
*/
TEST_F(WingCSTProfile, tiglWingCSTProfile_samplePoints)
{
    // project sample points on curve and check distance
    for (unsigned int i = 0; i < psi.size(); ++i) {
        gp_Pnt samplePoint(Standard_Real(psi[i]), Standard_Real(0.0), Standard_Real(tigl::cstcurve(upperN1, upperN2, upperB, psi[i])));
        GeomAPI_ProjectPointOnCurve projection(samplePoint, upperCurve);
        gp_Pnt projectedPoint=projection.NearestPoint();
        outputXY(i, samplePoint.X(), samplePoint.Z(), "./TestData/analysis/tiglWingCSTProfile_samplePoints_cst.dat");
        outputXY(i, projectedPoint.X(), projectedPoint.Z(), "./TestData/analysis/tiglWingCSTProfile_samplePoints_bspline.dat");
        ASSERT_NEAR(0., samplePoint.Distance(projectedPoint), 1e-10);
    }  
}

/**
* Tests if CST B-spline curve is 'near' the real CST curve
*/
TEST_F(WingCSTProfile, tiglWingCSTProfile_approximation)
{
    // maximal deviation
    double devmax=0.0;
    // project sample points on curve and calculate distance
    std::vector<double> x;
    for (int i = 0; i < 100; ++i) {
        x.push_back(double(i)/1000.0);
    }
    for (int i = 100; i <= 1000; i=i+10) {
        x.push_back(double(i)/1000.0);
    }
    for (unsigned int i = 0; i < x.size(); ++i) {
        gp_Pnt samplePoint(Standard_Real(x[i]), Standard_Real(0.0), Standard_Real(tigl::cstcurve(upperN1, upperN2, upperB, x[i])));
        GeomAPI_ProjectPointOnCurve projection(samplePoint, upperCurve);
        gp_Pnt projectedPoint=projection.NearestPoint();
        double deviation=samplePoint.Distance(projectedPoint);
        if (deviation>=devmax)
        {
            devmax=deviation;
        }
        outputXY(i, samplePoint.X(), samplePoint.Z(), "./TestData/analysis/tiglWingCSTProfile_approximation_cst.dat");
        outputXY(i, projectedPoint.X(), projectedPoint.Z(), "./TestData/analysis/tiglWingCSTProfile_approximation_bspline.dat");
        outputXY(i, x[i], deviation, "./TestData/analysis/tiglWingCSTProfile_approximation_deviation.dat");
    }
    ASSERT_NEAR(0.0, devmax, 5E-5);
}
/**
* Tests success of VTK export for wing with CST profiles 
*/
TEST_F(WingCSTProfile, tiglWingCSTProfile_VTK_export)
{
    const char* vtkWingFilename = "TestData/export/CPACS_21_CST_simple_wing1.vtp";
    ASSERT_TRUE(tiglExportMeshedWingVTKSimpleByUID(tiglHandle, "CSTExample_W1", vtkWingFilename, 0.01) == TIGL_SUCCESS);
}
