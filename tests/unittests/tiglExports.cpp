/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief Tests for testing export functions.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"

#include "CTiglTriangularizer.h"
#include "CTiglExportCollada.h"
#include "CTiglExportVtk.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWing.h"
#include "CTiglExporterFactory.h"
#include "CGlobalExporterConfigs.h"
#include "CTiglExportIges.h"


/******************************************************************************/

class tiglExport : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/CPACS_30_D150.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle tiglExport::tixiHandle = 0;
TiglCPACSConfigurationHandle tiglExport::tiglHandle = 0;

class tiglExportSimple : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglSimpleHandle = -1;
        tixiSimpleHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiSimpleHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiSimpleHandle, "", &tiglSimpleHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglSimpleHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiSimpleHandle) == SUCCESS);
        tiglSimpleHandle = -1;
        tixiSimpleHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}


    static TixiDocumentHandle           tixiSimpleHandle;
    static TiglCPACSConfigurationHandle tiglSimpleHandle;
};

TixiDocumentHandle tiglExportSimple::tixiSimpleHandle = 0;
TiglCPACSConfigurationHandle tiglExportSimple::tiglSimpleHandle = 0;

class tiglExportD150WGuides : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/D150_n_guides_m_profiles/D150_8_guides_8_profiles.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglD150WGuidesHandle = -1;
        tixiD150WGuidesHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiD150WGuidesHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiD150WGuidesHandle, "", &tiglD150WGuidesHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglD150WGuidesHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiD150WGuidesHandle) == SUCCESS);
        tiglD150WGuidesHandle = -1;
        tixiD150WGuidesHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}


    static TixiDocumentHandle           tixiD150WGuidesHandle;
    static TiglCPACSConfigurationHandle tiglD150WGuidesHandle;
};

TixiDocumentHandle tiglExportD150WGuides::tixiD150WGuidesHandle = 0;
TiglCPACSConfigurationHandle tiglExportD150WGuides::tiglD150WGuidesHandle = 0;


class tiglExportRectangularWing : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simple_rectangle_compseg.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglRectangularWingHandle = -1;
        tixiRectangularWingHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiRectangularWingHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiRectangularWingHandle, "", &tiglRectangularWingHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglRectangularWingHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiRectangularWingHandle) == SUCCESS);
        tiglRectangularWingHandle = -1;
        tixiRectangularWingHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}


    static TixiDocumentHandle           tixiRectangularWingHandle;
    static TiglCPACSConfigurationHandle tiglRectangularWingHandle;
};

TixiDocumentHandle tiglExportRectangularWing::tixiRectangularWingHandle = 0;
TiglCPACSConfigurationHandle tiglExportRectangularWing::tiglRectangularWingHandle = 0;



/******************************************************************************/

//void tiglxEportMeshedWingVTK_small_example()
//{
//    const BRepPrimAPI_MakeCylinder cone(/* radius */ 2.0, /* height */ 8.0);
//    const CTiglExportVtk writer(config);
//    writer.ExportMeshedWingVTK
//}

/**
* Tests tiglWingGetProfileName with invalid CPACS handle.
*/
TEST_F(tiglExport, export_meshed_wing_success)
{
    const char* vtkWingFilename = "TestData/export/D150modelID_wing1.vtp";
    ASSERT_TRUE(tiglExportMeshedWingVTKByIndex(tiglHandle, 1, vtkWingFilename, 0.01) == TIGL_SUCCESS);
}

/**
* Tests tiglWingGetProfileName with invalid CPACS handle.
*/
TEST_F(tiglExport, export_meshed_wing_simple_success)
{
    const char* vtkWingFilename = "TestData/export/D150modelID_wing1_simple.vtp";
    ASSERT_TRUE(tiglExportMeshedWingVTKSimpleByUID(tiglHandle, "D150_VAMP_W1", vtkWingFilename, 0.01) == TIGL_SUCCESS);
}

TEST_F(tiglExport, export_meshed_fuselage_success)
{
    const char* vtkFuselageFilename = "TestData/export/D150modelID_fuselage1.vtp";
    ASSERT_TRUE(tiglExportMeshedFuselageVTKByUID(tiglHandle, "D150_VAMP_FL1", vtkFuselageFilename, 0.03) == TIGL_SUCCESS);
}

TEST_F(tiglExport, export_fuselage_collada_success)
{
    const char* colladaFuselageFilename = "TestData/export/D150modelID_fuselage1.dae";
    ASSERT_TRUE(tiglExportFuselageColladaByUID(tiglHandle, "D150_VAMP_FL1", colladaFuselageFilename, 0.01) == TIGL_SUCCESS);
}

TEST_F(tiglExport, export_wing_collada_success)
{
    const char* colladaWing1Filename = "TestData/export/D150modelID_wing1.dae";
    ASSERT_TRUE(tiglExportWingColladaByUID(tiglHandle, "D150_VAMP_W1", colladaWing1Filename, 0.001) == TIGL_SUCCESS);
    const char* colladaWing2Filename = "TestData/export/D150modelID_wing2.dae";
    ASSERT_TRUE(tiglExportWingColladaByUID(tiglHandle, "D150_VAMP_HL1", colladaWing2Filename, 0.001) == TIGL_SUCCESS);
    const char* colladaWing3Filename = "TestData/export/D150modelID_wing3.dae";
    ASSERT_TRUE(tiglExportWingColladaByUID(tiglHandle, "D150_VAMP_SL1", colladaWing3Filename, 0.001) == TIGL_SUCCESS);

    ASSERT_EQ(TIGL_SUCCESS, tiglExportComponent(tiglHandle, "D150_VAMP_W1",  "TestData/export/D150modelID_wing1_new.dae", 0.001));
}

TEST_F(tiglExportSimple, export_wing_collada)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    tigl::CTiglExportCollada colladaWriter;
    colladaWriter.AddShape(wing.GetLoft(), tigl::TriangulatedExportOptions(0.001));
    bool ret = colladaWriter.Write("TestData/export/simpletest_wing.dae");

    ASSERT_EQ(true, ret);
}

TEST_F(tiglExportSimple, export_wing_vtk_newapi_simple)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    tigl::CTiglExportVtk vtkWriter;
    vtkWriter.AddShape(wing.GetLoft(), tigl::TriangulatedExportOptions(0.001));
    bool ret = vtkWriter.Write("TestData/export/simpletest_wing_simple_newapi.vtp");

    ASSERT_EQ(true, ret);
}

TEST_F(tiglExportSimple, export_wing_vtk_newapi_meta)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    tigl::CTiglExportVtk vtkWriter;
    vtkWriter.AddShape(wing.GetLoft(), &config, tigl::TriangulatedExportOptions(0.001));
    bool ret = vtkWriter.Write("TestData/export/simpletest_wing_meta_newapi.vtp");

    ASSERT_EQ(true, ret);
}

TEST_F(tiglExportSimple, export_fusedplane_vtk_newapi_meta)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);

    tigl::CTiglExportVtk vtkWriter;
    vtkWriter.AddFusedConfiguration(config, tigl::TriangulatedExportOptions(0.01));
    bool ret = vtkWriter.Write("TestData/export/simpletest_fusedplane_meta_newapi.vtp");

    ASSERT_EQ(true, ret);
}

TEST_F(tiglExportSimple, export_componentplane_vtk_newapi_meta)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);

    tigl::CTiglExportVtk vtkWriter;
    vtkWriter.AddConfiguration(config, tigl::TriangulatedExportOptions(0.01));
    bool ret = vtkWriter.Write("TestData/export/simpletest_nonfusedplane_meta_newapi.vtp");

    ASSERT_EQ(true, ret);
}

TEST_F(tiglExportSimple, export_generic_stl)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);

    tigl::CTiglExporterFactory& factory = tigl::CTiglExporterFactory::Instance();
    tigl::PTiglCADExporter stlExporter = factory.Create("stl");

    tigl::TriangulatedExportOptions options(0.01);
    stlExporter->AddConfiguration(config, options);
    bool ret = stlExporter->Write("TestData/export/simpletest_export_generic.stl");

    ASSERT_EQ(true, ret);
}

TEST_F(tiglExportSimple, export_iges_layers)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);

    tigl::PTiglCADExporter igesExporter = tigl::createExporter("iges");

    igesExporter->AddShape(config.GetWing(1).GetLoft(), tigl::IgesShapeOptions(111));
    igesExporter->AddShape(config.GetFuselage(1).GetLoft(), tigl::IgesShapeOptions(222));
    bool ret = igesExporter->Write("TestData/export/simpletest_export_igeslayer.igs");

    ASSERT_EQ(true, ret);
}

TEST_F(tiglExportSimple, export_component_api_errors)
{
    EXPECT_EQ(TIGL_NULL_POINTER, tiglExportComponent(tiglSimpleHandle, "Wing", 0, 0.001));
    EXPECT_EQ(TIGL_NULL_POINTER, tiglExportComponent(tiglSimpleHandle, 0, "TestData/export/simple_wing_new.dae", 0.001));
    
    EXPECT_EQ(TIGL_NOT_FOUND, tiglExportComponent(tiglSimpleHandle, "Wing", "TestData/export/simple_wing_new.txt", 0.001));
    EXPECT_EQ(TIGL_WRITE_FAILED, tiglExportComponent(tiglSimpleHandle, "Wing", "TestData/export/simple_wing_new", 0.001));
    EXPECT_EQ(TIGL_UID_ERROR, tiglExportComponent(tiglSimpleHandle, "NoComponentUid", "TestData/export/simple_wing_new.dae", 0.001));
}

TEST_F(tiglExportSimple, export_configuration_api_errors)
{
    EXPECT_EQ(TIGL_NULL_POINTER, tiglExportConfiguration(tiglSimpleHandle, 0, TIGL_FALSE, 0.001));
    EXPECT_EQ(TIGL_NOT_FOUND, tiglExportConfiguration(tiglSimpleHandle, "TestData/export/simpletest-export.txt", TIGL_FALSE, 0.001));
    EXPECT_EQ(TIGL_WRITE_FAILED, tiglExportConfiguration(tiglSimpleHandle, "TestData/export/simpletest-export", TIGL_FALSE, 0.001));
}

TEST_F(tiglExportSimple, set_export_options_api)
{
    EXPECT_EQ(TIGL_NOT_FOUND, tiglSetExportOptions("unknown", "ApplySymmetries", "true"));
    EXPECT_EQ(TIGL_NOT_FOUND, tiglSetExportOptions("vtk", "unknown", "true"));
    EXPECT_EQ(TIGL_ERROR, tiglSetExportOptions("vtk", "ApplySymmetries", "unknown"));

    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("vtk", "ApplySymmetries", "false"));
    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("vtk", "ApplySymmetries", "true"));
    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("vtk", "IncludeFarfield", "true"));
    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("vtk", "IncludeFarfield", "false"));
    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("vtk", "WriteNormals", "false"));
    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("vtk", "WriteNormals", "true"));
    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("vtk", "WriteNormals", "yes"));
    EXPECT_EQ(TIGL_ERROR, tiglSetExportOptions("vtk", "WriteNormals", "yyeeesss"));

    EXPECT_EQ(TIGL_NULL_POINTER, tiglSetExportOptions(0, "ApplySymmetries", "false"));
    EXPECT_EQ(TIGL_NULL_POINTER, tiglSetExportOptions("vtk", 0, "false"));
    EXPECT_EQ(TIGL_NULL_POINTER, tiglSetExportOptions("vtk", "ApplySymmetries", 0));

    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("brep", "ShapeGroupMode", "NAMED_COMPOUNDS"));
    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("brep", "ShapeGroupMode", "FACES"));
    EXPECT_EQ(TIGL_SUCCESS, tiglSetExportOptions("brep", "ShapeGroupMode", "WHOLE_SHAPE"));
    EXPECT_EQ(TIGL_ERROR, tiglSetExportOptions("brep", "ShapeGroupMode", "INVALID"));
}

TEST_F(tiglExportSimple, export_iges_symmetry)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);

    tigl::ExporterOptions options = tigl::getExportConfig("iges");
    options.SetApplySymmetries(true);
    options.SetIncludeFarfield(false);
    tigl::PTiglCADExporter igesExporter = tigl::createExporter("iges", options);

    igesExporter->AddConfiguration(config);
    bool ret = igesExporter->Write("TestData/export/simpletest_export_iges_sym.igs");

    ASSERT_EQ(true, ret);
}

// check if face names were set correctly in the case with a trailing edge
TEST_F(tiglExportSimple, check_face_traits)
{
    ASSERT_EQ(TIGL_SUCCESS, tiglExportIGES(tiglSimpleHandle,"TestData/export/simpletest.iges"));
    ASSERT_EQ(TIGL_SUCCESS, tiglExportFusedWingFuselageIGES(tiglSimpleHandle,"TestData/export/simpletest_fused.iges"));
}

TEST_F(tiglExportSimple, exportFusedBRep)
{
    ASSERT_EQ(TIGL_SUCCESS, tiglExportFusedBREP(tiglSimpleHandle,"TestData/export/simpletest.brep"));
    ASSERT_EQ(TIGL_SUCCESS, tiglExportConfiguration(tiglSimpleHandle,"TestData/export/simpletest2.brep", TIGL_TRUE, 0.));
}


// check if face names were set correctly in the case with a guide curves
TEST_F(tiglExportD150WGuides, check_face_traits)
{
    ASSERT_EQ(TIGL_SUCCESS, tiglExportIGES(tiglD150WGuidesHandle,"TestData/export/D150_guide_curves.iges"));
    ASSERT_EQ(TIGL_SUCCESS, tiglExportFusedWingFuselageIGES(tiglD150WGuidesHandle,"TestData/export/D150_fused.iges"));
}

// check if face names were set correctly in the case without a trailing edge
TEST_F(tiglExportRectangularWing, check_face_traits)
{
    ASSERT_EQ(TIGL_SUCCESS, tiglExportIGES(tiglRectangularWingHandle,"TestData/export/rectangular_wing_test.iges"));
    ASSERT_EQ(TIGL_SUCCESS, tiglExportFusedWingFuselageIGES(tiglRectangularWingHandle,"TestData/export/rectangular_wing_test_fused.iges"));
}

TEST(TiglExportFactory, supportedTypes)
{
    tigl::CTiglExporterFactory& factory = tigl::CTiglExporterFactory::Instance();

    ASSERT_TRUE(factory.ExporterSupported("step"));
    ASSERT_TRUE(factory.ExporterSupported("stp"));
    ASSERT_TRUE(factory.ExporterSupported("brep"));
    ASSERT_TRUE(factory.ExporterSupported("igs"));
    ASSERT_TRUE(factory.ExporterSupported("iges"));
    ASSERT_TRUE(factory.ExporterSupported("dae"));
    ASSERT_TRUE(factory.ExporterSupported("vtp"));
    ASSERT_TRUE(factory.ExporterSupported("stl"));

    ASSERT_FALSE(factory.ExporterSupported("unknown"));
}
