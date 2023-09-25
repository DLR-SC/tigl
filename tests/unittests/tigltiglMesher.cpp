#include "test.h"
#include "TiglMesher.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <gmsh.h>
#include <vector>
#include <TopoDS_Shape.hxx>
#include <tigl.h>
#include <CCPACSConfigurationManager.h>
#include <CCPACSWing.h>
#include "CCPACSConfigurationManager.h"
#include "TopoDS_Shape.hxx"
#include "PNamedShape.h"
#include "CNamedShape.h"
#include "CTiglExporterFactory.h"
#include "CTiglExportIges.h"
#include "CGlobalExporterConfigs.h"
#include "COptionList.h"
#include "CTiglExportBrep.h"
#include "TopoDS_Edge.hxx"


TEST(tigltiglMesher, wing)
{
    tigl::TiglMesher tm {};
    // import the D150 wing and convert to the TopoDS Shape "Wingshape"!
    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;

    const char* filename = "TestData/D150_v30.xml";
    ReturnCode tixiRet;
    TiglReturnCode tiglRet;

    tixiRet = tixiOpenDocument(filename, &tixiHandle);
    tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);
    tigl::CTiglUIDManager& uIDManager = config.GetUIDManager();

    auto& component = uIDManager.GetGeometricComponent("D150_wing_1ID");
    auto componentLoft = component.GetLoft();
    auto WingShape = componentLoft->Shape();

    // gmsh shuold already be intilizied so the cache is cleared
    gmsh::clear();

    tm.import(WingShape);
    tm.set_dimension(2);
    tm.set_minMeshSize1(0.3);
    tm.set_minMeshSize2(0.2);
    tm.set_minMeshSizeSurface(0.1);
    tm.set_maxMeshSize(2);
    tm.set_minDistance(1);
    tm.set_maxDistance(2);
    TopoDS_Edge Edge;
    tm.refineEdge1({1,7,11});
    tm.refineEdge2({3,8,12});
    tm.refineSurface({7});
    tm.mesh();

    std::vector<std::size_t>  nodeTags;
    std::vector<double>  coord;
    std::vector<double>  parametricCoord;
    gmsh::model::mesh::getNodes(nodeTags, coord, parametricCoord);

    ASSERT_EQ(nodeTags.size(), 3810);


}
