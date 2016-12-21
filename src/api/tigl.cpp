/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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
#include <cstdlib>

#include "tigl.h"
#include "tigl_version.h"
#include "tigl_config.h"
#include "tiglcommonfunctions.h"
#include "tigl_error_strings.h"
#include "CTiglTypeRegistry.h"
#include "CTiglError.h"
#include "CTiglIntersectionCalculation.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglIntersectionCalculation.h"
#include "CTiglUIDManager.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CTiglExportIges.h"
#include "CTiglExportStep.h"
#include "CTiglExportStl.h"
#include "CTiglExportVtk.h"
#include "CTiglExportCollada.h"
#include "CTiglExportBrep.h"
#include "CTiglLogging.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSFuselageSegment.h"
#include "PNamedShape.h"
#include "CNamedShape.h"

#include "gp_Pnt.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Edge.hxx"

/*****************************************************************************/
/* Private functions.                                                 */
/*****************************************************************************/

namespace
{
    static std::string version = std::string(TIGL_REVISION).size() > 0 ?
        TIGL_VERSION_STRING " rev" TIGL_REVISION :
        TIGL_VERSION_STRING;
}

TixiPrintMsgFnc oldTixiMessageHandler = NULL;

namespace
{

    void tiglCleanup(void);
    bool tiglInit(void);
    void TixiMessageHandler(MessageType type, const char *message);


    bool tiglInit(void)
    {
        atexit(tiglCleanup);
    
        // Initialize logger
        tigl::CTiglLogging::Instance();
        // Register dynamic tigl types
        tigl::CTiglTypeRegistry::Init();
        if (tixiGetPrintMsgFunc() != TixiMessageHandler) {
            oldTixiMessageHandler = tixiGetPrintMsgFunc();
        }
        tixiSetPrintMsgFunc(TixiMessageHandler);
        return true;
    }
    
    void tiglCleanup(void)
    {
    }
    
    // This function pulls all tixi messages, puts them into the tigl
    // log and pushes them back to tixi
    void TixiMessageHandler(MessageType type, const char *message)
    {
        std::string cppMessage(message);

        // reroute back to tixi
        if (oldTixiMessageHandler && oldTixiMessageHandler != TixiMessageHandler) {
            oldTixiMessageHandler(type, cppMessage.c_str());
        }

        cppMessage = "[TiXI] " + cppMessage;

        if (type == MESSAGETYPE_ERROR) {
            LOG(ERROR) << cppMessage;
        }
        else if (type == MESSAGETYPE_WARNING) {
            LOG(WARNING) << cppMessage;
        }
        else {
            LOG(INFO) << cppMessage;
        }

    }

}

// make tigl initialize on start
const bool tiglInitialized = tiglInit();

/*****************************************************************************/
/* Public visible functions.                                                 */
/*****************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglOpenCPACSConfiguration(TixiDocumentHandle tixiHandle, const char* configurationUID_cstr, TiglCPACSConfigurationHandle* cpacsHandlePtr)
{
    tiglInit();

    std::string configurationUID;
    if (configurationUID_cstr) {
        configurationUID = configurationUID_cstr;
    }
    
    if (cpacsHandlePtr == 0) {
        LOG(ERROR) << "Null pointer argument for cpacsHandlePtr in function call to tiglOpenCPACSConfiguration." << std::endl;
        return TIGL_NULL_POINTER;
    }

    /* check TIXI Version */
    if ( atof(tixiGetVersion()) < atof(tiglGetVersion()) ) {
        LOG(ERROR) << "Incompatible TIXI Version in use with this TIGL" << std::endl;
        return TIGL_WRONG_TIXI_VERSION;
    }

    /* check CPACS Version */
    {
        double dcpacsVersion = 1.0;
        ReturnCode tixiRet = tixiGetDoubleElement(tixiHandle, "/cpacs/header/cpacsVersion", &dcpacsVersion);
        if (tixiRet != SUCCESS) {
            // NO CPACS Version Information in Header
            if (tixiRet == ELEMENT_PATH_NOT_UNIQUE) {
                LOG(ERROR) << "Multiple CPACS version entries found. Please verify CPACS file." << std::endl;
            }
            else if (tixiRet == ELEMENT_NOT_FOUND) {
                LOG(ERROR) << "No CPACS version information in file header. CPACS file seems to be too old." << std::endl;
            }
            else {
                LOG(ERROR) << "Cannot read CPACS version," << std::endl;
            }
            return TIGL_WRONG_CPACS_VERSION;
        }
        else {
            if (dcpacsVersion < (double) TIGL_MAJOR_VERSION) {
                LOG(ERROR) << "Too old CPACS dataset. CPACS version has to be at least " << (double) TIGL_MAJOR_VERSION << "!" << std::endl;
                return TIGL_WRONG_CPACS_VERSION;
            }
            else if (dcpacsVersion > atof(tiglGetVersion())) {
                LOG(WARNING) << "CPACS dataset version is higher than TIGL library version!";
            }
        }
    }

    /* check if there is only one configuration in the data set. Then we open this */
    /* configuration automatically */
    if (configurationUID == "") {
        ReturnCode    tixiRet;
        int modelCount = 0;

        tixiRet = tixiGetNamedChildrenCount(tixiHandle, "/cpacs/vehicles/aircraft|/cpacs/vehicles/rotorcraft", "model", &modelCount);
        if (tixiRet != SUCCESS) {
            LOG(ERROR) << "No configuration specified!" << std::endl;
            return TIGL_ERROR;
        }
        char * tmpConfUID = NULL;
        tixiRet = tixiGetTextAttribute(tixiHandle, "/cpacs/vehicles/aircraft/model[1]", "uID", &tmpConfUID);
        if (tixiRet != SUCCESS) {
            tixiRet = tixiGetTextAttribute(tixiHandle, "/cpacs/vehicles/rotorcraft/model[1]", "uID", &tmpConfUID);
            if (tixiRet != SUCCESS) {
                LOG(ERROR) << "Problems reading configuration-uid!" << std::endl;
                return TIGL_ERROR;
            }
        }
        configurationUID = tmpConfUID;
    }
    else {
        /* Check if configuration exists */
        char *tmpString = NULL;
        char *tmpString2 = NULL;

        tixiUIDGetXPath(tixiHandle, configurationUID.c_str(), &tmpString2);
        std::string ConfigurationXPath = tmpString2;
        ConfigurationXPath += "[@uID=\"";
        ConfigurationXPath += configurationUID.c_str();
        ConfigurationXPath += "\"]";
        int tixiReturn = tixiGetTextElement( tixiHandle, ConfigurationXPath.c_str(), &tmpString);
        if (tixiReturn != 0) {
            LOG(ERROR) << "Configuration '" << configurationUID << "' not found!" << std::endl;
            return TIGL_ERROR;
        }
    }

    tigl::CCPACSConfiguration* config = 0;
    try {
        config = new tigl::CCPACSConfiguration(tixiHandle);
        // Build CPACS memory structure
        config->ReadCPACS(configurationUID.c_str());
        // Store configuration in handle container
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        *cpacsHandlePtr = manager.AddConfiguration(config);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        delete config;
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_OPEN_FAILED;
    }
    catch (tigl::CTiglError& ex) {
        delete config;
        LOG(ERROR) << ex.getError() << std::endl;
        return TIGL_OPEN_FAILED;
    }
    catch (...) {
        delete config;
        LOG(ERROR) << "Caught an exception in tiglOpenCPACSConfiguration!" << std::endl;
        return TIGL_OPEN_FAILED;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglSaveCPACSConfiguration(const char* configurationUID, TiglCPACSConfigurationHandle cpacsHandle)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();

    if (!manager.IsValid(cpacsHandle)) {
        LOG(ERROR) << "Invalid cpacsHandle passed to tiglSaveCPACSConfiguration!" << std::endl;
        return TIGL_UNINITIALIZED;
    }

    tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

    try {
        config.WriteCPACS(configurationUID);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglSaveCPACSConfiguration!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglCloseCPACSConfiguration(TiglCPACSConfigurationHandle cpacsHandle)
{
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        manager.DeleteConfiguration(cpacsHandle);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_CLOSE_FAILED;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return TIGL_CLOSE_FAILED;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglCloseCPACSConfiguration!" << std::endl;
        return TIGL_CLOSE_FAILED;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglGetCPACSTixiHandle(TiglCPACSConfigurationHandle cpacsHandle, TixiDocumentHandle* tixiHandlePtr)
{
    if (tixiHandlePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for tixiHandlePtr in function call to tiglGetCPACSTixiHandle." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *tixiHandlePtr = config.GetTixiDocumentHandle();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_NOT_FOUND;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return TIGL_NOT_FOUND;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetCPACSTixiHandle!" << std::endl;
        return TIGL_NOT_FOUND;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglIsCPACSConfigurationHandleValid(TiglCPACSConfigurationHandle cpacsHandle, TiglBoolean* isValidPtr)
{
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        *isValidPtr = (manager.IsValid(cpacsHandle) == true) ? TIGL_TRUE : TIGL_FALSE;
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_NOT_FOUND;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return TIGL_NOT_FOUND;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIsCPACSConfigurationHandleValid!" << std::endl;
        return TIGL_NOT_FOUND;
    }
}


/**
* gives the tigl version number
*/
TIGL_COMMON_EXPORT const char* tiglGetVersion()
{
    return version.c_str();
}

/*** General geometry function ***/


TIGL_COMMON_EXPORT TiglReturnCode tiglProfileGetBSplineCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                             const char* profileUID,
                                                             int* curveCount)
{
    // input data check
    if (!profileUID) {
        LOG(ERROR) << "Null pointer for argument profileUID in tiglProfileGetBSplineCount";
        return TIGL_NULL_POINTER;
    }
    if (!curveCount) {
        LOG(ERROR) << "Null pointer for argument curveCount in tiglProfileGetBSplineCount";
        return TIGL_NULL_POINTER;
    }
    
    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(cpacsHandle);
        
        if (config.HasWingProfile(profileUID)) {
            tigl::CCPACSWingProfile& profile = config.GetWingProfile(profileUID);
            int edgeCount = GetNumberOfEdges(profile.GetUpperWire()) 
                          + GetNumberOfEdges(profile.GetLowerWire());
            *curveCount = edgeCount;
            
            return TIGL_SUCCESS;
        }
        else if (config.HasFuselageProfile(profileUID)) {
            tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile(profileUID);
            int edgeCount = GetNumberOfEdges(profile.GetWire(false));
            *curveCount = edgeCount;
    
            return TIGL_SUCCESS;
        }
        else {
            LOG(ERROR) << "Profile " << profileUID << " not found in cpacs file.";
            return TIGL_UID_ERROR;
        }
    }
    catch (tigl::CTiglError& err) {
        LOG(ERROR) << err.getError();
        return err.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Unknown error in tiglProfileGetBSplineCount";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglProfileGetBSplineDataSizes(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 const char* profileUID,
                                                                 int curveid,
                                                                 int* degree,
                                                                 int* ncontrolPoints,
                                                                 int* nKnots)
{
    // input data check
    if (!profileUID) {
        LOG(ERROR) << "Null pointer for argument profileUID in tiglProfileGetBSplineDataSizes";
        return TIGL_NULL_POINTER;
    }
    if (!degree) {
        LOG(ERROR) << "Null pointer for argument degree in tiglProfileGetBSplineDataSizes";
        return TIGL_NULL_POINTER;
    }
    if (!ncontrolPoints) {
        LOG(ERROR) << "Null pointer for argument ncontrolPoints in tiglProfileGetBSplineDataSizes";
        return TIGL_NULL_POINTER;
    }
    if (!nKnots) {
        LOG(ERROR) << "Null pointer for argument nKnots in tiglProfileGetBSplineDataSizes";
        return TIGL_NULL_POINTER;
    }
    if (curveid <= 0) {
        LOG(ERROR) << "Negative index of argument curveid in tiglProfileGetBSplineDataSizes";
        return TIGL_INDEX_ERROR;
    }
    
    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(cpacsHandle);
        
        TopoDS_Edge e;
        
        // query wings
        if (config.HasWingProfile(profileUID)) {
            tigl::CCPACSWingProfile& profile = config.GetWingProfile(profileUID);
            int nel = GetNumberOfEdges(profile.GetLowerWire());
            int neu = GetNumberOfEdges(profile.GetUpperWire());
            
            if (curveid <= nel) {
                e = GetEdge(profile.GetLowerWire(), curveid - 1);
            }
            else if (curveid <= nel + neu) {
                e = GetEdge(profile.GetUpperWire(), curveid - nel - 1);
            }
            else {
                return TIGL_INDEX_ERROR;
            }
        }
        // query fuselages
        else if (config.HasFuselageProfile(profileUID)) {
            tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile(profileUID);
            TopoDS_Wire wire = profile.GetWire(false);
            int ne = GetNumberOfEdges(wire);
            
            if (curveid <= ne) {
                e = GetEdge(wire, curveid - 1);
            }
            else {
                return TIGL_INDEX_ERROR;
            }
        }
        // no profile found
        else {
            LOG(ERROR) << "Profile " << profileUID << " not found in cpacs file.";
            return TIGL_UID_ERROR;
        }
        
        // profile found, lets get bspline data
        
        if (!e.IsNull()) {
             Handle(Geom_BSplineCurve) bspl = GetBSplineCurve(e);
             *degree = bspl->Degree();
             *ncontrolPoints = bspl->NbPoles();
             
             *nKnots = 0;
             for (int i = 1; i <= bspl->NbKnots(); ++i) {
                 *nKnots += bspl->Multiplicity(i);
             }
             return TIGL_SUCCESS;
        }
        else {
            // this should normally not happen
            return TIGL_ERROR;
        }
    }
    catch (tigl::CTiglError& err) {
        LOG(ERROR) << err.getError();
        return err.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Unknown error in tiglProfileGetBSplineCount";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglProfileGetBSplineData(TiglCPACSConfigurationHandle cpacsHandle,
                                                            const char* profileUID,
                                                            int curveid,
                                                            int ncp, double* cpx, double* cpy, double* cpz,
                                                            int nk, double* knots)
{
    // input data check
    if (!profileUID) {
        LOG(ERROR) << "Null pointer for argument profileUID in tiglProfileGetBSplineData";
        return TIGL_NULL_POINTER;
    }
    if (!cpx || !cpy || !cpz) {
        LOG(ERROR) << "Null pointer for control polygon argument in tiglProfileGetBSplineData";
        return TIGL_NULL_POINTER;
    }
    if (!knots) {
        LOG(ERROR) << "Null pointer for argument ncontrolPoints in tiglProfileGetBSplineData";
        return TIGL_NULL_POINTER;
    }
    if (curveid <= 0) {
        LOG(ERROR) << "Negative index of argument curveid in tiglProfileGetBSplineData";
        return TIGL_INDEX_ERROR;
    }
    
    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(cpacsHandle);
        
        TopoDS_Edge e;
        
        // query wings
        if (config.HasWingProfile(profileUID)) {
            tigl::CCPACSWingProfile& profile = config.GetWingProfile(profileUID);
            int nel = GetNumberOfEdges(profile.GetLowerWire());
            int neu = GetNumberOfEdges(profile.GetUpperWire());
            
            if (curveid <= nel) {
                e = GetEdge(profile.GetLowerWire(), curveid - 1);
            }
            else if (curveid <= nel + neu) {
                e = GetEdge(profile.GetUpperWire(), curveid - nel - 1);
            }
            else {
                return TIGL_INDEX_ERROR;
            }
        }
        // query fuselages
        else if (config.HasFuselageProfile(profileUID)) {
            tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile(profileUID);
            TopoDS_Wire wire = profile.GetWire(false);
            int ne = GetNumberOfEdges(wire);
            
            if (curveid <= ne) {
                e = GetEdge(wire, curveid - 1);
            }
            else {
                return TIGL_INDEX_ERROR;
            }
        }
        // no profile found
        else {
            LOG(ERROR) << "Profile " << profileUID << " not found in cpacs file.";
            return TIGL_UID_ERROR;
        }
        
        // profile found, lets get bspline data
        
        if (!e.IsNull()) {
            Handle(Geom_BSplineCurve) bspl = GetBSplineCurve(e);
            // check correct sizes
            if (ncp != bspl->NbPoles()) {
                return TIGL_ERROR;
            }
            
            int nKnots = 0;
            for (int i = 1; i <= bspl->NbKnots(); ++i) {
                nKnots += bspl->Multiplicity(i);
            }
            
            if (nk != nKnots) {
                return TIGL_ERROR;
            }

            // copy control points
            for (int icp = 0; icp < bspl->NbPoles(); ++icp) {
                gp_Pnt p = bspl->Pole(icp+1);
                cpx[icp] = p.X();
                cpy[icp] = p.Y();
                cpz[icp] = p.Z();
            }
            
            // copy knots
            int ipos = 0;
            for (int iknot = 1; iknot <= bspl->NbKnots(); ++iknot) {
                double knot = bspl->Knot(iknot);
                for (int imult = 0; imult < bspl->Multiplicity(iknot); ++imult) {
                    knots[ipos] = knot;
                    ipos++;
                }
            }
            
            return TIGL_SUCCESS;
        }
        else {
            // this should normally not happen
            return TIGL_ERROR;
        }
    }
    catch (tigl::CTiglError& err) {
        LOG(ERROR) << err.getError();
        return err.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Unknown error in tiglProfileGetBSplineCount";
        return TIGL_ERROR;
    }
}


/******************************************************************************/
/* Wing Functions                                                             */
/******************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetUpperPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double xsi,
                                                        double* pointXPtr,
                                                        double* pointYPtr,
                                                        double* pointZPtr)
{
    if (pointXPtr == NULL || pointYPtr == NULL || pointZPtr == NULL) {
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglWingGetUpperPoint." << std::endl;
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
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingGetUpperPoint" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetLowerPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double xsi,
                                                        double* pointXPtr,
                                                        double* pointYPtr,
                                                        double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglWingGetLowerPoint." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetLowerPoint!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetChordPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double xsi,
                                                        double* pointXPtr,
                                                        double* pointYPtr,
                                                        double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglWingGetChordPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);
        
        gp_Pnt point = segment.GetChordPoint(eta, xsi);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetChordPoint!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetChordNormal(TiglCPACSConfigurationHandle cpacsHandle,
                                                         int wingIndex,
                                                         int segmentIndex,
                                                         double eta,
                                                         double xsi,
                                                         double* normalXPtr,
                                                         double* normalYPtr,
                                                         double* normalZPtr)
{
    if (normalXPtr == 0 || normalYPtr == 0 || normalZPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for normalXPtr, normalYPtr or normalZPtr ";
        LOG(ERROR) << "in function call to tiglWingGetChordNormal." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);
        
        gp_Pnt normal = segment.GetChordNormal(eta, xsi);
        // normalize normal
        double len = normal.X()*normal.X() + normal.Y()*normal.Y() + normal.Z()*normal.Z();
        len = sqrt(len);
        if (len < 1e-7) {
            LOG(ERROR) << "Computed normal vector on wing chord face is zero";
            return TIGL_ERROR;
        }
        
        *normalXPtr = normal.X()/len;
        *normalYPtr = normal.Y()/len;
        *normalZPtr = normal.Z()/len;
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetChordNormal!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetUpperPointAtDirection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int wingIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double xsi,
                                                                   double dirx,
                                                                   double diry,
                                                                   double dirz,
                                                                   double* pointXPtr,
                                                                   double* pointYPtr,
                                                                   double* pointZPtr,
                                                                   double* errorDistance)
{
    if (pointXPtr == NULL || pointYPtr == NULL || pointZPtr == NULL) {
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr "
                   << "in function call to tiglWingGetUpperPointAtDirection.";
        return TIGL_NULL_POINTER;
    }
    
    if (errorDistance == NULL) {
        LOG(ERROR) << "Null pointer in errorDistance argument in tiglWingGetUpperPointAtDirection.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);
   
        gp_Pnt point = segment.GetPointDirection(eta, xsi, dirx, diry, dirz, true, *errorDistance);
        
        if (*errorDistance > 1e-3) {
            LOG(WARNING) << "The wing's upper skin is missed by more than 1cm  in tiglWingGetUpperPointAtDirection.";
        }
        
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingGetUpperPointAtDirection" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetLowerPointAtDirection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int wingIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double xsi,
                                                                   double dirx,
                                                                   double diry,
                                                                   double dirz,
                                                                   double* pointXPtr,
                                                                   double* pointYPtr,
                                                                   double* pointZPtr,
                                                                   double* errorDistance)
{
    if (pointXPtr == NULL || pointYPtr == NULL || pointZPtr == NULL) {
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr "
                   << "in function call to tiglWingGetLowerPointAtDirection.";
        return TIGL_NULL_POINTER;
    }
    
    if (errorDistance == NULL) {
        LOG(ERROR) << "Null pointer in errorDistance argument in tiglWingGetLowerPointAtDirection.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);

        gp_Pnt point = segment.GetPointDirection(eta, xsi, dirx, diry, dirz, false, *errorDistance);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        
        if (*errorDistance > 1e-3) {
            LOG(WARNING) << "The wing's lower skin is missed by more than 1cm  in tiglWingGetLowerPointAtDirection.";
        }

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingGetLowerPointAtDirection" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int wingIndex,
                                                           double pointX,
                                                           double pointY,
                                                           double pointZ,
                                                           int* segmentIndex,
                                                           double* eta,
                                                           double* xsi,
                                                           int* isOnTop)
{
    if (eta == NULL || xsi == NULL || isOnTop == NULL) {
        LOG(ERROR) << "Error: Null pointer argument for eta, xsi or isOnTop ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentEtaXsi." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        bool onTop = false;
        *segmentIndex = wing.GetSegmentEtaXsi(gp_Pnt(pointX, pointY, pointZ),*eta, *xsi, onTop);
        if (*segmentIndex <= 0) {
            return TIGL_NOT_FOUND;
        }
        *isOnTop = onTop;

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingGetSegmentEtaXsi" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglGetWingCount(TiglCPACSConfigurationHandle cpacsHandle, int* wingCountPtr)
{
    if (wingCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingCountPtr ";
        LOG(ERROR) << "in function call to tiglGetWingCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *wingCountPtr = config.GetWingCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetWingCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                          int wingIndex,
                                                          int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentCountPtr ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentCount." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int wingIndex,
                                                                   int* compSegmentCountPtr)
{
    if (compSegmentCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for compSegmentCountPtr ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *compSegmentCountPtr = wing.GetComponentSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetComponentSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 int wingIndex,
                                                                 int compSegmentIndex,
                                                                 char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for uidNamePtr ";
        LOG(ERROR) << "in function call to tiglWingGetComponentSegmentUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || compSegmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetComponentSegmentUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(compSegmentIndex);
        *uidNamePtr = const_cast<char*>(segment.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetComponentSegmentUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int wingIndex,
                                                                   const char * compSegmentUID,
                                                                   int * segmentIndex) 
{
    if (compSegmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentUID ";
        LOG(ERROR) << "in function call to tiglWingGetComponentSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (segmentIndex == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentIndex ";
        LOG(ERROR) << "in function call to tiglWingGetComponentSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);

        int nseg = wing.GetComponentSegmentCount();
        for (int i = 1; i <= nseg; ++i) {
            tigl::CTiglAbstractSegment& actSegment = wing.GetComponentSegment(i);
            if ( actSegment.GetUID() == std::string(compSegmentUID)) {
                *segmentIndex = i;
                return TIGL_SUCCESS;
            }
        }

        LOG(ERROR) << "Error in tiglWingGetComponentSegmentIndex: the wing with index=" << wingIndex << " has no segment with an UID="
                   << compSegmentUID << "!" << std::endl;
        *segmentIndex = -1;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentGetSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentCountPtr ";
        LOG(ERROR) << "in function call to tiglWingGetInnerConnectedSegmentCount." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetInnerConnectedSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentCountPtr ";
        LOG(ERROR) << "in function call to tiglWingGetOuterConnectedSegmentCount." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetOuterConnectedSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int n,
                                                                        int* connectedIndexPtr)
{
    if (connectedIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for connectedIndexPtr ";
        LOG(ERROR) << "in function call to tiglWingGetInnerConnectedSegmentIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetInnerConnectedSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int n,
                                                                        int* connectedIndexPtr)
{
    if (connectedIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for connectedIndexPtr ";
        LOG(ERROR) << "in function call to tiglWingGetOuterConnectedSegmentIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetOuterConnectedSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         int wingIndex,
                                                                         int segmentIndex,
                                                                         int* sectionIndexPtr,
                                                                         int* elementIndexPtr)
{
    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetInnerSectionAndElementIndex." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        LOG(ERROR) << "in function call to tiglWingGetInnerSectionAndElementIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetInnerSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         int wingIndex,
                                                                         int segmentIndex,
                                                                         int* sectionIndexPtr,
                                                                         int* elementIndexPtr)
{
    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetOuterSectionAndElementIndex." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        LOG(ERROR) << "in function call to tiglWingGetOuterSectionAndElementIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetOuterSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                       int wingIndex,
                                                                       int segmentIndex,
                                                                       char** sectionUIDPtr,
                                                                       char** elementUIDPtr)
{

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetInnerSectionAndElementUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);

        *sectionUIDPtr = (char *) segment.GetInnerSectionUID().c_str();

        *elementUIDPtr = (char *) segment.GetInnerSectionElementUID().c_str();

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetInnerSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                       int wingIndex,
                                                                       int segmentIndex,
                                                                       char** sectionUIDPtr,
                                                                       char** elementUIDPtr)
{

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetOuterSectionAndElementUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);

        *sectionUIDPtr = (char *) segment.GetOuterSectionUID().c_str();
        *elementUIDPtr = (char *) segment.GetOuterSectionElementUID().c_str();

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetOuterSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetProfileName(TiglCPACSConfigurationHandle cpacsHandle,
                                                         int wingIndex,
                                                         int sectionIndex,
                                                         int elementIndex,
                                                         char** profileNamePtr)
{
    if (profileNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for profileNamePtr ";
        LOG(ERROR) << "in function call to tiglWingGetProfileName." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || sectionIndex < 1 || elementIndex < 1) {
        LOG(ERROR) << "Error: Wing, section or element index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetProfileName." << std::endl;
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

        *profileNamePtr = const_cast<char*>(profile.GetName().c_str());

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetProfileName!" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for uidNamePtr ";
        LOG(ERROR) << "in function call to tiglWingGetUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *uidNamePtr = const_cast<char*> (wing.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                   const char * wingUID,
                                                   int* wingIndexPtr)
{
    if (wingUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUID ";
        LOG(ERROR) << "in function call to tiglWingGetIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingIndexPtr ";
        LOG(ERROR) << "in function call to tiglWingGetIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *wingIndexPtr = config.GetWingIndex(std::string(wingUID));
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        *wingIndexPtr = -1;
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetIndex!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for uidNamePtr ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *uidNamePtr = const_cast<char*>(segment.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                          const char * segmentUID,
                                                          int * segmentIndex, 
                                                          int * wingIndex) 
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentUID ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (segmentIndex == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentIndex ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingIndex == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingIndex ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        
        for (int iWing = 1; iWing <= config.GetWingCount(); ++iWing) {
            tigl::CCPACSWing& wing = config.GetWing(iWing);
            try {
                int wingSegIndex = wing.GetSegment(segmentUID).GetSegmentIndex();
                *segmentIndex = wingSegIndex;
                *wingIndex = iWing;
                return TIGL_SUCCESS;
            }
            catch (tigl::CTiglError&) {
                continue;
            }
        }

        LOG(ERROR) << "Error in tiglWingGetSegmentIndex: could not find a wing index with given uid \"" << segmentUID << "\".";
        *segmentIndex = -1;
        *wingIndex = -1;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSectionCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                          int wingIndex,
                                                          int* sectionCount)
{
    if (sectionCount == 0) {
        LOG(ERROR) << "Error: Null pointer argument for sectionCount "
                   << "in function call to tiglWingGetSectionCount.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1) {
        LOG(ERROR) << "Error: Wing index is less than zero "
                   << "in function call to tiglWingGetSectionCount.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *sectionCount = wing.GetSectionCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSectionCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int sectionIndex,
                                                        char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for uidNamePtr "
                   << "in function call to tiglWingGetSectionUID.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || sectionIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index is less than zero "
                   << "in function call to tiglWingGetSectionUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSection& section = wing.GetSection(sectionIndex);
        *uidNamePtr = const_cast<char*>(section.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSectionUID!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, 
                                                      int wingIndex,
                                                      TiglSymmetryAxis* symmetryAxisPtr)
{

    if (wingIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglGetWingSymmetry." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetWingSymmetry!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentFindSegment(TiglCPACSConfigurationHandle cpacsHandle,
                                                                      const char *componentSegmentUID, double x, double y,
                                                                      double z, char** segmentUID, char** wingUID)
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentFindSegment." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentFindSegment." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // iterate through wings and find componentSegment

        for (int wingIndex=1; wingIndex <= config.GetWingCount(); wingIndex++) {
            tigl::CCPACSWing& wing = config.GetWing(wingIndex);

            for (int componentSegment = 1; componentSegment <= wing.GetComponentSegmentCount(); componentSegment++) {
                tigl::CCPACSWingComponentSegment& cs = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(componentSegment);
                if ( cs.GetUID() == componentSegmentUID) {
                    gp_Pnt nearestPointOnSegment;
                    double distance = 0;
                    const tigl::CTiglAbstractSegment* segment =  cs.findSegment(x, y, z, nearestPointOnSegment, distance);
                    if (!segment) {
                        LOG(ERROR) << "Can not find any segment that does belong to the provided point.";
                        return TIGL_NOT_FOUND;
                    }
                    if (distance > 1e-2) {
                        // point does not lie on component segment
                        LOG(ERROR) << "Given point does not lie on component segment within 1cm tolerance."
                                   << " Diviation is " << distance*1000. << " mm.";
                        return TIGL_NOT_FOUND;
                    }

                    *segmentUID = (char*) segment->GetUID().c_str();
                    *wingUID    = (char*) wing.GetUID().c_str();

                    return TIGL_SUCCESS;
                }
            }
        }
        return TIGL_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentFindSegment!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   const char *componentSegmentUID, double eta, double xsi,
                                                                   double * x, double * y, double * z)
{
    if (!componentSegmentUID) {
        LOG(ERROR) << "Error: Null pointer argument for componentSegmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentPointGetPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    if (!x || !y || !z){
        LOG(ERROR) << "Error: Null pointer argument for x, y, or z ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentPointGetPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            int ncompSegs = wing.GetComponentSegmentCount();
            for (int jcompSeg = 1; jcompSeg <= ncompSegs; ++jcompSeg){
                tigl::CCPACSWingComponentSegment & compSeg = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(jcompSeg);
                if ( compSeg.GetUID() == std::string(componentSegmentUID) ) {
                    //now do the calculations
                    gp_Pnt pnt = compSeg.GetPoint(eta, xsi);
                    *x = pnt.X();
                    *y = pnt.Y();
                    *z = pnt.Z();

                    return TIGL_SUCCESS;
                }
            }
        }

        // the component segment was not found
        LOG(ERROR) << "Error: Invalid uid in tiglWingComponentSegmentPointGetPoint" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentPointGetPoint!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentPointGetSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                const char *componentSegmentUID, double eta, double xsi,
                                                                                char** wingUID, char** segmentUID,
                                                                                double *segmentEta, double *segmentXsi,
                                                                                double *errorDistance)
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentUID "
                   << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }

    if (wingUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUID "
                   << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }
    
    if (errorDistance == 0) {
        LOG(ERROR) << "Error: Null pointer argument for errorDistance "
                   << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            int ncompSegs = wing.GetComponentSegmentCount();
            for (int jcompSeg = 1; jcompSeg <= ncompSegs; ++jcompSeg) {
                tigl::CCPACSWingComponentSegment & compSeg = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(jcompSeg);
                if ( compSeg.GetUID() == std::string(componentSegmentUID) ) {
                    //now do the calculations
                    gp_Pnt pnt = compSeg.GetPoint(eta, xsi);
                    *segmentXsi = xsi;


                    gp_Pnt nearestPoint;
                    double tmp = 0;
                    tigl::CCPACSWingSegment* segment = (tigl::CCPACSWingSegment*) compSeg.findSegment(pnt.X(), pnt.Y(), pnt.Z(), nearestPoint, tmp);
                    if (!segment) {
                        LOG(ERROR) << "Can not find any segment that does belong to the provided point.";
                        return TIGL_MATH_ERROR;
                    }
                    *errorDistance = nearestPoint.Distance(pnt);
                    // warn if cs point is more than 1mm outside from segment
                    if ( *errorDistance > 1e-3) {
                        LOG(WARNING) << "Given point is located more than 1mm from the wing component segment body."
                                     << " The actual diviation is " << *errorDistance*1000.  << " mm."
                                     << " The point will be projected onto the wing segment.";
                        segment->GetEtaXsi(nearestPoint, *segmentEta, *segmentXsi);
                    }
                    else {
                        *segmentEta = segment->GetEta(pnt, *segmentXsi);
                    }

                    *segmentUID = (char*) segment->GetUID().c_str();
                    *wingUID    = (char*) wing.GetUID().c_str();

                    return TIGL_SUCCESS;
                }
            }
        }

        // the component segment was not found
        LOG(ERROR) << "Error: Invalid uid in tiglWingComponentSegmentPointGetSegmentEtaXsi" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentPointGetSegmentEtaXsi!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingSegmentPointGetComponentSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                const char *segmentUID, const char * componentSegmentUID, 
                                                                                double segmentEta, double segmentXsi,
                                                                                double *eta, double *xsi)
{
                                    
    if (segmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentUID ";
        LOG(ERROR) << "in function call to tiglWingSegmentPointGetComponentSegmentEtaXsi." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for componentSegmentUID ";
        LOG(ERROR) << "in function call to tiglWingSegmentPointGetComponentSegmentEtaXsi." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    if (eta == 0) {
        LOG(ERROR) << "Error: Null pointer argument for eta ";
        LOG(ERROR) << "in function call to tiglWingSegmentPointGetComponentSegmentEtaXsi." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    if (xsi == 0) {
        LOG(ERROR) << "Error: Null pointer argument for xsi ";
        LOG(ERROR) << "in function call to tiglWingSegmentPointGetComponentSegmentEtaXsi." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(componentSegmentUID);
                compSeg.GetEtaXsiFromSegmentEtaXsi(segmentUID, segmentEta, segmentXsi, *eta, *xsi);
                return TIGL_SUCCESS;
            }
            catch (tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
            
        }
        
        // the component segment was not found
        LOG(ERROR) << "Error: Invalid component segment uid in tiglWingSegmentPointGetComponentSegmentEtaXsi" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingSegmentPointGetComponentSegmentEtaXsi!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetSegmentIntersection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                 const char* componentSegmentUID,
                                                                                 const char* segmentUID,
                                                                                 double csEta1, double csXsi1,
                                                                                 double csEta2, double csXsi2,
                                                                                 double   segmentEta,
                                                                                 double * segmentXsi,
                                                                                 TiglBoolean* hasWarning)
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetSegmentIntersection." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for componentSegmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetSegmentIntersection." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    if (segmentXsi == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentXsi ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetSegmentIntersection." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        
        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(componentSegmentUID);
                compSeg.GetSegmentIntersection(segmentUID, csEta1, csXsi1, csEta2, csXsi2, segmentEta, *segmentXsi);

                // check if xsi is valid
                if (hasWarning) {
                    if (*segmentXsi < 0. || *segmentXsi > 1.) {
                        *hasWarning = TIGL_TRUE;
                    }
                    else {
                        *hasWarning = TIGL_FALSE;
                    }
                }

                return TIGL_SUCCESS;
            }
            catch (tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
        }
        
        // the component segment was not found
        LOG(ERROR) << "Error: Invalid component segment uid in tiglWingComponentSegmentGetSegmentIntersection" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingComponentSegmentGetSegmentIntersection!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentComputeEtaIntersection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                 const char* componentSegmentUID,
                                                                                 double csEta1, double csXsi1,
                                                                                 double csEta2, double csXsi2,
                                                                                 double eta,
                                                                                 double* xsi,
                                                                                 TiglBoolean* hasWarning)
{
    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer for argument componentSegmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentComputeEtaIntersection." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (xsi == 0) {
        LOG(ERROR) << "Error: Null pointer for argument xsi ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentComputeEtaIntersection." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(componentSegmentUID);
                double xsiTemp = 0.;
                double distanceTmp = 0.;
                compSeg.InterpolateOnLine(csEta1, csXsi1, csEta2, csXsi2, eta, xsiTemp, distanceTmp);
                *xsi = xsiTemp;

                // check if xsi is valid
                if (hasWarning) {
                    if (*xsi < 0. || *xsi > 1.) {
                        *hasWarning = TIGL_TRUE;
                    }
                    else {
                        *hasWarning = TIGL_FALSE;
                    }
                }

                return TIGL_SUCCESS;
            }
            catch (tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
        }

        // the component segment was not found
        LOG(ERROR) << "Error: Invalid component segment uid in tiglWingComponentSegmentComputeEtaIntersection" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingComponentSegmentComputeEtaIntersection!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetNumberOfSegments(TiglCPACSConfigurationHandle cpacsHandle,
                                                                              const char * componentSegmentUID,
                                                                              int * nsegments)
{

    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for componentSegmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetNumberOfSegments." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    if (nsegments == 0) {
        LOG(ERROR) << "Error: Null pointer argument for nsegments ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetNumberOfSegments." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(componentSegmentUID);
                const tigl::SegmentList& segments = compSeg.GetSegmentList();
                *nsegments = (int) segments.size();
                return TIGL_SUCCESS;
            }
            catch (tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
        }
        // the component segment was not found
        LOG(ERROR) << "Error: Invalid component segment uid in tiglWingComponentSegmentGetNumberOfSegments" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetNumberOfSegments!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        const char * componentSegmentUID,
                                                                        int  segmentIndex,
                                                                        char ** segmentUID) 
{
    
    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for componentSegmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetSegmentUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    if (segmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for nsegments ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetSegmentUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(componentSegmentUID);
                const tigl::SegmentList& segments = compSeg.GetSegmentList();
                if (segmentIndex < 1 || segmentIndex > (int) segments.size()) {
                    LOG(ERROR) << "Error: Invalid segment index in tiglWingComponentSegmentGetSegmentUID" << std::endl;
                    return TIGL_INDEX_ERROR;
                }
                *segmentUID = const_cast<char*>(segments[segmentIndex-1]->GetUID().c_str());
                
                return TIGL_SUCCESS;
            }
            catch (tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
        }
        // the component segment was not found
        LOG(ERROR) << "Error: Invalid component segment uid in tiglWingComponentSegmentGetSegmentUID" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetSegmentUID!" << std::endl;
        return TIGL_ERROR;
    }
}


/******************************************************************************/
/* Fuselage Functions                                                         */
/******************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglGetFuselageCount(TiglCPACSConfigurationHandle cpacsHandle, int* fuselageCountPtr)
{
    if (fuselageCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for fuselageCountPtr ";
        LOG(ERROR) << "in function call to tiglGetFuselageCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *fuselageCountPtr = config.GetFuselageCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetFuselageCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int fuselageIndex,
                                                              int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentCountPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentCount." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int fuselageIndex,
                                                       int segmentIndex,
                                                       double eta,
                                                       double zeta,
                                                       double* pointXPtr,
                                                       double* pointYPtr,
                                                       double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetPoint." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPoint!" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPointAngle(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int fuselageIndex,
                                                            int segmentIndex,
                                                            double eta,
                                                            double alpha,
                                                            double* pointXPtr,
                                                            double* pointYPtr,
                                                            double* pointZPtr)
{
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetPointAngle." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        gp_Pnt point = segment.GetPointAngle(eta, alpha, 0.0, 0.0);        
        if ((point.X() == 0.0) && (point.Y() == 0.0) && (point.Z() == 0.0)) {
            return TIGL_ERROR;
        }
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointAngle!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPointAngleTranslated(TiglCPACSConfigurationHandle cpacsHandle,
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
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetPointAngleTranslated." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        gp_Pnt point = segment.GetPointAngle(eta, alpha, y_cs, z_cs);
        if ((point.X() == 0.0) && (point.Y() == 0.0) && (point.Z() == 0.0)) {
            return TIGL_ERROR;
        }
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointAngleTranslated!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPointOnXPlane(TiglCPACSConfigurationHandle cpacsHandle,
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
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetPointOnXPlane." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointOnXPlane!" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPointOnYPlane(TiglCPACSConfigurationHandle cpacsHandle,
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
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetPointOnYPlane." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointOnYPlane!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetNumPointsOnXPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int fuselageIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double xpos,
                                                                   int* numPointsPtr)
{
    if (numPointsPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for numPointsPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetNumPointsOnXPlane." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetNumPointsOnXPlane!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetNumPointsOnYPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int fuselageIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double ypos,
                                                                   int* numPointsPtr)
{
    if (numPointsPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for numPointsPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetNumPointsOnYPlane." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetNumPointsOnYPlane!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetCircumference(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int fuselageIndex,
                                                               int segmentIndex,
                                                               double eta,
                                                               double* circumferencePtr)
{
    if (circumferencePtr == 0) {
         LOG(ERROR) << "Error: Null pointer argument for circumferencePtr ";
         LOG(ERROR) << "in function call to tiglFuselageGetCircumference." << std::endl;
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
         LOG(ERROR) << ex.what() << std::endl;
         return TIGL_ERROR;
     }
     catch (tigl::CTiglError& ex) {
         LOG(ERROR) << ex.getError() << std::endl;
         return ex.getCode();
     }
     catch (...) {
         LOG(ERROR) << "Caught an exception in tiglFuselageGetCircumference!" << std::endl;
         return TIGL_ERROR;
     }
}




TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int fuselageIndex,
                                                                            int segmentIndex,
                                                                            int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentCountPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetStartConnectedSegmentCount." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetStartConnectedSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                          int fuselageIndex,
                                                                          int segmentIndex,
                                                                          int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentCountPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetEndConnectedSegmentCount." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetEndConnectedSegmentCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int fuselageIndex,
                                                                            int segmentIndex,
                                                                            int n,
                                                                            int* connectedIndexPtr)
{
    if (connectedIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for connectedIndexPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetStartConnectedSegmentIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetStartConnectedSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                          int fuselageIndex,
                                                                          int segmentIndex,
                                                                          int n,
                                                                          int* connectedIndexPtr)
{
    if (connectedIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for connectedIndexPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetEndConnectedSegmentIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetEndConnectedSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                           int fuselageIndex,
                                                                           int segmentIndex,
                                                                           char** sectionUIDPtr,
                                                                           char** elementUIDPtr)
{

    if (fuselageIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: fuselageIndex or segmentIndex argument is less than one ";
        LOG(ERROR) << "in function call to tiglFuselageGetStartSectionAndElementUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);

        *sectionUIDPtr = (char *) segment.GetStartSectionUID().c_str();

        *elementUIDPtr = (char *)segment.GetStartSectionElementUID().c_str();

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetStartSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         int fuselageIndex,
                                                                         int segmentIndex,
                                                                         char** sectionUIDPtr,
                                                                         char** elementUIDPtr)
{

    if (fuselageIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: fuselageIndex or segmentIndex argument less than one ";
        LOG(ERROR) << "in function call to tiglFuselageGetEndSectionAndElementUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);

        *sectionUIDPtr = (char *) segment.GetEndSectionUID().c_str();
        *elementUIDPtr = (char *) segment.GetEndSectionElementUID().c_str();

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetEndSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartSectionAndElementIndex (TiglCPACSConfigurationHandle cpacsHandle,
                                                                              int fuselageIndex,
                                                                              int segmentIndex,
                                                                              int* sectionIndexPtr,
                                                                              int* elementIndexPtr)
{
    if (sectionIndexPtr == 0 || elementIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetStartSectionAndElementIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetStartSectionAndElementIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetStartSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                           int fuselageIndex,
                                                                           int segmentIndex,
                                                                           int* sectionIndexPtr,
                                                                           int* elementIndexPtr)
{
    if (sectionIndexPtr == 0 || elementIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetEndSectionAndElementIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for sectionIndexPtr or elementIndexPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetEndSectionAndElementIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetEndSectionAndElementIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetProfileName(TiglCPACSConfigurationHandle cpacsHandle,
                                                             int fuselageIndex,
                                                             int sectionIndex,
                                                             int elementIndex,
                                                             char** profileNamePtr)
{
    if (profileNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for profileNamePtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetProfileName." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || sectionIndex < 1 || elementIndex < 1) {
        LOG(ERROR) << "Error: Fuselage, section or element index index in less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetProfileName." << std::endl;
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

        *profileNamePtr = const_cast<char*>(profile.GetName().c_str());
        
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetProfileName!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int fuselageIndex,
                                                     char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for uidNamePtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: Fuselage or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *uidNamePtr = const_cast<char*>(fuselage.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                       const char * fuselageUID,
                                                       int* fuselageIndexPtr)
{
    if (fuselageUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for fuselageUID ";
        LOG(ERROR) << "in function call to tiglFuselageGetIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for fuselageIndexPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *fuselageIndexPtr = config.GetFuselageIndex(std::string(fuselageUID));
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        *fuselageIndexPtr = -1;
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int fuselageIndex,
                                                            int segmentIndex,
                                                            char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for uidNamePtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Fuselage or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *uidNamePtr = const_cast<char*>(segment.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentUID!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                              const char * segmentUID,
                                                              int * segmentIndex,
                                                              int * fuselageIndex)
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentUID ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (segmentIndex == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentIndex ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndex == 0) {
        LOG(ERROR) << "Error: Null pointer argument for fuselageIndex ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        for (int iFuselage = 1; iFuselage <= config.GetFuselageCount(); ++iFuselage) {
            tigl::CCPACSFuselage& fuselage = config.GetFuselage(iFuselage);
            try {
                int fuselageSegIndex = fuselage.GetSegment(segmentUID).GetSegmentIndex();
                *segmentIndex = fuselageSegIndex;
                *fuselageIndex = iFuselage;
                return TIGL_SUCCESS;
            }
            catch (tigl::CTiglError&) {
                continue;
            }
        }

        LOG(ERROR) << "Error in tiglFuselageGetSegmentIndex: could not find a fuselage index with given uid \"" << segmentUID << "\".";
        *segmentIndex = -1;
        *fuselageIndex = -1;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSectionCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int fuselageIndex,
                                                              int* sectionCount)
{
    if (sectionCount == 0) {
        LOG(ERROR) << "Error: Null pointer argument for sectionCount "
                   << "in function call to tiglFuselageGetSectionCount.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: Fuselage index is less than zero "
                   << "in function call to tiglFuselageGetSectionCount.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *sectionCount = fuselage.GetSectionCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSectionCount!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int fuselageIndex,
                                                            int sectionIndex,
                                                            char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for uidNamePtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetSectionUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || sectionIndex < 1) {
        LOG(ERROR) << "Error: Fuselage or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetSectionUID." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSectionUID!" << std::endl;
        return TIGL_ERROR;
    }
}








TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                          TiglSymmetryAxis* symmetryAxisPtr)
{
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: Fuselage or segment index index is less than zero ";
        LOG(ERROR) << "in function call to tiglGetFuselageSymmetry." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetFuselageSymmetry!" << std::endl;
        return TIGL_ERROR;
    }
}




TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetMinumumDistanceToGround(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         char *fuselageUID,
                                                                         double axisPntX,
                                                                         double axisPntY,
                                                                         double axisPntZ,
                                                                         double axisDirX,
                                                                         double axisDirY,
                                                                         double axisDirZ,
                                                                         double angle,
                                                                         double* pointXPtr,
                                                                         double* pointYPtr,
                                                                         double* pointZPtr)
{
     if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetMinumumDistanceToGround." << std::endl;
        return TIGL_NULL_POINTER;
    }

     // Definition of the axis of rotation
     gp_Ax1 RAxis(gp_Pnt(axisPntX, axisPntY, axisPntZ), gp_Dir(axisDirX, axisDirY, axisDirZ));


        try {
            tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
            tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
            tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageUID);
            gp_Pnt point = fuselage.GetMinumumDistanceToGround(RAxis, angle);
            *pointXPtr = point.X();
            *pointYPtr = point.Y();
            *pointZPtr = point.Z();
            return TIGL_SUCCESS;
        }
        catch (std::exception& ex) {
            LOG(ERROR) << ex.what() << std::endl;
            return TIGL_ERROR;
        }
        catch (tigl::CTiglError& ex) {
            LOG(ERROR) << ex.getError() << std::endl;
            return ex.getCode();
        }
        catch (...) {
            LOG(ERROR) << "Caught an exception in tiglFuselageGetPointOnYPlane!" << std::endl;
            return TIGL_ERROR;
        }
}


/******************************************************************************/
/* Rotor Functions                                                            */
/******************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglGetRotorCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                    int* rotorCountPtr)
{
    if (rotorCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for rotorCountPtr ";
        LOG(ERROR) << "in function call to tiglGetRotorCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *rotorCountPtr = config.GetRotorCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetRotorCount!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                  int rotorIndex,
                                                  char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for uidNamePtr ";
        LOG(ERROR) << "in function call to tiglRotorGetUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: Rotor index is less than one ";
        LOG(ERROR) << "in function call to tiglRotorGetUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *uidNamePtr = const_cast<char*> (rotor.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                    const char* rotorUID,
                                                    int* rotorIndexPtr)
{
    if (rotorUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for rotorUID ";
        LOG(ERROR) << "in function call to tiglRotorGetIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for rotorIndexPtr ";
        LOG(ERROR) << "in function call to tiglRotorGetIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *rotorIndexPtr = config.GetRotorIndex(std::string(rotorUID));
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        *rotorIndexPtr = -1;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        *rotorIndexPtr = -1;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetIndex!" << std::endl;
        *rotorIndexPtr = -1;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetRadius(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int rotorIndex,
                                                     double *radiusPtr)
{
    if (radiusPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for radiusPtr ";
        LOG(ERROR) << "in function call to tiglRotorGetRadius." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorGetRadius.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *radiusPtr = rotor.GetRadius();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetRadius!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetReferenceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int rotorIndex,
                                                            double *referenceAreaPtr)
{
    if (referenceAreaPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for referenceAreaPtr ";
        LOG(ERROR) << "in function call to tiglRotorGetReferenceArea." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglrotorGetReferenceArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *referenceAreaPtr = rotor.GetReferenceArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetReferenceArea!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetTotalBladePlanformArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                     int rotorIndex,
                                                                     double *totalBladePlanformAreaPtr)
{
    if (totalBladePlanformAreaPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for totalBladePlanformAreaPtr ";
        LOG(ERROR) << "in function call to tiglRotorGetTotalBladePlanformArea." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorGetTotalBladePlanformArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *totalBladePlanformAreaPtr = rotor.GetTotalBladePlanformArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetTotalBladePlanformArea!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetSolidity(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int rotorIndex,
                                                       double *solidityPtr)
{
    if (solidityPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for solidityPtr ";
        LOG(ERROR) << "in function call to tiglRotorGetSolidity." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorGetSolidity.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *solidityPtr = rotor.GetSolidity();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetSolidity!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                          int rotorIndex,
                                                          double *surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for surfaceAreaPtr ";
        LOG(ERROR) << "in function call to tiglRotorGetSurfaceArea." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: Rotor index is less than one ";
        LOG(ERROR) << "in function call to tiglRotorGetSurfaceArea." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *surfaceAreaPtr = rotor.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetVolume(TiglCPACSConfigurationHandle cpacsHandle, 
                                                     int rotorIndex,
                                                     double *volumePtr)
{
    if (volumePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for volumePtr ";
        LOG(ERROR) << "in function call to tiglRotorGetVolume." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one ";
        LOG(ERROR) << "in function call to tiglrotorGetVolume." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *volumePtr = rotor.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglrotorGetVolume!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetTipSpeed(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int rotorIndex,
                                                       double *tipSpeedPtr)
{
    if (tipSpeedPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for tipSpeedPtr ";
        LOG(ERROR) << "in function call to tiglRotorGetTipSpeed." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorGetTipSpeed.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *tipSpeedPtr = rotor.GetTipSpeed();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetTipSpeed!" << std::endl;
        return TIGL_ERROR;
    }
}


/******************************************************************************/
/* Rotor Blade Functions                                                      */
/******************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetRotorBladeCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int rotorIndex,
                                                              int* rotorBladeCountPtr)
{
    if (rotorBladeCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for rotorBladeCountPtr ";
        LOG(ERROR) << "in function call to tiglRotorGetRotorBladeCount." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorGetRotorBladeCount.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *rotorBladeCountPtr = rotor.GetRotorBladeCount();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetRotorBladeCount!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetWingIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                             int rotorIndex,
                                                             int rotorBladeIndex,
                                                             int* wingIndexPtr)
{
    if (wingIndexPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingIndexPtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetWingIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetWingIndex.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetWingIndex.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        tigl::CCPACSRotorBladeAttachment& rotorBladeAttachment = rotorBlade.GetRotorBladeAttachment();
        *wingIndexPtr = rotorBladeAttachment.GetWingIndex();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetWingIndex!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetWingUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int rotorIndex,
                                                           int rotorBladeIndex,
                                                           char** wingUIDPtr)
{
    if (wingUIDPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUIDPtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetWingUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetWingUID.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetWingUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        tigl::CCPACSRotorBladeAttachment& rotorBladeAttachment = rotorBlade.GetRotorBladeAttachment();
        *wingUIDPtr = const_cast<char*> (rotorBladeAttachment.GetWingUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetWingUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetAzimuthAngle(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int rotorIndex,
                                                                int rotorBladeIndex,
                                                                double* azimuthAnglePtr)
{
    if (azimuthAnglePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for azimuthAnglePtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetAzimuthAngle." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetAzimuthAngle.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetAzimuthAngle.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *azimuthAnglePtr = rotorBlade.GetAzimuthAngle();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetAzimuthAngle!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetRadius(TiglCPACSConfigurationHandle cpacsHandle,
                                                          int rotorIndex,
                                                          int rotorBladeIndex,
                                                          double* radiusPtr)
{
    if (radiusPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for radiusPtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetRadius." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetRadius.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetRadius.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *radiusPtr = rotorBlade.GetRadius();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetRadius!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetPlanformArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                                int rotorIndex,
                                                                int rotorBladeIndex,
                                                                double *planformAreaPtr)
{
    if (planformAreaPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for planformAreaPtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetPlanformArea." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetPlanformArea.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetPlanformArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *planformAreaPtr = rotorBlade.GetPlanformArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetPlanformArea!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int rotorIndex,
                                                               int rotorBladeIndex,
                                                               double *surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for surfaceAreaPtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetSurfaceArea." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetSurfaceArea.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetSurfaceArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *surfaceAreaPtr = rotorBlade.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetVolume(TiglCPACSConfigurationHandle cpacsHandle, 
                                                          int rotorIndex,
                                                          int rotorBladeIndex,
                                                          double *volumePtr)
{
    if (volumePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for volumePtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetVolume." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetVolume.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetVolume.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *volumePtr = rotorBlade.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetVolume!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetTipSpeed(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int rotorIndex,
                                                            int rotorBladeIndex,
                                                            double* tipSpeedPtr)
{
    if (tipSpeedPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for tipSpeedPtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetTipSpeed." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetTipSpeed.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetTipSpeed.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *tipSpeedPtr = rotorBlade.GetTipSpeed();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetTipSpeed!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetLocalRadius(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int rotorIndex,
                                                               int rotorBladeIndex,
                                                               int segmentIndex,
                                                               double eta,
                                                               double* radiusPtr)
{
    if (radiusPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for radiusPtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetLocalRadius." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_INDEX_ERROR;
    }
    if (segmentIndex < 1) {
        LOG(ERROR) << "Error: segment index is less than one "
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_INDEX_ERROR;
    }
    if ((eta < 0.) || (eta > 1.)) {
        LOG(ERROR) << "Error: eta not in range [0,1] "
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *radiusPtr = rotorBlade.GetLocalRadius(segmentIndex, eta);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetLocalRadius!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetLocalChord(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int rotorIndex,
                                                              int rotorBladeIndex,
                                                              int segmentIndex,
                                                              double eta,
                                                              double* chordPtr)
{
    if (chordPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for chordPtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetLocalChord." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_INDEX_ERROR;
    }
    if (segmentIndex < 1) {
        LOG(ERROR) << "Error: segment index is less than one "
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_INDEX_ERROR;
    }
    if ((eta < 0.) || (eta > 1.)) {
        LOG(ERROR) << "Error: eta not in range [0,1] "
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *chordPtr = rotorBlade.GetLocalChord(segmentIndex, eta);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetLocalChord!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetLocalTwistAngle(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int rotorIndex,
                                                                   int rotorBladeIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double* twistAnglePtr)
{
    if (twistAnglePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for twistAnglePtr ";
        LOG(ERROR) << "in function call to tiglRotorBladeGetLocalTwistAngle." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Error: rotor index is less than one "
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Error: rotor blade index is less than one "
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_INDEX_ERROR;
    }
    if (segmentIndex < 1) {
        LOG(ERROR) << "Error: segment index is less than one "
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_INDEX_ERROR;
    }
    if ((eta < 0.) || (eta > 1.)) {
        LOG(ERROR) << "Error: eta not in range [0,1] "
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CCPACSRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *twistAnglePtr = rotorBlade.GetLocalTwistAngle(segmentIndex, eta);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetLocalTwistAngle!" << std::endl;
        return TIGL_ERROR;
    }
}


/******************************************************************************/
/* Boolean Functions                                                          */
/******************************************************************************/

// DEPRECATED
TIGL_COMMON_EXPORT TiglReturnCode tiglComponentIntersectionPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 const char*  componentUidOne,
                                                                 const char*  componentUidTwo,
                                                                 int lineID,
                                                                 double eta,
                                                                 double* pointXPtr,
                                                                 double* pointYPtr,
                                                                 double* pointZPtr)
{
    LOG(WARNING) << "The function tiglComponentIntersectionPoint is deprecated and will be removed in future releases. "
                 << "More information can be found in the documentation.";
    if (pointXPtr == 0 || pointYPtr == 0 || pointZPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for pointXPtr, pointYPtr or pointZPtr ";
        LOG(ERROR) << "in function call to tiglComponentIntersectionPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if ( (componentUidOne == 0) || (componentUidTwo == 0)) {
        LOG(ERROR) << "Error: Null pointer argument for at least one given UID ";
        LOG(ERROR) << "in function call to tiglComponentIntersectionPoint." << std::endl;
        return TIGL_NULL_POINTER;
    }


    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        TopoDS_Shape compoundOne = uidManager.GetComponent(componentUidOne)->GetLoft()->Shape();
        TopoDS_Shape compoundTwo = uidManager.GetComponent(componentUidTwo)->GetLoft()->Shape();

        tigl::CTiglIntersectionCalculation Intersector(&config.GetShapeCache(), componentUidOne, componentUidTwo, compoundOne, compoundTwo);
        gp_Pnt point = Intersector.GetPoint(eta, lineID);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglComponentIntersectionPoint!" << std::endl;
        return TIGL_ERROR;
    }
}

// DEPRECATED
TIGL_COMMON_EXPORT TiglReturnCode tiglComponentIntersectionPoints(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char *componentUidOne,
                                                                  const char *componentUidTwo,
                                                                  int lineID,
                                                                  const double* etaArray,
                                                                  int nPoints,
                                                                  double* pointXArray,
                                                                  double* pointYArray,
                                                                  double* pointZArray)
{
    LOG(WARNING) << "The function tiglComponentIntersectionPoints is deprecated and will be removed in future releases. "
                 << "More information can be found in the documentation.";
    if (etaArray == 0) {
        LOG(ERROR) << "Error: Null pointer argument for etaArray ";
        LOG(ERROR) << "in function call to tiglComponentIntersectionPoints." << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    if (pointXArray == 0 || pointYArray == 0 || pointZArray == 0) {
        LOG(ERROR) << "Error: Null pointer argument for pointXArray, pointYArray or pointZArray ";
        LOG(ERROR) << "in function call to tiglComponentIntersectionPoints." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if ( (componentUidOne == 0) || (componentUidTwo == 0)) {
        LOG(ERROR) << "Error: Null pointer argument for at least one given UID ";
        LOG(ERROR) << "in function call to tiglComponentIntersectionPoints." << std::endl;
        return TIGL_NULL_POINTER;
    }


    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        TopoDS_Shape compoundOne = uidManager.GetComponent(componentUidOne)->GetLoft()->Shape();
        TopoDS_Shape compoundTwo = uidManager.GetComponent(componentUidTwo)->GetLoft()->Shape();

        tigl::CTiglIntersectionCalculation Intersector(&config.GetShapeCache(), componentUidOne, componentUidTwo, compoundOne, compoundTwo);
        
        for (int i = 0; i < nPoints; ++i) {
            gp_Pnt point = Intersector.GetPoint(etaArray[i], lineID);
            pointXArray[i] = point.X();
            pointYArray[i] = point.Y();
            pointZArray[i] = point.Z();
        }
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglComponentIntersectionPoints!" << std::endl;
        return TIGL_ERROR;
    }
}

// DEPRECATED
TIGL_COMMON_EXPORT TiglReturnCode tiglComponentIntersectionLineCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                     const char *componentUidOne,
                                                                     const char *componentUidTwo,
                                                                     int* numWires)
{
    LOG(WARNING) << "The function tiglComponentIntersectionLineCount is deprecated and will be removed in future releases. "
                 << "More information can be found in the documentation.";
    if ( (componentUidOne == 0) || (componentUidTwo == 0)) {
        LOG(ERROR) << "Error: Null pointer argument for at least one given UID ";
        LOG(ERROR) << "in function call to tiglFuselageWingSurfaceIntersectionLineCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        TopoDS_Shape compoundOne = uidManager.GetComponent(componentUidOne)->GetLoft()->Shape();
        TopoDS_Shape compoundTwo = uidManager.GetComponent(componentUidTwo)->GetLoft()->Shape();

        tigl::CTiglIntersectionCalculation Intersector(&config.GetShapeCache(), componentUidOne, componentUidTwo, compoundOne, compoundTwo);
        *numWires = Intersector.GetCountIntersectionLines();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageWingSurfaceIntersectionLineCount!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectComponents(TiglCPACSConfigurationHandle cpacsHandle,
                                                          const char*  componentUidOne,
                                                          const char*  componentUidTwo,
                                                          char** intersectionID)
{
    if (!componentUidOne) {
        LOG(ERROR) << "Null pointer for argument componentUidOne in tiglIntersectComponents.";
        return TIGL_NULL_POINTER;
    }
    if (!componentUidTwo) {
        LOG(ERROR) << "Null pointer for argument componentUidTwo in tiglIntersectComponents.";
        return TIGL_NULL_POINTER;
    }
    if (!intersectionID) {
        LOG(ERROR) << "Null pointer for argument intersectionID in tiglIntersectComponents.";
        return TIGL_NULL_POINTER;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        tigl::ITiglGeometricComponent* c1 = uidManager.GetComponent(componentUidOne);
        tigl::ITiglGeometricComponent* c2 = uidManager.GetComponent(componentUidTwo);
        if (c1 && c2) {
            TopoDS_Shape compoundOne = c1->GetLoft()->Shape();
            TopoDS_Shape compoundTwo = c2->GetLoft()->Shape();
            
            tigl::CTiglIntersectionCalculation Intersector(&config.GetShapeCache(),
                                                           componentUidOne, 
                                                           componentUidTwo, 
                                                           compoundOne, 
                                                           compoundTwo);

            std::string id = Intersector.GetID();
            *intersectionID = (char*) config.GetMemoryPool().MakeNontempString(id.c_str());

            return TIGL_SUCCESS;
        }
        else {
            LOG(ERROR) << "UID can not be found in tiglIntersectComponents.";
            return TIGL_UID_ERROR;
        }
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIntersectComponents!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectWithPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                         const char*  componentUid,
                                                         double px, double py, double pz,
                                                         double nx, double ny, double nz,
                                                         char** intersectionID)
{
    if (!componentUid) {
        LOG(ERROR) << "Null pointer for argument componentUid in tiglIntersectWithPlane.";
        return TIGL_NULL_POINTER;
    }
    if (!intersectionID) {
        LOG(ERROR) << "Null pointer for argument intersectionID in tiglIntersectWithPlane.";
        return TIGL_NULL_POINTER;
    }
    if (nx*nx + ny*ny + nz*nz < 1e-10) {
        LOG(ERROR) << "Normal vector must not be zero in tiglIntersectWithPlane.";
        return TIGL_MATH_ERROR;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        tigl::ITiglGeometricComponent* component = uidManager.GetComponent(componentUid);
        if (component) {
            TopoDS_Shape shape = component->GetLoft()->Shape();
            gp_Pnt p(px, py, pz);
            gp_Dir n(nx, ny, nz);
            
            tigl::CTiglIntersectionCalculation Intersector(&config.GetShapeCache(),
                                                           componentUid,
                                                           shape,
                                                           p, n);

            std::string id = Intersector.GetID();
            *intersectionID = (char*) config.GetMemoryPool().MakeNontempString(id.c_str());

            return TIGL_SUCCESS;
        }
        else {
            LOG(ERROR) << "UID can not be found in tiglIntersectWithPlane.";
            return TIGL_UID_ERROR;
        }
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIntersectWithPlane!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectGetLineCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                            const char* intersectionID,
                                                            int* lineCount)
{
    if (!lineCount) {
        LOG(ERROR) << "Null pointer for argument lineCount in tiglIntersectGetLineCount.";
        return TIGL_NULL_POINTER;
    }
    if (!intersectionID) {
        LOG(ERROR) << "Null pointer for argument intersectionID in tiglIntersectGetLineCount.";
        return TIGL_NULL_POINTER;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglShapeCache& cache = config.GetShapeCache();

        tigl::CTiglIntersectionCalculation Intersector(cache, intersectionID);
        *lineCount = Intersector.GetCountIntersectionLines();
        
        return TIGL_SUCCESS;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIntersectComponents!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                        const char* intersectionID,
                                                        int lineIdx,
                                                        double eta,
                                                        double* pointX,
                                                        double* pointY,
                                                        double* pointZ)
{
    if (!pointX) {
        LOG(ERROR) << "Null pointer for argument pointX in tiglIntersectGetPoint.";
        return TIGL_NULL_POINTER;
    }
    if (!pointY) {
        LOG(ERROR) << "Null pointer for argument pointY in tiglIntersectGetPoint.";
        return TIGL_NULL_POINTER;
    }
    if (!pointZ) {
        LOG(ERROR) << "Null pointer for argument pointZ in tiglIntersectGetPoint.";
        return TIGL_NULL_POINTER;
    }
    if (!intersectionID) {
        LOG(ERROR) << "Null pointer for argument intersectionID in tiglIntersectGetPoint.";
        return TIGL_NULL_POINTER;
    }
    if (eta < 0.0 || eta > 1.0) {
        LOG(ERROR) << "Parameter eta not in valid the range 0.0 <= eta <= 1.0 in tiglIntersectGetPoint";
        return TIGL_MATH_ERROR;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglShapeCache& cache = config.GetShapeCache();

        tigl::CTiglIntersectionCalculation Intersector(cache, intersectionID);
        gp_Pnt p = Intersector.GetPoint(eta, lineIdx);
        
        *pointX = p.X();
        *pointY = p.Y();
        *pointZ = p.Z();
        
        return TIGL_SUCCESS;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIntersectComponents!";
        return TIGL_ERROR;
    }
}


/*****************************************************************************************************/
/*                         Export Functions                                                          */
/*****************************************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglExportIGES(TiglCPACSConfigurationHandle cpacsHandle, const char *filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportIGES." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportIges exporter;
        exporter.AddConfiguration(config);
        bool ret = exporter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportIGES!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedWingFuselageIGES(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char *filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportFusedWingFuselageIGES." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportIges exporter;
        exporter.AddFusedConfiguration(config);
        bool ret = exporter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFusedWingFuselageIGES!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportSTEP(TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportSTEP." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStep exporter;
        exporter.AddConfiguration(config);
        bool ret = exporter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportSTEP!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedSTEP(TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportFusedSTEP." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStep exporter;
        exporter.AddFusedConfiguration(config);
        bool ret = exporter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFusedSTEP!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingSTL(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                          const char *filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedWingSTL." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingIndex < 1) {
        LOG(ERROR) << "Error: wingIndex in less or equal zero";
        LOG(ERROR) << "in function call to tiglExportMeshedWingSTL." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        PNamedShape loft = wing.GetLoft();
        
        tigl::CTiglExportStl exporter;
        exporter.AddShape(loft, deflection);
        bool ret = exporter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedWingSTL!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingSTLByUID(TiglCPACSConfigurationHandle cpacsHandle, 
                                                               const char* wingUID,
                                                               const char *filenamePtr, 
                                                               double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr"
                   << "in function call to tiglExportMeshedWingSTLByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUID"
                   << "in function call to tiglExportMeshedWingSTLByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        for (int iWing = 1; iWing <= config.GetWingCount(); ++iWing) {
            tigl::CCPACSWing& wing = config.GetWing(iWing);
            if (wing.GetUID() == wingUID) {
                PNamedShape loft = wing.GetLoft();
                
                tigl::CTiglExportStl exporter;
                exporter.AddShape(loft, deflection);
                bool ret = exporter.Write(filenamePtr);
                return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
            }
        }
        
        LOG(ERROR) << "Wing with UID " << wingUID << " not found"
                   << "in function call to tiglExportMeshedWingSTLByUID." << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedWingSTLByUID!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageSTL(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                              const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedFuselageSTL." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: fuselageIndex in less or equal zero";
        LOG(ERROR) << "in function call to tiglExportMeshedFuselageSTL." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        PNamedShape loft = fuselage.GetLoft();
        
        tigl::CTiglExportStl exporter;
        exporter.AddShape(loft, deflection);
        bool ret = exporter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageSTL!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageSTLByUID(TiglCPACSConfigurationHandle cpacsHandle, 
                                                                   const char* fuselageUID,
                                                                   const char* filenamePtr, 
                                                                   double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr"
                   << "in function call to tiglExportMeshedFuselageSTLByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageUID"
                   << "in function call to tiglExportMeshedFuselageSTLByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        
        for (int ifusel = 1; ifusel <= config.GetFuselageCount(); ++ifusel) {
            tigl::CCPACSFuselage& fuselage = config.GetFuselage(ifusel);
            if (fuselage.GetUID() == fuselageUID) {
                PNamedShape loft = fuselage.GetLoft();
                
                tigl::CTiglExportStl exporter;
                exporter.AddShape(loft, deflection);
                bool ret = exporter.Write(filenamePtr);
                return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
            }
        }
        
        LOG(ERROR) << "Fuselage with UID " << fuselageUID << " not found"
                   << "in function call to tiglExportMeshedFuselageSTLByUID." << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageSTLByUID!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometrySTL(TiglCPACSConfigurationHandle cpacsHandle, const char *filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedGeometrySTL." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter;
        tigl::ExportOptions options;
        options.deflection = deflection;
        options.applySymmetries = true;
        options.includeFarField = false;

        exporter.AddConfiguration(config, options);
        bool ret = exporter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedGeometrySTL!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, const int wingIndex,
                                                                 const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedWingVTKByIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingIndex < 1) {
        LOG(ERROR) << "Error: wingIndex in less or equal zero";
        LOG(ERROR) << "in function call to tiglExportMeshedWingVTKByIndex." << std::endl;
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
    // all exceptions from the standard library 
    catch (std::exception & ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError & ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch(char *str) {
        LOG(ERROR) << str << std::endl;
        return TIGL_ERROR;
    }
    catch(std::string& str) {
        LOG(ERROR) << str << std::endl;
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglExportMeshedWingVTKByIndex" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID,
                                                               const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedWingVTKByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUID";
        LOG(ERROR) << "in function call to tiglExportMeshedWingVTKByUID." << std::endl;
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
    // all exceptions from the standard library
    catch (std::exception & ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError & ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch(char *str) {
        LOG(ERROR) << str << std::endl;
        return TIGL_ERROR;
    }
    catch(std::string& str) {
        LOG(ERROR) << str << std::endl;
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglExportMeshedWingVTKByUID" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, const int fuselageIndex,
                                                                     const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedFuselageVTKByIndex." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: fuselageIndex in less or equal zero";
        LOG(ERROR) << "in function call to tiglExportMeshedFuselageVTKByIndex." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageVTKByIndex!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID,
                                                                   const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedFuselageVTKByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for fuselageIndex";
        LOG(ERROR) << "in function call to tiglExportMeshedFuselageVTKByUID." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageVTKByUID!" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometryVTK(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedGeometryVTK." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedGeometryVTK!" << std::endl;
        return TIGL_ERROR;
    }
}




TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKSimpleByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID,
                                                                     const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedWingVTKSimpleByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUID";
        LOG(ERROR) << "in function call to tiglExportMeshedWingVTKSimpleByUID." << std::endl;
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
    // all exceptions from the standard library
    catch (std::exception & ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError & ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch(char *str) {
        LOG(ERROR) << str << std::endl;
        return TIGL_ERROR;
    }
    catch(std::string& str) {
        LOG(ERROR) << str << std::endl;
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglExportMeshedWingVTKSimpleByUID" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKSimpleByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID,
                                                                         const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedFuselageVTKSimpleByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for fuselageIndex";
        LOG(ERROR) << "in function call to tiglExportMeshedFuselageVTKSimpleByUID." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageVTKSimpleByUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportFuselageColladaByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID, const char* filenamePtr, double deflection) 
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportFuselageColladaByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for fuselageUID";
        LOG(ERROR) << "in function call to tiglExportFuselageColladaByUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageUID);
        tigl::CTiglExportCollada colladaWriter;
        colladaWriter.AddShape(fuselage.GetLoft(), deflection);
        bool ret = colladaWriter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFuselageColladaByUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportWingColladaByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID, const char* filenamePtr, double deflection) 
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportWingColladaByUID." << std::endl;
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUID";
        LOG(ERROR) << "in function call to tiglExportWingColladaByUID." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        tigl::CTiglExportCollada colladaWriter;
        colladaWriter.AddShape(wing.GetLoft(), deflection);
        bool ret = colladaWriter.Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportWingColladaByUID!" << std::endl;
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometryVTKSimple(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedGeometryVTKSimple." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedGeometryVTKSimple!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedBREP(TiglCPACSConfigurationHandle cpacsHandle, const char* filename)
{
    if (filename == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filename";
        LOG(ERROR) << "in function call to tiglExportFusedBREP." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportBrep exporter;
        exporter.AddFusedConfiguration(config);
        bool ret = exporter.Write(filename);
        return ret == true? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFusedBREP!" << std::endl;
        return TIGL_ERROR;
    }
}


/*****************************************************************************************************/
/*                     Material functions                                                            */
/*****************************************************************************************************/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetMaterialCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                           const char *componentSegmentUID,
                                                                           TiglStructureType structureType,
                                                                           double eta, double xsi,
                                                                           int * materialCount) 
{
    if (!componentSegmentUID) {
        LOG(ERROR) << "Error: Null pointer argument for componentSegmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetMaterialCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (!materialCount) {
        LOG(ERROR) << "Error: Null pointer argument for materialCount ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentGetMaterialCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg
                        = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(componentSegmentUID);
                //now do the calculations
                tigl::MaterialList list = compSeg.GetMaterials(eta, xsi, structureType);

                *materialCount = (int) list.size();
                return TIGL_SUCCESS;
            }
            catch(tigl::CTiglError& ex) {
                if (ex.getCode() != TIGL_UID_ERROR) {
                    throw;
                }
                else {
                    continue;
                }
            }
        }
        // the component segment was not found
        LOG(ERROR) << "Error: Invalid uid in tiglWingComponentSegmentGetMaterialCount" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetMaterialCount!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetMaterialUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         const char *componentSegmentUID,
                                                                         TiglStructureType structureType,
                                                                         double eta, double xsi,
                                                                         int materialIndex,
                                                                         char ** uid) 
{
    if (!componentSegmentUID) {
        LOG(ERROR) << "Error: Null pointer argument for componentSegmentUID "
                   << "in function call to tiglWingComponentSegmentGetMaterialUID.";
        return TIGL_NULL_POINTER;
    }

    if (!uid) {
        LOG(ERROR) << "Error: Null pointer argument for uid "
                   << "in function call to tiglWingComponentSegmentGetMaterialUID.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg
                        = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(componentSegmentUID);
                //now do the calculations
                tigl::MaterialList list = compSeg.GetMaterials(eta, xsi, structureType);

                // 1 <= index  <= ncount
                unsigned int matindex = (unsigned int) materialIndex;
                if (matindex < 1 || matindex > list.size()){
                    LOG(ERROR) << "Invalid material index in tiglWingComponentSegmentGetMaterialUID";
                    return TIGL_INDEX_ERROR;
                }

                const tigl::CCPACSMaterial* material = list.at(materialIndex-1);
                if (!material) {
                    return TIGL_ERROR;
                }
                *uid = (char*)material->GetUID().c_str();

                return TIGL_SUCCESS;
            }
            catch(tigl::CTiglError& ex){
                if (ex.getCode() != TIGL_UID_ERROR) {
                    throw;
                }
                else {
                    continue;
                }
            }
        }
        // the component segment was not found
        LOG(ERROR) << "Error: Invalid uid in tiglWingComponentSegmentGetMaterialUID" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetMaterialUID!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetMaterialThickness(TiglCPACSConfigurationHandle cpacsHandle,
                                                                               const char *componentSegmentUID,
                                                                               TiglStructureType structureType,
                                                                               double eta, double xsi,
                                                                               int materialIndex,
                                                                               double * thickness)
{
    if (!componentSegmentUID){
        LOG(ERROR) << "Error: Null pointer argument for componentSegmentUID "
                   << "in function call to tiglWingComponentSegmentGetMaterialThickness.";
        return TIGL_NULL_POINTER;
    }

    if (!thickness){
        LOG(ERROR) << "Error: Null pointer argument for thickness "
                   << "in function call to tiglWingComponentSegmentGetMaterialThickness.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg
                        = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(componentSegmentUID);
                //now do the calculations
                tigl::MaterialList list = compSeg.GetMaterials(eta, xsi, structureType);

                // 1 <= index  <= ncount
                unsigned int matindex = (unsigned int) materialIndex;
                if (matindex < 1 || matindex > list.size()){
                    LOG(ERROR) << "Invalid material index in tiglWingComponentSegmentGetMaterialThickness";
                    return TIGL_INDEX_ERROR;
                }

                const tigl::CCPACSMaterial* material = list.at(materialIndex-1);
                if (!material) {
                    return TIGL_ERROR;
                }
                *thickness = material->GetThickness();

                if (*thickness < 0) {
                    return TIGL_UNINITIALIZED;
                }
                else {
                    return TIGL_SUCCESS;
                }
            }
            catch(tigl::CTiglError& ex){
                if (ex.getCode() != TIGL_UID_ERROR) {
                    throw;
                }
                else {
                    continue;
                }
            }
        }
        // the component segment was not found
        LOG(ERROR) << "Error: Invalid uid in tiglWingComponentSegmentGetMaterialThickness" << std::endl;
        return TIGL_UID_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetMaterialThickness!" << std::endl;
        return TIGL_ERROR;
    }
}

/*****************************************************************************************************/
/*                     Volume calculations                                                           */
/*****************************************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetVolume(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                        double *volumePtr)
{
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: Fuselage index index is less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetVolume." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetVolume!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetVolume(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                    double *volumePtr)
{
    if (wingIndex < 1) {
        LOG(ERROR) << "Error: Wing index index is less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetVolume." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetVolume!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int wingIndex,
                                                           int segmentIndex,
                                                           double* volumePtr)
{
    if (volumePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for volumePtr ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentVolume." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentVolume." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentVolume!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int fuselageIndex,
                                                               int segmentIndex,
                                                               double* volumePtr)
{
    if (volumePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for volumePtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentVolume." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Fuselage or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentVolume." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentVolume!" << std::endl;
        return TIGL_ERROR;
    }
}

/*****************************************************************************************************/
/*                     Surface Area calculations                                                     */
/*****************************************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                         double *surfaceAreaPtr)
{
    if (wingIndex < 1) {
        LOG(ERROR) << "Error: Wing index index is less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetSurfaceArea." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                             double *surfaceAreaPtr)
{
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: Fuselage index index is less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetSurfaceArea." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                double* surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for surfaceAreaPtr ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentSurfaceArea." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetSegmentSurfaceArea." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                    int fuselageIndex,
                                                                    int segmentIndex,
                                                                    double* surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for surfaceAreaPtr ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentSurfaceArea." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Fuselage or segment index index in less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetSegmentSurfaceArea." << std::endl;
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
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentSurfaceArea!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentUpperSurfaceAreaTrimmed(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int wingIndex,
                                                                            int segmentIndex,
                                                                            double eta1, double xsi1,
                                                                            double eta2, double xsi2,
                                                                            double eta3, double xsi3,
                                                                            double eta4, double xsi4,
                                                                            double* surfaceArea)
{
    if (surfaceArea == 0) {
        LOG(ERROR) << "Error: Null pointer argument for surfaceArea "
                   << "in function call to tiglWingGetSegmentUpperSurfaceAreaTrimmed.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than one "
                   << "in function call to tiglWingGetSegmentUpperSurfaceAreaTrimmed.";
        return TIGL_INDEX_ERROR;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);
        *surfaceArea = segment.GetSurfaceArea(true, 
                                              eta1, xsi1,
                                              eta2, xsi2,
                                              eta3, xsi3,
                                              eta4, xsi4);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentUpperSurfaceAreaTrimmed!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentLowerSurfaceAreaTrimmed(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int wingIndex,
                                                                            int segmentIndex,
                                                                            double eta1, double xsi1,
                                                                            double eta2, double xsi2,
                                                                            double eta3, double xsi3,
                                                                            double eta4, double xsi4,
                                                                            double* surfaceArea)
{
    if (surfaceArea == 0) {
        LOG(ERROR) << "Error: Null pointer argument for surfaceArea "
                   << "in function call to tiglWingGetSegmentLowerSurfaceAreaTrimmed.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Error: Wing or segment index index in less than one "
                   << "in function call to tiglWingGetSegmentLowerSurfaceAreaTrimmed.";
        return TIGL_INDEX_ERROR;
    }
    
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);
        *surfaceArea = segment.GetSurfaceArea(false, 
                                              eta1, xsi1,
                                              eta2, xsi2,
                                              eta3, xsi3,
                                              eta4, xsi4);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentLowerSurfaceAreaTrimmed!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetReferenceArea(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                           TiglSymmetryAxis symPlane,
                                                           double *referenceAreaPtr)
{
    if (wingIndex < 1) {
        LOG(ERROR) << "Error: Wing index index is less than zero "
                   << "in function call to tiglWingGetReferenceArea.";
        return TIGL_INDEX_ERROR;
    }

    if (symPlane < TIGL_NO_SYMMETRY || symPlane > TIGL_Y_Z_PLANE) {
        LOG(ERROR) << "Error: invalid symmetry "
                   << "in function call to tiglWingGetReferenceArea.";
        return TIGL_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *referenceAreaPtr = wing.GetReferenceArea(symPlane);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetReferenceArea!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetMAC(TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID, double *mac_chord, double *mac_x, double *mac_y, double *mac_z)
{


    if (wingUID == NULL) {
        LOG(ERROR) << "Argument wingUID is NULL in tiglWingGetMAC!";
        return TIGL_NULL_POINTER;
    }

    if (!mac_chord) {
        LOG(ERROR) << "Argument mac_chord is NULL in tiglWingGetMAC!";
        return TIGL_NULL_POINTER;
    }

    if (!mac_x) {
        LOG(ERROR) << "Argument mac_x is NULL in tiglWingGetMAC!";
        return TIGL_NULL_POINTER;
    }

    if (!mac_y) {
        LOG(ERROR) << "Argument mac_y is NULL in tiglWingGetMAC!";
        return TIGL_NULL_POINTER;
    }

    if (!mac_z) {
        LOG(ERROR) << "Argument mac_z is NULL in tiglWingGetMAC!";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        wing.GetWingMAC( *mac_chord,  *mac_x,  *mac_y,  *mac_z);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglConfigurationGetLength!" << std::endl;
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetWettedArea(TiglCPACSConfigurationHandle cpacsHandle, char* wingUID,
                                                        double *wettedAreaPtr)
{
    if (wingUID == NULL) {
        LOG(ERROR) << "Error: WingUID is empty ";
        LOG(ERROR) << "in function call to tiglWingGetWettedArea." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        TopoDS_Shape parent = config.GetParentLoft(wingUID);
        *wettedAreaPtr = wing.GetWettedArea(parent);
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetWettedArea!" << std::endl;
        return TIGL_ERROR;
    }
}




/*****************************************************************************************************/
/*                     Component Utility Functions                                                      */
/*****************************************************************************************************/


TIGL_COMMON_EXPORT TiglReturnCode tiglComponentGetHashCode(TiglCPACSConfigurationHandle cpacsHandle,
                                                           const char* componentUID,
                                                           int* hashCodePtr)
{
    if (componentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for componentUID ";
        LOG(ERROR) << "in function call to tiglComponentGetHashCode." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (hashCodePtr == NULL) {
        LOG(ERROR) << "Error: Null pointer argument for hashCodePtr ";
        LOG(ERROR) << "in function call to tiglComponentGetHashCode." << std::endl;
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        tigl::ITiglGeometricComponent* component = uidManager.GetComponent(componentUID);
        if (component) {
            int hash = GetComponentHashCode(*component);
            *hashCodePtr = hash;
        }
        else {
            return TIGL_UID_ERROR;
        }

        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglComponentGetHashCode!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT const char * tiglGetErrorString(TiglReturnCode code)
{
    if (code > TIGL_MATH_ERROR || code < 0) {
        LOG(ERROR) << "TIGL error code " << code << " is unknown!" << std::endl;
        return "TIGL_UNKNOWN_ERROR";
    }
    return TiglErrorStrings[code];
}

TIGL_COMMON_EXPORT TiglReturnCode tiglConfigurationGetLength(TiglCPACSConfigurationHandle cpacsHandle, double * pLength)
{
    if (pLength == NULL) {
        LOG(ERROR) << "Error: argument pLength is NULL in tiglConfigurationGetLength!";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *pLength = config.GetAirplaneLenth();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglConfigurationGetLength!" << std::endl;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSpan(TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID, double * pSpan)
{
    if (pSpan == NULL) {
        LOG(ERROR) << "Error: argument pSpan is NULL in tiglConfigurationGetLength!";
        return TIGL_NULL_POINTER;
    }

    if (wingUID == NULL) {
        LOG(ERROR) << "Error: argument wingUID is NULL in tiglConfigurationGetLength!";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        *pSpan = wing.GetWingspan();
        return TIGL_SUCCESS;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return TIGL_ERROR;
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglConfigurationGetLength!" << std::endl;
        return TIGL_ERROR;
    }
}

/*****************************************************************************/
/* Logging functions.                                                        */
/*****************************************************************************/
TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileEnabled(const char *filePrefix) 
{
    tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
    if (filePrefix == NULL) {
        LOG(ERROR) << "Error: argument filePrefix is NULL in tiglLogToFileEnabled!";
        return TIGL_NULL_POINTER;
    }

    try {
        logger.LogToFile(filePrefix);
    }
    catch (tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileStreamEnabled(FILE * fp) 
{
    tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
    if (fp == NULL) {
        LOG(ERROR) << "Error: argument fp is NULL in tiglLogToFileStreamEnabled!";
        return TIGL_NULL_POINTER;
    }

    try {
        logger.LogToStream(fp);
    }
    catch (tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}


TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetFileEnding(const char *ending) 
{
    tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
    if (ending == NULL) {
        LOG(ERROR) << "Error: argument ending is NULL in tiglLogSetFileEnding!";
        return TIGL_NULL_POINTER;
    }

    try {
        logger.SetLogFileEnding(ending);
    }
    catch (tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetTimeInFilenameEnabled(TiglBoolean enabled) 
{
    tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();

    try {
        logger.SetTimeIdInFilenameEnabled(enabled > 0);
    }
    catch (tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileDisabled() 
{
    tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();

    try {
        logger.LogToConsole();
    }
    catch (tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetVerbosity(TiglLogLevel consoleVerbosity) 
{
    tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();

    try {
        logger.SetConsoleVerbosity(consoleVerbosity);
    }
    catch (tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}
