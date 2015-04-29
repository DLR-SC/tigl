/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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
#include "CCPACSExternalObject.h"

class TiglExternalComponent : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        const char* filename = "TestData/external_component.xml";
        ReturnCode tixiRet;

        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
    }
    virtual void TearDown()
    {
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tixiHandle = -1;
    }


    TixiDocumentHandle           tixiHandle;
};

TEST_F(TiglExternalComponent, getFileNameRelative)
{
    tigl::CCPACSExternalObject object;
    object.ReadCPACS(tixiHandle, "/root/externalComponent[1]");
    
    ASSERT_STREQ("TestData/nacelle.stp", object.GetFilePath().c_str());
}

TEST_F(TiglExternalComponent, getFileNameAbsolute)
{
    tigl::CCPACSExternalObject object;
    object.ReadCPACS(tixiHandle, "/root/externalComponent[2]");
    
    ASSERT_STREQ("d:/data/nacelle.stp", object.GetFilePath().c_str());
}
