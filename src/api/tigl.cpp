/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
#include "CCPACSWingSection.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingComponentSegment.h"
#include "CTiglExporterFactory.h"
#include "CTiglLogging.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSFuselageSegment.h"
#include "PNamedShape.h"
#include "CNamedShape.h"
#include "CCPACSRotor.h"
#include "CCPACSRotorBladeAttachment.h"
#include "CTiglAttachedRotorBlade.h"
#include "CGlobalExporterConfigs.h"

#include "CTiglPoint.h"

#include "BRep_Tool.hxx"
#include "gp_Pnt.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Vertex.hxx"

/*****************************************************************************/
/* Private functions.                                                 */
/*****************************************************************************/

namespace
{
    static std::string version = std::string(TIGL_REVISION).size() > 0 ?
        TIGL_VERSION_STRING + std::string("-r") + std::string(TIGL_REVISION).substr(0,8) :
        TIGL_VERSION_STRING;

    static std::string const emptyString = "";
}

TixiPrintMsgFnc oldTixiMessageHandler = NULL;

namespace
{

    void tiglCleanup();
    bool tiglInit();
    void TixiMessageHandler(MessageType type, const char *message);


    bool tiglInit()
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
    
    void tiglCleanup()
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
        LOG(ERROR) << "Null pointer argument for cpacsHandlePtr in function call to tiglOpenCPACSConfiguration.";
        return TIGL_NULL_POINTER;
    }

    /* check TIXI Version */
    if ( atof(tixiGetVersion()) < 2.2 ) {
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
                LOG(ERROR) << "Multiple CPACS version entries found. Please verify CPACS file.";
            }
            else if (tixiRet == ELEMENT_NOT_FOUND) {
                LOG(ERROR) << "No CPACS version information in file header. CPACS file seems to be too old.";
            }
            else {
                LOG(ERROR) << "Cannot read CPACS version,";
            }
            return TIGL_WRONG_CPACS_VERSION;
        }
        else {
            if (dcpacsVersion < (double) TIGL_MAJOR_VERSION) {
                LOG(ERROR) << "Too old CPACS dataset. CPACS version has to be at least " << (double) TIGL_MAJOR_VERSION << "!";
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
            LOG(ERROR) << "No configuration specified!";
            return TIGL_ERROR;
        }
        char * tmpConfUID = NULL;
        tixiRet = tixiGetTextAttribute(tixiHandle, "/cpacs/vehicles/aircraft/model[1]", "uID", &tmpConfUID);
        if (tixiRet != SUCCESS) {
            tixiRet = tixiGetTextAttribute(tixiHandle, "/cpacs/vehicles/rotorcraft/model[1]", "uID", &tmpConfUID);
            if (tixiRet != SUCCESS) {
                LOG(ERROR) << "Problems reading configuration-uid!";
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
            LOG(ERROR) << "Configuration '" << configurationUID << "' not found!";
            return TIGL_ERROR;
        }
    }

    try {
        tigl::unique_ptr<tigl::CCPACSConfiguration> config(new tigl::CCPACSConfiguration(tixiHandle));
        // Build CPACS memory structure
        config->ReadCPACS(configurationUID.c_str());
        // Store configuration in handle container
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        *cpacsHandlePtr = manager.AddConfiguration(config.release());
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_OPEN_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_OPEN_FAILED;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglOpenCPACSConfiguration!";
        return TIGL_OPEN_FAILED;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglSaveCPACSConfiguration(const char* configurationUID, TiglCPACSConfigurationHandle cpacsHandle)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();

    if (!manager.IsValid(cpacsHandle)) {
        LOG(ERROR) << "Invalid cpacsHandle passed to tiglSaveCPACSConfiguration!";
        return TIGL_UNINITIALIZED;
    }

    tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

    try {
        config.WriteCPACS(configurationUID);
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglSaveCPACSConfiguration!";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_CLOSE_FAILED;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_CLOSE_FAILED;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglCloseCPACSConfiguration!";
        return TIGL_CLOSE_FAILED;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglGetCPACSTixiHandle(TiglCPACSConfigurationHandle cpacsHandle, TixiDocumentHandle* tixiHandlePtr)
{
    if (tixiHandlePtr == 0) {
        LOG(ERROR) << "Null pointer argument for tixiHandlePtr in function call to tiglGetCPACSTixiHandle.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *tixiHandlePtr = config.GetTixiDocumentHandle();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_NOT_FOUND;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_NOT_FOUND;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetCPACSTixiHandle!";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_NOT_FOUND;
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_NOT_FOUND;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIsCPACSConfigurationHandleValid!";
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
    catch (const tigl::CTiglError& err) {
        LOG(ERROR) << err.what();
        return err.getCode();
    }
    catch (const std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
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
    catch (const tigl::CTiglError& err) {
        LOG(ERROR) << err.what();
        return err.getCode();
    }
    catch (const std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
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
    catch (const tigl::CTiglError& err) {
        LOG(ERROR) << err.what();
        return err.getCode();
    }
    catch (const std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglWingGetUpperPoint.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingGetUpperPoint";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglWingGetLowerPoint.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetLowerPoint!";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglWingGetChordPoint.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetChordPoint!";
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
        LOG(ERROR) << "Null pointer argument for normalXPtr, normalYPtr or normalZPtr\n"
                   << "in function call to tiglWingGetChordNormal.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetChordNormal!";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingGetUpperPointAtDirection";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingGetLowerPointAtDirection";
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
        LOG(ERROR) << "Null pointer argument for eta, xsi or isOnTop\n"
                   << "in function call to tiglWingGetSegmentEtaXsi.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingGetSegmentEtaXsi";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglGetWingCount(TiglCPACSConfigurationHandle cpacsHandle, int* wingCountPtr)
{
    if (wingCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for wingCountPtr\n"
                   << "in function call to tiglGetWingCount.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *wingCountPtr = config.GetWingCount();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetWingCount!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                          int wingIndex,
                                                          int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for segmentCountPtr\n"
                   << "in function call to tiglWingGetSegmentCount.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *segmentCountPtr = wing.GetSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentCount!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int wingIndex,
                                                                   int* compSegmentCountPtr)
{
    if (compSegmentCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for compSegmentCountPtr\n"
                   << "in function call to tiglWingGetSegmentCount.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *compSegmentCountPtr = wing.GetComponentSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetComponentSegmentCount!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 int wingIndex,
                                                                 int compSegmentIndex,
                                                                 char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for uidNamePtr\n"
                   << "in function call to tiglWingGetComponentSegmentUID.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || compSegmentIndex < 1) {
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetComponentSegmentUID.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetComponentSegmentUID!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int wingIndex,
                                                                   const char * compSegmentUID,
                                                                   int * segmentIndex) 
{
    if (compSegmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for segmentUID\n"
                   << "in function call to tiglWingGetComponentSegmentIndex.";
        return TIGL_NULL_POINTER;
    }
    if (segmentIndex == 0) {
        LOG(ERROR) << "Null pointer argument for segmentIndex\n"
                   << "in function call to tiglWingGetComponentSegmentIndex.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);

        int nseg = wing.GetComponentSegmentCount();
        for (int i = 1; i <= nseg; ++i) {
            tigl::CCPACSWingComponentSegment& actSegment = wing.GetComponentSegment(i);
            if ( actSegment.GetUID() == std::string(compSegmentUID)) {
                *segmentIndex = i;
                return TIGL_SUCCESS;
            }
        }

        LOG(ERROR) << "Error in tiglWingGetComponentSegmentIndex: the wing with index=" << wingIndex << " has no segment with an UID="
                   << compSegmentUID << "!";
        *segmentIndex = -1;
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentGetSegmentIndex!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for segmentCountPtr\n"
                   << "in function call to tiglWingGetInnerConnectedSegmentCount.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetInnerConnectedSegmentCount!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for segmentCountPtr\n"
                   << "in function call to tiglWingGetOuterConnectedSegmentCount.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetOuterConnectedSegmentCount!";
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
        LOG(ERROR) << "Null pointer argument for connectedIndexPtr\n"
                   << "in function call to tiglWingGetInnerConnectedSegmentIndex.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetInnerConnectedSegmentIndex!";
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
        LOG(ERROR) << "Null pointer argument for connectedIndexPtr\n"
                   << "in function call to tiglWingGetOuterConnectedSegmentIndex.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetOuterConnectedSegmentIndex!";
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
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetInnerSectionAndElementIndex.";
        return TIGL_INDEX_ERROR;
    }

    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        LOG(ERROR) << "Null pointer argument for sectionIndexPtr or elementIndexPtr\n"
                   << "in function call to tiglWingGetInnerSectionAndElementIndex.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetInnerSectionAndElementIndex!";
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
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetOuterSectionAndElementIndex.";
        return TIGL_INDEX_ERROR;
    }

    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        LOG(ERROR) << "Null pointer argument for sectionIndexPtr or elementIndexPtr\n"
                   << "in function call to tiglWingGetOuterSectionAndElementIndex.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetOuterSectionAndElementIndex!";
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
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetInnerSectionAndElementUID.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetInnerSectionAndElementIndex!";
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
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetOuterSectionAndElementUID.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetOuterSectionAndElementIndex!";
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
        LOG(ERROR) << "Null pointer argument for profileNamePtr\n"
                   << "in function call to tiglWingGetProfileName.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || sectionIndex < 1 || elementIndex < 1) {
        LOG(ERROR) << "Wing, section or element index index in less than zero\n"
                   << "in function call to tiglWingGetProfileName.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        const tigl::CCPACSWingSection& section = wing.GetSection(sectionIndex);
        const tigl::CCPACSWingSectionElement& element = section.GetSectionElement(elementIndex);
        std::string profileUID = element.GetAirfoilUID();
        tigl::CCPACSWingProfile& profile = config.GetWingProfile(profileUID);

        *profileNamePtr = const_cast<char*>(profile.GetName().c_str());

        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetProfileName!";
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for uidNamePtr\n"
                   << "in function call to tiglWingGetUID.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1) {
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *uidNamePtr = const_cast<char*> (wing.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetUID!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                   const char * wingUID,
                                                   int* wingIndexPtr)
{
    if (wingUID == 0) {
        LOG(ERROR) << "Null pointer argument for wingUID\n"
                   << "in function call to tiglWingGetIndex.";
        return TIGL_NULL_POINTER;
    }
    if (wingIndexPtr == 0) {
        LOG(ERROR) << "Null pointer argument for wingIndexPtr\n"
                   << "in function call to tiglWingGetIndex.";
        return TIGL_NULL_POINTER;
    }

    *wingIndexPtr = -1;
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *wingIndexPtr = config.GetWingIndex(std::string(wingUID));
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        *wingIndexPtr = -1;
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetIndex!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for uidNamePtr\n"
                   << "in function call to tiglWingGetSegmentUID.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetSegmentUID.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentUID!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                          const char * segmentUID,
                                                          int * segmentIndex, 
                                                          int * wingIndex) 
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for segmentUID\n"
                   << "in function call to tiglWingGetSegmentIndex.";
        return TIGL_NULL_POINTER;
    }
    if (segmentIndex == 0) {
        LOG(ERROR) << "Null pointer argument for segmentIndex\n"
                   << "in function call to tiglWingGetSegmentIndex.";
        return TIGL_NULL_POINTER;
    }
    if (wingIndex == 0) {
        LOG(ERROR) << "Null pointer argument for wingIndex\n"
                   << "in function call to tiglWingGetSegmentIndex.";
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
            catch (const tigl::CTiglError&) {
                continue;
            }
        }

        LOG(ERROR) << "Error in tiglWingGetSegmentIndex: could not find a wing index with given uid \"" << segmentUID << "\".";
        *segmentIndex = -1;
        *wingIndex = -1;
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentIndex!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSectionCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                          int wingIndex,
                                                          int* sectionCount)
{
    if (sectionCount == 0) {
        LOG(ERROR) << "Null pointer argument for sectionCount\n"
                   << "in function call to tiglWingGetSectionCount.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1) {
        LOG(ERROR) << "Wing index is less than zero\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSectionCount!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int sectionIndex,
                                                        char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for uidNamePtr\n"
                   << "in function call to tiglWingGetSectionUID.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || sectionIndex < 1) {
        LOG(ERROR) << "Wing or segment index is less than zero\n"
                   << "in function call to tiglWingGetSectionUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        const tigl::CCPACSWingSection& section = wing.GetSection(sectionIndex);
        *uidNamePtr = const_cast<char*>(section.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSectionUID!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, 
                                                      int wingIndex,
                                                      TiglSymmetryAxis* symmetryAxisPtr)
{

    if (wingIndex < 1) {
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglGetWingSymmetry.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *symmetryAxisPtr = wing.GetSymmetryAxis();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetWingSymmetry!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentFindSegment(TiglCPACSConfigurationHandle cpacsHandle,
                                                                      const char *componentSegmentUID, double x, double y,
                                                                      double z, char** segmentUID, char** wingUID)
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for segmentUID\n"
                   << "in function call to tiglWingComponentSegmentFindSegment.";
        return TIGL_NULL_POINTER;
    }

    if (wingUID == 0) {
        LOG(ERROR) << "Null pointer argument for wingUID\n"
                   << "in function call to tiglWingComponentSegmentFindSegment.";
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
                if (cs.GetUID() == std::string(componentSegmentUID)) {
                    gp_Pnt nearestPointOnSegment;
                    double distance = 0;
                    const tigl::CCPACSWingSegment* segment =  cs.findSegment(x, y, z, nearestPointOnSegment, distance);
                    if (!segment) {
                        LOG(ERROR) << "Can not find any segment that does belong to the provided point.";
                        return TIGL_NOT_FOUND;
                    }
                    if (distance > 1e-2) {
                        // point does not lie on component segment
                        LOG(ERROR) << "Given point does not lie on component segment within 1cm tolerance."
                                   << " Deviation is " << distance*1000. << " mm.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentFindSegment!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   const char *componentSegmentUID, double eta, double xsi,
                                                                   double * x, double * y, double * z)
{
    if (!componentSegmentUID) {
        LOG(ERROR) << "Null pointer argument for componentSegmentUID \n"
                   << "in function call to tiglWingComponentSegmentPointGetPoint.";
        return TIGL_NULL_POINTER;
    }
    
    if (!x || !y || !z){
        LOG(ERROR) << "Null pointer argument for x, y, or z\n"
                   << "in function call to tiglWingComponentSegmentPointGetPoint.";
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
        LOG(ERROR) << "Invalid uid in tiglWingComponentSegmentPointGetPoint!";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentPointGetPoint!";
        return TIGL_ERROR;
    }
}

TiglReturnCode tiglWingComponentSegmentPointGetEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                      const char *componentSegmentUID,
                                                      double pX, double pY, double pZ,
                                                      double *eta, double *xsi, double *errorDistance)
{
    // check validity of inputs
    if (!componentSegmentUID) {
        LOG(ERROR) << "Null pointer argument for componentSegmentUID\n"
                   << "in function call to tiglWingComponentSegmentPointGetPoint.";
        return TIGL_NULL_POINTER;
    }

    if (!eta) {
        LOG(ERROR) << "Null pointer argument for eta\n"
                   << "in function call to tiglWingComponentSegmentPointGetPoint.";
        return TIGL_NULL_POINTER;
    }

    if (!xsi) {
        LOG(ERROR) << "Null pointer argument for xsi\n"
                   << "in function call to tiglWingComponentSegmentPointGetPoint.";
        return TIGL_NULL_POINTER;
    }

    if (!errorDistance) {
        LOG(ERROR) << "Null pointer argument for errorDistance\n"
                   << "in function call to tiglWingComponentSegmentPointGetPoint.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // get component segment
        tigl::CCPACSWingComponentSegment& segment = config.GetUIDManager()
                .ResolveObject<tigl::CCPACSWingComponentSegment>(componentSegmentUID);

        gp_Pnt pnt(pX, pY, pZ);
        segment.GetEtaXsi(pnt, *eta, *xsi);

        *errorDistance =  segment.GetPoint(*eta, *xsi).Distance(pnt);
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentPointGetPoint!";
        return TIGL_ERROR;
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentPointGetSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                const char *componentSegmentUID, double eta, double xsi,
                                                                                char** wingUID, char** segmentUID,
                                                                                double *segmentEta, double *segmentXsi)
{
    if (!segmentUID) {
        LOG(ERROR) << "Null pointer argument for segmentUID\n"
                   << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }

    if (!wingUID) {
        LOG(ERROR) << "Null pointer argument for wingUID\n"
                   << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }

    if (!segmentEta) {
        LOG(ERROR) << "Null pointer argument for segmentEta\n"
                   << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }

    if (!segmentXsi) {
        LOG(ERROR) << "Null pointer argument for segmentXsi\n"
                   << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CCPACSWingComponentSegment& cs = config.GetUIDManager()
                .ResolveObject<tigl::CCPACSWingComponentSegment>(componentSegmentUID);


        std::string segmentUIDTmp;
        cs.GetSegmentEtaXsi(eta, xsi, segmentUIDTmp, *segmentEta, *segmentXsi);

        tigl::CCPACSWing* wing = cs.GetParent()->GetParent();
        *wingUID = const_cast<char*>(wing->GetUID().c_str());

        // Get the segment
        tigl::CCPACSWingSegment& segment = config.GetUIDManager()
                .ResolveObject<tigl::CCPACSWingSegment>(segmentUIDTmp);

        *segmentUID = const_cast<char*>(segment.GetUID().c_str());

        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentPointGetSegmentEtaXsi!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingSegmentPointGetComponentSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                const char *segmentUID, const char * componentSegmentUID, 
                                                                                double segmentEta, double segmentXsi,
                                                                                double *eta, double *xsi)
{
                                    
    if (segmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for segmentUID\n"
                   << "in function call to tiglWingSegmentPointGetComponentSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }

    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for componentSegmentUID\n"
                   << "in function call to tiglWingSegmentPointGetComponentSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }
    
    if (eta == 0) {
        LOG(ERROR) << "Null pointer argument for eta\n"
                   << "in function call to tiglWingSegmentPointGetComponentSegmentEtaXsi.";
        return TIGL_NULL_POINTER;
    }
    
    if (xsi == 0) {
        LOG(ERROR) << "Null pointer argument for xsi\n"
                   << "in function call to tiglWingSegmentPointGetComponentSegmentEtaXsi.";
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
            catch (const tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
            
        }
        
        // the component segment was not found
        LOG(ERROR) << "Invalid component segment uid in tiglWingSegmentPointGetComponentSegmentEtaXsi";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingSegmentPointGetComponentSegmentEtaXsi!";
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
        LOG(ERROR) << "Null pointer argument for segmentUID\n"
                   << "in function call to tiglWingComponentSegmentGetSegmentIntersection.";
        return TIGL_NULL_POINTER;
    }

    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for componentSegmentUID\n"
                   << "in function call to tiglWingComponentSegmentGetSegmentIntersection.";
        return TIGL_NULL_POINTER;
    }
    
    if (segmentXsi == 0) {
        LOG(ERROR) << "Null pointer argument for segmentXsi\n"
                   << "in function call to tiglWingComponentSegmentGetSegmentIntersection.";
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
            catch (const tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
        }
        
        // the component segment was not found
        LOG(ERROR) << "Invalid component segment uid in tiglWingComponentSegmentGetSegmentIntersection";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingComponentSegmentGetSegmentIntersection!";
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
        LOG(ERROR) << "Null pointer for argument componentSegmentUID\n"
                   << "in function call to tiglWingComponentSegmentComputeEtaIntersection.";
        return TIGL_NULL_POINTER;
    }

    if (xsi == 0) {
        LOG(ERROR) << "Null pointer for argument xsi\n"
                   << "in function call to tiglWingComponentSegmentComputeEtaIntersection.";
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
            catch (const tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
        }

        // the component segment was not found
        LOG(ERROR) << "Invalid component segment uid in tiglWingComponentSegmentComputeEtaIntersection";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglWingComponentSegmentComputeEtaIntersection!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetNumberOfSegments(TiglCPACSConfigurationHandle cpacsHandle,
                                                                              const char * componentSegmentUID,
                                                                              int * nsegments)
{

    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for componentSegmentUID\n"
                   << "in function call to tiglWingComponentSegmentGetNumberOfSegments.";
        return TIGL_NULL_POINTER;
    }
    
    if (nsegments == 0) {
        LOG(ERROR) << "Null pointer argument for nsegments\n"
                   << "in function call to tiglWingComponentSegmentGetNumberOfSegments.";
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
            catch (const tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
        }
        // the component segment was not found
        LOG(ERROR) << "Invalid component segment uid in tiglWingComponentSegmentGetNumberOfSegments";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetNumberOfSegments!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        const char * componentSegmentUID,
                                                                        int  segmentIndex,
                                                                        char ** segmentUID) 
{
    
    if (componentSegmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for componentSegmentUID\n"
                   << "in function call to tiglWingComponentSegmentGetSegmentUID.";
        return TIGL_NULL_POINTER;
    }
    
    if (segmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for nsegments\n"
                   << "in function call to tiglWingComponentSegmentGetSegmentUID.";
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
                    LOG(ERROR) << "Invalid segment index in tiglWingComponentSegmentGetSegmentUID";
                    return TIGL_INDEX_ERROR;
                }
                *segmentUID = const_cast<char*>(segments[segmentIndex-1]->GetUID().c_str());
                
                return TIGL_SUCCESS;
            }
            catch (const tigl::CTiglError& err){
                if (err.getCode() == TIGL_UID_ERROR) {
                    continue;
                }
                else {
                    throw;
                }
            }
        }
        // the component segment was not found
        LOG(ERROR) << "Invalid component segment uid in tiglWingComponentSegmentGetSegmentUID";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetSegmentUID!";
        return TIGL_ERROR;
    }
}


/******************************************************************************/
/* Fuselage Functions                                                         */
/******************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglGetFuselageCount(TiglCPACSConfigurationHandle cpacsHandle, int* fuselageCountPtr)
{
    if (fuselageCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageCountPtr\n"
                   << "in function call to tiglGetFuselageCount.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *fuselageCountPtr = config.GetFuselageCount();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetFuselageCount!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int fuselageIndex,
                                                              int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for segmentCountPtr\n"
                   << "in function call to tiglFuselageGetSegmentCount.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *segmentCountPtr = fuselage.GetSegmentCount();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentCount!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSectionCenter(TiglCPACSConfigurationHandle cpacsHandle,
                                                               const char *fuselageSegmentUID,
                                                               double eta,
                                                               double * pointX,
                                                               double * pointY,
                                                               double * pointZ)
{
    if (pointX == 0 || pointY == 0 || pointZ == 0) {
        LOG(ERROR) << "Null pointer argument for pointX, pointY or pointZ\n"
                   << "in function call to tiglFuselageGetSectionCenter.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageSegmentUID == NULL) {
        LOG(ERROR) << "Null pointer argument for fuselageSegmentUID\n"
                   << "in function call to tiglFuselageGetSectionCenter.";
        return TIGL_NULL_POINTER;
    }

    if (eta < 0 || eta > 1) {
        LOG(ERROR) << "Argument eta is not in obligatory range [0, 1]\n"
                   << "in function call to tiglFuselageGetSectionCenter.";
        return TIGL_MATH_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // get component segment
        tigl::CCPACSFuselageSegment& segment = config.GetUIDManager()
                .ResolveObject<tigl::CCPACSFuselageSegment>(fuselageSegmentUID);

        // get ISO curve
        TopoDS_Shape curve = segment.getWireOnLoft(eta);

         // compute center of the ISO curve
         gp_Pnt centerPoint = GetCenterOfMass(curve);

         // assigne solution to return point
         *pointX = centerPoint.X();
         *pointY = centerPoint.Y();
         *pointZ = centerPoint.Z();

    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSectionCenter!";
        return TIGL_ERROR;
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetCrossSectionArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char *fuselageSegmentUID,
                                                                  double eta,
                                                                  double *area)
{
    if (area == NULL) {
        LOG(ERROR) << "Null pointer argument for area\n"
                   << "in function call to tiglFuselageGetCrossSectionArea.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageSegmentUID == NULL) {
        LOG(ERROR) << "Null pointer argument for fuselageSegmentUID\n"
                   << "in function call to tiglFuselageGetCrossSectionArea.";
        return TIGL_NULL_POINTER;
    }

    if (eta < 0 || eta > 1) {
        LOG(ERROR) << "Argument eta is not in obligatory range [0, 1]\n"
                   << "in function call to tiglFuselageGetCrossSectionArea.";
        return TIGL_MATH_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // get component segment
        tigl::CCPACSFuselageSegment& segment = config.GetUIDManager()
                .ResolveObject<tigl::CCPACSFuselageSegment>(fuselageSegmentUID);

        // get ISO curve as a wire
        TopoDS_Wire curve = TopoDS::Wire(segment.getWireOnLoft(eta));

        // get surface which is framed by the ISO curve
        TopoDS_Face surface = BuildFace(curve);

        // compute area
        double framedArea = GetArea(surface);

        // assigne solution to return value
        *area = framedArea;

    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetCrossSectionArea!";
        return TIGL_ERROR;
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetCenterLineLength(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char *fuselageUID,
                                                                  double *length)
{
    if (length == NULL) {
        LOG(ERROR) << "Null pointer argument for length\n"
                   << "in function call to tiglFuselageGetCenterLineLength.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageUID == NULL) {
        LOG(ERROR) << "Null pointer argument for fuselageUID\n"
                   << "in function call to tiglFuselageGetCenterLineLength.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageUID);

        double centerLineLength = 0;
        for (int segment_idx = 1; segment_idx <= fuselage.GetSegmentCount(); ++segment_idx) {
            tigl::CCPACSFuselageSegment& segment = fuselage.GetSegment(segment_idx);

            double pointX = 0.;
            double pointY = 0.;
            double pointZ = 0.;
            tiglFuselageGetSectionCenter(cpacsHandle, segment.GetUID().c_str(), 0., &pointX, &pointY, &pointZ);
            tigl::CTiglPoint centerPointBeginning(pointX, pointY, pointZ);

            tiglFuselageGetSectionCenter(cpacsHandle, segment.GetUID().c_str(), 1., &pointX, &pointY, &pointZ);
            tigl::CTiglPoint centerPointEnd(pointX, pointY, pointZ);

            // add distance of current segment to total distance
            centerLineLength += sqrt(centerPointBeginning.distance2(centerPointEnd));
        }

        // assigne solution to return value
        *length = centerLineLength;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetCenterLineLength!";
        return TIGL_ERROR;
    }

    return TIGL_SUCCESS;
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglFuselageGetPoint.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPoint!";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglFuselageGetPointAngle.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);

        gp_Pnt point = segment.GetPointAngle(eta, alpha);
        if ((point.X() == 0.0) && (point.Y() == 0.0) && (point.Z() == 0.0)) {
            return TIGL_ERROR;
        }
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointAngle!";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglFuselageGetPointAngleTranslated.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        gp_Pnt point = segment.GetPointAngle(eta, alpha, y_cs, z_cs, true);
        if ((point.X() == 0.0) && (point.Y() == 0.0) && (point.Z() == 0.0)) {
            return TIGL_ERROR;
        }
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointAngleTranslated!";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglFuselageGetPointOnXPlane.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointOnXPlane!";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglFuselageGetPointOnYPlane.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointOnYPlane!";
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
        LOG(ERROR) << "Null pointer argument for numPointsPtr\n"
                   << "in function call to tiglFuselageGetNumPointsOnXPlane.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetNumPointsOnXPlane!";
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
        LOG(ERROR) << "Null pointer argument for numPointsPtr\n"
                   << "in function call to tiglFuselageGetNumPointsOnYPlane.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetNumPointsOnYPlane!";
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
         LOG(ERROR) << "Null pointer argument for circumferencePtr\n"
                    << "in function call to tiglFuselageGetCircumference.";
         return TIGL_NULL_POINTER;
     }

     try {
         tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
         tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
         tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
         *circumferencePtr = fuselage.GetCircumference(segmentIndex, eta);
         return TIGL_SUCCESS;
     }
     catch (const tigl::CTiglError& ex) {
         LOG(ERROR) << ex.what();
         return ex.getCode();
     }
     catch (std::exception& ex) {
         LOG(ERROR) << ex.what();
         return TIGL_ERROR;
     }
     catch (...) {
         LOG(ERROR) << "Caught an exception in tiglFuselageGetCircumference!";
         return TIGL_ERROR;
     }
}




TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int fuselageIndex,
                                                                            int segmentIndex,
                                                                            int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for segmentCountPtr\n"
                   << "in function call to tiglFuselageGetStartConnectedSegmentCount.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetStartConnectedSegmentCount!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                          int fuselageIndex,
                                                                          int segmentIndex,
                                                                          int* segmentCountPtr)
{
    if (segmentCountPtr == 0) {
        LOG(ERROR) << "Null pointer argument for segmentCountPtr\n"
                   << "in function call to tiglFuselageGetEndConnectedSegmentCount.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetEndConnectedSegmentCount!";
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
        LOG(ERROR) << "Null pointer argument for connectedIndexPtr\n"
                   << "in function call to tiglFuselageGetStartConnectedSegmentIndex.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetStartConnectedSegmentIndex!";
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
        LOG(ERROR) << "Null pointer argument for connectedIndexPtr\n"
                   << "in function call to tiglFuselageGetEndConnectedSegmentIndex.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetEndConnectedSegmentIndex!";
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
        LOG(ERROR) << "fuselageIndex or segmentIndex argument is less than one\n"
                   << "in function call to tiglFuselageGetStartSectionAndElementUID.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetStartSectionAndElementIndex!";
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
        LOG(ERROR) << "FuselageIndex or segmentIndex argument less than one\n"
                   << "in function call to tiglFuselageGetEndSectionAndElementUID.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetEndSectionAndElementIndex!";
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
        LOG(ERROR) << "Null pointer argument for sectionIndexPtr or elementIndexPtr\n"
                   << "in function call to tiglFuselageGetStartSectionAndElementIndex.";
        return TIGL_NULL_POINTER;
    }
    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        LOG(ERROR) << "Null pointer argument for sectionIndexPtr or elementIndexPtr\n"
                   << "in function call to tiglFuselageGetStartSectionAndElementIndex.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetStartSectionAndElementIndex!";
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
        LOG(ERROR) << "Null pointer argument for sectionIndexPtr or elementIndexPtr\n"
                   << "in function call to tiglFuselageGetEndSectionAndElementIndex.";
        return TIGL_NULL_POINTER;
    }
    if (elementIndexPtr == 0 || sectionIndexPtr == 0) {
        LOG(ERROR) << "Null pointer argument for sectionIndexPtr or elementIndexPtr\n"
                   << "in function call to tiglFuselageGetEndSectionAndElementIndex.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetEndSectionAndElementIndex!";
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
        LOG(ERROR) << "Null pointer argument for profileNamePtr\n"
                   << "in function call to tiglFuselageGetProfileName.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || sectionIndex < 1 || elementIndex < 1) {
        LOG(ERROR) << "Fuselage, section or element index index in less than zero\n"
                   << "in function call to tiglFuselageGetProfileName.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSection& section = fuselage.GetSection(sectionIndex);
        tigl::CCPACSFuselageSectionElement& element = section.GetSectionElement(elementIndex);
        std::string profileUID = element.GetProfileUID();
        tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile(profileUID);

        *profileNamePtr = const_cast<char*>(profile.GetName().c_str());
        
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetProfileName!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int fuselageIndex,
                                                     char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for uidNamePtr\n"
                   << "in function call to tiglFuselageGetUID.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1) {
        LOG(ERROR) << "Fuselage or segment index index in less than zero\n"
                   << "in function call to tiglFuselageGetUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *uidNamePtr = const_cast<char*>(fuselage.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetUID!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                       const char * fuselageUID,
                                                       int* fuselageIndexPtr)
{
    if (fuselageUID == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageUID\n"
                   << "in function call to tiglFuselageGetIndex.";
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndexPtr == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageIndexPtr\n"
                   << "in function call to tiglFuselageGetIndex.";
        return TIGL_NULL_POINTER;
    }

    *fuselageIndexPtr = -1;
    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *fuselageIndexPtr = config.GetFuselageIndex(std::string(fuselageUID));
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        *fuselageIndexPtr = -1;
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetIndex!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int fuselageIndex,
                                                            int segmentIndex,
                                                            char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for uidNamePtr\n"
                   << "in function call to tiglFuselageGetSegmentUID.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Fuselage or segment index index in less than zero\n"
                   << "in function call to tiglFuselageGetSegmentUID.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentUID!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                              const char * segmentUID,
                                                              int * segmentIndex,
                                                              int * fuselageIndex)
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Null pointer argument for segmentUID\n"
                   << "in function call to tiglFuselageGetSegmentIndex.";
        return TIGL_NULL_POINTER;
    }
    if (segmentIndex == 0) {
        LOG(ERROR) << "Null pointer argument for segmentIndex\n"
                   << "in function call to tiglFuselageGetSegmentIndex.";
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndex == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageIndex\n"
                   << "in function call to tiglFuselageGetSegmentIndex.";
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
            catch (const tigl::CTiglError&) {
                continue;
            }
        }

        LOG(ERROR) << "Error in tiglFuselageGetSegmentIndex: could not find a fuselage index with given uid \"" << segmentUID << "\".";
        *segmentIndex = -1;
        *fuselageIndex = -1;
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentIndex!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSectionCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int fuselageIndex,
                                                              int* sectionCount)
{
    if (sectionCount == 0) {
        LOG(ERROR) << "Null pointer argument for sectionCount "
                   << "in function call to tiglFuselageGetSectionCount.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1) {
        LOG(ERROR) << "Fuselage index is less than zero "
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSectionCount!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int fuselageIndex,
                                                            int sectionIndex,
                                                            char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for uidNamePtr\n"
                   << "in function call to tiglFuselageGetSectionUID.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || sectionIndex < 1) {
        LOG(ERROR) << "Fuselage or segment index index in less than zero\n"
                   << "in function call to tiglFuselageGetSectionUID.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSectionUID!";
        return TIGL_ERROR;
    }
}








TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                          TiglSymmetryAxis* symmetryAxisPtr)
{
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Fuselage or segment index index is less than zero\n"
                   << "in function call to tiglGetFuselageSymmetry.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *symmetryAxisPtr = fuselage.GetSymmetryAxis();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetFuselageSymmetry!";
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
        LOG(ERROR) << "Null pointer argument for pointXPtr, pointYPtr or pointZPtr\n"
                   << "in function call to tiglFuselageGetMinumumDistanceToGround.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetPointOnYPlane!";
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
        LOG(ERROR) << "Null pointer argument for rotorCountPtr\n"
                   << "in function call to tiglGetRotorCount.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *rotorCountPtr = config.GetRotorCount();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetRotorCount!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                  int rotorIndex,
                                                  char** uidNamePtr)
{
    if (uidNamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for uidNamePtr\n"
                   << "in function call to tiglRotorGetUID.";
        return TIGL_NULL_POINTER;
    }

    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorGetUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *uidNamePtr = const_cast<char*> (rotor.GetUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetUID!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                    const char* rotorUID,
                                                    int* rotorIndexPtr)
{
    if (rotorUID == 0) {
        LOG(ERROR) << "Null pointer argument for rotorUID\n"
                   << "in function call to tiglRotorGetIndex.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndexPtr == 0) {
        LOG(ERROR) << "Null pointer argument for rotorIndexPtr\n"
                   << "in function call to tiglRotorGetIndex.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *rotorIndexPtr = config.GetRotorIndex(std::string(rotorUID));
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        *rotorIndexPtr = -1;
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        *rotorIndexPtr = -1;
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetIndex!";
        *rotorIndexPtr = -1;
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetRadius(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int rotorIndex,
                                                     double *radiusPtr)
{
    if (radiusPtr == 0) {
        LOG(ERROR) << "Null pointer argument for radiusPtr\n"
                   << "in function call to tiglRotorGetRadius.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetRadius!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetReferenceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int rotorIndex,
                                                            double *referenceAreaPtr)
{
    if (referenceAreaPtr == 0) {
        LOG(ERROR) << "Null pointer argument for referenceAreaPtr\n"
                   << "in function call to tiglRotorGetReferenceArea.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetReferenceArea!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetTotalBladePlanformArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                     int rotorIndex,
                                                                     double *totalBladePlanformAreaPtr)
{
    if (totalBladePlanformAreaPtr == 0) {
        LOG(ERROR) << "Null pointer argument for totalBladePlanformAreaPtr\n"
                   << "in function call to tiglRotorGetTotalBladePlanformArea.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetTotalBladePlanformArea!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetSolidity(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int rotorIndex,
                                                       double *solidityPtr)
{
    if (solidityPtr == 0) {
        LOG(ERROR) << "Null pointer argument for solidityPtr\n"
                   << "in function call to tiglRotorGetSolidity.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetSolidity!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                          int rotorIndex,
                                                          double *surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        LOG(ERROR) << "Null pointer argument for surfaceAreaPtr\n"
                   << "in function call to tiglRotorGetSurfaceArea.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorGetSurfaceArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *surfaceAreaPtr = rotor.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetSurfaceArea!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetVolume(TiglCPACSConfigurationHandle cpacsHandle, 
                                                     int rotorIndex,
                                                     double *volumePtr)
{
    if (volumePtr == 0) {
        LOG(ERROR) << "Null pointer argument for volumePtr\n"
                   << "in function call to tiglRotorGetVolume.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglrotorGetVolume.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        *volumePtr = rotor.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglrotorGetVolume!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetTipSpeed(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int rotorIndex,
                                                       double *tipSpeedPtr)
{
    if (tipSpeedPtr == 0) {
        LOG(ERROR) << "Null pointer argument for tipSpeedPtr\n"
                   << "in function call to tiglRotorGetTipSpeed.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetTipSpeed!";
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
        LOG(ERROR) << "Null pointer argument for rotorBladeCountPtr\n"
                   << "in function call to tiglRotorGetRotorBladeCount.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorGetRotorBladeCount!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetWingIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                             int rotorIndex,
                                                             int rotorBladeIndex,
                                                             int* wingIndexPtr)
{
    if (wingIndexPtr == 0) {
        LOG(ERROR) << "Null pointer argument for wingIndexPtr\n"
                   << "in function call to tiglRotorBladeGetWingIndex.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetWingIndex.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetWingIndex.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        tigl::CCPACSRotorBladeAttachment& rotorBladeAttachment = rotorBlade.GetRotorBladeAttachment();
        *wingIndexPtr = rotorBladeAttachment.GetWingIndex();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetWingIndex!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetWingUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int rotorIndex,
                                                           int rotorBladeIndex,
                                                           char** wingUIDPtr)
{
    if (wingUIDPtr == 0) {
        LOG(ERROR) << "Null pointer argument for wingUIDPtr\n"
                   << "in function call to tiglRotorBladeGetWingUID.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetWingUID.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetWingUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        tigl::CCPACSRotorBladeAttachment& rotorBladeAttachment = rotorBlade.GetRotorBladeAttachment();
        *wingUIDPtr = const_cast<char*> (rotorBladeAttachment.GetRotorBladeUID().c_str());
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetWingUID!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetAzimuthAngle(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int rotorIndex,
                                                                int rotorBladeIndex,
                                                                double* azimuthAnglePtr)
{
    if (azimuthAnglePtr == 0) {
        LOG(ERROR) << "Null pointer argument for azimuthAnglePtr\n"
                   << "in function call to tiglRotorBladeGetAzimuthAngle.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetAzimuthAngle.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetAzimuthAngle.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *azimuthAnglePtr = rotorBlade.GetAzimuthAngle();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetAzimuthAngle!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetRadius(TiglCPACSConfigurationHandle cpacsHandle,
                                                          int rotorIndex,
                                                          int rotorBladeIndex,
                                                          double* radiusPtr)
{
    if (radiusPtr == 0) {
        LOG(ERROR) << "Null pointer argument for radiusPtr\n"
                   << "in function call to tiglRotorBladeGetRadius.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetRadius.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetRadius.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *radiusPtr = rotorBlade.GetRadius();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetRadius!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetPlanformArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                                int rotorIndex,
                                                                int rotorBladeIndex,
                                                                double *planformAreaPtr)
{
    if (planformAreaPtr == 0) {
        LOG(ERROR) << "Null pointer argument for planformAreaPtr\n"
                   << "in function call to tiglRotorBladeGetPlanformArea.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetPlanformArea.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetPlanformArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *planformAreaPtr = rotorBlade.GetPlanformArea();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetPlanformArea!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int rotorIndex,
                                                               int rotorBladeIndex,
                                                               double *surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        LOG(ERROR) << "Null pointer argument for surfaceAreaPtr\n"
                   << "in function call to tiglRotorBladeGetSurfaceArea.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetSurfaceArea.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetSurfaceArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *surfaceAreaPtr = rotorBlade.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetSurfaceArea!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetVolume(TiglCPACSConfigurationHandle cpacsHandle, 
                                                          int rotorIndex,
                                                          int rotorBladeIndex,
                                                          double *volumePtr)
{
    if (volumePtr == 0) {
        LOG(ERROR) << "Null pointer argument for volumePtr\n"
                   << "in function call to tiglRotorBladeGetVolume.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetVolume.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetVolume.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *volumePtr = rotorBlade.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetVolume!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetTipSpeed(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int rotorIndex,
                                                            int rotorBladeIndex,
                                                            double* tipSpeedPtr)
{
    if (tipSpeedPtr == 0) {
        LOG(ERROR) << "Null pointer argument for tipSpeedPtr\n"
                   << "in function call to tiglRotorBladeGetTipSpeed.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetTipSpeed.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetTipSpeed.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *tipSpeedPtr = rotorBlade.GetTipSpeed();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetTipSpeed!";
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
        LOG(ERROR) << "Null pointer argument for radiusPtr\n"
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_INDEX_ERROR;
    }
    if (segmentIndex < 1) {
        LOG(ERROR) << "Segment index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_INDEX_ERROR;
    }
    if ((eta < 0.) || (eta > 1.)) {
        LOG(ERROR) << "Eta not in range [0,1]\n"
                   << "in function call to tiglRotorBladeGetLocalRadius.";
        return TIGL_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *radiusPtr = rotorBlade.GetLocalRadius(segmentIndex, eta);
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetLocalRadius!";
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
        LOG(ERROR) << "Null pointer argument for chordPtr\n"
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_INDEX_ERROR;
    }
    if (segmentIndex < 1) {
        LOG(ERROR) << "Segment index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_INDEX_ERROR;
    }
    if ((eta < 0.) || (eta > 1.)) {
        LOG(ERROR) << "Eta not in range [0,1]\n"
                   << "in function call to tiglRotorBladeGetLocalChord.";
        return TIGL_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *chordPtr = rotorBlade.GetLocalChord(segmentIndex, eta);
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetLocalChord!";
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
        LOG(ERROR) << "Null pointer argument for twistAnglePtr\n"
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_NULL_POINTER;
    }
    if (rotorIndex < 1) {
        LOG(ERROR) << "Rotor index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_INDEX_ERROR;
    }
    if (rotorBladeIndex < 1) {
        LOG(ERROR) << "Rotor blade index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_INDEX_ERROR;
    }
    if (segmentIndex < 1) {
        LOG(ERROR) << "segment index is less than one\n"
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_INDEX_ERROR;
    }
    if ((eta < 0.) || (eta > 1.)) {
        LOG(ERROR) << "eta not in range [0,1]\n"
                   << "in function call to tiglRotorBladeGetLocalTwistAngle.";
        return TIGL_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSRotor& rotor = config.GetRotor(rotorIndex);
        tigl::CTiglAttachedRotorBlade& rotorBlade = rotor.GetRotorBlade(rotorBladeIndex);
        *twistAnglePtr = rotorBlade.GetLocalTwistAngle(segmentIndex, eta);
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglRotorBladeGetLocalTwistAngle!";
        return TIGL_ERROR;
    }
}


/******************************************************************************/
/* Boolean Functions                                                          */
/******************************************************************************/

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

        if (uidManager.HasGeometricComponent(componentUidOne) && uidManager.HasGeometricComponent(componentUidTwo)) {
            TopoDS_Shape compoundOne = uidManager.GetGeometricComponent(componentUidOne).GetLoft()->Shape();
            TopoDS_Shape compoundTwo = uidManager.GetGeometricComponent(componentUidTwo).GetLoft()->Shape();
            
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
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

        if (uidManager.HasGeometricComponent(componentUid)) {
            TopoDS_Shape shape = uidManager.GetGeometricComponent(componentUid).GetLoft()->Shape();
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIntersectWithPlane!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectWithPlaneSegment(TiglCPACSConfigurationHandle cpacsHandle,
                                                                const char*  componentUid,
                                                                double p1x, double p1y, double p1z,
                                                                double p2x, double p2y, double p2z,
                                                                double wx, double wy, double wz,
                                                                char** intersectionID)
{
    if (!componentUid) {
        LOG(ERROR) << "Null pointer for argument componentUid in tiglIntersectWithPlaneSegment.";
        return TIGL_NULL_POINTER;
    }
    if (!intersectionID) {
        LOG(ERROR) << "Null pointer for argument intersectionID in tiglIntersectWithPlaneSegment.";
        return TIGL_NULL_POINTER;
    }

    tigl::CTiglPoint P1(p1x, p1y, p1z);
    tigl::CTiglPoint P2(p2x, p2y, p2z);
    tigl::CTiglPoint W (wx,  wy,  wz );

    if ( W.norm2Sqr() < 1e-10) {
        LOG(ERROR) << "Normal vector must not be zero in tiglIntersectWithPlaneSegment.";
        return TIGL_MATH_ERROR;
    }

    // check if p1 and p2 are distinct points
    if ( P1.distance2(P2) < 1e-10 ) {
        LOG(ERROR) << "Point 1 and Point 2 must be unequal.";
        return TIGL_MATH_ERROR;
    }

    // check if u = (p2 - p1) and w are linearly dependent
    double uw = tigl::CTiglPoint::inner_prod(W, P2-P1);
    double uu = (P2-P1).norm2Sqr();
    double ww = W.norm2Sqr();
    if ( uu*ww - uw*uw < 1e-10 ) {
        LOG(ERROR) << "( Point 2 - Point 1 ) and w must be linearly independent";
        return TIGL_MATH_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        tigl::ITiglGeometricComponent* component = &uidManager.GetGeometricComponent(componentUid);
        if (component) {
            TopoDS_Shape shape = component->GetLoft()->Shape();
            gp_Pnt p1(p1x, p1y, p1z);
            gp_Pnt p2(p2x, p2y, p2z);
            gp_Dir w(wx, wy, wz);

            tigl::CTiglIntersectionCalculation Intersector(&config.GetShapeCache(),
                                                           componentUid,
                                                           shape,
                                                           p1, p2, w,
                                                           false);

            std::string id = Intersector.GetID();
            *intersectionID = (char*) config.GetMemoryPool().MakeNontempString(id.c_str());

            return TIGL_SUCCESS;
        }
        else {
            LOG(ERROR) << "UID can not be found in tiglIntersectWithPlaneSegment.";
            return TIGL_UID_ERROR;
        }
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIntersectWithPlaneSegment!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglGetCurveIntersection(TiglCPACSConfigurationHandle cpacsHandle,
                                                           const char* curvesID1, int curve1Idx,
                                                           const char* curvesID2, int curve2Idx,
                                                           double tolerance,
                                                           char** intersectionID)
{

    if (!curvesID1) {
        LOG(ERROR) << "Null pointer for argument curvesID1 in tiglGetCurveIntersection.";
        return TIGL_NULL_POINTER;
    }

    if (!curvesID2) {
        LOG(ERROR) << "Null pointer for argument curvesID2 in tiglGetCurveIntersection.";
        return TIGL_NULL_POINTER;
    }

    if (!intersectionID) {
        LOG(ERROR) << "Null pointer for argument intersectionID in tiglGetCurveIntersection.";
        return TIGL_NULL_POINTER;
    }

    if ( tolerance < 0 ) {
        LOG(ERROR) << "tolerance musst be non negative in tiglGetCurveIntersection (tolerance = "<<tolerance<<").";
        return TIGL_MATH_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // check if the cuve indices are valid
        int linecount1;
        tiglIntersectGetLineCount(cpacsHandle, curvesID1, &linecount1);
        if ( (curve1Idx > linecount1) || (curve1Idx<1) ) {
            LOG(ERROR) << "argument curve1Idx = "<<curvesID1<<" is invalid in tiglGetCurveIntersection (lineCount of curve "<<curvesID1<<": "<<linecount1<<").";
            return TIGL_INDEX_ERROR;
        }

        int linecount2;
        tiglIntersectGetLineCount(cpacsHandle, curvesID2, &linecount2);
        if ( (curve2Idx > linecount2) || (curve2Idx<1) ) {
            LOG(ERROR) << "argument curve2Idx = "<<curvesID2<<" is invalid in tiglGetCurveIntersection (lineCount of curve "<<curvesID2<<": "<<linecount2<<").";
            return TIGL_INDEX_ERROR;
        }

        // now calculate the intersection
        tigl::CTiglIntersectionCalculation Intersector(&config.GetShapeCache(),
                                                       curvesID1, curve1Idx,
                                                       curvesID2, curve2Idx,
                                                       tolerance);

        std::string id = Intersector.GetID();
        *intersectionID = (char*) config.GetMemoryPool().MakeNontempString(id.c_str());

        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch(...) {
        LOG(ERROR) << "Caught an exception in tiglGetCurveIntersection";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIntersectComponents!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglGetCurveIntersectionCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                const char* intersectionID,
                                                                int* pointCount)
{
    if (!pointCount) {
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
        *pointCount = Intersector.GetCountIntersectionPoints();

        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglIntersectComponents!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglGetCurveIntersectionPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                                const char* intersectionID,
                                                                int pointIdx,
                                                                double* pointX,
                                                                double* pointY,
                                                                double* pointZ)
{
    if (!pointX) {
        LOG(ERROR) << "Null pointer for argument pointX in tiglGetCurveIntersectionPoint.";
        return TIGL_NULL_POINTER;
    }
    if (!pointY) {
        LOG(ERROR) << "Null pointer for argument pointY in tiglGetCurveIntersectionPoint.";
        return TIGL_NULL_POINTER;
    }
    if (!pointZ) {
        LOG(ERROR) << "Null pointer for argument pointZ in tiglGetCurveIntersectionPoint.";
        return TIGL_NULL_POINTER;
    }
    if (!intersectionID) {
        LOG(ERROR) << "Null pointer for argument intersectionID in tiglGetCurveIntersectionPoint.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglShapeCache& cache = config.GetShapeCache();

        tigl::CTiglIntersectionCalculation Intersector(cache, intersectionID);

        int npoints = Intersector.GetCountIntersectionPoints();
        if ( pointIdx < 1 || pointIdx > npoints ) {
            return TIGL_INDEX_ERROR;
        }

        gp_Pnt p = BRep_Tool::Pnt( Intersector.GetVertex(pointIdx) );

        *pointX = p.X();
        *pointY = p.Y();
        *pointZ = p.Z();

        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetCurveIntersectionPoint!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglGetCurveParameter (TiglCPACSConfigurationHandle cpacsHandle,
                                                         const char* curveID,
                                                         int curveIdx,
                                                         double pointX,
                                                         double pointY,
                                                         double pointZ,
                                                         double* eta)
{
    if (!eta) {
        LOG(ERROR) << "Null pointer for argument eta in tiglGetCurveParameter .";
        return TIGL_NULL_POINTER;
    }
    if (!curveID) {
        LOG(ERROR) << "Null pointer for argument curveID in tiglGetCurveParameter .";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglShapeCache& cache = config.GetShapeCache();

        gp_Pnt p(pointX, pointY, pointZ);

        tigl::CTiglIntersectionCalculation IntersectorLine(cache, curveID);
        TopoDS_Wire wire = IntersectorLine.GetWire(curveIdx);

        *eta = ProjectPointOnWire(wire, p);

        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglGetCurveParameter !";
        return TIGL_ERROR;
    }
}



/*****************************************************************************************************/
/*                         Export Functions                                                          */
/*****************************************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglExportIGES(TiglCPACSConfigurationHandle cpacsHandle, const char *filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr"
                   << "in function call to tiglExportIGES.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::PTiglCADExporter exporter = tigl::createExporter("iges");
        exporter->AddConfiguration(config);
        bool ret = exporter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportIGES!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedWingFuselageIGES(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char *filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportFusedWingFuselageIGES.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::PTiglCADExporter exporter = tigl::createExporter("iges");
        exporter->AddFusedConfiguration(config);
        bool ret = exporter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFusedWingFuselageIGES!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportSTEP(TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportSTEP.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::PTiglCADExporter exporter = tigl::createExporter("step");
        exporter->AddConfiguration(config);
        bool ret = exporter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportSTEP!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedSTEP(TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportFusedSTEP.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::PTiglCADExporter exporter = tigl::createExporter("step");
        exporter->AddFusedConfiguration(config);
        bool ret = exporter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFusedSTEP!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingSTL(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                          const char *filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedWingSTL.";
        return TIGL_NULL_POINTER;
    }
    if (wingIndex < 1) {
        LOG(ERROR) << "wingIndex in less or equal zero\n"
                   << "in function call to tiglExportMeshedWingSTL.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        PNamedShape loft = wing.GetLoft();
        
        tigl::PTiglCADExporter exporter = tigl::createExporter("stl");
        exporter->AddShape(loft, tigl::TriangulatedExportOptions(deflection));
        bool ret = exporter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedWingSTL!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingSTLByUID(TiglCPACSConfigurationHandle cpacsHandle, 
                                                               const char* wingUID,
                                                               const char *filenamePtr, 
                                                               double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr"
                   << "in function call to tiglExportMeshedWingSTLByUID.";
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Null pointer argument for wingUID"
                   << "in function call to tiglExportMeshedWingSTLByUID.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        for (int iWing = 1; iWing <= config.GetWingCount(); ++iWing) {
            tigl::CCPACSWing& wing = config.GetWing(iWing);
            if (wing.GetUID() == wingUID) {
                PNamedShape loft = wing.GetLoft();

                tigl::PTiglCADExporter exporter = tigl::createExporter("stl");
                exporter->AddShape(loft, tigl::TriangulatedExportOptions(deflection));
                bool ret = exporter->Write(filenamePtr);
                return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
            }
        }
        
        LOG(ERROR) << "Wing with UID " << wingUID << " not found"
                   << "in function call to tiglExportMeshedWingSTLByUID.";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedWingSTLByUID!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageSTL(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                              const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedFuselageSTL.";
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndex < 1) {
        LOG(ERROR) << "fuselageIndex in less or equal zero\n"
                   << "in function call to tiglExportMeshedFuselageSTL.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        PNamedShape loft = fuselage.GetLoft();

        tigl::PTiglCADExporter exporter = tigl::createExporter("stl");
        exporter->AddShape(loft, tigl::TriangulatedExportOptions(deflection));
        bool ret = exporter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageSTL!";
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
                   << "in function call to tiglExportMeshedFuselageSTLByUID.";
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageUID"
                   << "in function call to tiglExportMeshedFuselageSTLByUID.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        
        for (int ifusel = 1; ifusel <= config.GetFuselageCount(); ++ifusel) {
            tigl::CCPACSFuselage& fuselage = config.GetFuselage(ifusel);
            if (fuselage.GetUID() == fuselageUID) {
                PNamedShape loft = fuselage.GetLoft();
                
                tigl::PTiglCADExporter exporter = tigl::createExporter("stl");
                exporter->AddShape(loft, tigl::TriangulatedExportOptions(deflection));
                bool ret = exporter->Write(filenamePtr);
                return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
            }
        }
        
        LOG(ERROR) << "Fuselage with UID " << fuselageUID << " not found"
                   << "in function call to tiglExportMeshedFuselageSTLByUID.";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageSTLByUID!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometrySTL(TiglCPACSConfigurationHandle cpacsHandle, const char *filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedGeometrySTL.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::PTiglCADExporter exporter = tigl::createExporter("stl");

        exporter->AddConfiguration(config, tigl::TriangulatedExportOptions(deflection));
        bool ret = exporter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedGeometrySTL!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, const int wingIndex,
                                                                 const char* filenamePtr, const double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedWingVTKByIndex.";
        return TIGL_NULL_POINTER;
    }
    if (wingIndex < 1) {
        LOG(ERROR) << "wingIndex in less or equal zero\n"
                   << "in function call to tiglExportMeshedWingVTKByIndex.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::PTiglCADExporter exporter = tigl::createExporter("vtk");

        exporter->AddShape(wing.GetLoft(), &config, tigl::TriangulatedExportOptions(deflection));
        if (exporter->Write(filenamePtr)) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    // all exceptions from the standard library 
    catch (tigl::CTiglError & ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch(char *str) {
        LOG(ERROR) << str;
        return TIGL_ERROR;
    }
    catch(std::string& str) {
        LOG(ERROR) << str;
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglExportMeshedWingVTKByIndex";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID,
                                                               const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedWingVTKByUID.";
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Null pointer argument for wingUID\n"
                   << "in function call to tiglExportMeshedWingVTKByUID.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        tigl::PTiglCADExporter exporter = tigl::createExporter("vtk");

        exporter->AddShape(wing.GetLoft(), &config, tigl::TriangulatedExportOptions(deflection));
        if (exporter->Write(filenamePtr)) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    // all exceptions from the standard library
    catch (tigl::CTiglError & ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch(char *str) {
        LOG(ERROR) << str;
        return TIGL_ERROR;
    }
    catch(std::string& str) {
        LOG(ERROR) << str;
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglExportMeshedWingVTKByUID";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, const int fuselageIndex,
                                                                     const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedFuselageVTKByIndex.";
        return TIGL_NULL_POINTER;
    }
    if (fuselageIndex < 1) {
        LOG(ERROR) << "fuselageIndex in less or equal zero\n"
                   << "in function call to tiglExportMeshedFuselageVTKByIndex.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::PTiglCADExporter exporter = tigl::createExporter("vtk");
        exporter->AddShape(fuselage.GetLoft(), tigl::TriangulatedExportOptions(deflection));
        if (exporter->Write(filenamePtr)) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageVTKByIndex!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID,
                                                                   const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedFuselageVTKByUID.";
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageIndex\n"
                   << "in function call to tiglExportMeshedFuselageVTKByUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageUID);
        tigl::PTiglCADExporter exporter = tigl::createExporter("vtk");
        exporter->AddShape(fuselage.GetLoft(), tigl::TriangulatedExportOptions(deflection));
        if (exporter->Write(filenamePtr)) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedFuselageVTKByUID!";
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometryVTK(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedGeometryVTK.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::PTiglCADExporter exporter = tigl::createExporter("vtk");
        exporter->AddFusedConfiguration(config, tigl::TriangulatedExportOptions(deflection));
        if (exporter->Write(filenamePtr)) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedGeometryVTK!";
        return TIGL_ERROR;
    }
}




TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKSimpleByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID,
                                                                     const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedWingVTKSimpleByUID.";
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Null pointer argument for wingUID\n"
                   << "in function call to tiglExportMeshedWingVTKSimpleByUID.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        tigl::ExporterOptions exportOptions = tigl::getExportConfig("vtk");
        exportOptions.Set("WriteMetaData", false);
        tigl::PTiglCADExporter exporter = tigl::createExporter("vtk", exportOptions);

        exporter->AddShape(wing.GetLoft(), &config, tigl::TriangulatedExportOptions(deflection));
        if (exporter->Write(filenamePtr)) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    // all exceptions from the standard library
    catch (tigl::CTiglError & ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch(char *str) {
        LOG(ERROR) << str;
        return TIGL_ERROR;
    }
    catch(std::string& str) {
        LOG(ERROR) << str;
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an unknown exception in tiglExportMeshedWingVTKSimpleByUID";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportFuselageColladaByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID, const char* filenamePtr, double deflection) 
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportFuselageColladaByUID.";
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageUID\n"
                   << "in function call to tiglExportFuselageColladaByUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageUID);
        tigl::PTiglCADExporter colladaWriter = tigl::createExporter("dae");
        colladaWriter->AddShape(fuselage.GetLoft(), tigl::TriangulatedExportOptions(deflection));
        bool ret = colladaWriter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFuselageColladaByUID!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportWingColladaByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID, const char* filenamePtr, double deflection) 
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportWingColladaByUID.";
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Null pointer argument for wingUID\n"
                   << "in function call to tiglExportWingColladaByUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        tigl::PTiglCADExporter colladaWriter = tigl::createExporter("dae");
        colladaWriter->AddShape(wing.GetLoft(), tigl::TriangulatedExportOptions(deflection));
        bool ret = colladaWriter->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportWingColladaByUID!";
        return TIGL_ERROR;
    }
}



TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometryVTKSimple(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportMeshedGeometryVTKSimple.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::ExporterOptions expConfig = tigl::getExportConfig("vtk");
        expConfig.Set("WriteMetaData", false);
        tigl::PTiglCADExporter exporter = tigl::createExporter("vtk", expConfig);

        exporter->AddFusedConfiguration(config, tigl::TriangulatedExportOptions(deflection));
        if (exporter->Write(filenamePtr)) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportMeshedGeometryVTKSimple!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedBREP(TiglCPACSConfigurationHandle cpacsHandle, const char* filename)
{
    if (filename == 0) {
        LOG(ERROR) << "Null pointer argument for filename\n"
                   << "in function call to tiglExportFusedBREP.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::PTiglCADExporter exporter = tigl::createExporter("brep");
        exporter->AddFusedConfiguration(config);
        bool ret = exporter->Write(filename);
        return ret == true? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFusedBREP!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportFuselageBREPByUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                              const char* fuselageUID,
                                                              const char* filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportFuselageBREPByUID.";
        return TIGL_NULL_POINTER;
    }
    if (fuselageUID == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageUID\n"
                   << "in function call to tiglExportFuselageBREPByUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageUID);
        tigl::PTiglCADExporter writer = tigl::createExporter("brep");
        writer->AddShape(fuselage.GetLoft());
        bool ret = writer->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportFuselageBREPByUID!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportWingBREPByUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                              const char* wingUID,
                                                              const char* filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Null pointer argument for filenamePtr\n"
                   << "in function call to tiglExportWingBREPByUID.";
        return TIGL_NULL_POINTER;
    }
    if (wingUID == 0) {
        LOG(ERROR) << "Null pointer argument for fuselageUID\n"
                   << "in function call to tiglExportWingBREPByUID.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        tigl::PTiglCADExporter writer = tigl::createExporter("brep");
        writer->AddShape(wing.GetLoft());
        bool ret = writer->Write(filenamePtr);
        return ret ? TIGL_SUCCESS : TIGL_WRITE_FAILED;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportWingBREPByUID!";
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
        LOG(ERROR) << "Null pointer argument for componentSegmentUID\n"
                   << "in function call to tiglWingComponentSegmentGetMaterialCount.";
        return TIGL_NULL_POINTER;
    }

    if (!materialCount) {
        LOG(ERROR) << "Null pointer argument for materialCount\n"
                   << "in function call to tiglWingComponentSegmentGetMaterialCount.";
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
        LOG(ERROR) << "Invalid uid in tiglWingComponentSegmentGetMaterialCount";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetMaterialCount!";
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
        LOG(ERROR) << "Null pointer argument for componentSegmentUID\n"
                   << "in function call to tiglWingComponentSegmentGetMaterialUID.";
        return TIGL_NULL_POINTER;
    }

    if (!uid) {
        LOG(ERROR) << "Null pointer argument for uid\n"
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

                const tigl::CCPACSMaterialDefinition* material = list.at(materialIndex-1);
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
        LOG(ERROR) << "Invalid uid in tiglWingComponentSegmentGetMaterialUID";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetMaterialUID!";
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
        LOG(ERROR) << "Null pointer argument for componentSegmentUID\n"
                   << "in function call to tiglWingComponentSegmentGetMaterialThickness.";
        return TIGL_NULL_POINTER;
    }

    if (!thickness){
        LOG(ERROR) << "Null pointer argument for thickness\n"
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

                const tigl::CCPACSMaterialDefinition* material = list.at(materialIndex-1);
                if (!material) {
                    return TIGL_ERROR;
                }

                if (material->GetThickness_choice2()) {
                    *thickness = *material->GetThickness_choice2();
                    return TIGL_SUCCESS;
                } else {
                    return TIGL_UNINITIALIZED;
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
        LOG(ERROR) << "invalid uid in tiglWingComponentSegmentGetMaterialThickness";
        return TIGL_UID_ERROR;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingComponentSegmentGetMaterialThickness!";
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
        LOG(ERROR) << "Fuselage index index is less than zero\n"
                   << "in function call to tiglFuselageGetVolume.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *volumePtr = fuselage.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetVolume!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetVolume(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                    double *volumePtr)
{
    if (wingIndex < 1) {
        LOG(ERROR) << "Wing index index is less than zero\n"
                   << "in function call to tiglWingGetVolume.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *volumePtr = wing.GetVolume();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetVolume!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int wingIndex,
                                                           int segmentIndex,
                                                           double* volumePtr)
{
    if (volumePtr == 0) {
        LOG(ERROR) << "Null pointer argument for volumePtr\n"
                   << "in function call to tiglWingGetSegmentVolume.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetSegmentVolume.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentVolume!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int fuselageIndex,
                                                               int segmentIndex,
                                                               double* volumePtr)
{
    if (volumePtr == 0) {
        LOG(ERROR) << "Null pointer argument for volumePtr\n"
                   << "in function call to tiglFuselageGetSegmentVolume.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Fuselage or segment index index in less than zero\n"
                   << "in function call to tiglFuselageGetSegmentVolume.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentVolume!";
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
        LOG(ERROR) << "Wing index index is less than zero\n"
                   << "in function call to tiglWingGetSurfaceArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *surfaceAreaPtr = wing.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSurfaceArea!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                             double *surfaceAreaPtr)
{
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Fuselage index index is less than zero\n"
                   << "in function call to tiglFuselageGetSurfaceArea.";
        return TIGL_INDEX_ERROR;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *surfaceAreaPtr = fuselage.GetSurfaceArea();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSurfaceArea!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                double* surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        LOG(ERROR) << "Null pointer argument for surfaceAreaPtr\n"
                   << "in function call to tiglWingGetSegmentSurfaceArea.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Wing or segment index index in less than zero\n"
                   << "in function call to tiglWingGetSegmentSurfaceArea.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentSurfaceArea!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                    int fuselageIndex,
                                                                    int segmentIndex,
                                                                    double* surfaceAreaPtr)
{
    if (surfaceAreaPtr == 0) {
        LOG(ERROR) << "Null pointer argument for surfaceAreaPtr\n"
                   << "in function call to tiglFuselageGetSegmentSurfaceArea.";
        return TIGL_NULL_POINTER;
    }

    if (fuselageIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Fuselage or segment index index in less than zero\n"
                   << "in function call to tiglFuselageGetSegmentSurfaceArea.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglFuselageGetSegmentSurfaceArea!";
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
        LOG(ERROR) << "Null pointer argument for surfaceArea "
                   << "in function call to tiglWingGetSegmentUpperSurfaceAreaTrimmed.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Wing or segment index index in less than one\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentUpperSurfaceAreaTrimmed!";
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
        LOG(ERROR) << "Null pointer argument for surfaceArea\n"
                   << "in function call to tiglWingGetSegmentLowerSurfaceAreaTrimmed.";
        return TIGL_NULL_POINTER;
    }

    if (wingIndex < 1 || segmentIndex < 1) {
        LOG(ERROR) << "Wing or segment index index in less than one\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetSegmentLowerSurfaceAreaTrimmed!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetReferenceArea(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                           TiglSymmetryAxis symPlane,
                                                           double *referenceAreaPtr)
{
    if (wingIndex < 1) {
        LOG(ERROR) << "Wing index index is less than zero\n"
                   << "in function call to tiglWingGetReferenceArea.";
        return TIGL_INDEX_ERROR;
    }

    if (symPlane < TIGL_NO_SYMMETRY || symPlane > TIGL_Y_Z_PLANE) {
        LOG(ERROR) << "Invalid symmetry\n"
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetReferenceArea!";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglConfigurationGetLength!";
        return TIGL_ERROR;
    }
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetWettedArea(TiglCPACSConfigurationHandle cpacsHandle, char* wingUID,
                                                        double *wettedAreaPtr)
{
    if (wingUID == NULL) {
        LOG(ERROR) << "WingUID is empty\n"
                   << "in function call to tiglWingGetWettedArea.";
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
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglWingGetWettedArea!";
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
        LOG(ERROR) << "Null pointer argument for componentUID\n"
                   << "in function call to tiglComponentGetHashCode.";
        return TIGL_NULL_POINTER;
    }

    if (hashCodePtr == NULL) {
        LOG(ERROR) << "Null pointer argument for hashCodePtr\n"
                   << "in function call to tiglComponentGetHashCode.";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        if (uidManager.HasGeometricComponent(componentUID)) {
            int hash = GetComponentHashCode(uidManager.GetGeometricComponent(componentUID));
            *hashCodePtr = hash;
        }
        else {
            return TIGL_UID_ERROR;
        }

        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglComponentGetHashCode!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT const char * tiglGetErrorString(TiglReturnCode code)
{
    if (code > TIGL_MATH_ERROR || code < 0) {
        LOG(ERROR) << "TIGL error code " << code << " is unknown!";
        return "TIGL_UNKNOWN_ERROR";
    }
    return TiglErrorStrings[code];
}

TIGL_COMMON_EXPORT TiglReturnCode tiglConfigurationGetLength(TiglCPACSConfigurationHandle cpacsHandle, double * pLength)
{
    if (pLength == NULL) {
        LOG(ERROR) << "argument pLength is NULL in tiglConfigurationGetLength!";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *pLength = config.GetAirplaneLenth();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglConfigurationGetLength!";
        return TIGL_ERROR;
    }
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSpan(TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID, double * pSpan)
{
    if (pSpan == NULL) {
        LOG(ERROR) << "argument pSpan is NULL in tiglConfigurationGetLength!";
        return TIGL_NULL_POINTER;
    }

    if (wingUID == NULL) {
        LOG(ERROR) << "argument wingUID is NULL in tiglConfigurationGetLength!";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        *pSpan = wing.GetWingspan();
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
        return TIGL_ERROR;
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglConfigurationGetLength!";
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
        LOG(ERROR) << "argument filePrefix is NULL in tiglLogToFileEnabled!";
        return TIGL_NULL_POINTER;
    }

    try {
        logger.LogToFile(filePrefix);
    }
    catch (const tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileStreamEnabled(FILE * fp) 
{
    tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
    if (fp == NULL) {
        LOG(ERROR) << "argument fp is NULL in tiglLogToFileStreamEnabled!";
        return TIGL_NULL_POINTER;
    }

    try {
        logger.LogToStream(fp);
    }
    catch (const tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}


TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetFileEnding(const char *ending) 
{
    tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
    if (ending == NULL) {
        LOG(ERROR) << "argument ending is NULL in tiglLogSetFileEnding!";
        return TIGL_NULL_POINTER;
    }

    try {
        logger.SetLogFileEnding(ending);
    }
    catch (const tigl::CTiglError& err) {
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
    catch (const tigl::CTiglError& err) {
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
    catch (const tigl::CTiglError& err) {
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
    catch (const tigl::CTiglError& err) {
        return err.getCode();
    }

    return TIGL_SUCCESS;
}

TIGL_COMMON_EXPORT TiglReturnCode tiglCheckPointInside(TiglCPACSConfigurationHandle cpacsHandle,
                                                       double px, double py, double pz,
                                                       const char *componentUID, TiglBoolean *isInside)
{
    if (!componentUID) {
        LOG(ERROR) << "Argument componentUID is NULL in tiglCheckPointInside!";
        return TIGL_NULL_POINTER;
    }

    if (!isInside) {
        LOG(ERROR) << "Argument isInside is NULL in tiglCheckPointInside!";
        return TIGL_NULL_POINTER;
    }

    try {
        const tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // get component
        tigl::ITiglGeometricComponent& component = config.GetUIDManager().GetGeometricComponent(componentUID);

        const TopoDS_Shape shape = component.GetLoft()->Shape();
        *isInside = IsPointInsideShape(shape, gp_Pnt(px, py, pz)) ? TIGL_TRUE : TIGL_FALSE;

        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << "In tiglCheckPointInside: " << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << "In tiglCheckPointInside: " << ex.what();
    }
    catch (Standard_Failure& err) {
        LOG(ERROR) << "Cannot compute shape `" << componentUID << "`: " << err.GetMessageString();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglCheckPointInside!";
    }
    return TIGL_ERROR;
}

TiglReturnCode tiglSetExportOptions(const char *exporter_name, const char *option_name, const char *option_value)
{
    if (!exporter_name) {
        LOG(ERROR) << "Argument exporter_name is NULL in tiglSetExportOptions!";
        return TIGL_NULL_POINTER;
    }

    if (!option_name) {
        LOG(ERROR) << "Argument option_name is NULL in tiglSetExportOptions!";
        return TIGL_NULL_POINTER;
    }

    if (!option_value) {
        LOG(ERROR) << "Argument option_value is NULL in tiglSetExportOptions!";
        return TIGL_NULL_POINTER;
    }

    try {
        tigl::ExporterOptions& options = tigl::getExportConfig(exporter_name);
        options.SetFromString(option_name, option_value);
        return TIGL_SUCCESS;
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << "In tiglSetExportOptions: " << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << "In tiglSetExportOptions: " << ex.what();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglSetExportOptions!";
    }
    return TIGL_ERROR;
}

TiglReturnCode tiglExportComponent(TiglCPACSConfigurationHandle cpacsHandle, const char *uid, const char *fileName, double deflection)
{
    if (!uid) {
        LOG(ERROR) << "Argument uid is NULL in tiglExportComponent!";
        return TIGL_NULL_POINTER;
    }

    if (!fileName) {
        LOG(ERROR) << "Argument fileName is NULL in tiglExportComponent!";
        return TIGL_NULL_POINTER;
    }

    try {
        const tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::ITiglGeometricComponent& component = config.GetUIDManager().GetGeometricComponent(uid);

        std::string extension = FileExtension(fileName);
        if (extension.empty()) {
            LOG(ERROR) << "In tiglExportComponent: Cannot export to '" << fileName << "'. No file extension given.";
            return TIGL_WRITE_FAILED;
        }

        tigl::PTiglCADExporter exporter = tigl::createExporter(extension);
        exporter->AddShape(component.GetLoft(), &config, tigl::TriangulatedExportOptions(deflection));
        if (exporter->Write(fileName) == true) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << "In tiglExportComponent: " << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << "In tiglExportComponent: " << ex.what();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportComponent!";
    }
    return TIGL_ERROR;
}

TiglReturnCode tiglExportConfiguration(TiglCPACSConfigurationHandle cpacsHandle, const char *fileName, TiglBoolean fuseAllShapes, double deflection)
{
    if (!fileName) {
        LOG(ERROR) << "Argument fileName is NULL in tiglExportConfiguration!";
        return TIGL_NULL_POINTER;
    }

    try {
        const tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        std::string extension = FileExtension(fileName);
        if (extension.empty()) {
            LOG(ERROR) << "In tiglExportConfiguration: Cannot export to '" << fileName << "'. No file extension given.";
            return TIGL_WRITE_FAILED;
        }

        tigl::PTiglCADExporter exporter = tigl::createExporter(extension);
        if (fuseAllShapes) {
            exporter->AddFusedConfiguration(config, tigl::TriangulatedExportOptions(deflection));
        }
        else {
            exporter->AddConfiguration(config, tigl::TriangulatedExportOptions(deflection));
        }

        if (exporter->Write(fileName) == true) {
            return TIGL_SUCCESS;
        }
        else {
            return TIGL_WRITE_FAILED;
        }
    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << "In tiglExportConfiguration: " << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << "In tiglExportConfiguration: " << ex.what();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglExportConfiguration!";
    }
    return TIGL_ERROR;
}

TiglReturnCode tiglConfigurationGetBoundingBox(TiglCPACSConfigurationHandle cpacsHandle, double *minX, double *minY, double *minZ, double *maxX, double *maxY, double *maxZ)
{
    if (!minX) {
        LOG(ERROR) << "Null pointer for argument minX in tiglConfigurationGetBoundingBox";
        return TIGL_NULL_POINTER;
    }

    if (!minY) {
        LOG(ERROR) << "Null pointer for argument minY in tiglConfigurationGetBoundingBox";
        return TIGL_NULL_POINTER;
    }

    if (!minZ) {
        LOG(ERROR) << "Null pointer for argument minZ in tiglConfigurationGetBoundingBox";
        return TIGL_NULL_POINTER;
    }

    if (!maxX) {
        LOG(ERROR) << "Null pointer for argument maxX in tiglConfigurationGetBoundingBox";
        return TIGL_NULL_POINTER;
    }

    if (!maxY) {
        LOG(ERROR) << "Null pointer for argument maxY in tiglConfigurationGetBoundingBox";
        return TIGL_NULL_POINTER;
    }

    if (!maxZ) {
        LOG(ERROR) << "Null pointer for argument maxZ in tiglConfigurationGetBoundingBox";
        return TIGL_NULL_POINTER;
    }

    try {
        const tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CTiglPoint min, max;
        tigl::ConfigurationGetBoundingBox(config, min, max);
        *minX = min.x;
        *minY = min.y;
        *minZ = min.z;
        *maxX = max.x;
        *maxY = max.y;
        *maxZ = max.z;

        return TIGL_SUCCESS;

    }
    catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << "In tiglConfigurationGetBoundingBox: " << ex.what();
        return ex.getCode();
    }
    catch (std::exception& ex) {
        LOG(ERROR) << "In tiglConfigurationGetBoundingBox: " << ex.what();
    }
    catch (...) {
        LOG(ERROR) << "Caught an exception in tiglConfigurationGetBoundingBox!";
    }
    return TIGL_ERROR;

}
