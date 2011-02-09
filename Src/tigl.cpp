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
* @brief  Implementation of the TIGL interface.
*/

#include <iostream>
#include <exception>

#include "tigl.h"
#include "CTiglError.h"
#include "CTiglIntersectionCalculation.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglAlgorithmManager.h"
#include "CTiglIntersectionCalculation.h"
#include "CTiglUIDManager.h"
#include "CCPACSWing.h"
#include "CTiglExportIges.h"
#include "CTiglExportStl.h"
#include "CTiglExportVtk.h"

#include "gp_Pnt.hxx"
#include "TopoDS_Shape.hxx"

/*****************************************************************************/
/* Public visible functions.                                                 */
/*****************************************************************************/

DLL_EXPORT TiglReturnCode tiglOpenCPACSConfiguration(TixiDocumentHandle tixiHandle, char* configurationUID, TiglCPACSConfigurationHandle* cpacsHandlePtr)
{
    if (cpacsHandlePtr == 0) {
        std::cerr << "Error: Null pointer argument for cpacsHandlePtr in function call to tiglOpenCPACSConfiguration." << std::endl;
        return TIGL_NULL_POINTER;
    }
	if (configurationUID == 0) {
        std::cerr << "Error: Null pointer argument for configurationUID";
        std::cerr << "in function call to tiglOpenCPACSConfiguration." << std::endl;
        return TIGL_NULL_POINTER;
    }

	/* check TIXI Version */
	if( atof(tixiGetVersion()) < atof(tiglGetVersion()) ) {
		std::cerr << "Incompatible TIXI Version in use with this TIGL" << std::endl;
		return TIGL_WRONG_TIXI_VERSION;
	}

	/* Check if configuration exists */
	char *ConfigurationXPathPrt = NULL;
	char *tmpString = NULL;
	char *tmpString2 = NULL;

	tixiUIDGetXPath(tixiHandle, configurationUID, &tmpString2);
	ConfigurationXPathPrt = (char *) malloc(sizeof(char) * (strlen(tmpString2) + 50));
	strcpy(ConfigurationXPathPrt, tmpString2);
	strcat(ConfigurationXPathPrt, "[@uID=\"");
	strcat(ConfigurationXPathPrt, configurationUID);
	strcat(ConfigurationXPathPrt, "\"]");
	int tixiReturn = tixiGetTextElement( tixiHandle, ConfigurationXPathPrt, &tmpString);
	if(tixiReturn != 0) {
		std::cerr << "Configuration '" << configurationUID << "' not found!" << std::endl;
		return TIGL_ERROR;
	}

	free(ConfigurationXPathPrt);

    tigl::CCPACSConfiguration* config = 0;
    try {
        config = new tigl::CCPACSConfiguration(tixiHandle);
        // Build CPACS memory structure
		config->ReadCPACS(configurationUID);
        // Store configuration in handle container
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        *cpacsHandlePtr = manager.AddConfiguration(config);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        delete config;
        std::cerr << ex.what() << std::endl;
        return TIGL_OPEN_FAILED;
    }
    catch (tigl::CTiglError& ex) {
        delete config;
        std::cerr << ex.getError() << std::endl;
        return TIGL_OPEN_FAILED;
    }
    catch (...) {
        delete config;
        std::cerr << "Caught an exception in tiglOpenCPACSConfiguration!" << std::endl;
        return TIGL_OPEN_FAILED;
    }
}


DLL_EXPORT TiglReturnCode tiglCloseCPACSConfiguration(TiglCPACSConfigurationHandle cpacsHandle)
{
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        manager.DeleteConfiguration(cpacsHandle);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_CLOSE_FAILED;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return TIGL_CLOSE_FAILED;
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglCloseCPACSConfiguration!" << std::endl;
        return TIGL_CLOSE_FAILED;
    }
}


DLL_EXPORT TiglReturnCode tiglGetCPACSTixiHandle(TiglCPACSConfigurationHandle cpacsHandle, TixiDocumentHandle* tixiHandlePtr)
{
    if (tixiHandlePtr == 0) {
        std::cerr << "Error: Null pointer argument for tixiHandlePtr in function call to tiglGetCPACSTixiHandle." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *tixiHandlePtr = config.GetTixiDocumentHandle();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_NOT_FOUND;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return TIGL_NOT_FOUND;
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglGetCPACSTixiHandle!" << std::endl;
        return TIGL_NOT_FOUND;
    }
}


DLL_EXPORT TiglReturnCode tiglIsCPACSConfigurationHandleValid(TiglCPACSConfigurationHandle cpacsHandle, TiglBoolean* isValidPtr)
{
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        *isValidPtr = (manager.IsValid(cpacsHandle) == true) ? TIGL_TRUE : TIGL_FALSE;
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_NOT_FOUND;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return TIGL_NOT_FOUND;
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglIsCPACSConfigurationHandleValid!" << std::endl;
        return TIGL_NOT_FOUND;
    }
}



DLL_EXPORT TiglReturnCode tiglUseAlgorithm(TiglAlgorithmCode anAlgorithm)
{
    try {
        tigl::CTiglAlgorithmManager& manager = tigl::CTiglAlgorithmManager::GetInstance();
        manager.SetAlgorithm(anAlgorithm);
        tigl::CCPACSConfigurationManager& configurations = tigl::CCPACSConfigurationManager::GetInstance();
        configurations.Invalidate();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglUseAlgorithm!" << std::endl;
        return TIGL_ERROR;
    }
}


/**
	gives the tigl version number
*/
DLL_EXPORT char* tiglGetVersion()
{
	static char version[] = TIGL_VERSION;
	return version;
}


/**********************************************************************************************/


DLL_EXPORT TiglReturnCode tiglWingGetUpperPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                int wingIndex,
                                                int segmentIndex,
                                                double eta,
                                                double xsi,
                                                double* pointXPtr,
                                                double* pointYPtr,
                                                double* pointZPtr)
{
    if (pointXPtr == NULL || pointYPtr == NULL || pointZPtr == NULL) {
        std::cerr << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        std::cerr << "in function call to tiglWingGetUpperPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        gp_Pnt point = wing.GetUpperPoint(segmentIndex, eta, xsi);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex)
    {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...)
    {
        std::cerr << "Caught an unknown exception in tiglWingGetUpperPoint" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetLowerPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                int wingIndex,
                                                int segmentIndex,
                                                double eta,
                                                double xsi,
                                                double* pointXPtr,
                                                double* pointYPtr,
                                                double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        std::cerr << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        std::cerr << "in function call to tiglWingGetLowerPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        gp_Pnt point = wing.GetLowerPoint(segmentIndex, eta, xsi);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetLowerPoint!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglGetWingCount(TiglCPACSConfigurationHandle cpacsHandle, int* wingCountPtr)
{
    if (wingCountPtr == 0) {
        std::cerr << "Error: Null pointer argument for wingCountPtr ";
        std::cerr << "in function call to tiglGetWingCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *wingCountPtr = config.GetWingCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglGetWingCount!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                  int wingIndex,
                                                  int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        std::cerr << "Error: Null pointer argument for segmentCountPtr ";
        std::cerr << "in function call to tiglWingGetSegmentCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *segmentCountPtr = wing.GetSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        std::cerr << "Error: Null pointer argument for segmentCountPtr ";
        std::cerr << "in function call to tiglWingGetInnerConnectedSegmentCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *segmentCountPtr = segment.GetInnerConnectedSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetInnerConnectedSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        std::cerr << "Error: Null pointer argument for segmentCountPtr ";
        std::cerr << "in function call to tiglWingGetOuterConnectedSegmentCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *segmentCountPtr = segment.GetOuterConnectedSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetOuterConnectedSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int n,
                                                                int* connectedIndexPtr)
{
    if (connectedIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for connectedIndexPtr ";
        std::cerr << "in function call to tiglWingGetInnerConnectedSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *connectedIndexPtr = segment.GetInnerConnectedSegmentIndex(n);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetInnerConnectedSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int n,
                                                                int* connectedIndexPtr)
{
    if (connectedIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for connectedIndexPtr ";
        std::cerr << "in function call to tiglWingGetOuterConnectedSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *connectedIndexPtr = segment.GetOuterConnectedSegmentIndex(n);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetOuterConnectedSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetInnerSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int* sectionIndexPtr,
                                                                int* elementIndexPtr)
{
    if (wingIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetInnerSectionAndElementIndex." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        std::cerr << "in function call to tiglWingGetInnerSectionAndElementIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *sectionIndexPtr = segment.GetInnerSectionIndex();
        *elementIndexPtr = segment.GetInnerSectionElementIndex();

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetInnerSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetOuterSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int* sectionIndexPtr,
                                                                int* elementIndexPtr)
{
    if (wingIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetOuterSectionAndElementIndex." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        std::cerr << "in function call to tiglWingGetOuterSectionAndElementIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *sectionIndexPtr = segment.GetOuterSectionIndex();
        *elementIndexPtr = segment.GetOuterSectionElementIndex();

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetOuterSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglWingGetInnerSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                char** sectionUIDPtr,
                                                                char** elementUIDPtr)
{
	std::string sectionIndexUID;
	std::string elementIndexUID;

    if (wingIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetInnerSectionAndElementUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);

		sectionIndexUID = segment.GetInnerSectionUID();
        *sectionUIDPtr = (char *) malloc(strlen(sectionIndexUID.c_str()) * sizeof(char) + 1);
        strcpy(*sectionUIDPtr, const_cast<char*>(sectionIndexUID.c_str()));

		elementIndexUID = segment.GetInnerSectionElementUID();
        *elementUIDPtr = (char *) malloc(strlen(elementIndexUID.c_str()) * sizeof(char) + 1);
        strcpy(*elementUIDPtr, const_cast<char*>(elementIndexUID.c_str()));

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetInnerSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetOuterSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                char** sectionUIDPtr,
                                                                char** elementUIDPtr)
{
	std::string sectionIndexUID;
	std::string elementIndexUID;

    if (wingIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetOuterSectionAndElementUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);

		sectionIndexUID = segment.GetOuterSectionUID();
        *sectionUIDPtr = (char *) malloc(strlen(sectionIndexUID.c_str()) * sizeof(char) + 1);
        strcpy(*sectionUIDPtr, const_cast<char*>(sectionIndexUID.c_str()));

		elementIndexUID = segment.GetOuterSectionElementUID();
        *elementUIDPtr = (char *) malloc(strlen(elementIndexUID.c_str()) * sizeof(char) + 1);
        strcpy(*elementUIDPtr, const_cast<char*>(elementIndexUID.c_str()));

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetOuterSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetProfileName(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 int sectionIndex,
                                                 int elementIndex,
                                                 char** profileNamePtr)
{
    if (profileNamePtr == 0) {
        std::cerr << "Error: Null pointer argument for profileNamePtr ";
        std::cerr << "in function call to tiglWingGetProfileName." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || sectionIndex < 1 || elementIndex < 1) {
        std::cerr << "Error: Wing, section or element index index in less than zero ";
        std::cerr << "in function call to tiglWingGetProfileName." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSection& section = wing.GetSection(sectionIndex);
        tigl::CCPACSWingSectionElement& element = section.GetSectionElement(elementIndex);
		std::string profileUID = element.GetProfileIndex();
        tigl::CCPACSWingProfile& profile = config.GetWingProfile(profileUID);

        *profileNamePtr = (char *) malloc(profile.GetName().length() * sizeof(char) + 1);
        strcpy(*profileNamePtr, profile.GetName().c_str());

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetProfileName!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglWingGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        std::cerr << "Error: Null pointer argument for uidNamePtr ";
        std::cerr << "in function call to tiglWingGetUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

	if (wingIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *uidNamePtr = const_cast<char*>(wing.GetUIDPtr());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetUID!" << std::endl;
        return TIGL_ERROR;
    }
}

DLL_EXPORT TiglReturnCode tiglWingGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 int segmentIndex,
                                                 char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        std::cerr << "Error: Null pointer argument for uidNamePtr ";
        std::cerr << "in function call to tiglWingGetSegmentUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

	if (wingIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetSegmentUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *uidNamePtr = const_cast<char*>(segment.GetUIDPtr());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetSegmentUID!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 int sectionIndex,
                                                 char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        std::cerr << "Error: Null pointer argument for uidNamePtr ";
        std::cerr << "in function call to tiglWingGetSectionUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

	if (wingIndex < 1 || sectionIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetSectionUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSection& section = wing.GetSection(sectionIndex);
        *uidNamePtr = const_cast<char*>(section.GetUIDPtr());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetSectionUID!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                TiglSymmetryAxis* symmetryAxisPtr)
{

    if (wingIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglGetWingSymmetry." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *symmetryAxisPtr = wing.GetSymmetryAxis();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglGetWingSymmetry!" << std::endl;
        return TIGL_ERROR;
    }
}


/**********************************************************************************************/


DLL_EXPORT TiglReturnCode tiglGetFuselageCount(TiglCPACSConfigurationHandle cpacsHandle, int* fuselageCountPtr)
{
    if (fuselageCountPtr == 0) {
        std::cerr << "Error: Null pointer argument for fuselageCountPtr ";
        std::cerr << "in function call to tiglGetFuselageCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *fuselageCountPtr = config.GetFuselageCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglGetFuselageCount!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                      int fuselageIndex,
                                                      int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        std::cerr << "Error: Null pointer argument for segmentCountPtr ";
        std::cerr << "in function call to tiglFuselageGetSegmentCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *segmentCountPtr = fuselage.GetSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                               int fuselageIndex,
                                               int segmentIndex,
                                               double eta,
                                               double zeta,
                                               double* pointXPtr,
                                               double* pointYPtr,
                                               double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        std::cerr << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        std::cerr << "in function call to tiglFuselageGetPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        gp_Pnt point = fuselage.GetPoint(segmentIndex, eta, zeta);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetPoint!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglFuselageGetPointAngle(TiglCPACSConfigurationHandle cpacsHandle,
                                                    int fuselageIndex,
                                                    int segmentIndex,
                                                    double eta,
                                                    double alpha,
                                                    double* pointXPtr,
                                                    double* pointYPtr,
                                                    double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        std::cerr << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        std::cerr << "in function call to tiglFuselageGetPointAngle." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
		gp_Pnt point = segment.GetPointAngle(eta, alpha, 0.0, 0.0);		
		if((point.X() == 0.0) && (point.Y() == 0.0) && (point.Z() == 0.0)) {
			return TIGL_ERROR;
		}
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetPointAngle!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetPointAngleTranslated(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int fuselageIndex,
                                                              int segmentIndex,
                                                              double eta,
                                                              double alpha,
                                                              double y_cs,
                                                              double z_cs,
                                                              double* pointXPtr,
                                                              double* pointYPtr,
                                                              double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        std::cerr << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        std::cerr << "in function call to tiglFuselageGetPointAngleTranslated." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        gp_Pnt point = segment.GetPointAngle(eta, alpha, y_cs, z_cs);
		if((point.X() == 0.0) && (point.Y() == 0.0) && (point.Z() == 0.0)) {
			return TIGL_ERROR;
		}
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetPointAngleTranslated!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetPointOnXPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int fuselageIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double xpos,
                                                        int pointIndex,
                                                        double* pointXPtr,
                                                        double* pointYPtr,
                                                        double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        std::cerr << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        std::cerr << "in function call to tiglFuselageGetPointOnXPlane." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        gp_Pnt point = segment.GetPointOnXPlane(eta, xpos, pointIndex);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetPointOnXPlane!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglFuselageGetPointOnYPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int fuselageIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double ypos,
                                                        int pointIndex,
                                                        double* pointXPtr,
                                                        double* pointYPtr,
                                                        double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        std::cerr << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        std::cerr << "in function call to tiglFuselageGetPointOnYPlane." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        gp_Pnt point = segment.GetPointOnXPlane(eta, ypos, pointIndex);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetPointOnYPlane!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetNumPointsOnXPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int fuselageIndex,
                                                           int segmentIndex,
                                                           double eta,
                                                           double xpos,
                                                           int* numPointsPtr)
{
    if (numPointsPtr == 0) {
        std::cerr << "Error: Null pointer argument for numPointsPtr ";
        std::cerr << "in function call to tiglFuselageGetNumPointsOnXPlane." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *numPointsPtr = segment.GetNumPointsOnYPlane(eta, xpos);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetNumPointsOnXPlane!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetNumPointsOnYPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int fuselageIndex,
                                                           int segmentIndex,
                                                           double eta,
                                                           double ypos,
                                                           int* numPointsPtr)
{
    if (numPointsPtr == 0) {
        std::cerr << "Error: Null pointer argument for numPointsPtr ";
        std::cerr << "in function call to tiglFuselageGetNumPointsOnYPlane." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *numPointsPtr = segment.GetNumPointsOnYPlane(eta, ypos);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetNumPointsOnYPlane!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetCircumference(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int fuselageIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double* circumferencePtr)
{
    if (circumferencePtr == 0) {
         std::cerr << "Error: Null pointer argument for circumferencePtr ";
         std::cerr << "in function call to tiglFuselageGetCircumference." << std::endl;
         return TIGL_NULL_POINTER;
     }

     try {
         tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
         tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
         tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
         *circumferencePtr = fuselage.GetCircumference(segmentIndex, eta);
         return TIGL_SUCCESS;
     }
     catch (std::exception& ex) {
         std::cerr << ex.what() << std::endl;
         return TIGL_ERROR;
     }
     catch (tigl::CTiglError& ex) {
         std::cerr << ex.getError() << std::endl;
         return ex.getCode();
     }
     catch (...) {
         std::cerr << "Caught an exception in tiglFuselageGetCircumference!" << std::endl;
         return TIGL_ERROR;
     }
}




DLL_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                    int fuselageIndex,
                                                                    int segmentIndex,
                                                                    int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        std::cerr << "Error: Null pointer argument for segmentCountPtr ";
        std::cerr << "in function call to tiglFuselageGetStartConnectedSegmentCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *segmentCountPtr = segment.GetStartConnectedSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetStartConnectedSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  int fuselageIndex,
                                                                  int segmentIndex,
                                                                  int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        std::cerr << "Error: Null pointer argument for segmentCountPtr ";
        std::cerr << "in function call to tiglFuselageGetEndConnectedSegmentCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *segmentCountPtr = segment.GetEndConnectedSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetEndConnectedSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                    int fuselageIndex,
                                                                    int segmentIndex,
                                                                    int n,
                                                                    int* connectedIndexPtr)
{
    if (connectedIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for connectedIndexPtr ";
        std::cerr << "in function call to tiglFuselageGetStartConnectedSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *connectedIndexPtr = segment.GetStartConnectedSegmentIndex(n);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetStartConnectedSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  int fuselageIndex,
                                                                  int segmentIndex,
                                                                  int n,
                                                                  int* connectedIndexPtr)
{
    if (connectedIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for connectedIndexPtr ";
        std::cerr << "in function call to tiglFuselageGetEndConnectedSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *connectedIndexPtr = segment.GetEndConnectedSegmentIndex(n);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetEndConnectedSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}

DLL_EXPORT TiglReturnCode tiglFuselageGetStartSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                     int fuselageIndex,
                                                                     int segmentIndex,
                                                                     char** sectionUIDPtr,
                                                                     char** elementUIDPtr)
{
	std::string sectionIndexUID;
	std::string elementIndexUID;

    if (fuselageIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: fuselageIndex or segmentIndex argument is less than one ";
        std::cerr << "in function call to tiglFuselageGetStartSectionAndElementUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);

        sectionIndexUID = segment.GetStartSectionUID();
        *sectionUIDPtr = (char *) malloc(strlen(sectionIndexUID.c_str()) * sizeof(char) + 1);
        strcpy(*sectionUIDPtr, const_cast<char*>(sectionIndexUID.c_str()));

        elementIndexUID = segment.GetStartSectionElementUID();
        *elementUIDPtr = (char *) malloc(strlen(elementIndexUID.c_str()) * sizeof(char) + 1);
        strcpy(*elementUIDPtr, const_cast<char*>(elementIndexUID.c_str()));

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetStartSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglFuselageGetEndSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int fuselageIndex,
                                                                   int segmentIndex,
                                                                   char** sectionUIDPtr,
                                                                   char** elementUIDPtr)
{
	std::string sectionIndexUID;
	std::string elementIndexUID;

    if (fuselageIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: fuselageIndex or segmentIndex argument less than one ";
        std::cerr << "in function call to tiglFuselageGetEndSectionAndElementUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);

        sectionIndexUID = segment.GetEndSectionUID();
        *sectionUIDPtr = (char *) malloc(strlen(sectionIndexUID.c_str()) * sizeof(char) + 1);
        strcpy(*sectionUIDPtr, const_cast<char*>(sectionIndexUID.c_str()));

        elementIndexUID = segment.GetEndSectionElementUID();
        *elementUIDPtr = (char *) malloc(strlen(elementIndexUID.c_str()) * sizeof(char) + 1);
        strcpy(*elementUIDPtr, const_cast<char*>(elementIndexUID.c_str()));

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetEndSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglFuselageGetStartSectionAndElementIndex (TiglCPACSConfigurationHandle cpacsHandle,
                                                                     int fuselageIndex,
                                                                     int segmentIndex,
                                                                     int* sectionIndexPtr,
                                                                     int* elementIndexPtr)
{
    if (sectionIndexPtr == 0 || elementIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        std::cerr << "in function call to tiglFuselageGetStartSectionAndElementIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        std::cerr << "in function call to tiglFuselageGetStartSectionAndElementIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
		*sectionIndexPtr = segment.GetStartSectionIndex();
		*elementIndexPtr = segment.GetStartSectionElementIndex();

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetStartSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetEndSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int fuselageIndex,
                                                                   int segmentIndex,
                                                                   int* sectionIndexPtr,
                                                                   int* elementIndexPtr)
{
    if (sectionIndexPtr == 0 || elementIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        std::cerr << "in function call to tiglFuselageGetEndSectionAndElementIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        std::cerr << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        std::cerr << "in function call to tiglFuselageGetEndSectionAndElementIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *sectionIndexPtr = segment.GetEndSectionIndex();
		*elementIndexPtr = segment.GetEndSectionElementIndex();

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetEndSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetProfileName(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int fuselageIndex,
                                                     int sectionIndex,
                                                     int elementIndex,
                                                     char** profileNamePtr)
{
    if (profileNamePtr == 0) {
        std::cerr << "Error: Null pointer argument for profileNamePtr ";
        std::cerr << "in function call to tiglFuselageGetProfileName." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || sectionIndex < 1 || elementIndex < 1) {
        std::cerr << "Error: Fuselage, section or element index index in less than zero ";
        std::cerr << "in function call to tiglFuselageGetProfileName." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSection& section = fuselage.GetSection(sectionIndex);
        tigl::CCPACSFuselageSectionElement& element = section.GetSectionElement(elementIndex);
        std::string profileUID = element.GetProfileIndex();
        tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile(profileUID);

        *profileNamePtr = (char *) malloc(profile.GetName().length() * sizeof(char) + 1);
        strcpy(*profileNamePtr, profile.GetName().c_str());

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetProfileName!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                             int fuselageIndex,
                                             char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        std::cerr << "Error: Null pointer argument for uidNamePtr ";
        std::cerr << "in function call to tiglFuselageGetUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1) {
        std::cerr << "Error: Fuselage or segment index index in less than zero ";
        std::cerr << "in function call to tiglFuselageGetUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *uidNamePtr = const_cast<char*>(fuselage.GetUIDPtr());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetUID!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int fuselageIndex,
                                                 int segmentIndex,
                                                 char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        std::cerr << "Error: Null pointer argument for uidNamePtr ";
        std::cerr << "in function call to tiglFuselageGetSegmentUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Fuselage or segment index index in less than zero ";
        std::cerr << "in function call to tiglFuselageGetSegmentUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *uidNamePtr = const_cast<char*>(segment.GetUIDPtr());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetSegmentUID!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int fuselageIndex,
                                                     int sectionIndex,
                                                     char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        std::cerr << "Error: Null pointer argument for uidNamePtr ";
        std::cerr << "in function call to tiglFuselageGetSectionUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || sectionIndex < 1) {
        std::cerr << "Error: Fuselage or segment index index in less than zero ";
        std::cerr << "in function call to tiglFuselageGetSectionUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSection& section = fuselage.GetSection(sectionIndex);
        *uidNamePtr = const_cast<char*>(section.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetSectionUID!" << std::endl;
        return TIGL_ERROR;
    }
}








DLL_EXPORT TiglReturnCode tiglFuselageGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                TiglSymmetryAxis* symmetryAxisPtr)
{
    if (fuselageIndex < 1) {
        std::cerr << "Error: Fuselage or segment index index is less than zero ";
        std::cerr << "in function call to tiglGetFuselageSymmetry." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *symmetryAxisPtr = fuselage.GetSymmetryAxis();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglGetFuselageSymmetry!" << std::endl;
        return TIGL_ERROR;
    }
}



/*****************************************************************************************************/



DLL_EXPORT TiglReturnCode tiglComponentIntersectionPoint(TiglCPACSConfigurationHandle cpacsHandle,
														 char*  componentUidOne,
														 char*  componentUidTwo,
														 int lineID,
														 double eta,
														 double* pointXPtr,
														 double* pointYPtr,
														 double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        std::cerr << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        std::cerr << "in function call to tiglComponentIntersectionPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }

	if ( (componentUidOne == 0) || (componentUidTwo == 0)) {
		std::cerr << "Error: Null pointer argument for at least one given UID ";
		std::cerr << "in function call to tiglComponentIntersectionPoint." << std::endl;
		return TIGL_NULL_POINTER;
	}


    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

		TopoDS_Shape compoundOne = uidManager.GetComponent(componentUidOne)->GetLoft();
		TopoDS_Shape compoundTwo = uidManager.GetComponent(componentUidTwo)->GetLoft();

		tigl::CTiglIntersectionCalculation Intersector(compoundOne, compoundTwo);
        gp_Pnt point = Intersector.GetPoint(eta, lineID);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglComponentIntersectionPoint!" << std::endl;
        return TIGL_ERROR;
    }
}




DLL_EXPORT TiglReturnCode tiglComponentIntersectionLineCount(TiglCPACSConfigurationHandle cpacsHandle,
																		char*  componentUidOne,
																		char*  componentUidTwo,
																		int* numWires)
{
	if ( (componentUidOne == 0) || (componentUidTwo == 0)) {
		std::cerr << "Error: Null pointer argument for at least one given UID ";
		std::cerr << "in function call to tiglFuselageWingSurfaceIntersectionLineCount." << std::endl;
		return TIGL_NULL_POINTER;
	}

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

		tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        TopoDS_Shape compoundOne = uidManager.GetComponent(componentUidOne)->GetLoft();
        TopoDS_Shape compoundTwo = uidManager.GetComponent(componentUidTwo)->GetLoft();

		tigl::CTiglIntersectionCalculation Intersector(compoundOne, compoundTwo);
		*numWires = Intersector.GetNumWires();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageWingSurfaceIntersectionLineCount!" << std::endl;
        return TIGL_ERROR;
    }
}

/*****************************************************************************************************/
/*                         Export Functions                                                          */
/*****************************************************************************************************/

DLL_EXPORT TiglReturnCode tiglExportIGES(TiglCPACSConfigurationHandle cpacsHandle, char* filenamePtr)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportIGES." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportIges exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportIGES(filename);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportIGES!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglExportFusedWingFuselageIGES(TiglCPACSConfigurationHandle cpacsHandle,
                              char* filenamePtr)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportFusedWingFuselageIGES." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportIges exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportFusedIGES(filename);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportFusedWingFuselageIGES!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglExportMeshedWingSTL(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedWingSTL." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingIndex < 1) {
        std::cerr << "Error: wingIndex in less or equal zero";
        std::cerr << "in function call to tiglExportMeshedWingSTL." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedWingSTL(wingIndex, filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportMeshedWingSTL!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglExportMeshedFuselageSTL(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedFuselageSTL." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndex < 1) {
        std::cerr << "Error: fuselageIndex in less or equal zero";
        std::cerr << "in function call to tiglExportMeshedFuselageSTL." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedFuselageSTL(fuselageIndex, filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportMeshedFuselageSTL!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglExportMeshedGeometrySTL(TiglCPACSConfigurationHandle cpacsHandle, char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedGeometrySTL." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedGeometrySTL(filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportMeshedGeometrySTL!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglExportMeshedWingVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, const int wingIndex,
                                                  const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedWingVTKByIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingIndex < 1) {
        std::cerr << "Error: wingIndex in less or equal zero";
        std::cerr << "in function call to tiglExportMeshedWingVTKByIndex." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedWingVTKByIndex(wingIndex, filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception & ex) // all exceptions from the standard library
    {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError & ex)
    {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch(char *str)
    {
        std::cerr << str << std::endl;
    }
    catch(std::string str)
    {
        std::cerr << str << std::endl;
    }
    catch (...)
    {
        std::cerr << "Caught an unknown exception in tiglExportMeshedWingVTKByIndex" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglExportMeshedWingVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID,
                                                        const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedWingVTKByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        std::cerr << "Error: Null pointer argument for wingUID";
        std::cerr << "in function call to tiglExportMeshedWingVTKByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedWingVTKByUID(wingUID, filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception & ex) // all exceptions from the standard library
    {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError & ex)
    {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch(char *str)
    {
        std::cerr << str << std::endl;
    }
    catch(std::string str)
    {
        std::cerr << str << std::endl;
    }
    catch (...)
    {
        std::cerr << "Caught an unknown exception in tiglExportMeshedWingVTKByUID" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, const int fuselageIndex,
                                                      const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedFuselageVTKByIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndex < 1) {
        std::cerr << "Error: fuselageIndex in less or equal zero";
        std::cerr << "in function call to tiglExportMeshedFuselageVTKByIndex." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedFuselageVTKByIndex(fuselageIndex, filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportMeshedFuselageVTKByIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID,
                                                           const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedFuselageVTKByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        std::cerr << "Error: Null pointer argument for fuselageIndex";
        std::cerr << "in function call to tiglExportMeshedFuselageVTKByUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedFuselageVTKByUID(fuselageUID, filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportMeshedFuselageVTKByUID!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglExportMeshedGeometryVTK(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedGeometryVTK." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedGeometryVTK(filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportMeshedGeometryVTK!" << std::endl;
        return TIGL_ERROR;
    }
}




DLL_EXPORT TiglReturnCode tiglExportMeshedWingVTKSimpleByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID,
                                                             const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedWingVTKSimpleByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        std::cerr << "Error: Null pointer argument for wingUID";
        std::cerr << "in function call to tiglExportMeshedWingVTKSimpleByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedWingVTKSimpleByUID(wingUID, filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception & ex) // all exceptions from the standard library
    {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError & ex)
    {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch(char *str)
    {
        std::cerr << str << std::endl;
    }
    catch(std::string str)
    {
        std::cerr << str << std::endl;
    }
    catch (...)
    {
        std::cerr << "Caught an unknown exception in tiglExportMeshedWingVTKSimpleByUID" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKSimpleByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID,
                                                                   const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedFuselageVTKSimpleByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        std::cerr << "Error: Null pointer argument for fuselageIndex";
        std::cerr << "in function call to tiglExportMeshedFuselageVTKSimpleByUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedFuselageVTKSimpleByUID(fuselageUID, filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportMeshedFuselageVTKSimpleByUID!" << std::endl;
        return TIGL_ERROR;
    }
}



DLL_EXPORT TiglReturnCode tiglExportMeshedGeometryVTKSimple(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        std::cerr << "Error: Null pointer argument for filenamePtr";
        std::cerr << "in function call to tiglExportMeshedGeometryVTKSimple." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedGeometryVTKSimple(filename, deflection);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglExportMeshedGeometryVTKSimple!" << std::endl;
        return TIGL_ERROR;
    }
}




/*****************************************************************************************************/
/*                     Material functions                                                            */
/*****************************************************************************************************/
DLL_EXPORT TiglReturnCode tiglGetMaterialUID(const TiglCPACSConfigurationHandle cpacsHandle, char* segmentUID,
                                             double eta, double xsi, char** uidMaterialPtr)
{
    if (segmentUID == 0) {
        std::cerr << "Error: Null pointer argument for wingUID ";
        std::cerr << "in function call to tiglGetMaterialUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        std::string matUID = "DummyMaterial";
        //*uidMaterialPtr = (char *) malloc(matUID.length() * sizeof(char) + 1);
        //strcpy(*uidMaterialPtr, matUID.c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglGetMaterialUID!" << std::endl;
        return TIGL_ERROR;
    }
}


/*****************************************************************************************************/
/*                     Volume calculations                                                           */
/*****************************************************************************************************/

DLL_EXPORT TiglReturnCode tiglFuselageGetVolume(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                  double *volumePtr)
{
    if (fuselageIndex < 1) {
        std::cerr << "Error: Fuselage index index is less than zero ";
        std::cerr << "in function call to tiglFuselageGetVolume." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *volumePtr = fuselage.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetVolume!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetVolume(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                  double *volumePtr)
{
    if (wingIndex < 1) {
        std::cerr << "Error: Wing index index is less than zero ";
        std::cerr << "in function call to tiglWingGetVolume." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *volumePtr = wing.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetVolume!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                   int wingIndex,
                                                   int segmentIndex,
                                                   double* volumePtr)
{
    if (volumePtr == 0) {
        std::cerr << "Error: Null pointer argument for volumePtr ";
        std::cerr << "in function call to tiglWingGetSegmentVolume." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetSegmentVolume." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *volumePtr = segment.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetSegmentVolume!" << std::endl;
        return TIGL_ERROR;
    }
}

DLL_EXPORT TiglReturnCode tiglFuselageGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                   int fuselageIndex,
                                                   int segmentIndex,
                                                   double* volumePtr)
{
    if (volumePtr == 0) {
        std::cerr << "Error: Null pointer argument for volumePtr ";
        std::cerr << "in function call to tiglFuselageGetSegmentVolume." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Fuselage or segment index index in less than zero ";
        std::cerr << "in function call to tiglFuselageGetSegmentVolume." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment =(tigl::CCPACSFuselageSegment &)  fuselage.GetSegment(segmentIndex);
        *volumePtr = segment.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetSegmentVolume!" << std::endl;
        return TIGL_ERROR;
    }
}

/*****************************************************************************************************/
/*                     Surface Area calculations                                                     */
/*****************************************************************************************************/

DLL_EXPORT TiglReturnCode tiglWingGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
																				double *surfaceAreaPtr)
{
    if (wingIndex < 1) {
        std::cerr << "Error: Wing index index is less than zero ";
        std::cerr << "in function call to tiglWingGetSurfaceArea." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *surfaceAreaPtr = wing.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
																					double *surfaceAreaPtr)
{
    if (fuselageIndex < 1) {
        std::cerr << "Error: Fuselage index index is less than zero ";
        std::cerr << "in function call to tiglFuselageGetSurfaceArea." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *surfaceAreaPtr = fuselage.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglWingGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
														int wingIndex,
														int segmentIndex,
														double* surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        std::cerr << "Error: Null pointer argument for surfaceAreaPtr ";
        std::cerr << "in function call to tiglWingGetSegmentSurfaceArea." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Wing or segment index index in less than zero ";
        std::cerr << "in function call to tiglWingGetSegmentSurfaceArea." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);
        *surfaceAreaPtr = segment.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglWingGetSegmentSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}


DLL_EXPORT TiglReturnCode tiglFuselageGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
															int fuselageIndex,
															int segmentIndex,
															double* surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        std::cerr << "Error: Null pointer argument for surfaceAreaPtr ";
        std::cerr << "in function call to tiglFuselageGetSegmentSurfaceArea." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        std::cerr << "Error: Fuselage or segment index index in less than zero ";
        std::cerr << "in function call to tiglFuselageGetSegmentSurfaceArea." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *surfaceAreaPtr = segment.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        std::cerr << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        std::cerr << "Caught an exception in tiglFuselageGetSegmentSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}


/*****************************************************************************************************/
/*                     Component Utility Functions                                                      */
/*****************************************************************************************************/


DLL_EXPORT TiglReturnCode tiglComponentGetHashCode(TiglCPACSConfigurationHandle cpacsHandle,
												   char* componentUID,
												   int* hashCodePtr)
{
	if (componentUID == 0) {
		std::cerr << "Error: Null pointer argument for componentUID ";
		std::cerr << "in function call to tiglComponentGetHashCode." << std::endl;
		return TIGL_NULL_POINTER;
	}

	if (hashCodePtr == NULL) {
		std::cerr << "Error: Null pointer argument for hashCodePtr ";
		std::cerr << "in function call to tiglComponentGetHashCode." << std::endl;
		return TIGL_NULL_POINTER;
	}

	try {
		tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
		tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

		tigl::CTiglUIDManager& uidManager = config.GetUIDManager();
		int hash = uidManager.GetComponent(componentUID)->GetComponentHashCode();
		*hashCodePtr = hash;

		return TIGL_SUCCESS;
	}
	catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return TIGL_ERROR;
	}
	catch (tigl::CTiglError& ex) {
		std::cerr << ex.getError() << std::endl;
		return ex.getCode();
	}
	catch (...) {
		std::cerr << "Caught an exception in tiglComponentGetHashCode!" << std::endl;
		return TIGL_ERROR;
	}
}
