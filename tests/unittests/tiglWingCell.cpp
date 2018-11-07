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

#include "CCPACSConfigurationManager.h"
#include "CCPACSWing.h"
#include "CCPACSWingCell.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSWingRibsPositioning.h"
#include "CCPACSWingSegment.h"

#include <BRepTools.hxx>

using namespace tigl;

typedef std::pair<double, double> DP;

/******************************************************************************/

class WingCellSpar : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/cell_test_spars.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ(SUCCESS, tixiRet);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "model", &tiglHandle);
        ASSERT_EQ(TIGL_SUCCESS, tiglRet);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
        ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}

    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle WingCellSpar::tixiHandle = 0;
TiglCPACSConfigurationHandle WingCellSpar::tiglHandle = 0;


class WingCellRibSpar : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/cell_rib_spar_test.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ(SUCCESS, tixiRet);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "model", &tiglHandle);
        ASSERT_EQ(TIGL_SUCCESS, tiglRet);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
        ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}

    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle WingCellRibSpar::tixiHandle = 0;
TiglCPACSConfigurationHandle WingCellRibSpar::tiglHandle = 0;

/******************************************************************************/

TEST(WingCell, IsInner)
{
    tigl::CCPACSWingCell cell(NULL, NULL);
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
    tigl::CCPACSWingCell cell(NULL, NULL);
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
    tigl::CCPACSWingCell cell(NULL, NULL);
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

TEST(WingCell, area)
{
    WingCellInternal::Point2D p1, p2, p3;
    p1.x = 1.; p1.y = 1.;
    p2.x = 2.; p2.y = 1.;
    p3.x = 1.7; p3.y = 2.;
    
    ASSERT_NEAR(0.5, WingCellInternal::area(p1,p2,p3), 1e-7);
    ASSERT_NEAR(0.5, WingCellInternal::area(p2,p3,p1), 1e-7);
    ASSERT_NEAR(0.5, WingCellInternal::area(p3,p1,p2), 1e-7);
}

TEST_F(WingCellSpar, sparCellXsi) {
    // See: cell_test_spars.png for placement of cells

    const std::pair<double, double> arr[] = { DP(0.2, 0.25), DP(0.35, 0.4), DP(0.3, 0.43), DP(0.275, 0.4), DP(0.47, 0.4), DP(0.5, 0.3) };
    std::vector< std::pair<double, double> > expectedXsis(arr, arr + sizeof(arr) / sizeof(arr[0]));

    // get Component Segment
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));

    int cellIndex = 1;
    std::vector< std::pair<double, double> >::const_iterator it;
    for (it = expectedXsis.begin(); it != expectedXsis.end(); ++it) {
        double xsi1Exp = it->first;
        double xsi2Exp = it->second;

        tigl::CCPACSWingCell& cell = componentSegment.GetStructure()->GetUpperShell().GetCell(cellIndex++);
        tigl::EtaXsi etaxsi1 = cell.GetLeadingEdgeInnerPoint();
        tigl::EtaXsi etaxsi2 = cell.GetLeadingEdgeOuterPoint();
        ASSERT_NEAR(etaxsi1.xsi, xsi1Exp, 1e-7);
        ASSERT_NEAR(etaxsi2.xsi, xsi2Exp, 1e-7);
    }
}

namespace {
    void checkCellEtaXsis(const tigl::CCPACSWingCell& cell, const std::vector< std::pair<double, double> >& expectedEtaXsi, double precision = 1e-7) {
        tigl::EtaXsi etaxsi;
        unsigned int etaXsiIndex = 0;
        etaxsi = cell.GetLeadingEdgeInnerPoint();
        ASSERT_NEAR(etaxsi.eta, expectedEtaXsi[etaXsiIndex].first, precision);
        ASSERT_NEAR(etaxsi.xsi, expectedEtaXsi[etaXsiIndex].second, precision);
        ++etaXsiIndex;
        etaxsi = cell.GetLeadingEdgeOuterPoint();
        ASSERT_NEAR(etaxsi.eta, expectedEtaXsi[etaXsiIndex].first, precision);
        ASSERT_NEAR(etaxsi.xsi, expectedEtaXsi[etaXsiIndex].second, precision);
        ++etaXsiIndex;
        etaxsi = cell.GetTrailingEdgeInnerPoint();
        ASSERT_NEAR(etaxsi.eta, expectedEtaXsi[etaXsiIndex].first, precision);
        ASSERT_NEAR(etaxsi.xsi, expectedEtaXsi[etaXsiIndex].second, precision);
        ++etaXsiIndex;
        etaxsi = cell.GetTrailingEdgeOuterPoint();
        ASSERT_NEAR(etaxsi.eta, expectedEtaXsi[etaXsiIndex].first, precision);
        ASSERT_NEAR(etaxsi.xsi, expectedEtaXsi[etaXsiIndex].second, precision);
    }
}

TEST_F(WingCellRibSpar, etaXsi) {
    // See: cell_rib_spar_test.png for placement of cells

    // next compute the expected XSI values on the spar
    const std::pair<double, double> arr[] = { DP(0.2, 0.3), DP(0.95, 0.4), DP(0.2, 0.8), DP(0.95, 1.0) };
    std::vector< std::pair<double, double> > expectedEtaXsi (arr, arr + sizeof(arr) / sizeof(arr[0]));

    // get Component Segment
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    tigl::CCPACSWingCSStructure& structure = *componentSegment.GetStructure();

    tigl::CCPACSWingCell& cell = componentSegment.GetStructure()->GetUpperShell().GetCell(1);
    checkCellEtaXsis(cell, expectedEtaXsi);

    // now we change the rib definition and watch whether the cell is correctly updated
    structure.GetRibsDefinition(1).GetRibsPositioning_choice1()->SetEtaEnd(0.8);
    const std::pair<double, double> arr2[] = { DP(0.2, 0.3), DP(0.8, 0.48), DP(0.2, 0.8), DP(0.8, 1.0) };
    expectedEtaXsi = std::vector< std::pair<double, double> > (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]));
    checkCellEtaXsis(cell, expectedEtaXsi);

    // next we change the z-rotation of the rib
    // See: cell_rib_spar_test_2.png for placement of cells
    structure.GetRibsDefinition(1).GetRibsPositioning_choice1()->GetRibRotation().SetZ(75);
    const std::pair<double, double> arr3[] = { DP(0.16, 0.28), DP(0.74, 0.47), DP(0.09, 0.8), DP(0.67, 1.0) };
    expectedEtaXsi = std::vector< std::pair<double, double> > (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]));
    // precision at 1E-2 since expected values are estimated based on geometric inspection
    checkCellEtaXsis(cell, expectedEtaXsi, 1.E-2);
}

TEST_F(WingCellRibSpar, computeGeometry) {
    // See: cell_rib_spar_test.png for placement of cells

    // next compute the expected XSI values on the spar
    const std::pair<double, double> arr[] = { DP(0.2, 0.3), DP(0.95, 0.4), DP(0.2, 0.8), DP(0.95, 1.0) };
    std::vector< std::pair<double, double> > expectedEtaXsi (arr, arr + sizeof(arr) / sizeof(arr[0]));

    // get Component Segment
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    tigl::CCPACSWingCSStructure& structure = *componentSegment.GetStructure();

    tigl::CCPACSWingCell& cell = componentSegment.GetStructure()->GetUpperShell().GetCell(1);
    TopoDS_Shape cellGeom = cell.GetSkinGeometry();
    BRepTools::Write(cellGeom, "TestData/export/WingCellRibSpar_CellGeometry.brep");
}
