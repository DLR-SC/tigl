/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2015-01-07 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief Tests for testing duct functions.
*/

#include "test.h" // Brings in the GTest framework

#include "tigl.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"
#include "CCPACSDuct.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Pnt.hxx>

#include <TopoDS_Shape.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>

#include <TopoDS_Iterator.hxx>

/******************************************************************************/

class DuctSimple : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest_ducts.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ (tixiRet, SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "DuctColaDose", &tiglHandle);
        ASSERT_EQ(tiglRet, TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() override {

    }
    void TearDown() override {
        // reset WithDuctCutouts flags
        tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, TIGL_FALSE);
    }

    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    tigl::CTiglUIDManager& uidMgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetUIDManager();

    // ducts
    std::vector<std::unique_ptr<tigl::CCPACSDuct>>& ductsN = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetDucts()->GetDucts();

    // duct assemblies
    std::vector<std::unique_ptr<tigl::CCPACSDuctAssembly>>& ductAssembliesN = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetDucts()->GetDuctAssemblys();

    // wing
    tigl::CCPACSWing& wing = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetWing(1);

    // fuselages
    tigl::CCPACSFuselage& simpleFuselage = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetFuselage(1);
    tigl::CCPACSFuselage& simpleFuselage2 = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetFuselage(2);

    // expected values
    std::vector<int> expectedNumberOfDuctsInAssembly{6,1,3,3,3,3,3,3,3,3};
    std::vector<int> expectedNumberOfFusedSolidsInAssembly{2,1,3,3,3,3,3,3,3,3};

    // duct samples
    tigl::CCPACSDuct const* ductSimpleDuct = &uidMgr.ResolveObject<tigl::CCPACSDuct>("SimpleDuct");
    tigl::CCPACSDuct const* ductSimpleDuct2 = &uidMgr.ResolveObject<tigl::CCPACSDuct>("SimpleDuct2");
    tigl::CCPACSDuct const* ductYDuct1 = &uidMgr.ResolveObject<tigl::CCPACSDuct>("YDuct1");

    // duct assembly samples
    tigl::CCPACSDuctAssembly const* ductAssembly1 = &uidMgr.ResolveObject<tigl::CCPACSDuctAssembly>("DuctAssembly1");
    tigl::CCPACSDuctAssembly const* ductAssembly2 = &uidMgr.ResolveObject<tigl::CCPACSDuctAssembly>("DuctAssembly2");

};

TixiDocumentHandle DuctSimple::tixiHandle = 0;
TiglCPACSConfigurationHandle DuctSimple::tiglHandle = 0;

TEST_F(DuctSimple, DuctLevel)
{
    // Check number of ducts
    EXPECT_EQ(ductsN.size(), 31);

    // Check if ducts are solids
    for(std::unique_ptr<tigl::CCPACSDuct>& d : ductsN)
    {
        auto loftD = d->GetLoft();
        const TopoDS_Shape& shapeD = loftD->Shape();
        EXPECT_TRUE(shapeD.ShapeType() == TopAbs_SOLID);
    }

    // Check the position of a sample duct with help of its bounding box
    auto loftSimpleDuct = ductSimpleDuct->GetLoft();
    const TopoDS_Shape& shapeSimpleDuct = loftSimpleDuct->Shape();

    Bnd_Box ductBBox;
    BRepBndLib::AddOptimal(shapeSimpleDuct, ductBBox);

    // X-position of sample duct in coordinate system
    const double precisionX = 1.E-5 * (ductBBox.CornerMax().X() - ductBBox.CornerMin().X());

    ASSERT_NEAR(ductBBox.CornerMax().X(), 1.5, precisionX );
    ASSERT_NEAR(ductBBox.CornerMin().X(), -0.5, precisionX );

    // Y-position of sample duct in coordinate system
    const double precisionY = 1.E-5 * (ductBBox.CornerMax().Y() - ductBBox.CornerMin().Y());

    ASSERT_NEAR(ductBBox.CornerMax().Y(), 0.2, precisionY );
    ASSERT_NEAR(ductBBox.CornerMin().Y(), -0.2, precisionY );

    // Z-position of sample duct in coordinate system
    const double precisionZ = 1.E-5 * (ductBBox.CornerMax().Z() - ductBBox.CornerMin().Z());

    ASSERT_NEAR(ductBBox.CornerMax().Z(), 0.1, precisionZ );
    ASSERT_NEAR(ductBBox.CornerMin().Z(), -0.1, precisionZ );

    // Check number of faces of sample duct
    EXPECT_EQ(GetNumberOfFaces(shapeSimpleDuct), 4);

    // Check number of edges of sample duct
    EXPECT_EQ(GetNumberOfEdges(shapeSimpleDuct),10);

    // Check number of segments of sample duct
    EXPECT_EQ(ductSimpleDuct->GetSegments().GetSegmentCount(), 2);

    // Check number of sections of sample duct
    EXPECT_EQ(ductSimpleDuct->GetSections().GetSectionCount(), 3);

    // Check number of positionings in sample duct
    // ASSERT_TRUE(ductSimpleDuct->GetPositionings());
    EXPECT_EQ(ductSimpleDuct->GetPositionings()->GetPositionings().size(), 2);

}

TEST_F(DuctSimple, DuctAssemblyLevel)
{
    // Check number of duct assemblies
    EXPECT_EQ(ductAssembliesN.size(), 10);

    // Check number of ducts in each assembly
    if(ductAssembliesN.size() != expectedNumberOfDuctsInAssembly.size())
    {
        EXPECT_TRUE(false);
    }

    else
    {
        for(int i = 1; i < ductAssembliesN.size(); i++)
        {
            EXPECT_EQ(ductAssembliesN[i]->GetDuctUIDs().GetUIDs().size(), expectedNumberOfDuctsInAssembly[i]);
        }
    }

    // Check if duct assemblies are solids or compounds of solids
    for(std::unique_ptr<tigl::CCPACSDuctAssembly>& dA : ductAssembliesN)
    {
        auto loftDA = dA->GetLoft();
        const TopoDS_Shape& shapeDA = loftDA->Shape();

        if(shapeDA.ShapeType() == TopAbs_COMPOUND)
        {
            for (TopoDS_Iterator anIter(shapeDA); anIter.More(); anIter.Next())
            {
                TopoDS_Shape shapeIT = anIter.Value();
                EXPECT_TRUE(shapeIT.ShapeType() == TopAbs_SOLID);
            }  
        }

        else if(shapeDA.ShapeType() == TopAbs_SOLID)
            continue;

        else
             EXPECT_TRUE(false);
    }

    // Check number of solids in each duct assembly after fusing
    for(int i = 1; i < ductAssembliesN.size(); i++)
    {
        std::unique_ptr<tigl::CCPACSDuctAssembly>& dA = ductAssembliesN[i];
        auto loftDA = dA->GetLoft();
        const TopoDS_Shape& shapeDA = loftDA->Shape();

        if(shapeDA.ShapeType() == TopAbs_COMPOUND)
        {
           TopTools_IndexedMapOfShape solids;
           TopExp::MapShapes (shapeDA, TopAbs_SOLID, solids);
           EXPECT_EQ(solids.Extent(), expectedNumberOfFusedSolidsInAssembly[i]);
        }

        else if(shapeDA.ShapeType() == TopAbs_SOLID)
            EXPECT_EQ(expectedNumberOfFusedSolidsInAssembly[i], 1);

        else
             EXPECT_TRUE(false);
    }

    // Check the position of a sample duct assembly with help of its bounding box
    auto loftDuctAssembly1 = ductAssembly1->GetLoft();
    const TopoDS_Shape& shapeDuctAssembly1 = loftDuctAssembly1->Shape();

    Bnd_Box ductBBox;
    BRepBndLib::AddOptimal(shapeDuctAssembly1, ductBBox);

    // X-position of sample duct assembly in coordinate system
    const double precisionX = 1.E-5 * (ductBBox.CornerMax().X() - ductBBox.CornerMin().X());

    ASSERT_NEAR(ductBBox.CornerMax().X(), 1.65, precisionX );
    ASSERT_NEAR(ductBBox.CornerMin().X(), -1.25, precisionX );

    // Y-position of sample duct assembly in coordinate system
    const double precisionY = 1.E-5 * (ductBBox.CornerMax().Y() - ductBBox.CornerMin().Y());

    ASSERT_NEAR(ductBBox.CornerMax().Y(), 1.1, precisionY );
    ASSERT_NEAR(ductBBox.CornerMin().Y(), -1.0, precisionY );

    // Z-position of sample duct assembly in coordinate system
    const double precisionZ = 1.E-5 * (ductBBox.CornerMax().Z() - ductBBox.CornerMin().Z());

    ASSERT_NEAR(ductBBox.CornerMax().Z(), 1.5, precisionZ );
    ASSERT_NEAR(ductBBox.CornerMin().Z(), -0.625, precisionZ );

    // Check duct symmetry
    auto loftYDuctAssembly2 = ductAssembly2->GetLoft();
     const TopoDS_Shape& shapeDuctAssembly2 = loftYDuctAssembly2->Shape();

     for (TopoDS_Iterator anIter(shapeDuctAssembly2); anIter.More(); anIter.Next())
     {
         TopoDS_Shape shapeIT = anIter.Value();

         {
             EXPECT_TRUE(IsPointInsideShape(shapeIT, gp_Pnt(5.0, -0.4, 0.0)));
         }
     }
}

// DuctCutOutLevel

TEST_F(DuctSimple, WithDuctCutoutsFalse)
{
    // Check if flag for duct cutouts is set to false by default
    auto loftWing = wing.GetLoft();
    const TopoDS_Shape& shapeWing = loftWing->Shape();

    auto loftSimpleFuselage = simpleFuselage.GetLoft();
    const TopoDS_Shape& shapeSimpleFuselage = loftSimpleFuselage->Shape();

    auto loftSimpleFuselage2 = simpleFuselage2.GetLoft();
    const TopoDS_Shape& shapeSimpleFuselage2 = loftSimpleFuselage2->Shape();

    // std::vector <TopoDS_Shape> shapes{shapeWing, shapeSimpleFuselage /*,shapeSimpleFuselage2*/};

    // Check if flag for duct cutouts is set to false by default

    EXPECT_TRUE(IsPointInsideShape(shapeWing, gp_Pnt(0.2, 1.0, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(-0.2, 0.0, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(6.0, 0.0, 0.0)));
}

TEST_F(DuctSimple, WithDuctCutoutsTrue)
{
    // Set the flag for duct cutouts to true
    tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, TIGL_TRUE);

    auto loftWing = wing.GetLoft();
    const TopoDS_Shape& shapeWing = loftWing->Shape();

    auto loftSimpleFuselage = simpleFuselage.GetLoft();
    const TopoDS_Shape& shapeSimpleFuselage = loftSimpleFuselage->Shape();

    auto loftSimpleFuselage2 = simpleFuselage2.GetLoft();
    const TopoDS_Shape& shapeSimpleFuselage2 = loftSimpleFuselage2->Shape();

    EXPECT_FALSE(IsPointInsideShape(shapeWing, gp_Pnt(0.2, 1.0, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(-0.2, 0.0, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(6.0, 0.0, 0.0)));

    // Check if the wing symmetry is respected
    EXPECT_FALSE(IsPointInsideShape(shapeWing, gp_Pnt(0.2, -1.0, 0.0)));

    // Check if duct symmetry is respected in the cutout
    EXPECT_FALSE(IsPointInsideShape(shapeWing, gp_Pnt(5.0, -0.3, 0.0)));

    // Check the exclusion list functionality
    // Assembly 3, empty exclusion list
    EXPECT_FALSE(IsPointInsideShape(shapeWing, gp_Pnt(0.9, 1.9, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(2.4, -0.4, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(6.4, 0.4, 0.0)));

    // Assembly 4, excludes SimpleFuselage
    EXPECT_FALSE(IsPointInsideShape(shapeWing, gp_Pnt(0.9, 1.7, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(2.2, -0.4, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(6.2, 0.4, 0.0)));

    // Assembly 5, excludes SimpleFuselage2
    EXPECT_FALSE(IsPointInsideShape(shapeWing, gp_Pnt(0.9, 1.5, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(2.0, -0.4, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(6.0, 0.4, 0.0)));

    // Assembly 6, excludes Wing
    EXPECT_TRUE(IsPointInsideShape(shapeWing, gp_Pnt(0.9, 1.3, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(1.8, -0.4, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(5.8, 0.4, 0.0)));

    // Assembly 7, excludes SimpleFuselage and SimpleFuselage2
    EXPECT_FALSE(IsPointInsideShape(shapeWing, gp_Pnt(0.7, 1.9, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(2.4, 0.4, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(6.4, -0.4, 0.0)));

    // Assembly 8, excludes Wing and SimpleFuselage
    EXPECT_TRUE(IsPointInsideShape(shapeWing, gp_Pnt(0.7, 1.7, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(2.2, 0.4, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(6.2, -0.4, 0.0)));

    // Assembly 9, excludes Wing and SimpleFuselage2
    EXPECT_TRUE(IsPointInsideShape(shapeWing, gp_Pnt(0.7, 1.5, 0.0)));
    EXPECT_FALSE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(2.0, 0.4, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(6.0, -0.4, 0.0)));

    // Assembly 10, excludes all ducts
    EXPECT_TRUE(IsPointInsideShape(shapeWing, gp_Pnt(0.7, 1.3, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage, gp_Pnt(1.8, 0.4, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(shapeSimpleFuselage2, gp_Pnt(5.8, -0.4, 0.0)));

/*
    // Check if loft with duct coutouts is solid or a compound of solids
    // Check number of solids at the same time

    std::vector <TopoDS_Shape> shapes{shapeWing, shapeSimpleFuselage,shapeSimpleFuselage2};

    int numberOfSolids = 0;

    for(TopoDS_Shape& sh : shapes)
    {
        //auto shapetype = sh.ShapeType();
        //if(sh.ShapeType() == TopAbs_COMPOUND)
        {
            for (TopoDS_Iterator anIter(sh); anIter.More(); anIter.Next())
            {
                TopoDS_Shape shapeIT = anIter.Value();
                EXPECT_TRUE(shapeIT.ShapeType() == TopAbs_SOLID);

                numberOfSolids += 1;
            }
        }

        else if(sh.ShapeType() == TopAbs_SOLID)
        {
            numberOfSolids += 11;
            continue;
        }

        else
             EXPECT_TRUE(false);

    }
    EXPECT_EQ(numberOfSolids,222);
*/
}

TEST_F(DuctSimple, tiglConfigurationSetWithDuctCutouts){

    // test errors
    EXPECT_EQ(TIGL_NOT_FOUND, tiglConfigurationSetWithDuctCutouts(-1, TIGL_TRUE));

    // ducts are not considered by default
    EXPECT_FALSE(tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetDucts()->IsEnabled());

    // set the flag for all components
    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, TIGL_TRUE));
    EXPECT_TRUE(tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetDucts()->IsEnabled());
}

TEST_F(DuctSimple, tiglConfigurationGetWithDuctCutouts)
{
    TiglBoolean flag;

    // test errors
    EXPECT_EQ(TIGL_NOT_FOUND, tiglConfigurationGetWithDuctCutouts(-1, &flag));

    // ducts are not considered by default
    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationGetWithDuctCutouts(DuctSimple::tiglHandle, &flag));
    EXPECT_FALSE(tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetDucts()->IsEnabled());
    EXPECT_FALSE(flag);

    // check if the flag changes after a call to tiglConfigurationSetWithDuctCutouts
    tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, TIGL_TRUE);
    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationGetWithDuctCutouts(DuctSimple::tiglHandle, &flag));
    EXPECT_TRUE(tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetDucts()->IsEnabled());
    EXPECT_TRUE(flag);
}

TEST_F(DuctSimple, Export)
{
    tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, TIGL_TRUE);
    tiglSetExportOptions("iges", "ApplySymmetries", "true");
    tiglSetExportOptions("iges", "IncludeFarfield", "false");
    tiglSetExportOptions("iges", "FaceNames", "UIDandFaceName");
    EXPECT_EQ(TIGL_SUCCESS, tiglExportIGES(DuctSimple::tiglHandle,"TestData/export/duct_simple.iges"));
    EXPECT_EQ(TIGL_SUCCESS, tiglExportFusedWingFuselageIGES(DuctSimple::tiglHandle,"TestData/export/duct_simple_fused.iges"));
}


