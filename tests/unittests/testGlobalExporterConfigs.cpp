/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-04-04 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CGlobalExporterConfigs.h"
#include "CTiglExportStep.h"

TEST(CGlobalExporterConfigs, CheckExporterAvailable)
{
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("iges"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("igs"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("vtp"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("brep"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("step"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("stp"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("step"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("dae"));

    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("vtk"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("collada"));

    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("STEP"));
    EXPECT_NO_THROW(tigl::CGlobalExporterConfigs::Instance().Get("IGES"));

    EXPECT_THROW(tigl::CGlobalExporterConfigs::Instance().Get("unknown"), tigl::CTiglError);
}

TEST(CGlobalExporterConfigs, CheckSameConfig)
{
    tigl::ExporterOptions& stepOptions = tigl::CGlobalExporterConfigs::Instance().Get("step");
    ASSERT_FALSE(stepOptions.Get<bool>("ApplySymmetries"));
    ASSERT_FALSE(tigl::CGlobalExporterConfigs::Instance().Get("stp").Get<bool>("ApplySymmetries"));

    stepOptions.SetApplySymmetries(true);
    ASSERT_TRUE(stepOptions.Get<bool>("ApplySymmetries"));
    ASSERT_TRUE(tigl::CGlobalExporterConfigs::Instance().Get("stp").Get<bool>("ApplySymmetries"));

    // This is a global config, hence we have to reset it!
    tigl::CGlobalExporterConfigs::Instance().Get("stp") = tigl::StepOptions();
}
