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

TEST(WingCell, IssueCellsNoOverlapSpanwise)
{
    std::string fileName = "TestData/IEA-22-280-RWT_DLR_loads_CPACS.xml";
    std::string configName = "aircraft";
    std::string cell1Name = "span02_circ02";
    std::string cell2Name = "span03_circ02";

    ReturnCode tixiRet;
    TiglReturnCode tiglRet;
    Standard_Real first1, last1, first2, last2;
    gp_Pnt pntCurve1, pntCurve2;

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

    // Take the outermost edge of the first cell and the innermost edge of the second cell which should be compared
    // If these edges are identical, the cut of the cells is exactely defined by this edge
    // Hence, the cells have no overlap
    // In the following, the edges are compared by comparing three points at parameters 0, 0.5 and 1
    // If the resulting points are pairwise the same (up to tolerance), the edges are the same and the cells do not overlap
    TopTools_IndexedMapOfShape edges1, edges2;
    TopExp::MapShapes (cellLoft1, TopAbs_EDGE, edges1);
    TopExp::MapShapes (cellLoft2, TopAbs_EDGE, edges2);

    TopoDS_Edge edge1 = TopoDS::Edge(edges1(8)); // Since the cell is defined on two segments, it consists of two faces (-> 8 edges)
    TopoDS_Edge edge2 = TopoDS::Edge(edges2(2));
    Handle(Geom_BSplineCurve) curve1 = Handle(Geom_BSplineCurve)::DownCast(BRep_Tool::Curve(edge1, first1, last1));
    Handle(Geom_BSplineCurve) curve2 = Handle(Geom_BSplineCurve)::DownCast(BRep_Tool::Curve(edge2, first2, last2));

    curve1->D0(0., pntCurve1);
    curve2->D0(0., pntCurve2);
    ASSERT_NEAR(pntCurve1.X(), pntCurve2.X(), 1e-3);
    ASSERT_NEAR(pntCurve1.Y(), pntCurve2.Y(), 1e-3);
    ASSERT_NEAR(pntCurve1.Z(), pntCurve2.Z(), 1e-3);

    curve1->D0(0.5, pntCurve1);
    curve2->D0(0.5, pntCurve2);
    ASSERT_NEAR(pntCurve1.X(), pntCurve2.X(), 1e-3);
    ASSERT_NEAR(pntCurve1.Y(), pntCurve2.Y(), 1e-3);
    ASSERT_NEAR(pntCurve1.Z(), pntCurve2.Z(), 1e-3);

    curve1->D0(1., pntCurve1);
    curve2->D0(1., pntCurve2);
    ASSERT_NEAR(pntCurve1.X(), pntCurve2.X(), 1e-3);
    ASSERT_NEAR(pntCurve1.Y(), pntCurve2.Y(), 1e-3);
    ASSERT_NEAR(pntCurve1.Z(), pntCurve2.Z(), 1e-3);
}

TEST(WingCell, IssueCellsNoOverlapChordwise)
{
    std::string fileName = "TestData/IEA-22-280-RWT_DLR_loads_CPACS.xml";
    std::string configName = "aircraft";
    std::string cell1Name = "span03_circ01";
    std::string cell2Name = "span03_circ02";

    ReturnCode tixiRet;
    TiglReturnCode tiglRet;
    Standard_Real first1, last1, first2, last2;
    gp_Pnt pntCurve1, pntCurve2;

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

    // Take the first cell's edge adjacent to cell 2 and the second cell's edge adjacent to cell 1 (-> Should be shared edge)
    // If these edges are identical, the cut of the cells is exactely defined by this edge
    // Hence, the cells have no overlap
    // In the following, the edges are compared by comparing three points at parameters 0, 0.5 and 1
    // If the resulting points are pairwise the same (up to tolerance), the edges are the same and the cells do not overlap
    TopTools_IndexedMapOfShape edges1, edges2;
    TopExp::MapShapes (cellLoft1, TopAbs_EDGE, edges1);
    TopExp::MapShapes (cellLoft2, TopAbs_EDGE, edges2);

    TopoDS_Edge edge1 = TopoDS::Edge(edges1(1));
    TopoDS_Edge edge2 = TopoDS::Edge(edges2(3));
    Handle(Geom_BSplineCurve) curve1 = Handle(Geom_BSplineCurve)::DownCast(BRep_Tool::Curve(edge1, first1, last1));
    Handle(Geom_BSplineCurve) curve2 = Handle(Geom_BSplineCurve)::DownCast(BRep_Tool::Curve(edge2, first2, last2));

    curve1->D0(0., pntCurve1);
    curve2->D0(0., pntCurve2);
    ASSERT_NEAR(pntCurve1.X(), pntCurve2.X(), 1e-3);
    ASSERT_NEAR(pntCurve1.Y(), pntCurve2.Y(), 1e-3);
    ASSERT_NEAR(pntCurve1.Z(), pntCurve2.Z(), 1e-3);

    curve1->D0(0.5, pntCurve1);
    curve2->D0(0.5, pntCurve2);
    ASSERT_NEAR(pntCurve1.X(), pntCurve2.X(), 1e-3);
    ASSERT_NEAR(pntCurve1.Y(), pntCurve2.Y(), 1e-3);
    ASSERT_NEAR(pntCurve1.Z(), pntCurve2.Z(), 1e-3);

    curve1->D0(1., pntCurve1);
    curve2->D0(1., pntCurve2);
    ASSERT_NEAR(pntCurve1.X(), pntCurve2.X(), 1e-3);
    ASSERT_NEAR(pntCurve1.Y(), pntCurve2.Y(), 1e-3);
    ASSERT_NEAR(pntCurve1.Z(), pntCurve2.Z(), 1e-3);
}
