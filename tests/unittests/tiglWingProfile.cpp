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

#include "CCPACSCurvePointListXYZ.h"
#include "CCPACSCurveParamPointMap.h"

TEST(WingProfileBugs, getPoint1)
{
    TiglHandleWrapper tiglHandle("TestData/WingProfileBug1.xml", "");

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
    TiglHandleWrapper tiglHandle("TestData/WingProfileBug1.xml", "");

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

    tigl::CCPACSWingProfile& profile = config.GetWingProfile("NACA653218");
    gp_Pnt p = profile.GetLowerPoint(0.97994);
    ASSERT_NEAR(0.97994, p.X(), 1e-5);
    ASSERT_LE(p.Z(), 1.0);
    ASSERT_NEAR(0.0, p.Y(), 1e-7);
}

TEST(WingProfileParams, GetKinksVector)
{
    TixiHandleWrapper tixiHandle("TestData/wingprofilewithparams.xml");
    tigl::CCPACSCurvePointListXYZ curve(nullptr);

    EXPECT_TRUE(curve.GetKinksAsVector().empty());

    ASSERT_NO_THROW(curve.ReadCPACS(tixiHandle, "/cpacs/vehicles/profiles/wingAirfoils/wingAirfoil[1]/pointList"));

    EXPECT_FALSE(curve.GetKinksAsVector().empty());
    EXPECT_TRUE(ArraysMatch({-0.2}, curve.GetKinksAsVector()));
}

TEST(WingProfileParams, GetParamMap)
{
    TixiHandleWrapper tixiHandle("TestData/wingprofilewithparams.xml");
    tigl::CCPACSCurvePointListXYZ curve(nullptr);

    ASSERT_FALSE (curve.GetParameterMap().is_initialized());
    ASSERT_NO_THROW(curve.ReadCPACS(tixiHandle, "/cpacs/vehicles/profiles/wingAirfoils/wingAirfoil[1]/pointList"));

    ASSERT_TRUE(curve.GetParameterMap().is_initialized());

    EXPECT_TRUE(ArraysMatch({30}, curve.GetParameterMap()->GetPointIndexAsVector()));
    EXPECT_TRUE(ArraysMatch({-0.2}, curve.GetParameterMap()->GetParamAsVector()));
}
