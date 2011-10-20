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
* @brief Implementation of addTestFunctions, which adds test suites and test
* functions to the CUnit framework.
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "CUnit/CUnit.h"
#include "testFunctions.h"

#define PRINT_ERROR { fprintf(stderr, "At line %d in file %s:\n %s\n", __LINE__, __FILE__, CU_get_error_msg() ); exit(1);}

void addTestFunctions(void) 
{
  CU_pSuite pSuite;
	
  assert(CU_get_registry() != NULL);
  assert(!CU_is_test_running());
	
  printf("Add suite for export routines\n");
  if (!(pSuite = CU_add_suite("tiglExport", preTiglExport, postTiglExport))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglExportMeshedWingVTK", tiglExportMeshedWingVTK_success))) PRINT_ERROR; 
 
  printf("Add suite for tiglOpenCPACSConfiguration\n");
  if (!(pSuite = CU_add_suite("tiglOpenCPACSConfiguration", preTiglOpenCPACSConfiguration, postTiglOpenCPACSConfiguration))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglOpenCPACSConfiguration_nullPointerArgument", tiglOpenCPACSConfiguration_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglOpenCPACSConfiguration_success", tiglOpenCPACSConfiguration_success))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglOpenCPACSConfiguration_without_uid", tiglOpenCPACSConfiguration_without_uid))) PRINT_ERROR;

  printf("Add suite for tiglGetCPACSTixiHandle\n");
  if (!(pSuite = CU_add_suite("tiglGetCPACSTixiHandle", preTiglGetCPACSTixiHandle, postTiglGetCPACSTixiHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglGetCPACSTixiHandle_nullPointerArgument", tiglGetCPACSTixiHandle_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglGetCPACSTixiHandle_notFound", tiglGetCPACSTixiHandle_notFound))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglGetCPACSTixiHandle_success", tiglGetCPACSTixiHandle_success))) PRINT_ERROR;

  printf("Add suite for tiglIsCPACSConfigurationHandleValid\n");
  if (!(pSuite = CU_add_suite("tiglIsCPACSConfigurationHandleValid", preTiglIsCPACSConfigurationHandleValid, postTiglIsCPACSConfigurationHandleValid))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglIsCPACSConfigurationHandleValid_invalidHandle", tiglIsCPACSConfigurationHandleValid_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglIsCPACSConfigurationHandleValid_validHandle", tiglIsCPACSConfigurationHandleValid_validHandle))) PRINT_ERROR;

  printf("Tests version number with tiglGetVersion\n");
  if (!(CU_add_test(pSuite, "tiglGetVersion_valid", tiglGetVersion_valid))) PRINT_ERROR;

  printf("Add suite for tiglUseAlgorithm\n");
  if (!(pSuite = CU_add_suite("tiglUseAlgorithm", 0, 0))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglUseAlgorithm_invalidAlgorithm", tiglUseAlgorithm_invalidAlgorithm))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglUseAlgorithm_validAlgorithm", tiglUseAlgorithm_validAlgorithm))) PRINT_ERROR;

  printf("Add suite for tiglWingGetUpperPoint and tiglWingGetLowerPoint\n");
  if (!(pSuite = CU_add_suite("tiglWingGetUpperPoint/tiglWingGetLowerPoint", preTiglWingGetPoint, postTiglWingGetPoint))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetUpperPoint_invalidHandle", tiglWingGetUpperPoint_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetLowerPoint_invalidHandle", tiglWingGetLowerPoint_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetUpperPoint_invalidWing", tiglWingGetUpperPoint_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetLowerPoint_invalidWing", tiglWingGetLowerPoint_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetUpperPoint_invalidSegment", tiglWingGetUpperPoint_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetLowerPoint_invalidSegment", tiglWingGetLowerPoint_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetUpperPoint_invalidEta", tiglWingGetUpperPoint_invalidEta))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetLowerPoint_invalidEta", tiglWingGetLowerPoint_invalidEta))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetUpperPoint_invalidXsi", tiglWingGetUpperPoint_invalidXsi))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetLowerPoint_invalidXsi", tiglWingGetLowerPoint_invalidXsi))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetUpperPoint_nullPointerArgument", tiglWingGetUpperPoint_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetLowerPoint_nullPointerArgument", tiglWingGetLowerPoint_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetUpperPoint_success", tiglWingGetUpperPoint_success))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetLowerPoint_success", tiglWingGetLowerPoint_success))) PRINT_ERROR;

  printf("Add suite for wing segment handling and query routines\n");
  if (!(pSuite = CU_add_suite("Wing segment handling and query", preTiglWingSegment, postTiglWingSegment))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglGetWingCount_invalidHandle", tiglGetWingCount_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglGetWingCount_nullPointerArgument", tiglGetWingCount_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglGetWingCount_success", tiglGetWingCount_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetSegmentCount_invalidHandle", tiglWingGetSegmentCount_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetSegmentCount_invalidWing", tiglWingGetSegmentCount_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetSegmentCount_nullPointerArgument", tiglWingGetSegmentCount_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetSegmentCount_success", tiglWingGetSegmentCount_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentCount_invalidHandle", tiglWingGetInnerConnectedSegmentCount_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentCount_invalidWing", tiglWingGetInnerConnectedSegmentCount_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentCount_invalidSegment", tiglWingGetInnerConnectedSegmentCount_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentCount_success", tiglWingGetInnerConnectedSegmentCount_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentCount_invalidHandle", tiglWingGetOuterConnectedSegmentCount_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentCount_invalidWing", tiglWingGetOuterConnectedSegmentCount_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentCount_invalidSegment", tiglWingGetOuterConnectedSegmentCount_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentCount_nullPointerArgument", tiglWingGetOuterConnectedSegmentCount_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentCount_success", tiglWingGetOuterConnectedSegmentCount_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentIndex_invalidHandle", tiglWingGetInnerConnectedSegmentIndex_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentIndex_invalidWing", tiglWingGetInnerConnectedSegmentIndex_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentIndex_invalidSegment", tiglWingGetInnerConnectedSegmentIndex_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentIndex_invalidN", tiglWingGetInnerConnectedSegmentIndex_invalidN))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerConnectedSegmentIndex_success", tiglWingGetInnerConnectedSegmentIndex_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentIndex_invalidHandle", tiglWingGetOuterConnectedSegmentIndex_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentIndex_invalidWing", tiglWingGetOuterConnectedSegmentIndex_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentIndex_invalidSegment", tiglWingGetOuterConnectedSegmentIndex_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentIndex_invalidN", tiglWingGetOuterConnectedSegmentIndex_invalidN))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterConnectedSegmentIndex_success", tiglWingGetOuterConnectedSegmentIndex_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementIndex_invalidHandle", tiglWingGetInnerSectionAndElementIndex_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementIndex_invalidWing", tiglWingGetInnerSectionAndElementIndex_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementIndex_invalidSegment", tiglWingGetInnerSectionAndElementIndex_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementIndex_nullPointerArgument", tiglWingGetInnerSectionAndElementIndex_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementIndex_success", tiglWingGetInnerSectionAndElementIndex_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementIndex_invalidHandle", tiglWingGetOuterSectionAndElementIndex_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementIndex_invalidWing", tiglWingGetOuterSectionAndElementIndex_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementIndex_invalidSegment", tiglWingGetOuterSectionAndElementIndex_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementIndex_nullPointerArgument", tiglWingGetOuterSectionAndElementIndex_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementIndex_success", tiglWingGetOuterSectionAndElementIndex_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementUID_invalidHandle", tiglWingGetInnerSectionAndElementUID_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementUID_invalidWing", tiglWingGetInnerSectionAndElementUID_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementUID_invalidSegment", tiglWingGetInnerSectionAndElementUID_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetInnerSectionAndElementUID_success", tiglWingGetInnerSectionAndElementUID_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementUID_invalidHandle", tiglWingGetOuterSectionAndElementUID_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementUID_invalidWing", tiglWingGetOuterSectionAndElementUID_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementUID_invalidSegment", tiglWingGetOuterSectionAndElementUID_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetOuterSectionAndElementUID_success", tiglWingGetOuterSectionAndElementUID_success))) PRINT_ERROR;


  printf("Add suite for non classified wing routines\n");
  if (!(pSuite = CU_add_suite("tiglWing", preTiglWing, postTiglWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetProfileName_invalidHandle", tiglWingGetProfileName_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetProfileName_invalidWing", tiglWingGetProfileName_invalidWing))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetProfileName_invalidSection", tiglWingGetProfileName_invalidSection))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetProfileName_invalidElement", tiglWingGetProfileName_invalidElement))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetProfileName_nullPointerArgument", tiglWingGetProfileName_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglWingGetProfileName_success", tiglWingGetProfileName_success))) PRINT_ERROR; 
  
  printf("Add tests some uid wing routines\n");
  if (!(CU_add_test(pSuite, "tiglWingGetUID_success", tiglWingGetUID_success))) PRINT_ERROR; 
  if (!(CU_add_test(pSuite, "tiglWingGetSegmentUID_success", tiglWingGetSegmentUID_success))) PRINT_ERROR; 
  if (!(CU_add_test(pSuite, "tiglWingGetSectionUID_success", tiglWingGetSectionUID_success))) PRINT_ERROR; 
 

  printf("Add suite for tiglFuselageGetPoint\n");
  if (!(pSuite = CU_add_suite("tiglFuselageGetPoint", preTiglFuselageGetPoint, postTiglFuselageGetPoint))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetPoint_invalidHandle", tiglFuselageGetPoint_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetPoint_invalidFuselage", tiglFuselageGetPoint_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetPoint_invalidSegment", tiglFuselageGetPoint_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetPoint_invalidEta", tiglFuselageGetPoint_invalidEta))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetPoint_invalidZeta", tiglFuselageGetPoint_invalidZeta))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetPoint_nullPointerArgument", tiglFuselageGetPoint_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetPoint_success", tiglFuselageGetPoint_success))) PRINT_ERROR;

  printf("Add suite for fuselage segment handling and query routines\n");
  if (!(pSuite = CU_add_suite("Fuselage segment handling and query", preTiglFuselageSegment, postTiglFuselageSegment))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglGetFuselageCount_invalidHandle", tiglGetFuselageCount_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglGetFuselageCount_nullPointerArgument", tiglGetFuselageCount_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglGetFuselageCount_success", tiglGetFuselageCount_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetSegmentCount_invalidHandle", tiglFuselageGetSegmentCount_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetSegmentCount_invalidFuselage", tiglFuselageGetSegmentCount_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetSegmentCount_nullPointerArgument", tiglFuselageGetSegmentCount_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetSegmentCount_success", tiglFuselageGetSegmentCount_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentCount_invalidHandle", tiglFuselageGetStartConnectedSegmentCount_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentCount_invalidFuselage", tiglFuselageGetStartConnectedSegmentCount_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentCount_invalidSegment", tiglFuselageGetStartConnectedSegmentCount_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentCount_nullPointerArgument", tiglFuselageGetStartConnectedSegmentCount_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentCount_success", tiglFuselageGetStartConnectedSegmentCount_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentCount_invalidHandle", tiglFuselageGetEndConnectedSegmentCount_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentCount_invalidFuselage", tiglFuselageGetEndConnectedSegmentCount_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentCount_invalidSegment", tiglFuselageGetEndConnectedSegmentCount_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentCount_nullPointerArgument", tiglFuselageGetEndConnectedSegmentCount_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentCount_success", tiglFuselageGetEndConnectedSegmentCount_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentIndex_invalidHandle", tiglFuselageGetStartConnectedSegmentIndex_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentIndex_invalidFuselage", tiglFuselageGetStartConnectedSegmentIndex_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentIndex_invalidSegment", tiglFuselageGetStartConnectedSegmentIndex_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentIndex_invalidN", tiglFuselageGetStartConnectedSegmentIndex_invalidN))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentIndex_nullPointerArgument", tiglFuselageGetStartConnectedSegmentIndex_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartConnectedSegmentIndex_success", tiglFuselageGetStartConnectedSegmentIndex_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentIndex_invalidHandle", tiglFuselageGetEndConnectedSegmentIndex_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentIndex_invalidFuselage", tiglFuselageGetEndConnectedSegmentIndex_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentIndex_invalidSegment", tiglFuselageGetEndConnectedSegmentIndex_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentIndex_invalidN", tiglFuselageGetEndConnectedSegmentIndex_invalidN))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentIndex_nullPointerArgument", tiglFuselageGetEndConnectedSegmentIndex_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndConnectedSegmentIndex_success", tiglFuselageGetEndConnectedSegmentIndex_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementIndex_invalidHandle", tiglFuselageGetStartSectionAndElementIndex_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementIndex_invalidFuselage", tiglFuselageGetStartSectionAndElementIndex_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementIndex_invalidSegment", tiglFuselageGetStartSectionAndElementIndex_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementIndex_nullPointerArgument", tiglFuselageGetStartSectionAndElementIndex_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementIndex_success", tiglFuselageGetStartSectionAndElementIndex_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementIndex_invalidHandle", tiglFuselageGetEndSectionAndElementIndex_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementIndex_invalidFuselage", tiglFuselageGetEndSectionAndElementIndex_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementIndex_invalidSegment", tiglFuselageGetEndSectionAndElementIndex_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementIndex_nullPointerArgument", tiglFuselageGetEndSectionAndElementIndex_nullPointerArgument))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementIndex_success", tiglFuselageGetEndSectionAndElementIndex_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementUID_invalidHandle", tiglFuselageGetStartSectionAndElementUID_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementUID_invalidFuselage", tiglFuselageGetStartSectionAndElementUID_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementUID_invalidSegment", tiglFuselageGetStartSectionAndElementUID_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetStartSectionAndElementUID_success", tiglFuselageGetStartSectionAndElementUID_success))) PRINT_ERROR;

  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementUID_invalidHandle", tiglFuselageGetEndSectionAndElementUID_invalidHandle))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementUID_invalidFuselage", tiglFuselageGetEndSectionAndElementUID_invalidFuselage))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementUID_invalidSegment", tiglFuselageGetEndSectionAndElementUID_invalidSegment))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglFuselageGetEndSectionAndElementUID_success", tiglFuselageGetEndSectionAndElementUID_success))) PRINT_ERROR;


  printf("Add suite for tiglIntersectionCalculation\n");
  if (!(pSuite = CU_add_suite("tiglIntersectionCalculation", 0, 0 ))) PRINT_ERROR;

  if (!(pSuite = CU_add_suite("FuselageWingIntersects", preTiglIntersection, postTiglIntersection ))) PRINT_ERROR;
  if (!(CU_add_test(pSuite, "tiglIntersection_FuselageWingIntersects", tiglIntersection_FuselageWingIntersects ))) PRINT_ERROR;


}

