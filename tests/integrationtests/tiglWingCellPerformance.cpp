/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-12-23 Martin Siggel <Martin.Siggel@dlr.de>
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


#include "test.h" // Brings in the GTest framework
#include "tigl.h"

#include "CNamedShape.h"
#include "CCPACSConfigurationManager.h"

#include "CCPACSWingCell.h"

#include <string.h>
#include <ctime>


TEST(TestPerformanceWingCell, GetLoft)
{
    TixiDocumentHandle tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    ReturnCode tixiRet;
    TiglReturnCode tiglRet;
    tixiRet = tixiOpenDocument("TestData/IEA-15-240-RWT_CPACS_d.xml", &tixiHandle);
    ASSERT_EQ(SUCCESS, tixiRet);
    tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "IEA-15-240-RWT", &tiglHandle);
    ASSERT_EQ(SUCCESS, tiglRet);

    // get cell
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    tigl::CCPACSWingCell& cell = componentSegment.GetStructure()->GetUpperShell().GetCell(1);

    int nruns = 50;
    double n;

    clock_t start, stop, pause, resume;
    double time_elapsed;

    // test performance of GetLoft of cell.
    n = 0.;
    start = clock();
    for(int i = 0; i < nruns; ++i){
        n+=1;
        TopoDS_Shape cellGeom = cell.GetLoft()->Shape();
        pause = clock();
        cell.Invalidate();
        componentSegment.Invalidate();
        resume = clock();
    }
    stop = clock();

    time_elapsed = (double)(stop - start - resume + pause)/(double)CLOCKS_PER_SEC/(double)nruns * 1000000.;
    std::cout << "Time CCPACSWingCell.GetSkinGeometry [us]: " << time_elapsed << std::endl;


    ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
    ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
}

TEST(TestPerformanceWingCell, GetLoftContourCoordinate)
{
    TixiDocumentHandle tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    ReturnCode tixiRet;
    TiglReturnCode tiglRet;
    tixiRet = tixiOpenDocument("TestData/IEA-15-240-RWT_CPACS_d.xml", &tixiHandle);
    ASSERT_EQ(SUCCESS, tixiRet);
    tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "IEA-15-240-RWT", &tiglHandle);
    ASSERT_EQ(SUCCESS, tiglRet);

    // get cell
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    tigl::CCPACSWingCell& cell = componentSegment.GetStructure()->GetUpperShell().GetCell(3);

    int nruns = 50;
    double n;

    clock_t start, stop, pause, resume;
    double time_elapsed;

    // test performance of GetLoft of cell.
    n = 0.;
    start = clock();
    for(int i = 0; i < nruns; ++i){
        n+=1;
        TopoDS_Shape cellGeom = cell.GetLoft()->Shape();
        pause = clock();
        cell.Invalidate();
        componentSegment.Invalidate();
        resume = clock();
    }
    stop = clock();

    time_elapsed = (double)(stop - start - resume + pause)/(double)CLOCKS_PER_SEC/(double)nruns * 1000000.;
    std::cout << "Time CCPACSWingCell.GetSkinGeometry [us]: " << time_elapsed << std::endl;


    ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
    ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
}

