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

    // we don't compare build or pre-release
    EXPECT_TRUE(Version("1.2.3+1") == Version("1.2.3+2"));
    EXPECT_TRUE(Version("1.2.3-beta") == Version("1.2.3-alpha"));
}

TEST(Version, InvalidFormat)
{
    EXPECT_THROW(Version("1"), std::invalid_argument);
}

