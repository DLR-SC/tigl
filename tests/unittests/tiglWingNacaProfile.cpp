/* 
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-14 Hannah Gedler <hannah.gedler@dlr.de>
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
* @brief Tests for NACA wing profiles
*/

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

/******************************************************************************/

class WingNACAProfile : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        const char* filename = "TestData/naca_test.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // read configuration
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
        Standard_Real u1, u2;

        // get profile curves of 1st airfoil
        tigl::CCPACSWingProfile & profile = config.GetWingProfile("NACA0012");
        TopoDS_Edge upperWire = profile.GetUpperWire();
        upperCurve = BRep_Tool::Curve(upperWire, u1, u2);

        // set CST parameters for 1st airfoil (corresponding to the data file)
        const double temp1[] = {0, 0.005, 0.010, 0.015, 0.020, 0.025, 0.030, 0.035, 0.040, 0.045, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0};
        psi=std::vector<double>(temp1, temp1 + sizeof(temp1) / sizeof(temp1[0]) );
        upperN1=0.5;
        upperN2=1.0;
        const double temp2[] = {0.2, 0.5, 0.2, 0.1};
        upperB=std::vector<double> (temp2, temp2 + sizeof(temp2) / sizeof(temp2[0]) );

    }

    void TearDown() override 
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
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
TEST_F(WingNACAProfile, tiglWingCSTProfile_samplePoints)
{
    // project sample points on curve and check distance
    for (unsigned int i = 0; i < psi.size(); ++i) {
        gp_Pnt samplePoint(Standard_Real(psi[i]), Standard_Real(0.0), Standard_Real(tigl::cstcurve(upperN1, upperN2, upperB, 0., psi[i])));
        GeomAPI_ProjectPointOnCurve projection(samplePoint, upperCurve);
        gp_Pnt projectedPoint=projection.NearestPoint();
        outputXY(i, samplePoint.X(), samplePoint.Z(), "./TestData/analysis/tiglWingCSTProfile_samplePoints_cst.dat");
        outputXY(i, projectedPoint.X(), projectedPoint.Z(), "./TestData/analysis/tiglWingCSTProfile_samplePoints_bspline.dat");
        // the approximation is no longer exact at the sample points (no interpolation anymore)
        ASSERT_NEAR(0., samplePoint.Distance(projectedPoint), 1e-4);
    }  
}