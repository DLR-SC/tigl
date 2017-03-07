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
#include "CTiglError.h"

namespace tigl
{
namespace external_object_private
{
    std::string getPathRelativeToApp(const std::string& cpacsPath, const std::string& linkedFilePath);
}
}

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
    tigl::CCPACSExternalObject object(NULL);
    object.ReadCPACS(tixiHandle, "/root/genericGeometryComponent[1]");
    
    ASSERT_STREQ("TestData/nacelle.stp", object.GetFilePath().c_str());
}

TEST_F(TiglExternalComponent, getShape)
{
    tigl::CCPACSExternalObject object(NULL);
    object.ReadCPACS(tixiHandle, "/root/genericGeometryComponent[1]");
    
    PNamedShape shape = object.GetLoft();
    ASSERT_TRUE(shape != NULL);
    ASSERT_STREQ("nacelle", shape->Name());
}

TEST_F(TiglExternalComponent, invalidFiletype)
{
    tigl::CCPACSExternalObject object(NULL);
    ASSERT_THROW(object.ReadCPACS(tixiHandle, "/root/genericGeometryComponent[2]"), tigl::CTiglError);
}

TEST(TiglExternalComponentInternal, getPathRelativeToApp)
{
    std::string resultPath;
    using tigl::external_object_private::getPathRelativeToApp;

    resultPath = getPathRelativeToApp("TestData/aircraft.xml", "nacelle.stp");
    ASSERT_STREQ("TestData/nacelle.stp", resultPath.c_str());

    resultPath = getPathRelativeToApp("TestData\\aircraft.xml", "nacelle.stp");
    ASSERT_STREQ("TestData/nacelle.stp", resultPath.c_str());

    // check some absolute paths
#ifdef _WIN32
    resultPath = getPathRelativeToApp("TestData/aircraft.xml", "c:/nacelle.stp");
    ASSERT_STREQ("c:/nacelle.stp", resultPath.c_str());

    resultPath = getPathRelativeToApp("TestData/aircraft.xml", "c:\\nacelle.stp");
    ASSERT_STREQ("c:\\nacelle.stp", resultPath.c_str());
#else
    resultPath = getPathRelativeToApp("TestData/aircraft.xml", "/data/nacelle.stp");
    ASSERT_STREQ("/data/nacelle.stp", resultPath.c_str());
#endif

    // check, if cpacs path not available
    resultPath = getPathRelativeToApp("", "mydata/nacelle.stp");
    ASSERT_STREQ("mydata/nacelle.stp", resultPath.c_str());

    resultPath = getPathRelativeToApp("aircraft.xml", "nacelle.stp");
    ASSERT_STREQ("nacelle.stp", resultPath.c_str());
}

