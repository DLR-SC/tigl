/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#include "test.h"
#include "tigl.h"
#include "tixi.h"

#include "CCPACSConfigurationManager.h"
#include "CTiglBSplineApproxInterp.h"
#include "ITiglWireAlgorithm.h"

TEST(FuselageProfile, getHeight_getWidth)
{

    double tolerance = 0.01;
    TixiDocumentHandle tixiHandle;
    ASSERT_EQ(SUCCESS, tixiOpenDocument("TestData/multiple_fuselages.xml", &tixiHandle));

    TiglCPACSConfigurationHandle tiglHandle;
    ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSFuselageProfile& profile      = config.GetFuselageProfile("fuselageCircleProfileuID");

    double width  = profile.GetWidth();
    double height = profile.GetHeight();

    EXPECT_NEAR(width, 2, tolerance);
    EXPECT_NEAR(height, 2, tolerance);

    tigl::CCPACSFuselageProfile& profile2 = config.GetFuselageProfile("fuselageTriangleProfileuID");

    width  = profile2.GetWidth();
    height = profile2.GetHeight();

    EXPECT_NEAR(width, 1.03, tolerance);
    EXPECT_NEAR(height, 1.06, tolerance);

    tigl::CCPACSFuselageProfile& profile3 = config.GetFuselageProfile("fuselageCircleProfileuIDShifted");

    width  = profile3.GetWidth();
    height = profile3.GetHeight();

    EXPECT_NEAR(width, 2, tolerance);
    EXPECT_NEAR(height, 2, tolerance);

    tigl::CCPACSFuselageProfile& profile4 = config.GetFuselageProfile("fuselageRectangleProfile");

    width  = profile4.GetWidth();
    height = profile4.GetHeight();

    EXPECT_NEAR(width, 0.92, tolerance);
    EXPECT_NEAR(height, 2.3, tolerance);
}

TEST(FuselageProfileApproximation, ComputeApproximatedProfile)
{
    TixiHandleWrapper tixiHandle("TestData/testProfileAirfoilApproximation.xml");
    tigl::CCPACSCurvePointListXYZ curve(nullptr);

    ASSERT_NO_THROW(curve.ReadCPACS(tixiHandle, "/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]/pointList"));

    // Get points
    auto yCoords = curve.GetY().AsVector();
    auto zCoords = curve.GetZ().AsVector();

    // Get profile options
    auto paramsMap = curve.GetParamsAsMap();
    auto kinks = curve.GetKinksAsVector();
    auto& approximationSettings = curve.GetApproximationSettings();
    int nrControlPoints;

    ASSERT_NO_THROW(nrControlPoints = *(approximationSettings->GetControlPointNumber_choice1()));
    ASSERT_TRUE(nrControlPoints == 12);
    ASSERT_TRUE(yCoords.size() == zCoords.size());

    Handle(TColgp_HArray1OfPnt) hpoints = new TColgp_HArray1OfPnt(1, yCoords.size());
    tigl::ITiglWireAlgorithm::CPointContainer cpoints;
    for (int j = 0; j < yCoords.size(); j++) {
        gp_Pnt pnt(0., yCoords[j], zCoords[j]);
        hpoints->SetValue(j + 1, pnt);
    }

    // Profile contains one kink to test for more robustness
    tigl::CTiglBSplineApproxInterp approx(*hpoints, nrControlPoints, 3, true);
    for(auto idx : kinks) {
        approx.InterpolatePoint(idx, true);
    }
    auto paramsVec = tigl::computeParams(hpoints, paramsMap, 0.5);

    // Compare two different ways to compute the approximation error
    tigl::CTiglApproxResult approxResult = approx.FitCurve(paramsVec, calcPointVecErrorRMSE);
    ASSERT_NEAR(approxResult.error, 0.012737449282103385, 1e-8);
    ASSERT_EQ(approxResult.curve->NbPoles(), 15);

    approxResult = approx.FitCurve(paramsVec, calcPointVecErrorMax);
    ASSERT_NEAR(approxResult.error, 0.04210473367562604, 1e-8);
    ASSERT_EQ(approxResult.curve->NbPoles(), 15);
}
