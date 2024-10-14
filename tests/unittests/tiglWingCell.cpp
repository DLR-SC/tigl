 /* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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


#include <CTiglUIDManager.h>
#include <CCPACSWingCell.h>
#include "CCPACSConfigurationManager.h"
#include "TopoDS_Edge.hxx"
#include "TopoDS.hxx"
#include "TopExp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"

TEST(WingCell, IsInner)
{
    tigl::CTiglUIDManager dummy_manager;
    tigl::CCPACSWingCell cell(NULL, &dummy_manager);
    cell.SetLeadingEdgeInnerPoint (0,0);
    cell.SetLeadingEdgeOuterPoint (1,0);
    cell.SetTrailingEdgeInnerPoint(0,1);
    cell.SetTrailingEdgeOuterPoint(1,1);
    
    // test inner lying points
    ASSERT_TRUE(cell.IsInside(0.5, 0.5));
    ASSERT_TRUE(cell.IsInside(0.2, 0.0));
    ASSERT_TRUE(cell.IsInside(0.1, 0.9));
    
    // test points on border
    ASSERT_TRUE(cell.IsInside(0.5, 0.0));
    ASSERT_TRUE(cell.IsInside(1.0, 0.5));
    ASSERT_TRUE(cell.IsInside(0.5, 1.0));
    ASSERT_TRUE(cell.IsInside(0.0, 0.5));
    
    // test points on corners
    ASSERT_TRUE(cell.IsInside(0.0, 0.0));
    ASSERT_TRUE(cell.IsInside(1.0, 0.0));
    ASSERT_TRUE(cell.IsInside(1.0, 1.0));
    ASSERT_TRUE(cell.IsInside(0.0, 1.0));
    
    ASSERT_FALSE(cell.IsInside(0.3, -0.4));
    ASSERT_FALSE(cell.IsInside(1.3, -0.2));
    ASSERT_FALSE(cell.IsInside(1.2, 0.5));
    ASSERT_FALSE(cell.IsInside(1.1, 1.3));
    ASSERT_FALSE(cell.IsInside(0.5, 1.6));
    ASSERT_FALSE(cell.IsInside(-0.2, 1.2));
    ASSERT_FALSE(cell.IsInside(-0.1, 0.3));
    ASSERT_FALSE(cell.IsInside(-0.2, -0.5));
}

TEST(WingCell, IsInner_NonConvex)
{
    tigl::CTiglUIDManager dummy_manager;
    tigl::CCPACSWingCell cell(NULL, &dummy_manager);
    cell.SetLeadingEdgeInnerPoint (0,0);
    cell.SetLeadingEdgeOuterPoint (1,0);
    cell.SetTrailingEdgeInnerPoint(0,1);
    cell.SetTrailingEdgeOuterPoint(0.3, 0.3);
    
    ASSERT_TRUE(cell.IsInside(0.3, 0.2));
    ASSERT_TRUE(cell.IsInside(0.2, 0.3));
    
    ASSERT_FALSE(cell.IsInside(0.4, 0.3));
    ASSERT_FALSE(cell.IsInside(0.4, 0.4));
    ASSERT_FALSE(cell.IsInside(0.3, 0.4));
    ASSERT_FALSE(cell.IsInside(1.0, 1.0));
}

TEST(WingCell, IsConvex)
{
    tigl::CTiglUIDManager dummy_manager;
    tigl::CCPACSWingCell cell(NULL, &dummy_manager);
    cell.SetLeadingEdgeInnerPoint (0,0);
    cell.SetLeadingEdgeOuterPoint (1,0);
    cell.SetTrailingEdgeInnerPoint(0,1);
    cell.SetTrailingEdgeOuterPoint(1,1);
    
    ASSERT_TRUE(cell.IsConvex());
    
    cell.SetTrailingEdgeOuterPoint(0.3, 0.3);
    ASSERT_FALSE(cell.IsConvex());
    
    cell.SetTrailingEdgeOuterPoint(0.499, 0.4999);
    ASSERT_FALSE(cell.IsConvex());
    
    cell.SetTrailingEdgeOuterPoint(0.5001, 0.5001);
    ASSERT_TRUE(cell.IsConvex());
    
    cell.SetTrailingEdgeOuterPoint(0.5, 0.5);
    ASSERT_TRUE(cell.IsConvex());
}

TEST(WingCell, IssueCellsNoOverlap)
{
    std::string fileName = "TestData/IEA-22-280-RWT_DLR_loads_CPACS.xml";
    std::string configName = "aircraft";
    std::string cell1Name = "span02_circ02";
    std::string cell2Name = "span03_circ02";

    ReturnCode tixiRet;
    TiglReturnCode tiglRet;

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;

    tixiRet = tixiOpenDocument(fileName.c_str(), &tixiHandle);
    ASSERT_TRUE(tixiRet == SUCCESS);

    tiglRet = tiglOpenCPACSConfiguration(tixiHandle, configName.c_str(), &tiglHandle);
    ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

    auto& uid_mgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();

    auto& cell1 = uid_mgr.ResolveObject<tigl::CCPACSWingCell>(cell1Name.c_str());
    TopoDS_Shape cellLoft1 = cell1.GetLoft()->Shape();

    auto& cell2 = uid_mgr.ResolveObject<tigl::CCPACSWingCell>(cell2Name.c_str());

    TopoDS_Shape cellLoft2 = cell2.GetLoft()->Shape();
    TopoDS_Shape cellCut = CutShapes(cellLoft1, cellLoft2);
    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes (cellCut, TopAbs_EDGE, faces);

    // Test whether the cut of the two cells is only one line and not an area
    // Therefore count the edges, must be equal to one
    ASSERT_EQ(1, faces.Extent());
}
