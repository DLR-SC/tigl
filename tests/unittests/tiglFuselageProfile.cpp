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
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <Precision.hxx>

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
    tigl::CCPACSCurvePointListXYZ profile(nullptr);

    // Read approximated profile that computes the error as RMSE
    ASSERT_NO_THROW(profile.ReadCPACS(tixiHandle, "/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]/pointList"));

    // Get points
    auto yCoords = profile.GetY().AsVector();
    auto zCoords = profile.GetZ().AsVector();

    // Get profile options
    auto paramsMap = profile.GetParamsAsMap();
    auto kinks = profile.GetKinksAsVector();
    auto& approximationSettings = profile.GetApproximationSettings();

    int nrControlPoints;
    std::string errorComputationMethod;
    std::vector<double> interpolatedPointsIndices;

    // Read out options of approximationSettings node
    ASSERT_NO_THROW(nrControlPoints = *(approximationSettings->GetControlPointNumber_choice1()));
    ASSERT_NO_THROW(errorComputationMethod = *(approximationSettings->GetErrorComputationMethod()));
    ASSERT_NO_THROW(interpolatedPointsIndices = approximationSettings->GetInterpolatedPointsIndices()->AsVector());

    ASSERT_TRUE(nrControlPoints == 12);
    ASSERT_TRUE(errorComputationMethod == "RMSE");
    ASSERT_TRUE(yCoords.size() == zCoords.size());
    ASSERT_TRUE(kinks.size() == 1);
    ASSERT_TRUE(interpolatedPointsIndices.size() == 1);

    Handle(TColgp_HArray1OfPnt) hpoints = new TColgp_HArray1OfPnt(1, yCoords.size());
    tigl::ITiglWireAlgorithm::CPointContainer cpoints;
    for (int j = 0; j < yCoords.size(); j++) {
        gp_Pnt pnt(0., yCoords[j], zCoords[j]);
        hpoints->SetValue(j + 1, pnt);
    }

    // Profile contains one kink and one additional interpolation point to test for more robustness
    tigl::CTiglBSplineApproxInterp approx(*hpoints, nrControlPoints, 3, true);
    approx.InterpolatePoint(kinks[0]-1, true);
    approx.InterpolatePoint(interpolatedPointsIndices[0]-1, false);

    auto paramsVec = tigl::computeParams(hpoints, paramsMap, 0.5);

    // Compare two different ways to compute the approximation error
    tigl::CTiglApproxResult approxResult = approx.FitCurve(paramsVec, calcPointVecErrorRMSE);
    ASSERT_NEAR(approxResult.error, 0.016571963592557491, 1e-8);
    ASSERT_EQ(approxResult.curve->NbPoles(), 15);

    approxResult = approx.FitCurve(paramsVec, calcPointVecErrorMax);
    ASSERT_NEAR(approxResult.error, 0.068584746277230407, 1e-8);
    ASSERT_EQ(approxResult.curve->NbPoles(), 15);

    const gp_Pnt pntKink(0., 0.999999, 0.); // As defined as kink in CPACS configuration
    const gp_Pnt pntInterp(0., -0.853516, 0.); // As defined as interpolation point in CPACS configuration

    // Check whether wanted points are really interpolated
    // => distance between point and its orthogonal projection on the curve has to vanish
    GeomAPI_ProjectPointOnCurve projectKink(pntKink, approxResult.curve);
    ASSERT_TRUE(projectKink.LowerDistance() < Precision::Confusion());

    GeomAPI_ProjectPointOnCurve projectInterp(pntInterp, approxResult.curve);
    ASSERT_TRUE(projectInterp.LowerDistance() < Precision::Confusion());
}
