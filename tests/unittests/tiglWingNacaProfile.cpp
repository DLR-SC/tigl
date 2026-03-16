/* 
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-14 Hannah Gedler <hannah.gedler@dlr.de>
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
* @brief Tests for NACA wing profiles
*/

#include "test.h" // Brings in the GTest framework
#include "testUtils.h"
#include "tigl.h"
#include "math/tiglmathfunctions.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWingProfile.h"
#include "CNamedShape.h"
#include "BRep_Tool.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepTools.hxx"
#include "Geom_Curve.hxx"
#include "gp_Pnt.hxx"
#include "gp_Pnt.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "Geom_BSplineCurve.hxx"
#include "CCSTCurveBuilder.h"

/******************************************************************************/

class WingNACAProfile : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        const char* filename = "TestData/naca_test.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    void TearDown() override 
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    
};

/******************************************************************************/

TEST_F(WingNACAProfile, nacacreateWing)
{
    // read configuration
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
        // get profile curves of 1st airfoil
        tigl::CCPACSWingProfile & profile = config.GetWingProfile("NACA0012");
        TopoDS_Edge upperWire = profile.GetUpperWire();
        TopoDS_Edge lowerWire = profile.GetLowerWire();
        TopoDS_Edge trailingEdge = profile.GetTrailingEdge();
        //auto wingShape = config.GetWing(1).GetLoft()->Shape();
        BRepTools::Write(upperWire, "upperWire.brep");
        BRepTools::Write(lowerWire, "lowerWire.brep");
        BRepTools::Write(trailingEdge, "trailingEdge.brep");     
}

TEST_F(WingNACAProfile, Nacacalculatortest23){

        // read configuration
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
        Standard_Real u1, u2;

        // get profile curves of airfoil
        tigl::CCPACSWingProfile & profile = config.GetWingProfile("NACA0012");
        TopoDS_Edge upperWire = profile.GetUpperWire();
        EXPECT_TRUE(!upperWire.IsNull());
        Handle(Geom_Curve) upperCurve = BRep_Tool::Curve(upperWire, u1, u2);
        ASSERT_TRUE(!upperCurve.IsNull());
}

TEST_F(WingNACAProfile, Nacacalculatortest24)
{

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

      Standard_Real u1, u2;
    tigl::CCPACSWingProfile& profile = config.GetWingProfile("NACA0012");
    TopoDS_Edge upperWire = profile.GetUpperWire();
        EXPECT_TRUE(!upperWire.IsNull());
        Handle(Geom_Curve) upperCurve = BRep_Tool::Curve(upperWire, u1, u2);
        ASSERT_TRUE(!upperCurve.IsNull());
}
