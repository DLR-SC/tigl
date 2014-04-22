/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-01-24 Martin Siggel <Martin.Siggel@dlr.de>

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

TEST(WingProfileBugs, getPoint1)
{
    TixiDocumentHandle tixiHandle;
    ASSERT_EQ(SUCCESS, tixiOpenDocument("TestData/WingProfileBug1.xml", &tixiHandle));

    TiglCPACSConfigurationHandle tiglHandle;
    ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWingProfile& profile = config.GetWingProfile("SampleProfile");

    gp_Pnt p = profile.GetUpperPoint(0.00012311467739434256);
    ASSERT_NEAR(0.00012311467739434256, p.X(), 1e-5);
    ASSERT_LE(p.Z(), 1.0);
    ASSERT_NEAR(0.0, p.Y(), 1e-7);
}

TEST(WingProfileBugs, Bug2)
{
    TixiDocumentHandle tixiHandle;
    ASSERT_EQ(SUCCESS, tixiOpenDocument("TestData/WingProfileBug1.xml", &tixiHandle));

    TiglCPACSConfigurationHandle tiglHandle;
    ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

    tigl::CCPACSWingProfile& profile = config.GetWingProfile("NACA653218");
    gp_Pnt p = profile.GetLowerPoint(0.97994);
    ASSERT_NEAR(0.97994, p.X(), 1e-5);
    ASSERT_LE(p.Z(), 1.0);
    ASSERT_NEAR(0.0, p.Y(), 1e-7);
}
