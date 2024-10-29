/*
* Copyright (C) 2021 German Aerospace Center
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

TEST(Bug802, wingGetPoint_onLinearLoft)
{
    TiglHandleWrapper handle("TestData/bugs/802/802.xml", "");

    double px, py, pz;
    tiglWingSetGetPointBehavior(handle, onLinearLoft);

    tiglWingGetLowerPoint(handle, 1, 2, 0, 0, &px, &py, &pz);

    EXPECT_NEAR(0.0, px, 1e-5);
    EXPECT_NEAR(1.0, py, 1e-5);
    EXPECT_NEAR(0.0, pz, 1e-2);

    tiglWingGetUpperPoint(handle, 1, 2, 0, 0, &px, &py, &pz);

    EXPECT_NEAR(0.0, px, 1e-5);
    EXPECT_NEAR(1.0, py, 1e-5);
    EXPECT_NEAR(0.0, pz, 1e-2);
}
