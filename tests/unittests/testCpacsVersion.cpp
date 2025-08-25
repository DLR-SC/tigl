/*
* Copyright (C) 2021 German Aerospace Center (DLR/SC)
*
* Created: 2021-11-02 Martin Siggel <Martin.Siggel@dlr.de>
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
#include "testUtils.h"

#include "tigl.h"


TEST(CPACSVersion, success1)
{
    TixiHandleWrapper tixihandle("TestData/testversion_good.xml");

    TiglCPACSConfigurationHandle tiglHandle = -1;
    EXPECT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixihandle, "", &tiglHandle));

    tiglCloseCPACSConfiguration(tiglHandle);
}

TEST(CPACSVersion, success2)
{
    TixiHandleWrapper tixihandle("TestData/simpletest.cpacs.xml");

    TiglCPACSConfigurationHandle tiglHandle = -1;
    EXPECT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixihandle, "", &tiglHandle));

    tiglCloseCPACSConfiguration(tiglHandle);
}

TEST(CPACSVersion, versionTooOld)
{
    TixiHandleWrapper tixihandle("TestData/testversion_too_old.xml");

    TiglCPACSConfigurationHandle tiglHandle = -1;
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, tiglOpenCPACSConfiguration(tixihandle, "", &tiglHandle));

    tiglCloseCPACSConfiguration(tiglHandle);
}

TEST(CPACSVersion, noCPACSVersion)
{
    TixiHandleWrapper tixihandle("TestData/testversion_no_cpacs_version.xml");

    TiglCPACSConfigurationHandle tiglHandle = -1;
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, tiglOpenCPACSConfiguration(tixihandle, "", &tiglHandle));

    tiglCloseCPACSConfiguration(tiglHandle);
}

TEST(CPACSVersion, newCPACSHeader)
{
    TixiHandleWrapper tixihandle("TestData/testversion_new_header.xml");

    TiglCPACSConfigurationHandle tiglHandle = -1;
    EXPECT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixihandle, "", &tiglHandle));

    tiglCloseCPACSConfiguration(tiglHandle);
}

TEST(CPACSVersion, deprecateOldCPACSHeader)
{
    TixiHandleWrapper tixihandle("TestData/testversion_old_header.xml");

    TiglCPACSConfigurationHandle tiglHandle = -1;

    // Check whether deprecation warning appears when cpacsVersion node is stored right at /cpacs/header/

    std::string logOutput;
    { // Scope to destroy object of type CaptureTiGLLog and therefore reset console verbosity
        CaptureTiGLLog t{TILOG_WARNING};
        EXPECT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixihandle, "", &tiglHandle));
        logOutput = t.log();
        std::string comparisonString = "The use of the cpacsVersion node right at /cpacs/header/ is deprecated since CPACS version 3.5.\n" \
                "This node has to be moved exclusively into the /cpacs/header/versionInfos[]/versionInfo node to be supported by future TiGL versions.\n" \
                "More information can be found in the CPACS documentation.";
        ASSERT_TRUE((logOutput.find(comparisonString)) != std::string::npos);
    } //scope

    tiglCloseCPACSConfiguration(tiglHandle);
}
