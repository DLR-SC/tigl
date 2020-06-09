#include "tigl.h"
#include "test.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSGenericSystem.h"
#include "CNamedShape.h"
#include "BRepTools.hxx"

class GenericSystems: public ::testing::TestWithParam<std::pair<std::string, std::vector<std::string>>>
{
protected:
    virtual void SetUp() override
    {
        std::string path_cpacs = "TestData/" + GetParam().first + ".xml";

        ASSERT_EQ(SUCCESS, tixiOpenDocument(path_cpacs.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));

        system_uids = GetParam().second;
        path_output_prefix = "TestData/" + GetParam().first;
    }

    void TearDown() override
    {
        ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
        ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
    }


    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    std::string path_output_prefix;
    std::vector<std::string> system_uids;
};

TEST_P(GenericSystems, numberOfSystems)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
    ASSERT_EQ(system_uids.size(), config.GetGenericSystemCount());

}

TEST_P(GenericSystems, uids)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
    int nSystems = config.GetGenericSystemCount();
    for (auto i=1; i < nSystems+1; ++i) {
        ASSERT_EQ(system_uids[i-1], config.GetGenericSystem(i).GetUID());
    }

}

TEST_P(GenericSystems, lofts)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
    for (auto const& uid: system_uids) {
        tigl::CCPACSGenericSystem& system = config.GetGenericSystem(uid);
        PNamedShape namedShape = system.GetLoft();
        TopoDS_Shape shape = namedShape->Shape();

        std::string filename = path_output_prefix + "_" + uid + ".brep";
        BRepTools::Write(shape, filename.c_str());
    }

}

INSTANTIATE_TEST_CASE_P(TiglACSystems, GenericSystems, ::testing::Values(
                            std::make_pair("singleModel_withGenericSystems", std::vector<std::string>{"mySystemUID",
                                                                                                      "mySystemUID2",
                                                                                                      "mySystemUID3",
                                                                                                      "mySystemUID4"}),
                            std::make_pair("singleModel_withGenericSystems_cone", std::vector<std::string>{"mySystemUID"}),
                            std::make_pair("singleModel_withGenericSystems_cube", std::vector<std::string>{"mySystemUID3"}),
                            std::make_pair("singleModel_withGenericSystems_cylinder", std::vector<std::string>{"mySystemUID2"}),
                            std::make_pair("singleModel_withGenericSystems_sphere", std::vector<std::string>{"mySystemUID4"})
                            ));
