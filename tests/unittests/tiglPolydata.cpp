/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2013-02-13 Martin Siggel <Martin.Siggel@dlr.de>
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
 *
 * @file tiglPolydata.cpp
 */

#include "test.h"

#include "tigl.h"
#include "tixi.h"

#include "CTiglError.h"
#include "CTiglPolyData.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWingSegment.h"
#include "CTiglTriangularizer.h"
#include "CTiglExportVtk.h"
#include "CNamedShape.h"

// #include <BRepMesh.hxx>
#include <TopoDS_CompSolid.hxx>

#include <ctime>

using namespace tigl;


class TriangularizeShape : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "Cpacs2Test", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle TriangularizeShape::tixiHandle = 0;
TiglCPACSConfigurationHandle TriangularizeShape::tiglHandle = 0;

TEST(TiglPolyData, simple)
{
    CTiglPolyData poly;

    poly.createNewObject();
    poly.createNewObject();
    poly.createNewObject();
    poly.createNewObject();

    ASSERT_EQ(5, poly.getNObjects());

    poly.createNewObject();

    ASSERT_EQ(6, poly.getNObjects());

    ASSERT_TRUE(!poly.currentObject().hasNormals());
}

TEST(TiglPolyData, cube_export_vtk_standard)
{
    CTiglPolyData poly;
    CTiglPolyObject& co = poly.currentObject();

    CTiglPoint p1(0, 0, 0);
    CTiglPoint p2(0, 1, 0);
    CTiglPoint p3(0, 1, 1);
    CTiglPoint p4(0, 0, 1);
    CTiglPoint p5(-1, 0, 0);
    CTiglPoint p6(-1, 1, 0);
    CTiglPoint p7(-1, 1, 1);
    CTiglPoint p8(-1, 0, 1);

    //face one
    CTiglPolygon f1;
    f1.addPoint(p1);
    f1.addPoint(p2);
    f1.addPoint(p3);
    f1.addPoint(p4);
    co.addPolygon(f1);

    CTiglPolygon f2;
    f2.addPoint(p6);
    f2.addPoint(p5);
    f2.addPoint(p8);
    f2.addPoint(p7);
    co.addPolygon(f2);

    CTiglPolygon f3;
    f3.addPoint(p5);
    f3.addPoint(p1);
    f3.addPoint(p4);
    f3.addPoint(p8);
    co.addPolygon(f3);

    CTiglPolygon f4;
    f4.addPoint(p2);
    f4.addPoint(p6);
    f4.addPoint(p7);
    f4.addPoint(p3);
    co.addPolygon(f4);

    CTiglPolygon f5;
    f5.addPoint(p4);
    f5.addPoint(p3);
    f5.addPoint(p7);
    f5.addPoint(p8);
    co.addPolygon(f5);

    CTiglPolygon f6;
    f6.addPoint(p5);
    f6.addPoint(p6);
    f6.addPoint(p2);
    f6.addPoint(p1);
    co.addPolygon(f6);

    //check polygon data
    ASSERT_EQ(6, co.getNPolygons());

    unsigned int index1, index2, index3, index4;
    ASSERT_EQ(4, co.getNPointsOfPolygon(0));

    index1 = co.getVertexIndexOfPolygon(0,0);
    index2 = co.getVertexIndexOfPolygon(1,0);
    index3 = co.getVertexIndexOfPolygon(2,0);
    index4 = co.getVertexIndexOfPolygon(3,0);
    ASSERT_NEAR(0., co.getVertexPoint(index1).distance2(p1), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index2).distance2(p2), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index3).distance2(p3), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index4).distance2(p4), 1e-10);

    ASSERT_EQ(4, co.getNPointsOfPolygon(1));
    index1 = co.getVertexIndexOfPolygon(0,1);
    index2 = co.getVertexIndexOfPolygon(1,1);
    index3 = co.getVertexIndexOfPolygon(2,1);
    index4 = co.getVertexIndexOfPolygon(3,1);
    ASSERT_NEAR(0., co.getVertexPoint(index1).distance2(p6), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index2).distance2(p5), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index3).distance2(p8), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index4).distance2(p7), 1e-10);

    ASSERT_EQ(4, co.getNPointsOfPolygon(2));
    index1 = co.getVertexIndexOfPolygon(0,2);
    index2 = co.getVertexIndexOfPolygon(1,2);
    index3 = co.getVertexIndexOfPolygon(2,2);
    index4 = co.getVertexIndexOfPolygon(3,2);
    ASSERT_NEAR(0., co.getVertexPoint(index1).distance2(p5), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index2).distance2(p1), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index3).distance2(p4), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index4).distance2(p8), 1e-10);

    ASSERT_EQ(4, co.getNPointsOfPolygon(3));
    index1 = co.getVertexIndexOfPolygon(0,3);
    index2 = co.getVertexIndexOfPolygon(1,3);
    index3 = co.getVertexIndexOfPolygon(2,3);
    index4 = co.getVertexIndexOfPolygon(3,3);
    ASSERT_NEAR(0., co.getVertexPoint(index1).distance2(p2), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index2).distance2(p6), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index3).distance2(p7), 1e-10);
    ASSERT_NEAR(0., co.getVertexPoint(index4).distance2(p3), 1e-10);
    
    co.setPolyDataReal(0,"value",0);
    co.setPolyDataReal(1,"value",1);
    co.setPolyDataReal(2,"value",2);
    co.setPolyDataReal(3,"value",3);
    co.setPolyDataReal(4,"value",4);
    co.setPolyDataReal(5,"value",5);
    
    CTiglExportVtk::WritePolys(poly, "vtk_cube_standard.vtp");
    
    ASSERT_EQ(0, co.getPolyDataReal(0,"value"));
    ASSERT_EQ(1, co.getPolyDataReal(1,"value"));
    ASSERT_EQ(2, co.getPolyDataReal(2,"value"));
    ASSERT_EQ(3, co.getPolyDataReal(3,"value"));
    ASSERT_EQ(4, co.getPolyDataReal(4,"value"));
    ASSERT_EQ(5, co.getPolyDataReal(5,"value"));
    
}

TEST(TiglPolyData, cube_export_vtk_withnormals)
{
    CTiglPolyData poly;
    CTiglPolyObject& co =  poly.currentObject();
    CTiglPoint p1(0, 0, 0);
    CTiglPoint p2(0, 1, 0);
    CTiglPoint p3(0, 1, 1);
    CTiglPoint p4(0, 0, 1);
    CTiglPoint p5(-1, 0, 0);
    CTiglPoint p6(-1, 1, 0);
    CTiglPoint p7(-1, 1, 1);
    CTiglPoint p8(-1, 0, 1);

    //face one
    co.enableNormals(true);
    CTiglPolygon f1;
    f1.addPoint(p1);
    f1.addPoint(p2);
    f1.addPoint(p3);
    f1.addPoint(p4);

    for (int i = 0; i < 4; ++i) {
        f1.addNormal(CTiglPoint(1, 0, 0));
    }

    co.addPolygon(f1);

    CTiglPolygon f2;
    f2.addPoint(p6);
    f2.addPoint(p5);
    f2.addPoint(p8);
    f2.addPoint(p7);

    for (int i = 0; i < 4; ++i) {
        f2.addNormal(CTiglPoint(-1, 0, 0));
    }

    co.addPolygon(f2);

    CTiglPolygon f3;
    f3.addPoint(p5);
    f3.addPoint(p1);
    f3.addPoint(p4);
    f3.addPoint(p8);

    for (int i = 0; i < 4; ++i) {
        f3.addNormal(CTiglPoint(0, -1, 0));
    }

    co.addPolygon(f3);

    CTiglPolygon f4;
    f4.addPoint(p2);
    f4.addPoint(p6);
    f4.addPoint(p7);
    f4.addPoint(p3);

    for (int i = 0; i < 4; ++i) {
        f4.addNormal(CTiglPoint(0, 1, 0));
    }

    co.addPolygon(f4);

    CTiglPolygon f5;
    f5.addPoint(p4);
    f5.addPoint(p3);
    f5.addPoint(p7);
    f5.addPoint(p8);

    for (int i = 0; i < 4; ++i) {
        f5.addNormal(CTiglPoint(0, 0, 1));
    }

    co.addPolygon(f5);

    CTiglPolygon f6;
    f6.addPoint(p5);
    f6.addPoint(p6);
    f6.addPoint(p2);
    f6.addPoint(p1);

    for (int i = 0; i < 4; ++i)
        f6.addNormal(CTiglPoint(0, 0, -1));

    co.addPolygon(f6);

    CTiglExportVtk::WritePolys(poly, "vtk_cube+normals.vtp");
}

TEST(TiglPolyData, cube_export_vtk_withpieces)
{
    CTiglPolyData poly;
    CTiglPolyObject * co = &poly.currentObject();
    CTiglPoint p1(0, 0, 0);
    CTiglPoint p2(0, 1, 0);
    CTiglPoint p3(0, 1, 1);
    CTiglPoint p4(0, 0, 1);
    CTiglPoint p5(-1, 0, 0);
    CTiglPoint p6(-1, 1, 0);
    CTiglPoint p7(-1, 1, 1);
    CTiglPoint p8(-1, 0, 1);
    
    CTiglPolygon f1;
    f1.addPoint(p1);
    f1.addPoint(p2);
    f1.addPoint(p3);
    f1.addPoint(p4);
    co->addPolygon(f1);

    co = &poly.createNewObject();

    CTiglPolygon f2;
    f2.addPoint(p6);
    f2.addPoint(p5);
    f2.addPoint(p8);
    f2.addPoint(p7);
    co->addPolygon(f2);

    co = &poly.createNewObject();
    CTiglPolygon f3;
    f3.addPoint(p5);
    f3.addPoint(p1);
    f3.addPoint(p4);
    f3.addPoint(p8);
    co->addPolygon(f3);

    co = &poly.createNewObject();
    CTiglPolygon f4;
    f4.addPoint(p2);
    f4.addPoint(p6);
    f4.addPoint(p7);
    f4.addPoint(p3);
    co->addPolygon(f4);

    co = &poly.createNewObject();
    CTiglPolygon f5;
    f5.addPoint(p4);
    f5.addPoint(p3);
    f5.addPoint(p7);
    f5.addPoint(p8);
    co->addPolygon(f5);

    co = &poly.createNewObject();
    CTiglPolygon f6;
    f6.addPoint(p5);
    f6.addPoint(p6);
    f6.addPoint(p2);
    f6.addPoint(p1);
    co->addPolygon(f6);

    CTiglExportVtk::WritePolys(poly, "vtk_cube+pieces.vtp");
}

TEST_F(TriangularizeShape, exportVTK_FusedWing)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    bool exportError = false;

    tigl::CTiglTriangularizer t(wing.GetLoft(), 0.001);
    try {
        CTiglExportVtk::WritePolys(t.getTriangulation(), "exported_fused_wing_simple.vtp");
    }
    catch (...) {
        exportError = true;
    }

    ASSERT_TRUE(exportError == false);
}

TEST_F(TriangularizeShape, exportVTK_CompoundWing)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    TopoDS_CompSolid compound;
    BRep_Builder builder;
    builder.MakeCompSolid(compound);

    for (int i = 1; i <= wing.GetSegmentCount(); ++i) {
        builder.Add(compound, wing.GetSegment(i).GetLoft()->Shape());
    }

    bool exportError = false;

    clock_t start, stop;
    start = clock();
    PNamedShape compundNS(new CNamedShape(compound, "WingCompound"));
    tigl::CTiglTriangularizer t(compundNS, 0.001);
    stop = clock();
    std::cout << "Triangularization time [ms]: " << (stop-start)/(double)CLOCKS_PER_SEC * 1000. << std::endl;
    std::cout << "Number of Polygons/Vertices: " << t.getTriangulation().currentObject().getNPolygons() << "/"
              << t.getTriangulation().currentObject().getNVertices()<<std::endl;
    try {
        CTiglExportVtk::WritePolys(t.getTriangulation(), "exported_compund_wing_simple.vtp");
    }
    catch (...) {
        exportError = true;
    }

    ASSERT_TRUE(exportError == false);
}

TEST_F(TriangularizeShape, exportVTK_WingSegmentInfo)
{
    const char* vtkWingFilename = "TestData/export/simplewing_segmentinfo.vtp";

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    clock_t start, stop;
    start = clock();
    tigl::CTiglTriangularizer mesher(&config.GetUIDManager(), wing.GetLoft(), 0.0001, SEGMENT_INFO);
    const tigl::CTiglPolyData& polys = mesher.getTriangulation();

    stop = clock();
    std::cout << "Triangularization time [ms]: " << (stop-start)/(double)CLOCKS_PER_SEC * 1000. << std::endl;
    std::cout << "Number of Polygons/Vertices: " << polys.currentObject().getNPolygons() << "/" << polys.currentObject().getNVertices()<<std::endl;
    ASSERT_NO_THROW(CTiglExportVtk::WritePolys(polys, vtkWingFilename));
}

TEST_F(TriangularizeShape, exportVTK_FullPlane_long)
{
    const char* vtkWingFilename = "TestData/export/simplewing_fusedplane.vtp";

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    PTiglFusePlane algo = config.AircraftFusingAlgo();
    algo->SetResultMode(FULL_PLANE);
    PNamedShape shape = algo->FusedPlane();

    tigl::CTiglTriangularizer mesher(&config.GetUIDManager(), shape, 0.001, SEGMENT_INFO);
    const tigl::CTiglPolyData& polys = mesher.getTriangulation();

    std::cout << "Number of Polygons/Vertices: " << polys.currentObject().getNPolygons() << "/" << polys.currentObject().getNVertices()<<std::endl;
    ASSERT_NO_THROW(CTiglExportVtk::WritePolys(polys, vtkWingFilename));
}
