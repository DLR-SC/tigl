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
* @brief Declaration of the test functions.
*/

#ifndef TESTFUNCTIONS_H
#define TESTFUNCTIONS_H
	
int preTiglOpenCPACSConfiguration(void);
int postTiglOpenCPACSConfiguration(void);
void tiglOpenCPACSConfiguration_nullPointerArgument(void);
void tiglOpenCPACSConfiguration_success(void); 
void tiglOpenCPACSConfiguration_without_uid(void);

int preTiglGetCPACSTixiHandle(void);
int postTiglGetCPACSTixiHandle(void);
void tiglGetCPACSTixiHandle_nullPointerArgument(void);
void tiglGetCPACSTixiHandle_notFound(void);
void tiglGetCPACSTixiHandle_success(void);

int preTiglIsCPACSConfigurationHandleValid(void);
int postTiglIsCPACSConfigurationHandleValid(void);
void tiglIsCPACSConfigurationHandleValid_invalidHandle(void);
void tiglIsCPACSConfigurationHandleValid_validHandle(void);
void tiglGetVersion_valid(void);

void tiglUseAlgorithm_invalidAlgorithm(void);
void tiglUseAlgorithm_validAlgorithm(void);

int preTiglWingGetPoint(void);
int postTiglWingGetPoint(void);
void tiglWingGetUpperPoint_invalidHandle(void);
void tiglWingGetLowerPoint_invalidHandle(void);
void tiglWingGetUpperPoint_invalidWing(void);
void tiglWingGetLowerPoint_invalidWing(void);
void tiglWingGetUpperPoint_invalidSegment(void);
void tiglWingGetLowerPoint_invalidSegment(void);
void tiglWingGetUpperPoint_invalidEta(void);
void tiglWingGetLowerPoint_invalidEta(void);
void tiglWingGetUpperPoint_invalidXsi(void);
void tiglWingGetLowerPoint_invalidXsi(void);
void tiglWingGetUpperPoint_nullPointerArgument(void);
void tiglWingGetLowerPoint_nullPointerArgument(void);
void tiglWingGetUpperPoint_success(void);
void tiglWingGetLowerPoint_success(void);

int preTiglWingSegment(void);
int postTiglWingSegment(void);
void tiglGetWingCount_invalidHandle(void);
void tiglGetWingCount_nullPointerArgument(void);
void tiglGetWingCount_success(void);
void tiglWingGetSegmentCount_invalidHandle(void);
void tiglWingGetSegmentCount_invalidWing(void);
void tiglWingGetSegmentCount_nullPointerArgument(void);
void tiglWingGetSegmentCount_success(void);
void tiglWingGetInnerConnectedSegmentCount_invalidHandle(void);
void tiglWingGetInnerConnectedSegmentCount_invalidWing(void);
void tiglWingGetInnerConnectedSegmentCount_invalidSegment(void);
void tiglWingGetInnerConnectedSegmentCount_success(void);
void tiglWingGetOuterConnectedSegmentCount_invalidHandle(void);
void tiglWingGetOuterConnectedSegmentCount_invalidWing(void);
void tiglWingGetOuterConnectedSegmentCount_invalidSegment(void);
void tiglWingGetOuterConnectedSegmentCount_nullPointerArgument(void);
void tiglWingGetOuterConnectedSegmentCount_success(void);
void tiglWingGetInnerConnectedSegmentIndex_invalidHandle(void);
void tiglWingGetInnerConnectedSegmentIndex_invalidWing(void);
void tiglWingGetInnerConnectedSegmentIndex_invalidSegment(void);
void tiglWingGetInnerConnectedSegmentIndex_invalidN(void);
void tiglWingGetInnerConnectedSegmentIndex_success(void);
void tiglWingGetOuterConnectedSegmentIndex_invalidHandle(void);
void tiglWingGetOuterConnectedSegmentIndex_invalidWing(void);
void tiglWingGetOuterConnectedSegmentIndex_invalidSegment(void);
void tiglWingGetOuterConnectedSegmentIndex_invalidN(void);
void tiglWingGetOuterConnectedSegmentIndex_nullPointerArgument(void);
void tiglWingGetOuterConnectedSegmentIndex_success(void);
void tiglWingGetInnerSectionAndElementIndex_invalidHandle(void);
void tiglWingGetInnerSectionAndElementIndex_invalidWing(void);
void tiglWingGetInnerSectionAndElementIndex_invalidSegment(void);
void tiglWingGetInnerSectionAndElementIndex_nullPointerArgument(void);
void tiglWingGetInnerSectionAndElementIndex_success(void);
void tiglWingGetOuterSectionAndElementIndex_invalidHandle(void);
void tiglWingGetOuterSectionAndElementIndex_invalidWing(void);
void tiglWingGetOuterSectionAndElementIndex_invalidSegment(void);
void tiglWingGetOuterSectionAndElementIndex_nullPointerArgument(void);
void tiglWingGetOuterSectionAndElementIndex_success(void);
void tiglWingGetInnerSectionAndElementUID_invalidHandle(void);
void tiglWingGetInnerSectionAndElementUID_invalidWing(void);
void tiglWingGetInnerSectionAndElementUID_invalidSegment(void);
void tiglWingGetInnerSectionAndElementUID_success(void);
void tiglWingGetOuterSectionAndElementUID_invalidHandle(void);
void tiglWingGetOuterSectionAndElementUID_invalidWing(void);
void tiglWingGetOuterSectionAndElementUID_invalidSegment(void);
void tiglWingGetOuterSectionAndElementUID_success(void);

int preTiglWing(void);
int postTiglWing(void);
void tiglWingGetProfileName_invalidHandle(void);
void tiglWingGetProfileName_invalidWing(void);
void tiglWingGetProfileName_invalidSection(void);
void tiglWingGetProfileName_invalidElement(void);
void tiglWingGetProfileName_nullPointerArgument(void);
void tiglWingGetProfileName_success(void);
void tiglWingGetUID_success(void);
void tiglWingGetSegmentUID_success(void);
void tiglWingGetSectionUID_success(void);

int preTiglFuselageGetPoint(void);
int postTiglFuselageGetPoint(void);
void tiglFuselageGetPoint_invalidHandle(void);
void tiglFuselageGetPoint_invalidFuselage(void);
void tiglFuselageGetPoint_invalidSegment(void);
void tiglFuselageGetPoint_invalidEta(void);
void tiglFuselageGetPoint_invalidZeta(void);
void tiglFuselageGetPoint_nullPointerArgument(void);
void tiglFuselageGetPoint_success(void);

int preTiglFuselageSegment(void);
int postTiglFuselageSegment(void);
void tiglGetFuselageCount_invalidHandle(void);
void tiglGetFuselageCount_nullPointerArgument(void);
void tiglGetFuselageCount_success(void);
void tiglFuselageGetSegmentCount_invalidHandle(void);
void tiglFuselageGetSegmentCount_invalidFuselage(void);
void tiglFuselageGetSegmentCount_nullPointerArgument(void);
void tiglFuselageGetSegmentCount_success(void);
void tiglFuselageGetStartConnectedSegmentCount_invalidHandle(void);
void tiglFuselageGetStartConnectedSegmentCount_invalidFuselage(void);
void tiglFuselageGetStartConnectedSegmentCount_invalidSegment(void);
void tiglFuselageGetStartConnectedSegmentCount_nullPointerArgument(void);
void tiglFuselageGetStartConnectedSegmentCount_success(void);
void tiglFuselageGetEndConnectedSegmentCount_invalidHandle(void);
void tiglFuselageGetEndConnectedSegmentCount_invalidFuselage(void);
void tiglFuselageGetEndConnectedSegmentCount_invalidSegment(void);
void tiglFuselageGetEndConnectedSegmentCount_nullPointerArgument(void);
void tiglFuselageGetEndConnectedSegmentCount_success(void);
void tiglFuselageGetStartConnectedSegmentIndex_invalidHandle(void);
void tiglFuselageGetStartConnectedSegmentIndex_invalidFuselage(void);
void tiglFuselageGetStartConnectedSegmentIndex_invalidSegment(void);
void tiglFuselageGetStartConnectedSegmentIndex_invalidN(void);
void tiglFuselageGetStartConnectedSegmentIndex_nullPointerArgument(void);
void tiglFuselageGetStartConnectedSegmentIndex_success(void);
void tiglFuselageGetEndConnectedSegmentIndex_invalidHandle(void);
void tiglFuselageGetEndConnectedSegmentIndex_invalidFuselage(void);
void tiglFuselageGetEndConnectedSegmentIndex_invalidSegment(void);
void tiglFuselageGetEndConnectedSegmentIndex_invalidN(void);
void tiglFuselageGetEndConnectedSegmentIndex_nullPointerArgument(void);
void tiglFuselageGetEndConnectedSegmentIndex_success(void);
void tiglFuselageGetStartSectionAndElementIndex_invalidHandle(void);
void tiglFuselageGetStartSectionAndElementIndex_invalidFuselage(void);
void tiglFuselageGetStartSectionAndElementIndex_invalidSegment(void);
void tiglFuselageGetStartSectionAndElementIndex_nullPointerArgument(void);
void tiglFuselageGetStartSectionAndElementIndex_success(void);
void tiglFuselageGetEndSectionAndElementIndex_invalidHandle(void);
void tiglFuselageGetEndSectionAndElementIndex_invalidFuselage(void);
void tiglFuselageGetEndSectionAndElementIndex_invalidSegment(void);
void tiglFuselageGetEndSectionAndElementIndex_nullPointerArgument(void);
void tiglFuselageGetEndSectionAndElementIndex_success(void);
void tiglFuselageGetStartSectionAndElementUID_invalidHandle(void);
void tiglFuselageGetStartSectionAndElementUID_invalidFuselage(void);
void tiglFuselageGetStartSectionAndElementUID_invalidSegment(void);
void tiglFuselageGetStartSectionAndElementUID_success(void);
void tiglFuselageGetEndSectionAndElementUID_invalidHandle(void);
void tiglFuselageGetEndSectionAndElementUID_invalidFuselage(void);
void tiglFuselageGetEndSectionAndElementUID_invalidSegment(void);
void tiglFuselageGetEndSectionAndElementUID_success(void);

int preTiglIntersection(void);
int postTiglIntersection(void);
void tiglIntersection_FuselageWingIntersects(void);

int preTiglExport(void);
int postTiglExport(void);
void tiglExportMeshedWingVTK_success(void);

#endif /* TESTFUNCTIONS_H */
