/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-03-18 Marko Alder <marko.alder@dlr.de>
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

#include <boost/optional/optional_io.hpp>

#include "CCPACSConfigurationManager.h"
#include "CCPACSDeckComponentBase.h"

class Decks : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-decks.cpacs.xml";
        ASSERT_EQ(tixiOpenDocument(filename, &tixiHandle), SUCCESS);
        ASSERT_EQ(tiglOpenCPACSConfiguration(tixiHandle, "testAircraft", &tiglHandle), TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    const tigl::CTiglUIDManager& GetUIDManager() const
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle Decks::tixiHandle           = 0;
TiglCPACSConfigurationHandle Decks::tiglHandle = 0;


TEST_F(Decks, Basics)
{
    const auto& cp = GetUIDManager().ResolveObject<tigl::CCPACSDeckComponentBase>("ceilingPanel1");
    const PNamedShape shape = cp.GetLoft();
    ASSERT_TRUE(shape);
}