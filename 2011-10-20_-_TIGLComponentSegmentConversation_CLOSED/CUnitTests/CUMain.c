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
* @brief Main programm for unit tests based on the CUnit framework
*/

#include <stdio.h>
#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

/** Function declarations */
void addTestFunctions(void);

int main(int argc, char* argv[])
{
	CU_BasicRunMode mode = CU_BRM_VERBOSE;
	CU_ErrorAction errorAction = CUEA_IGNORE;

	if (CU_initialize_registry()) {
		printf("\nInitialization of test registry failed.");
	}
	else {
		addTestFunctions();
		CU_basic_set_mode(mode);
		CU_set_error_action(errorAction);
		printf("\nTests completed with return value %d.\n", CU_basic_run_tests());
		CU_cleanup_registry();
	}
}
