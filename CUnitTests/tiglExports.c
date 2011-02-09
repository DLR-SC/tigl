/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
* @brief Tests for testing export functions.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"

//#include "BRepPrimAPI_MakeCylinder.hxx"


/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

int preTiglExport(void)
{
    char* filename = "../TestData/cpacs_25032009.xml";
	ReturnCode tixiRet;
	TiglReturnCode tiglRet;

	tiglHandle = -1;
	tixiHandle = -1;
	
	tixiRet = tixiOpenDocument(filename, &tixiHandle);
	if (tixiRet != SUCCESS) 
		return 1;

	tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "VFW-614", &tiglHandle);
	return (tiglRet == TIGL_SUCCESS ? 0 : 1);
}

int postTiglExport(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

/******************************************************************************/

//void tiglExportMeshedWingVTK_small_example(void)
//{
//    const BRepPrimAPI_MakeCylinder cone(/* radius */ 2.0, /* height */ 8.0);
//    const CTiglExportVtk writer(config);
//    writer.ExportMeshedWingVTK
//}

/**
* Tests tiglWingGetProfileName with invalid CPACS handle.
*/
void tiglExportMeshedWingVTK_success(void)
{
    char* vtkWingFilename = "../TestData/export/cpacs_25032009_wing1.vtp";
    char* vtkFuselageFilename = "../TestData/export/cpacs_25032009_fuselage1.vtp";
    char* vtkGeometryFilename = "../TestData/export/cpacs_25032009_geometry.vtp";
//    CU_ASSERT(tiglExportMeshedWingVTK(tiglHandle, 1, vtkWingFilename, 0.01) == TIGL_SUCCESS);
}
