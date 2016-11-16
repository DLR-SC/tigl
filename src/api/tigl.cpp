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
#include "CTiglError.h"
#include "CTiglIntersectionCalculation.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglIntersectionCalculation.h"
#include "CTiglUIDManager.h"
#include "CCPACSWing.h"
#include "CCPACSWingSection.h"
#include "CCPACSWingSegment.h"
#include "CTiglExportIges.h"
#include "CTiglExportStep.h"
#include "CTiglExportStl.h"
#include "CTiglExportVtk.h"
#include "CTiglExportCollada.h"
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

void tiglCleanup(void);
bool tiglInit(void);

// make tigl initialize on start
const bool tiglInitialized = tiglInit();

bool tiglInit(void)
{
    atexit(tiglCleanup);

    // Initialize logger
    tigl::CTiglLogging::Instance();
    // Register dynamic tigl types
    return true;
}

void tiglCleanup(void)
{
}

template<typename Func>
TiglReturnCode Try(const std::string& funcname, const TiglReturnCode& errorCode, Func func) throw() {
    try {
        return func();
    } catch (const tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
        return ex.getCode();
    } catch (const std::exception& ex) {
        LOG(ERROR) << ex.what() << std::endl;
        return errorCode;
    } catch (char *str) {
        LOG(ERROR) << str << std::endl;
        return errorCode;
    } catch (std::string& str) {
        LOG(ERROR) << str << std::endl;
        return errorCode;
    } catch (...) {
        LOG(ERROR) << "Caught an exception in " << funcname << "!" << std::endl;
        return errorCode;
    }
}

/*****************************************************************************/
/* Public visible functions.                                                 */
/*****************************************************************************/

TIGL_COMMON_EXPORT TiglReturnCode tiglOpenCPACSConfiguration(TixiDocumentHandle tixiHandle, const char* configurationUID_cstr, TiglCPACSConfigurationHandle* cpacsHandlePtr)
{
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
            if (dcpacsVersion < (double) TIGL_VERSION_MAJOR) {
                LOG(ERROR) << "Too old CPACS dataset. CPACS version has to be at least " << (double) TIGL_VERSION_MAJOR << "!" << std::endl;
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
        int sectionCount = 0;

        tixiRet = tixiGetNamedChildrenCount(tixiHandle, "/cpacs/vehicles/aircraft", "model", &sectionCount);
        if (tixiRet != SUCCESS) {
            LOG(ERROR) << "No configuration specified!" << std::endl;
            return TIGL_ERROR;
        }
        char * tmpConfUID = NULL;
        tixiGetTextAttribute(tixiHandle, "/cpacs/vehicles/aircraft/model", "uID", &tmpConfUID);
        if (tixiRet != SUCCESS) {
            LOG(ERROR) << "Problems reading configuration-uid!" << std::endl;
            return TIGL_ERROR;
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

    return Try("tiglOpenCPACSConfiguration", TIGL_OPEN_FAILED, [&]{
        std::unique_ptr<tigl::CCPACSConfiguration> config(new tigl::CCPACSConfiguration(tixiHandle));
        // Build CPACS memory structure
        config->ReadCPACS(configurationUID.c_str());
        // Store configuration in handle container
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        *cpacsHandlePtr = manager.AddConfiguration(config.release());
        return TIGL_SUCCESS;
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglSaveCPACSConfiguration(const char* configurationUID, TiglCPACSConfigurationHandle cpacsHandle)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();

    if (!manager.IsValid(cpacsHandle)) {
        LOG(ERROR) << "Invalid cpacsHandle passed to tiglSaveCPACSConfiguration!" << std::endl;
        return TIGL_UNINITIALIZED;
    }


    return Try("tiglSaveCPACSConfiguration", TIGL_ERROR, [&]{
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        config.WriteCPACS(configurationUID);
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglCloseCPACSConfiguration(TiglCPACSConfigurationHandle cpacsHandle)
{
    return Try("tiglCloseCPACSConfiguration", TIGL_CLOSE_FAILED, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        manager.DeleteConfiguration(cpacsHandle);
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglGetCPACSTixiHandle(TiglCPACSConfigurationHandle cpacsHandle, TixiDocumentHandle* tixiHandlePtr)
{
    if (tixiHandlePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for tixiHandlePtr in function call to tiglGetCPACSTixiHandle." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglGetCPACSTixiHandle", TIGL_NOT_FOUND, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *tixiHandlePtr = config.GetTixiDocumentHandle();
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglIsCPACSConfigurationHandleValid(TiglCPACSConfigurationHandle cpacsHandle, TiglBoolean* isValidPtr)
{
    return Try("tiglIsCPACSConfigurationHandleValid", TIGL_NOT_FOUND, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        *isValidPtr = (manager.IsValid(cpacsHandle) == true) ? TIGL_TRUE : TIGL_FALSE;
        return TIGL_SUCCESS;
    });
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
    
    return Try("tiglProfileGetBSplineCount", TIGL_ERROR, [&]{
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
    });
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
    
    return Try("tiglProfileGetBSplineDataSizes", TIGL_ERROR, [&]{
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
             Handle_Geom_BSplineCurve bspl = GetBSplineCurve(e);
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
    });
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
    
    return Try("tiglProfileGetBSplineData", TIGL_ERROR, [&]{
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
            Handle_Geom_BSplineCurve bspl = GetBSplineCurve(e);
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
    });
}


/**********************************************************************************************/


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

    return Try("tiglWingGetUpperPoint", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration & config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        gp_Pnt point = wing.GetUpperPoint(segmentIndex, eta, xsi);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetLowerPoint", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        gp_Pnt point = wing.GetLowerPoint(segmentIndex, eta, xsi);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetChordPoint", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);
        
        gp_Pnt point = segment.GetChordPoint(eta, xsi);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetChordNormal", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglWingGetUpperPointAtDirection", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglWingGetLowerPointAtDirection", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglWingGetSegmentEtaXsi", TIGL_ERROR, [&]{
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
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglGetWingCount(TiglCPACSConfigurationHandle cpacsHandle, int* wingCountPtr)
{
    if (wingCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingCountPtr ";
        LOG(ERROR) << "in function call to tiglGetWingCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglGetWingCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *wingCountPtr = config.GetWingCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetSegmentCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *segmentCountPtr = wing.GetSegmentCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetComponentSegmentCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *compSegmentCountPtr = wing.GetComponentSegmentCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetComponentSegmentUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(compSegmentIndex);
        *uidNamePtr = const_cast<char*>(segment.GetUID().c_str());
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetComponentSegmentIndex", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglWingGetInnerConnectedSegmentCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *segmentCountPtr = segment.GetInnerConnectedSegmentCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetOuterConnectedSegmentCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *segmentCountPtr = segment.GetOuterConnectedSegmentCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetInnerConnectedSegmentIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *connectedIndexPtr = segment.GetInnerConnectedSegmentIndex(n);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetOuterConnectedSegmentIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *connectedIndexPtr = segment.GetOuterConnectedSegmentIndex(n);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetInnerSectionAndElementIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *sectionIndexPtr = segment.GetInnerSectionIndex();
        *elementIndexPtr = segment.GetInnerSectionElementIndex();

        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetOuterSectionAndElementIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *sectionIndexPtr = segment.GetOuterSectionIndex();
        *elementIndexPtr = segment.GetOuterSectionElementIndex();

        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetInnerSectionAndElementUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);

        *sectionUIDPtr = (char *) segment.GetInnerSectionUID().c_str();

        *elementUIDPtr = (char *) segment.GetInnerSectionElementUID().c_str();

        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetOuterSectionAndElementUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);

        *sectionUIDPtr = (char *) segment.GetOuterSectionUID().c_str();
        *elementUIDPtr = (char *) segment.GetOuterSectionElementUID().c_str();

        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetProfileName", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        const tigl::CCPACSWingSection& section = wing.GetSection(sectionIndex);
        const tigl::CCPACSWingSectionElement& element = section.GetSectionElement(elementIndex);
        std::string profileUID = element.GetProfileIndex();
        tigl::CCPACSWingProfile& profile = config.GetWingProfile(profileUID);

        *profileNamePtr = const_cast<char*>(profile.GetName().c_str());

        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *uidNamePtr = const_cast<char*> (wing.GetUID().c_str());
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            if (wing.GetUID() == std::string(wingUID)) {
                *wingIndexPtr = iwing;
                return TIGL_SUCCESS;
            }
        }

        LOG(ERROR) << "Error in tiglWingGetIndex: the wing \"" << wingUID << "\" can not be found!" << std::endl;
        *wingIndexPtr = -1;
        return TIGL_UID_ERROR;
    });
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

    return Try("tiglWingGetSegmentUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *uidNamePtr = const_cast<char*>(segment.GetUID().c_str());
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetSegmentIndex", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglWingGetSectionCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *sectionCount = wing.GetSectionCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetSectionUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        const tigl::CCPACSWingSection& section = wing.GetSection(sectionIndex);
        *uidNamePtr = const_cast<char*>(section.GetUID().c_str());
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetSymmetry", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *symmetryAxisPtr = wing.GetSymmetryAxis();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingComponentSegmentFindSegment", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        // iterate through wings and find componentSegment

        for (int wingIndex=1; wingIndex <= config.GetWingCount(); wingIndex++) {
            tigl::CCPACSWing& wing = config.GetWing(wingIndex);

            for (int componentSegment = 1; componentSegment <= wing.GetComponentSegmentCount(); componentSegment++) {
                tigl::CCPACSWingComponentSegment& cs = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(componentSegment);
                if ( cs.GetUID() == componentSegmentUID) {
                    gp_Pnt nearestPointOnSegment;
                    const tigl::CTiglAbstractSegment* segment =  cs.findSegment(x, y, z, nearestPointOnSegment);
                    if (!segment) {
                        // point does not lie on component segment
                        LOG(ERROR) << "Given point does not lie on component segment within 1cm tolerance."
                                   << " Diviation is " << nearestPointOnSegment.Distance(gp_Pnt(x,y,z))*1000. << " mm.";
                        return TIGL_NOT_FOUND;
                    }

                    *segmentUID = (char*) segment->GetUID().c_str();
                    *wingUID    = (char*) wing.GetUID().c_str();

                    return TIGL_SUCCESS;
                }
            }
        }
        return TIGL_ERROR;
    });
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
    
    return Try("tiglWingComponentSegmentGetPoint", TIGL_ERROR, [&]{
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
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentPointGetSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                const char *componentSegmentUID, double eta, double xsi,
                                                                                char** wingUID, char** segmentUID,
                                                                                double *segmentEta, double *segmentXsi)
{
    if (segmentUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for segmentUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi." << std::endl;
        return TIGL_NULL_POINTER;
    }

    if (wingUID == 0) {
        LOG(ERROR) << "Error: Null pointer argument for wingUID ";
        LOG(ERROR) << "in function call to tiglWingComponentSegmentPointGetSegmentEtaXsi." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglWingComponentSegmentPointGetSegmentEtaXsi", TIGL_ERROR, [&]{
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
                    tigl::CCPACSWingSegment* segment = (tigl::CCPACSWingSegment*) compSeg.findSegment(pnt.X(), pnt.Y(), pnt.Z(), nearestPoint);
                    double deviation = nearestPoint.Distance(pnt);
                    if (!segment) {
                        LOG(ERROR) << "Given point does not lie on component segment within 1cm tolerance."
                                   << " The actual diviation is " << deviation*1000. << " mm.";
                        return TIGL_MATH_ERROR;
                    }
                    // warn if cs point is more than 1mm outside from segment
                    if ( deviation > 1e-3) {
                        LOG(WARNING) << "Given point is located more than 1mm from the wing component segment body."
                                     << " The actual diviation is " << deviation*1000.  << " mm."
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
    });
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
    
    return Try("tiglWingSegmentPointGetComponentSegmentEtaXsi", TIGL_ERROR, [&]{
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
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetSegmentIntersection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                 const char* componentSegmentUID,
                                                                                 const char* segmentUID,
                                                                                 double csEta1, double csXsi1,
                                                                                 double csEta2, double csXsi2,
                                                                                 double   segmentEta, 
                                                                                 double * segmentXsi) 
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
    
    return Try("tiglWingComponentSegmentGetSegmentIntersection", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        
        // search for component segment
        int nwings = config.GetWingCount();
        for (int iwing = 1; iwing <= nwings; ++iwing) {
            tigl::CCPACSWing& wing = config.GetWing(iwing);
            try {
                tigl::CCPACSWingComponentSegment & compSeg = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(componentSegmentUID);
                compSeg.GetSegmentIntersection(segmentUID, csEta1, csXsi1, csEta2, csXsi2, segmentEta, *segmentXsi);
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
    });
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
    
    return Try("tiglWingComponentSegmentGetNumberOfSegments", TIGL_ERROR, [&]{
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
    });
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
    
    return Try("tiglWingComponentSegmentGetSegmentUID", TIGL_ERROR, [&]{
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
    });
}

/**********************************************************************************************/


TIGL_COMMON_EXPORT TiglReturnCode tiglGetFuselageCount(TiglCPACSConfigurationHandle cpacsHandle, int* fuselageCountPtr)
{
    if (fuselageCountPtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for fuselageCountPtr ";
        LOG(ERROR) << "in function call to tiglGetFuselageCount." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglGetFuselageCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *fuselageCountPtr = config.GetFuselageCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetSegmentCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *segmentCountPtr = fuselage.GetSegmentCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetPoint", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        gp_Pnt point = fuselage.GetPoint(segmentIndex, eta, zeta);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetPointAngle", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglFuselageGetPointAngleTranslated", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglFuselageGetPointOnXPlane", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        gp_Pnt point = segment.GetPointOnXPlane(eta, xpos, pointIndex);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetPointOnYPlane", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        gp_Pnt point = segment.GetPointOnXPlane(eta, ypos, pointIndex);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetNumPointsOnXPlane", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *numPointsPtr = segment.GetNumPointsOnYPlane(eta, xpos);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetNumPointsOnYPlane", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *numPointsPtr = segment.GetNumPointsOnYPlane(eta, ypos);
        return TIGL_SUCCESS;
    });
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

     return Try("tiglFuselageGetCircumference", TIGL_ERROR, [&]{
         tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
         tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
         tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
         *circumferencePtr = fuselage.GetCircumference(segmentIndex, eta);
         return TIGL_SUCCESS;
     });
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

    return Try("tiglFuselageGetStartConnectedSegmentCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *segmentCountPtr = segment.GetStartConnectedSegmentCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetEndConnectedSegmentCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *segmentCountPtr = segment.GetEndConnectedSegmentCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetStartConnectedSegmentIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *connectedIndexPtr = segment.GetStartConnectedSegmentIndex(n);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetEndConnectedSegmentIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *connectedIndexPtr = segment.GetEndConnectedSegmentIndex(n);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetStartSectionAndElementUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);

        *sectionUIDPtr = (char *) segment.GetStartSectionUID().c_str();

        *elementUIDPtr = (char *)segment.GetStartSectionElementUID().c_str();

        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetEndSectionAndElementUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);

        *sectionUIDPtr = (char *) segment.GetEndSectionUID().c_str();
        *elementUIDPtr = (char *) segment.GetEndSectionElementUID().c_str();

        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetStartSectionAndElementIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *sectionIndexPtr = segment.GetStartSectionIndex();
        *elementIndexPtr = segment.GetStartSectionElementIndex();

        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetEndSectionAndElementIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
        *sectionIndexPtr = segment.GetEndSectionIndex();
        *elementIndexPtr = segment.GetEndSectionElementIndex();

        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetProfileName", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSection& section = fuselage.GetSection(sectionIndex);
        tigl::CCPACSFuselageSectionElement& element = section.GetSectionElement(elementIndex);
        std::string profileUID = element.GetProfileIndex();
        tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile(profileUID);

        *profileNamePtr = const_cast<char*>(profile.GetName().c_str());
        
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *uidNamePtr = const_cast<char*>(fuselage.GetUID().c_str());
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        int nfuselages = config.GetFuselageCount();
        for (int ifuselage = 1; ifuselage <= nfuselages; ++ifuselage) {
            tigl::CCPACSFuselage& fuselage = config.GetFuselage(ifuselage);
            if (fuselage.GetUID() == std::string(fuselageUID)) {
                *fuselageIndexPtr = ifuselage;
                return TIGL_SUCCESS;
            }
        }

        LOG(ERROR) << "Error in tiglFuselageGetIndex: the fuselage \"" << fuselageUID << "\" can not be found!" << std::endl;
        *fuselageIndexPtr = -1;
        return TIGL_UID_ERROR;
    });
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

    return Try("tiglFuselageGetSegmentUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *uidNamePtr = const_cast<char*>(segment.GetUID().c_str());
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetSegmentIndex", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglFuselageGetSectionCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *sectionCount = fuselage.GetSectionCount();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetSectionUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSection& section = fuselage.GetSection(sectionIndex);
        *uidNamePtr = const_cast<char*>(section.GetUID().c_str());
        return TIGL_SUCCESS;
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                          TiglSymmetryAxis* symmetryAxisPtr)
{
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: Fuselage or segment index index is less than zero ";
        LOG(ERROR) << "in function call to tiglGetFuselageSymmetry." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    return Try("tiglFuselageGetSymmetry", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *symmetryAxisPtr = fuselage.GetSymmetryAxis();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetMinumumDistanceToGround", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageUID);
        gp_Pnt point = fuselage.GetMinumumDistanceToGround(RAxis, angle);
        *pointXPtr = point.X();
        *pointYPtr = point.Y();
        *pointZPtr = point.Z();
        return TIGL_SUCCESS;
    });
}


/*****************************************************************************************************/


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


    return Try("tiglComponentIntersectionPoint", TIGL_ERROR, [&]{
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
    });
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


    return Try("tiglComponentIntersectionPoints", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglComponentIntersectionLineCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);

        tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

        TopoDS_Shape compoundOne = uidManager.GetComponent(componentUidOne)->GetLoft()->Shape();
        TopoDS_Shape compoundTwo = uidManager.GetComponent(componentUidTwo)->GetLoft()->Shape();

        tigl::CTiglIntersectionCalculation Intersector(&config.GetShapeCache(), componentUidOne, componentUidTwo, compoundOne, compoundTwo);
        *numWires = Intersector.GetCountIntersectionLines();
        return TIGL_SUCCESS;
    });
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
    
    return Try("tiglIntersectComponents", TIGL_ERROR, [&]{
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
    });
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
    
    return Try("tiglIntersectWithPlane", TIGL_ERROR, [&]{
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
    });
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
    
    return Try("tiglIntersectGetLineCount", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglShapeCache& cache = config.GetShapeCache();

        tigl::CTiglIntersectionCalculation Intersector(cache, intersectionID);
        *lineCount = Intersector.GetCountIntersectionLines();
        
        return TIGL_SUCCESS;
    });
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
    
    return Try("tiglIntersectGetPoint", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglShapeCache& cache = config.GetShapeCache();

        tigl::CTiglIntersectionCalculation Intersector(cache, intersectionID);
        gp_Pnt p = Intersector.GetPoint(eta, lineIdx);
        
        *pointX = p.X();
        *pointY = p.Y();
        *pointZ = p.Z();
        
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportIGES", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportIges exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportIGES(filename);
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedWingFuselageIGES(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char *filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportFusedWingFuselageIGES." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglExportFusedWingFuselageIGES", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportIges exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportFusedIGES(filename);
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportSTEP(TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportSTEP." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglExportSTEP", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStep exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportStep(filename);
        return TIGL_SUCCESS;
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedSTEP(TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportFusedSTEP." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglExportFusedSTEP", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStep exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportFusedStep(filename);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedWingSTL", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedWingSTL(wingIndex, filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedWingSTLByUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter(config);
        std::string filename = filenamePtr;
        for (int iWing = 1; iWing <= config.GetWingCount(); ++iWing) {
            tigl::CCPACSWing& wing = config.GetWing(iWing);
            if (wing.GetUID() == wingUID) {
                exporter.ExportMeshedWingSTL(iWing, filename, deflection);
                return TIGL_SUCCESS;
            }
        }
        
        LOG(ERROR) << "Wing with UID " << wingUID << " not found"
                   << "in function call to tiglExportMeshedWingSTLByUID." << std::endl;
        return TIGL_UID_ERROR;
    });
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

    return Try("tiglExportMeshedFuselageSTL", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedFuselageSTL(fuselageIndex, filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedFuselageSTLByUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter(config);
        std::string filename = filenamePtr;
        
        for (int ifusel = 1; ifusel <= config.GetFuselageCount(); ++ifusel) {
            tigl::CCPACSFuselage& fuselage = config.GetFuselage(ifusel);
            if (fuselage.GetUID() == fuselageUID) {
                exporter.ExportMeshedFuselageSTL(ifusel, filename, deflection);
                return TIGL_SUCCESS;
            }
        }
        
        LOG(ERROR) << "Fuselage with UID " << fuselageUID << " not found"
                   << "in function call to tiglExportMeshedFuselageSTLByUID." << std::endl;
        return TIGL_UID_ERROR;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometrySTL(TiglCPACSConfigurationHandle cpacsHandle, const char *filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedGeometrySTL." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglExportMeshedGeometrySTL", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportStl exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedGeometrySTL(filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedWingVTKByIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedWingVTKByIndex(wingIndex, filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedWingVTKByUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedWingVTKByUID(wingUID, filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedFuselageVTKByIndex", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedFuselageVTKByIndex(fuselageIndex, filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedFuselageVTKByUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedFuselageVTKByUID(fuselageUID, filename, deflection);
        return TIGL_SUCCESS;
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometryVTK(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedGeometryVTK." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglExportMeshedGeometryVTK", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedGeometryVTK(filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedWingVTKSimpleByUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedWingVTKSimpleByUID(wingUID, filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportMeshedFuselageVTKSimpleByUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedFuselageVTKSimpleByUID(fuselageUID, filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglExportFuselageColladaByUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageUID);
        return tigl::CTiglExportCollada::write(fuselage.GetLoft(), filenamePtr, deflection);
    });
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

    return Try("tiglExportWingColladaByUID", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        return tigl::CTiglExportCollada::write(wing.GetLoft(), filenamePtr, deflection);
    });
}



TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometryVTKSimple(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr, double deflection)
{
    if (filenamePtr == 0) {
        LOG(ERROR) << "Error: Null pointer argument for filenamePtr";
        LOG(ERROR) << "in function call to tiglExportMeshedGeometryVTKSimple." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglExportMeshedGeometryVTKSimple", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CTiglExportVtk exporter(config);
        std::string filename = filenamePtr;
        exporter.ExportMeshedGeometryVTKSimple(filename, deflection);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingComponentSegmentGetMaterialCount", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglWingComponentSegmentGetMaterialUID", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglWingComponentSegmentGetMaterialThickness", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglFuselageGetVolume", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *volumePtr = fuselage.GetVolume();
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetVolume(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                    double *volumePtr)
{
    if (wingIndex < 1) {
        LOG(ERROR) << "Error: Wing index index is less than zero ";
        LOG(ERROR) << "in function call to tiglWingGetVolume." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    return Try("tiglWingGetVolume", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *volumePtr = wing.GetVolume();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetSegmentVolume", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        *volumePtr = segment.GetVolume();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetSegmentVolume", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment =(tigl::CCPACSFuselageSegment &)  fuselage.GetSegment(segmentIndex);
        *volumePtr = segment.GetVolume();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetSurfaceArea", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *surfaceAreaPtr = wing.GetSurfaceArea();
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                             double *surfaceAreaPtr)
{
    if (fuselageIndex < 1) {
        LOG(ERROR) << "Error: Fuselage index index is less than zero ";
        LOG(ERROR) << "in function call to tiglFuselageGetSurfaceArea." << std::endl;
        return TIGL_INDEX_ERROR;
    }

    return Try("tiglFuselageGetSurfaceArea", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        *surfaceAreaPtr = fuselage.GetSurfaceArea();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetSegmentSurfaceArea", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(segmentIndex);
        *surfaceAreaPtr = segment.GetSurfaceArea();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglFuselageGetSegmentSurfaceArea", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(fuselageIndex);
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(segmentIndex);
        *surfaceAreaPtr = segment.GetSurfaceArea();
        return TIGL_SUCCESS;
    });
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
    
    return Try("tiglWingGetSegmentUpperSurfaceAreaTrimmed", TIGL_ERROR, [&]{
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
    });
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
    
    return Try("tiglWingGetSegmentLowerSurfaceAreaTrimmed", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglWingGetReferenceArea", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingIndex);
        *referenceAreaPtr = wing.GetReferenceArea(symPlane);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetMAC", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        wing.GetWingMAC( *mac_chord,  *mac_x,  *mac_y,  *mac_z);
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetWettedArea(TiglCPACSConfigurationHandle cpacsHandle, char* wingUID,
                                                        double *wettedAreaPtr)
{
    if (wingUID == NULL) {
        LOG(ERROR) << "Error: WingUID is empty ";
        LOG(ERROR) << "in function call to tiglWingGetWettedArea." << std::endl;
        return TIGL_NULL_POINTER;
    }

    return Try("tiglWingGetWettedArea", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        TopoDS_Shape parent = config.GetParentLoft(wingUID);
        *wettedAreaPtr = wing.GetWettedArea(parent);
        return TIGL_SUCCESS;
    });
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

    return Try("tiglComponentGetHashCode", TIGL_ERROR, [&]{
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
    });
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

    return Try("tiglConfigurationGetLength", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        *pLength = config.GetAirplaneLenth();
        return TIGL_SUCCESS;
    });
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

    return Try("tiglWingGetSpan", TIGL_ERROR, [&]{
        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        tigl::CCPACSWing& wing = config.GetWing(wingUID);
        *pSpan = wing.GetWingspan();
        return TIGL_SUCCESS;
    });
}

/*****************************************************************************/
/* Logging functions.                                                        */
/*****************************************************************************/
TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileEnabled(const char *filePrefix) 
{
    if (filePrefix == NULL) {
        LOG(ERROR) << "Error: argument filePrefix is NULL in tiglLogToFileEnabled!";
        return TIGL_NULL_POINTER;
    }

    return Try("tiglLogToFileEnabled", TIGL_ERROR, [&]{
        tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
        logger.LogToFile(filePrefix);
        return TIGL_SUCCESS;
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileStreamEnabled(FILE * fp) 
{
    if (fp == NULL) {
        LOG(ERROR) << "Error: argument fp is NULL in tiglLogToFileStreamEnabled!";
        return TIGL_NULL_POINTER;
    }

    return Try("tiglLogToFileStreamEnabled", TIGL_ERROR, [&]{
        tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
        logger.LogToStream(fp);
        return TIGL_SUCCESS;
    });
}


TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetFileEnding(const char *ending) 
{
    if (ending == NULL) {
        LOG(ERROR) << "Error: argument ending is NULL in tiglLogSetFileEnding!";
        return TIGL_NULL_POINTER;
    }

    return Try("tiglLogSetFileEnding", TIGL_ERROR, [&]{
        tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
        logger.SetLogFileEnding(ending);
        return TIGL_SUCCESS;
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetTimeInFilenameEnabled(TiglBoolean enabled) 
{
    return Try("tiglLogSetTimeInFilenameEnabled", TIGL_ERROR, [&]{
        tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
        logger.SetTimeIdInFilenameEnabled(enabled > 0);
        return TIGL_SUCCESS;
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileDisabled() 
{
    return Try("tiglLogToFileDisabled", TIGL_ERROR, [&]{
        tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
        logger.LogToConsole();
        return TIGL_SUCCESS;
    });
}

TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetVerbosity(TiglLogLevel consoleVerbosity) 
{
    return Try("tiglLogSetVerbosity", TIGL_ERROR, [&]{
        tigl::CTiglLogging& logger = tigl::CTiglLogging::Instance();
        logger.SetConsoleVerbosity(consoleVerbosity);
        return TIGL_SUCCESS;
    });
}
