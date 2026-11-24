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

#include <string>

namespace
{

const std::string deprecationWarning =
    "The use of the cpacsVersion node right at /cpacs/header/ is deprecated since CPACS version 3.5.\n"
    "This node has to be moved exclusively into the /cpacs/header/versionInfos[]/versionInfo node to be supported by future TiGL versions.\n"
    "More information can be found in the CPACS documentation.";

struct OpenResult
{
    TiglReturnCode returnCode;
    std::string log;
};

// Opens the given CPACS file and returns the TiGL return code together with the logged warnings and errors
OpenResult openWithLog(const char* filename)
{
    TixiHandleWrapper tixihandle(filename);
    TiglCPACSConfigurationHandle tiglHandle = -1;

    OpenResult result;
    { // Scope to destroy object of type CaptureTiGLLog and therefore reset console verbosity
        CaptureTiGLLog t{TILOG_WARNING};
        result.returnCode = tiglOpenCPACSConfiguration(tixihandle, "", &tiglHandle);
        result.log        = t.log();
    }

    if (result.returnCode == TIGL_SUCCESS) {
        tiglCloseCPACSConfiguration(tiglHandle);
    }
    return result;
}

bool contains(const std::string& text, const std::string& pattern)
{
    return text.find(pattern) != std::string::npos;
}

} // namespace


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
        ASSERT_TRUE(contains(logOutput, deprecationWarning));
    } //scope

    tiglCloseCPACSConfiguration(tiglHandle);
}

TEST(CPACSVersion, noCPACSVersionErrorMessage)
{
    // Neither a matching versionInfo nor the deprecated /cpacs/header/cpacsVersion node exists
    const auto result = openWithLog("TestData/testversion_no_cpacs_version.xml");
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, result.returnCode);
    EXPECT_TRUE(contains(result.log, "No CPACS version information in file header")) << result.log;
    EXPECT_FALSE(contains(result.log, deprecationWarning)) << result.log;
}

TEST(CPACSVersion, fallbackToOldCPACSHeader)
{
    // Only the deprecated /cpacs/header/cpacsVersion node exists: TiGL falls back to it and warns
    const auto result = openWithLog("TestData/testversion_old_header_only.xml");
    EXPECT_EQ(TIGL_SUCCESS, result.returnCode);
    EXPECT_TRUE(contains(result.log, deprecationWarning)) << result.log;
}

TEST(CPACSVersion, fallbackToOldCPACSHeaderTooOld)
{
    // The version read from the deprecated node is really used: a too old version is rejected
    const auto result = openWithLog("TestData/testversion_old_header_only_too_old.xml");
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, result.returnCode);
    EXPECT_TRUE(contains(result.log, "Too old CPACS dataset")) << result.log;
    EXPECT_TRUE(contains(result.log, deprecationWarning)) << result.log;
}

TEST(CPACSVersion, fallbackIfNoVersionInfoMatchesHeaderVersion)
{
    // versionInfos exist, but none belongs to the header version: TiGL ignores them and falls back
    // to the deprecated node. Its version is too old on purpose, so the fallback is observable.
    const auto result = openWithLog("TestData/testversion_no_matching_versioninfo_fallback.xml");
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, result.returnCode);
    EXPECT_TRUE(contains(result.log, "Too old CPACS dataset")) << result.log;
    EXPECT_TRUE(contains(result.log, deprecationWarning)) << result.log;
}

TEST(CPACSVersion, noVersionInfoMatchesHeaderVersionAndNoFallback)
{
    // versionInfos exist, but none belongs to the header version, and there is no deprecated node
    const auto result = openWithLog("TestData/testversion_no_matching_versioninfo.xml");
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, result.returnCode);
    EXPECT_TRUE(contains(result.log, "No CPACS version information in file header")) << result.log;
    EXPECT_FALSE(contains(result.log, deprecationWarning)) << result.log;
}

TEST(CPACSVersion, multipleOldCPACSHeaderVersions)
{
    // The deprecated node exists twice, so the version is ambiguous
    const auto result = openWithLog("TestData/testversion_old_header_multiple.xml");
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, result.returnCode);
    EXPECT_TRUE(contains(result.log, "Multiple CPACS version entries found")) << result.log;
}

TEST(CPACSVersion, invalidCPACSVersionFormat)
{
    const auto result = openWithLog("TestData/testversion_invalid_format.xml");
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, result.returnCode);
    EXPECT_TRUE(contains(result.log, "Cannot read CPACS Version")) << result.log;
}

TEST(CPACSVersion, headerVersionWithoutMinor)
{
    // The header version "1" belongs to the versionInfo "1.0.0". There is no fallback, so success proves the match.
    const auto result = openWithLog("TestData/testversion_header_version_without_minor.xml");
    EXPECT_EQ(TIGL_SUCCESS, result.returnCode);
    EXPECT_FALSE(contains(result.log, deprecationWarning)) << result.log;
}

TEST(CPACSVersion, ambiguousVersionInfos)
{
    // The versionInfos "1.0" and "1.0.0" both belong to the header version "1.0.0".
    // This is an error, which must not be hidden by the fallback to the deprecated node.
    const auto result = openWithLog("TestData/testversion_ambiguous_versioninfos.xml");
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, result.returnCode);
    EXPECT_TRUE(contains(result.log, "Multiple CPACS version entries found")) << result.log;
}

TEST(CPACSVersion, noHeaderVersion)
{
    // Invalid input: the header version is required by the CPACS schema, but TiGL does not validate against it.
    // Without a header version, no versionInfo belongs to it: TiGL falls back to the deprecated node.
    // Its version is too old on purpose, so the fallback is observable.
    const auto result = openWithLog("TestData/testversion_no_header_version.xml");
    EXPECT_EQ(TIGL_WRONG_CPACS_VERSION, result.returnCode);
    EXPECT_TRUE(contains(result.log, "Too old CPACS dataset")) << result.log;
    EXPECT_TRUE(contains(result.log, deprecationWarning)) << result.log;
}

TEST(CPACSVersion, versionInfoWithoutVersionAttribute)
{
    // Invalid input: the version attribute is required by the CPACS schema, but TiGL does not validate against it.
    // Like the generated CPACS reader, the version check skips such a versionInfo (the reader logs an error)
    // and continues with the next one. There is no fallback, so success proves that the second versionInfo was found.
    const auto result = openWithLog("TestData/testversion_versioninfo_without_version_attribute.xml");
    EXPECT_EQ(TIGL_SUCCESS, result.returnCode);
    EXPECT_TRUE(contains(result.log, "Required attribute version is missing at xpath /cpacs/header/versionInfos/versionInfo[1]")) << result.log;
    EXPECT_FALSE(contains(result.log, deprecationWarning)) << result.log;
}
