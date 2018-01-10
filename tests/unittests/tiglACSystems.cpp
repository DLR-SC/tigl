#include "tigl.h"

#include "test.h"

TEST(TiglACSystems, openFile)
{
    const char* filename = "TestData/singleModel_withGenericSystems_cylinder.xml";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;

    ASSERT_EQ(SUCCESS, tixiOpenDocument(filename, &tixiHandle));
    ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));

    ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
    ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
}
