/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-13 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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

#include "gtest/gtest.h"

#include "tigl.h"
#include "CTiglMakeLoft.h"
#include "CFuseShapes.h"
#include "PNamedShape.h"
#include "CNamedShape.h"
#include "ListPNamedShape.h"

#include <BRepAlgoAPI_Fuse.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopExp.hxx>

#include "CTiglExportBrep.h"
#include "CTiglFusePlane.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWing.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageSegment.h"
#include "CTiglExportIges.h"


/******************************************************************************/

class testcase
{
public:
    std::string name;
    std::string aircraftUID;
    unsigned int Nf;
    testcase(std::string n, std::string uid, unsigned int N)
    {
        name = n;
        aircraftUID = uid;
        Nf = N;
    }
};
class tiglFuseAircraftCPACS : public ::testing::TestWithParam<testcase>
{
protected:
    void SetUp() OVERRIDE
    {
        // get name of the test case
        name = GetParam().name;
        // get aircraft UID
        aircraftUID = GetParam().aircraftUID;
        // get number of faces
        Nf = GetParam().Nf;
        std::string filename = ("TestData/" + name + ".xml").c_str();
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename.c_str(), &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, aircraftUID.c_str(), &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);


        //*********************************************************************
        // Tests the successfull creation of the wing and fuselage surfaces
        //*********************************************************************
        TopoDS_Compound wingProfiles;
        TopoDS_Compound fuselageProfiles;
        TopoDS_Compound wingGuides;
        TopoDS_Compound fuselageGuides;
        TopoDS_Shape loftFuselage;
        TopoDS_Shape loftWing;
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
        tigl::CCPACSWing& wing = config.GetWing(1);
        BRep_Builder wingProfileBuilder;
        wingProfileBuilder.MakeCompound(wingProfiles);
        for (int isegment = 1; isegment <= wing.GetSegmentCount(); ++isegment) {
            // get wingProfiles
            tigl::CCPACSWingSegment& segment = static_cast<tigl::CCPACSWingSegment&>(wing.GetSegment(isegment));
            wingProfileBuilder.Add(wingProfiles, segment.GetInnerWire());
            if (isegment == wing.GetSegmentCount()) {
                wingProfileBuilder.Add(wingProfiles, segment.GetOuterWire());
            }
        }
        wingGuides = wing.GetGuideCurveWires();

        tigl::CCPACSFuselage& fuselage = config.GetFuselage(1);
        BRep_Builder fuselageProfileBuilder;
        fuselageProfileBuilder.MakeCompound(fuselageProfiles);
        for (int isegment = 1; isegment <= fuselage.GetSegmentCount(); ++isegment) {
            // get fuselageProfiles
            tigl::CCPACSFuselageSegment& segment = static_cast<tigl::CCPACSFuselageSegment&>(fuselage.GetSegment(isegment));
            fuselageProfileBuilder.Add(fuselageProfiles, segment.GetStartWire());
            if (isegment == fuselage.GetSegmentCount()) {
                fuselageProfileBuilder.Add(fuselageProfiles, segment.GetEndWire());
            }
        }
        fuselageGuides = fuselage.GetGuideCurveWires();
        ASSERT_FALSE(wingGuides.IsNull());
        ASSERT_FALSE(wingProfiles.IsNull());
        ASSERT_FALSE(fuselageGuides.IsNull());
        ASSERT_FALSE(fuselageProfiles.IsNull());
        BRepTools::Write(wingGuides, ("TestData/export/" + name + "_wingGuides.brep").c_str());
        BRepTools::Write(wingProfiles, ("TestData/export/" + name + "_wingProfiles.brep").c_str());
        BRepTools::Write(fuselageGuides, ("TestData/export/" + name + "_fuselageGuides.brep").c_str());
        BRepTools::Write(fuselageProfiles, ("TestData/export/" + name + "_fuselageProfiles.brep").c_str());

        CTiglMakeLoft loftMakerWing;
        loftMakerWing.addProfiles(wingProfiles);
        loftMakerWing.addGuides(wingGuides);
        loftMakerWing.setMakeSolid(true);
        loftWing = loftMakerWing.Shape();
        ASSERT_FALSE(loftWing.IsNull());


        CTiglMakeLoft loftMakerFuselage;
        loftMakerFuselage.addProfiles(fuselageProfiles);
        loftMakerFuselage.addGuides(fuselageGuides);
        loftMakerFuselage.setMakeSolid(true);
        loftFuselage = loftMakerFuselage.Shape();
        ASSERT_FALSE(loftFuselage.IsNull());
        ASSERT_EQ(TopAbs_SOLID, loftFuselage.ShapeType());
        BRepTools::Write(loftFuselage, ("TestData/export/" + name + "_fuselage.brep").c_str());
    }

    void TearDown() OVERRIDE
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    // test case name
    std::string name;
    // number of faces
    unsigned int Nf;
    // CPACS aircraft UID
    std::string aircraftUID;

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};


/**
* Tests fused export
*/
TEST_P(tiglFuseAircraftCPACS, exportFusedAircraftToBrep)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
    tigl::CTiglExportBrep exporter;
    exporter.AddFusedConfiguration(config);
    ASSERT_TRUE(exporter.Write("TestData/export/" + name + "_fused.brep"));
}

/**
* Tests fuse operation of wing and fuselage
*/
TEST_P(tiglFuseAircraftCPACS, fusedAircraft)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    PNamedShape wingLoft = wing.GetLoft();
    ASSERT_FALSE(wingLoft->Shape().IsNull());
    ASSERT_TRUE(BRepTools::Write(wingLoft->Shape(), ("TestData/export/" + name + "_wing.brep").c_str()));

    tigl::CTiglExportIges igesExporterWing;
    igesExporterWing.AddShape(wingLoft);
    igesExporterWing.Write(("TestData/export/" + name + "_wing.igs").c_str());

    tigl::CCPACSFuselage& fuselage = config.GetFuselage(1);
    PNamedShape fuselageLoft = fuselage.GetLoft();
    ASSERT_FALSE(fuselageLoft->Shape().IsNull());
    ASSERT_TRUE(BRepTools::Write(fuselageLoft->Shape(), ("TestData/export/" + name + "_fuselage.brep").c_str()));

    tigl::CTiglExportIges igesExporterFuselage;
    igesExporterFuselage.AddShape(fuselageLoft);
    igesExporterFuselage.Write(("TestData/export/" + name + "_fuselage.igs").c_str());

    ListPNamedShape fuseTools;
    fuseTools.push_back(fuselageLoft);
    CFuseShapes fuser(wingLoft, fuseTools);
    TopoDS_Shape result = fuser.NamedShape()->Shape();
    ASSERT_FALSE(result.IsNull());
    ASSERT_TRUE(BRepTools::Write(result, ("TestData/export/" + name + "_fusedAircraft.brep").c_str()));

    BRepCheck_Analyzer analyzer(result);
    ASSERT_TRUE(analyzer.IsValid(result));

    // check the number of faces of fused shape
    TopTools_IndexedMapOfShape faceMapResult;
    TopExp::MapShapes(result, TopAbs_FACE, faceMapResult);
    ASSERT_EQ(faceMapResult.Extent(), Nf);
}

/**
* Tests fuse operation of wing and fuselage to create mirror symmetric aircraft
*/
TEST_P(tiglFuseAircraftCPACS, fusedAircraftMirror)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
    tigl::PTiglFusePlane fuser = config.AircraftFusingAlgo();
    fuser->SetResultMode(tigl::FULL_PLANE);
    PNamedShape airplane = fuser->FusedPlane();
    ASSERT_TRUE(airplane != NULL);
    ASSERT_TRUE(BRepTools::Write(airplane->Shape(), ("TestData/export/" + name + "_fusedAircraftMirrored.brep").c_str()));
}

INSTANTIATE_TEST_CASE_P(xrf1, tiglFuseAircraftCPACS, ::testing::Values(
                        testcase("D150WithGuides", "D150modelID", 246) 
                        ));
