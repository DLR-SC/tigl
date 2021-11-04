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

#include "Version.h"

TEST(Version, MajorMinorPatch)
{
    const auto version = Version("1.2.3");
    EXPECT_EQ(1, version.vMajor());
    EXPECT_EQ(2, version.vMinor());
    EXPECT_EQ(3, version.vPatch());
    EXPECT_STREQ("", version.vLabel().c_str());
}

TEST(Version, MajorMinor)
{
    const auto version = Version("1.2");
    EXPECT_EQ(1, version.vMajor());
    EXPECT_EQ(2, version.vMinor());
    EXPECT_EQ(0, version.vPatch());
    EXPECT_STREQ("", version.vLabel().c_str());
}

TEST(Version, MajorMinorPatchLabel)
{
    const auto version = Version("1.2.3-beta");
    EXPECT_EQ(1, version.vMajor());
    EXPECT_EQ(2, version.vMinor());
    EXPECT_EQ(3, version.vPatch());
    EXPECT_STREQ("beta", version.vLabel().c_str());
}

TEST(Version, MajorMinorPatchBuild)
{
    const auto version = Version("1.2.3+4");
    EXPECT_EQ(1, version.vMajor());
    EXPECT_EQ(2, version.vMinor());
    EXPECT_EQ(3, version.vPatch());
    EXPECT_STREQ("", version.vLabel().c_str());
    EXPECT_STREQ("4", version.vBuild().c_str());
}

TEST(Version, MajorMinorPatchLabelBuild)
{
    const auto version = Version("1.2.3-alpha+4");
    EXPECT_EQ(1, version.vMajor());
    EXPECT_EQ(2, version.vMinor());
    EXPECT_EQ(3, version.vPatch());
    EXPECT_STREQ("alpha", version.vLabel().c_str());
    EXPECT_STREQ("4", version.vBuild().c_str());
}

TEST(Version, MajorMinorBuild)
{
    const auto version = Version("1.2+4");
    EXPECT_EQ(1, version.vMajor());
    EXPECT_EQ(2, version.vMinor());
    EXPECT_EQ(0, version.vPatch());
    EXPECT_STREQ("", version.vLabel().c_str());
    EXPECT_STREQ("4", version.vBuild().c_str());
}

TEST(Version, MajorMinorPatchLabelBuild_Complicated)
{
    const auto version = Version("1.2.3----RC-SNAPSHOT.12.9.1--.12+788");
    EXPECT_EQ(1, version.vMajor());
    EXPECT_EQ(2, version.vMinor());
    EXPECT_EQ(3, version.vPatch());
    EXPECT_STREQ("---RC-SNAPSHOT.12.9.1--.12", version.vLabel().c_str());
    EXPECT_STREQ("788", version.vBuild().c_str());
}

TEST(Version, Compare)
{
    EXPECT_TRUE(Version("1.2.3") < Version("2.2.3"));
    EXPECT_TRUE(Version("2.2.3") > Version("1.2.3"));
    EXPECT_TRUE(Version("1.2.3") == Version("1.2.3"));

    // compare minor level
    EXPECT_TRUE(Version("1.2.3") < Version("1.3.0"));

    // compare patch level
    EXPECT_TRUE(Version("1.2.3") < Version("1.2.4"));

    EXPECT_TRUE(Version("1.2") < Version("1.2.3"));

    // pre-release is always smaller than release version
    EXPECT_TRUE(Version("1.2.3-alpha") < Version("1.2.3"));

    // we don't compare build or pre-release
    EXPECT_TRUE(Version("1.2.3+1") == Version("1.2.3+2"));
    EXPECT_TRUE(Version("1.2.3-beta") == Version("1.2.3-alpha"));

}

/**
 * Tests valid semver version strings
 *
 * From: https://regex101.com/r/vkijKf/1/
 *
 * Modified to allow omitting patch level
 */
TEST(Version, ValidFormat)
{
    EXPECT_NO_THROW(Version("0.0.4"));
    EXPECT_NO_THROW(Version("1.2.3"));
    EXPECT_NO_THROW(Version("10.20.30"));
    EXPECT_NO_THROW(Version("1.1.2-prerelease+meta"));
    EXPECT_NO_THROW(Version("1.1.2+meta"));
    EXPECT_NO_THROW(Version("1.1.2+meta-valid"));
    EXPECT_NO_THROW(Version("1.0.0-alpha"));
    EXPECT_NO_THROW(Version("1.0.0-beta"));
    EXPECT_NO_THROW(Version("1.0.0-alpha.beta"));
    EXPECT_NO_THROW(Version("1.0.0-alpha.beta.1"));
    EXPECT_NO_THROW(Version("1.0.0-alpha.1"));
    EXPECT_NO_THROW(Version("1.0.0-alpha0.valid"));
    EXPECT_NO_THROW(Version("1.0.0-alpha.0valid"));
    EXPECT_NO_THROW(Version("1.0.0-alpha-a.b-c-somethinglong+build.1-aef.1-its-okay"));
    EXPECT_NO_THROW(Version("1.0.0-rc.1+build.1"));
    EXPECT_NO_THROW(Version("2.0.0-rc.1+build.123"));
    EXPECT_NO_THROW(Version("1.2.3-beta"));
    EXPECT_NO_THROW(Version("10.2.3-DEV-SNAPSHOT"));
    EXPECT_NO_THROW(Version("1.2.3-SNAPSHOT-123"));
    EXPECT_NO_THROW(Version("1.0.0"));
    EXPECT_NO_THROW(Version("2.0.0"));
    EXPECT_NO_THROW(Version("1.1.7"));
    EXPECT_NO_THROW(Version("2.0.0+build.1848"));
    EXPECT_NO_THROW(Version("2.0.1-alpha.1227"));
    EXPECT_NO_THROW(Version("1.0.0-alpha+beta"));
    EXPECT_NO_THROW(Version("1.2.3----RC-SNAPSHOT.12.9.1--.12+788"));
    EXPECT_NO_THROW(Version("1.2.3----R-S.12.9.1--.12+meta"));
    EXPECT_NO_THROW(Version("1.2.3----RC-SNAPSHOT.12.9.1--.12"));
    EXPECT_NO_THROW(Version("1.0.0+0.build.1-rc.10000aaa-kk-0.1"));
    EXPECT_NO_THROW(Version("99999999999999999999999.999999999999999999.99999999999999999"));
    EXPECT_NO_THROW(Version("1.0.0-0A.is.legal"));

    // These are modifications from the actual semver 2.0.0 definition that allow omitting the patch version
    EXPECT_NO_THROW(Version("1.2"));
    EXPECT_NO_THROW(Version("1.2-SNAPSHOT"));
    EXPECT_NO_THROW(Version("1.2-RC-SNAPSHOT"));
}


/**
 * Tests invalid semver version strings
 *
 * From: https://regex101.com/r/vkijKf/1/
 *
 * Modified to allow omitting patch level
 */
TEST(Version, InvalidFormat)
{
    EXPECT_THROW(Version("1"), std::invalid_argument);
    EXPECT_THROW(Version("1.2.3-0123"), std::invalid_argument);
    EXPECT_THROW(Version("1.2.3-0123.0123"), std::invalid_argument);
    EXPECT_THROW(Version("1.1.2+.123"), std::invalid_argument);
    EXPECT_THROW(Version("+invalid"), std::invalid_argument);
    EXPECT_THROW(Version("-invalid"), std::invalid_argument);
    EXPECT_THROW(Version("-invalid+invalid"), std::invalid_argument);
    EXPECT_THROW(Version("-invalid.01"), std::invalid_argument);
    EXPECT_THROW(Version("alpha"), std::invalid_argument);
    EXPECT_THROW(Version("alpha.beta"), std::invalid_argument);
    EXPECT_THROW(Version("alpha.beta.1"), std::invalid_argument);
    EXPECT_THROW(Version("alpha.1"), std::invalid_argument);
    EXPECT_THROW(Version("alpha+beta"), std::invalid_argument);
    EXPECT_THROW(Version("alpha_beta"), std::invalid_argument);
    EXPECT_THROW(Version("alpha."), std::invalid_argument);
    EXPECT_THROW(Version("alpha.."), std::invalid_argument);
    EXPECT_THROW(Version("beta"), std::invalid_argument);
    EXPECT_THROW(Version("1.0.0-alpha_beta"), std::invalid_argument);
    EXPECT_THROW(Version("-alpha."), std::invalid_argument);
    EXPECT_THROW(Version("1.0.0-alpha.."), std::invalid_argument);
    EXPECT_THROW(Version("1.0.0-alpha..1"), std::invalid_argument);
    EXPECT_THROW(Version("1.0.0-alpha...1"), std::invalid_argument);
    EXPECT_THROW(Version("1.0.0-alpha....1"), std::invalid_argument);
    EXPECT_THROW(Version("1.0.0-alpha.....1"), std::invalid_argument);
    EXPECT_THROW(Version("1.0.0-alpha......1"), std::invalid_argument);
    EXPECT_THROW(Version("1.0.0-alpha.......1"), std::invalid_argument);
    EXPECT_THROW(Version("01.1.1"), std::invalid_argument);
    EXPECT_THROW(Version("1.01.1"), std::invalid_argument);
    EXPECT_THROW(Version("1.1.01"), std::invalid_argument);
    EXPECT_THROW(Version("1.2.3.DEV"), std::invalid_argument);
    EXPECT_THROW(Version("1.2.31.2.3----RC-SNAPSHOT.12.09.1--..12+788"), std::invalid_argument);
    EXPECT_THROW(Version("-1.0.3-gamma+b7718"), std::invalid_argument);
    EXPECT_THROW(Version("+justmeta"), std::invalid_argument);
    EXPECT_THROW(Version("9.8.7+meta+meta"), std::invalid_argument);
    EXPECT_THROW(Version("9.8.7-whatever+meta+meta"), std::invalid_argument);
    EXPECT_THROW(Version("99999999999999999999999.999999999999999999.99999999999999999----RC-SNAPSHOT.12.09.1--------------------------------..12"), std::invalid_argument);
}

