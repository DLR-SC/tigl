/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
*
* Created: 2020-07-20 Martin Siggel <martin.siggel@dlr.de>
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

/**
* @file
* @brief Tests for the parent / child symmetry behavior
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include "tigl_version.h"

#include <CCPACSConfigurationManager.h>
#include <CCPACSConfiguration.h>
#include <CCPACSWing.h>
#include <CCPACSEnginePylon.h>
#include <CCPACSFuselage.h>

#include <memory>
#include <string>

/******************************************************************************/

class tiglSymmetryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::string filename = "TestData/simpletest-pylon.cpacs.xml";
        phandle.reset(new TiglHandleWrapper(filename, ""));

        auto& manager = tigl::CCPACSConfigurationManager::GetInstance();
        auto& config = manager.GetConfiguration(*phandle);

        pwing = &config.GetUIDManager().ResolveObject<tigl::CCPACSWing>("Wing");
        ppylon = &config.GetUIDManager().ResolveObject<tigl::CCPACSEnginePylon>("Pylon");
        pfuselage = &config.GetUIDManager().ResolveObject<tigl::CCPACSFuselage>("SimpleFuselage");
        ASSERT_TRUE(pwing && pwing->GetUID() == "Wing");
        ASSERT_TRUE(ppylon && ppylon->GetUID() == "Pylon");
        ASSERT_TRUE(pfuselage && pfuselage->GetUID() == "SimpleFuselage");
    }

    std::unique_ptr<TiglHandleWrapper> phandle;
    tigl::CCPACSWing* pwing;
    tigl::CCPACSEnginePylon* ppylon;
    tigl::CCPACSFuselage* pfuselage;
};

TEST_F(tiglSymmetryTest, symDefault)
{
    EXPECT_EQ(TIGL_X_Z_PLANE, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_X_Z_PLANE, ppylon->GetSymmetryAxis());
    EXPECT_EQ(TIGL_NO_SYMMETRY, pfuselage->GetSymmetryAxis());
}

TEST_F(tiglSymmetryTest, symInheritWithParentSymmetry)
{
    ppylon->SetSymmetryAxis(TIGL_INHERIT_SYMMETRY);
    EXPECT_EQ(TIGL_X_Z_PLANE, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_X_Z_PLANE, ppylon->GetSymmetryAxis());
}

TEST_F(tiglSymmetryTest, symInheritWithParentSymmetry2)
{
    pfuselage->SetSymmetryAxis(TIGL_X_Y_PLANE);
    pwing->SetSymmetryAxis(TIGL_INHERIT_SYMMETRY);
    EXPECT_EQ(TIGL_X_Y_PLANE, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_X_Y_PLANE, ppylon->GetSymmetryAxis());
}

TEST_F(tiglSymmetryTest, symInheritWithoutParentSymmetry)
{
    pwing->SetSymmetryAxis(TIGL_NO_SYMMETRY);
    ppylon->SetSymmetryAxis(TIGL_INHERIT_SYMMETRY);
    EXPECT_EQ(TIGL_NO_SYMMETRY, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_NO_SYMMETRY, ppylon->GetSymmetryAxis());
}

TEST_F(tiglSymmetryTest, symNone)
{
    ppylon->SetSymmetryAxis(TIGL_NO_SYMMETRY);
    EXPECT_EQ(TIGL_X_Z_PLANE, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_NO_SYMMETRY, ppylon->GetSymmetryAxis());

    // deleting the symmetry flag should fallback to default
    // behaviour of inheriting the parent symmetry flag
    ppylon->SetSymmetry(boost::optional<TiglSymmetryAxis>());

    EXPECT_EQ(TIGL_X_Z_PLANE, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_X_Z_PLANE, ppylon->GetSymmetryAxis());
}

TEST_F(tiglSymmetryTest, symNone2)
{
    pfuselage->SetSymmetryAxis(TIGL_X_Y_PLANE);

    pwing->SetSymmetryAxis(TIGL_NO_SYMMETRY);
    EXPECT_EQ(TIGL_NO_SYMMETRY, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_NO_SYMMETRY, ppylon->GetSymmetryAxis());

    // deleting the symmetry flag should fallback to default
    // behaviour of inheriting the parent symmetry flag
    pwing->SetSymmetry(boost::optional<TiglSymmetryAxis>());

    EXPECT_EQ(TIGL_X_Y_PLANE, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_X_Y_PLANE, ppylon->GetSymmetryAxis());
}

TEST_F(tiglSymmetryTest, differentSymmetries)
{
    pwing->SetSymmetryAxis(TIGL_X_Z_PLANE);
    ppylon->SetSymmetryAxis(TIGL_X_Y_PLANE);
    EXPECT_EQ(TIGL_X_Z_PLANE, pwing->GetSymmetryAxis());
    EXPECT_EQ(TIGL_X_Y_PLANE, ppylon->GetSymmetryAxis());
}

TEST_F(tiglSymmetryTest, toString)
{
   EXPECT_STREQ("none", tigl::TiglSymmetryAxisToString(TIGL_NO_SYMMETRY).c_str());
   EXPECT_STREQ("x-y-plane", tigl::TiglSymmetryAxisToString(TIGL_X_Y_PLANE).c_str());
   EXPECT_STREQ("x-z-plane", tigl::TiglSymmetryAxisToString(TIGL_X_Z_PLANE).c_str());
   EXPECT_STREQ("y-z-plane", tigl::TiglSymmetryAxisToString(TIGL_Y_Z_PLANE).c_str());
   EXPECT_STREQ("inherit", tigl::TiglSymmetryAxisToString(TIGL_INHERIT_SYMMETRY).c_str());

   EXPECT_THROW(tigl::TiglSymmetryAxisToString(static_cast<TiglSymmetryAxis>(-1)), tigl::CTiglError);
}

TEST_F(tiglSymmetryTest, fromString)
{
    EXPECT_EQ(TIGL_NO_SYMMETRY, tigl::stringToTiglSymmetryAxis("none"));
    EXPECT_EQ(TIGL_NO_SYMMETRY, tigl::stringToTiglSymmetryAxis(""));
    EXPECT_EQ(TIGL_INHERIT_SYMMETRY, tigl::stringToTiglSymmetryAxis("inherit"));
    EXPECT_EQ(TIGL_X_Y_PLANE, tigl::stringToTiglSymmetryAxis("x-y-plane"));
    EXPECT_EQ(TIGL_X_Z_PLANE, tigl::stringToTiglSymmetryAxis("x-z-plane"));
    EXPECT_EQ(TIGL_Y_Z_PLANE, tigl::stringToTiglSymmetryAxis("y-z-plane"));
}
