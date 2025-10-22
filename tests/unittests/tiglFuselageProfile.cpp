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
