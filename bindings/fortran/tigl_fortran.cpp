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
* @date   $Date$
*
* @brief  Implementation of the TIGL Fortran wrapper interface.
*/

#include "tigl_fortran.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

/**
@brief transformes a fortran string into a c string.
@param fortranString a pointer to the memory occupied by the fortran string
@param size of the fortran character variable holding the string
*/
static char* makeCString(char *fortranString, int fortranStringLength)
{
    int iEnd;
    int cStringLength = -1;
    char *cString     = 0;

    if (fortranString != 0) {

        /** Skip trailing blanks **/
        for (iEnd = fortranStringLength - 1; iEnd >= 0; iEnd--) {
            if (fortranString[iEnd] != ' ') {
                break;
            }
        }

        cStringLength = iEnd + 1;

        cString = (char *) malloc(sizeof(char) * (cStringLength + 1));

        if (!cString) {
            fprintf(stderr, "Fatal error: Failed to allocate memory in makeCstring");
            abort();
        }

        memcpy(cString, fortranString, cStringLength * sizeof(char));

        /** NULL terminate the c string **/

        cString[cStringLength] = '\0';
    }
    return cString;
}


static void copyToFortranString(const char *cString, int fortranStringLength, char *fortranString)
{
    if (cString) {

        size_t cStringLength = strlen(cString);

        if (cStringLength != 0) {

            size_t length = (cStringLength < (size_t) fortranStringLength) ? cStringLength : (size_t) fortranStringLength;

            size_t i;

            memcpy(fortranString, cString, length * sizeof(char));

            for (i = cStringLength; i < (size_t) fortranStringLength; i++) {
                fortranString[i] = ' ';
            }
        }
    }
}


void tiglOpenCPACSConfiguration_f(TixiDocumentHandle* tixiHandle,
                                  TiglCPACSConfigurationHandle* cpacsHandlePtr,
                                  char* configurationUID,
                                  TiglReturnCode* returnCode)
{
    *returnCode = tiglOpenCPACSConfiguration(*tixiHandle, configurationUID, cpacsHandlePtr);
}


void tiglCloseCPACSConfiguration_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                   TiglReturnCode* returnCode)
{
    *returnCode = tiglCloseCPACSConfiguration(*cpacsHandle);
}


void tiglGetCPACSTixiHandle_f(TiglCPACSConfigurationHandle* cpacsHandle,
                              TixiDocumentHandle* tixiHandlePtr,
                              TiglReturnCode* returnCode)
{
    *returnCode = tiglGetCPACSTixiHandle(*cpacsHandle, tixiHandlePtr);
}


void tiglIsCPACSConfigurationHandleValid_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                           TiglBoolean* isValidPtr,
                                           TiglReturnCode* returnCode)
{
    *returnCode = tiglIsCPACSConfigurationHandleValid(*cpacsHandle, isValidPtr);
}


void tiglGetVersion_f(char *version, int stringLength)
{
  char *cVersion = NULL;
  cVersion = tiglGetVersion();
  copyToFortranString(cVersion, stringLength, version);
}

void tiglExportIGES_f(TiglCPACSConfigurationHandle* cpacsHandle,
                      char* filenamePtr,
                      TiglReturnCode* returnCode,
                      int lengthString1)
{
  char *cString;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportIGES(*cpacsHandle, cString);
  free(cString);
}


void tiglExportSTEP_f(TiglCPACSConfigurationHandle* cpacsHandle,
                      char* filenamePtr,
                      TiglReturnCode* returnCode,
                      int lengthString1)
{
  char *cString;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportSTEP(*cpacsHandle, cString);
  free(cString);
}

void tiglExportFusedIGES_f(TiglCPACSConfigurationHandle* cpacsHandle,
                           char* filenamePtr,
                           TiglReturnCode* returnCode,
                           int lengthString1)
{
  char *cString;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportFusedWingFuselageIGES(*cpacsHandle, cString);
  free(cString);
}


void tiglExportMeshedWingSTL_f(TiglCPACSConfigurationHandle* cpacsHandle,
                           int* wingIndex,
                           char* filenamePtr,
                           double* deflection,
                           TiglReturnCode* returnCode,
                           int lengthString1)
{
  char *cString;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportMeshedWingSTL(*cpacsHandle, *wingIndex, cString, *deflection);
  free(cString);
}


void tiglExportMeshedFuselageSTL_f(TiglCPACSConfigurationHandle* cpacsHandle,
                           int* fuselageIndex,
                           char* filenamePtr,
                           double* deflection,
                           TiglReturnCode* returnCode,
                           int lengthString1)
{
  char *cString;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportMeshedFuselageSTL(*cpacsHandle, *fuselageIndex, cString, *deflection);
  free(cString);
}


void tiglExportMeshedGeometrySTL_f(TiglCPACSConfigurationHandle* cpacsHandle,
                           char* filenamePtr,
                           double* deflection,
                           TiglReturnCode* returnCode,
                           int lengthString1)
{
  char *cString;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportMeshedGeometrySTL(*cpacsHandle, cString, *deflection);
  free(cString);
}


void tiglGetWingCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                        int* wingCountPtr,
                        TiglReturnCode* returnCode)
{
    *returnCode = tiglGetWingCount(*cpacsHandle, wingCountPtr);
}


void tiglWingGetSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                               int* wingIndex,
                               int* segmentCountPtr,
                               TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetSegmentCount(*cpacsHandle, *wingIndex, segmentCountPtr);
}

void tiglWingGetComponentSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                               int* wingIndex,
                               int* compSegmentCountPtr,
                               TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetComponentSegmentCount(*cpacsHandle, *wingIndex, compSegmentCountPtr);
}

void tiglWingGetComponentSegmentUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* wingIndex,
                            int* segmentIndex,
                            char* uidNamePtr,
                            TiglReturnCode* returnCode,
                            int lengthString1)
{
    const char* namePtr = 0;
    *returnCode = tiglWingGetComponentSegmentUID(*cpacsHandle, *wingIndex, *segmentIndex, &uidNamePtr);
    
    if (*returnCode == TIGL_SUCCESS)
    {
        copyToFortranString(namePtr, lengthString1, uidNamePtr);
        if ((size_t) lengthString1 < strlen(namePtr) + 1)
        {
            *returnCode = TIGL_STRING_TRUNCATED;
        }
    }
}

void tiglWingGetComponentSegmentIndex(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* wingIndex,
                            char* uidNamePtr,
                            int* segmentIndex,
                            TiglReturnCode* returnCode,
                            int lengthString1)
{
    char * namePtr = makeCString(uidNamePtr, lengthString1);

    *returnCode = tiglWingGetComponentSegmentIndex(*cpacsHandle, *wingIndex, namePtr, segmentIndex);
    free(namePtr);
}

void tiglWingGetUpperPoint_f(TiglCPACSConfigurationHandle* cpacsHandle,
                             int* wingIndex,
                             int* segmentIndex,
                             double* eta,
                             double* xsi,
                             double* pointXPtr,
                             double* pointYPtr,
                             double* pointZPtr,
                             TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetUpperPoint(
        *cpacsHandle,
        *wingIndex,
        *segmentIndex,
        *eta,
        *xsi,
        pointXPtr,
        pointYPtr,
        pointZPtr);
}


void tiglWingGetLowerPoint_f(TiglCPACSConfigurationHandle* cpacsHandle,
                             int* wingIndex,
                             int* segmentIndex,
                             double* eta,
                             double* xsi,
                             double* pointXPtr,
                             double* pointYPtr,
                             double* pointZPtr,
                             TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetLowerPoint(
        *cpacsHandle,
        *wingIndex,
        *segmentIndex,
        *eta,
        *xsi,
        pointXPtr,
        pointYPtr,
        pointZPtr);
}


void tiglWingGetInnerConnectedSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             int* wingIndex,
                                             int* segmentIndex,
                                             int* segmentCountPtr,
                                             TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetInnerConnectedSegmentCount(
        *cpacsHandle,
        *wingIndex,
        *segmentIndex,
        segmentCountPtr);
}


void tiglWingGetOuterConnectedSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             int* wingIndex,
                                             int* segmentIndex,
                                             int* segmentCountPtr,
                                             TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetOuterConnectedSegmentCount(
        *cpacsHandle,
        *wingIndex,
        *segmentIndex,
        segmentCountPtr);
}


void tiglWingGetInnerConnectedSegmentIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             int* wingIndex,
                                             int* segmentIndex,
                                             int* n,
                                             int* connectedIndexPtr,
                                             TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetInnerConnectedSegmentIndex(
        *cpacsHandle,
        *wingIndex,
        *segmentIndex,
        *n,
        connectedIndexPtr);
}

void tiglWingGetOuterConnectedSegmentIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             int* wingIndex,
                                             int* segmentIndex,
                                             int* n,
                                             int* connectedIndexPtr,
                                             TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetOuterConnectedSegmentIndex(
        *cpacsHandle,
        *wingIndex,
        *segmentIndex,
        *n,
        connectedIndexPtr);
}


void tiglWingGetInnerSectionAndElementUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                              int* wingIndex,
                                              int* segmentIndex,
                                              char* sectionUIDPtr,
                                              char* elementUIDPtr,
                                              TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetInnerSectionAndElementUID(
        *cpacsHandle,
        *wingIndex,
        *segmentIndex,
        &sectionUIDPtr,
        &elementUIDPtr);
}


void tiglWingGetOuterSectionAndElementUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                              int* wingIndex,
                                              int* segmentIndex,
                                              char* sectionUIDPtr,
                                              char* elementUIDPtr,
                                              TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetOuterSectionAndElementUID(
        *cpacsHandle,
        *wingIndex,
        *segmentIndex,
        &sectionUIDPtr,
        &elementUIDPtr);
}


void tiglWingGetProfileName_f(TiglCPACSConfigurationHandle* cpacsHandle,
                              int* wingIndex,
                              int* sectionIndex,
                              int* elementIndex,
                              char* profileNamePtr,
                              TiglReturnCode* returnCode,
                              int lengthString1)
{
    char* namePtr = 0;
    *returnCode = tiglWingGetProfileName(*cpacsHandle, *wingIndex, *sectionIndex, *elementIndex, &namePtr);

    if (*returnCode == TIGL_SUCCESS)
    {
        copyToFortranString(namePtr, lengthString1, profileNamePtr);
        if ((size_t) lengthString1 < strlen(namePtr) + 1)
        {
            *returnCode = TIGL_STRING_TRUNCATED;
        }
    }
}


void tiglWingGetUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* wingIndex,
                            char* uidNamePtr,
                            TiglReturnCode* returnCode,
                            int lengthString1)
{
    const char* namePtr = 0;
    *returnCode = tiglWingGetUID(*cpacsHandle, *wingIndex, &uidNamePtr);
    
    if (*returnCode == TIGL_SUCCESS)
    {
        copyToFortranString(namePtr, lengthString1, uidNamePtr);
        if ((size_t) lengthString1 < strlen(namePtr) + 1)
        {
            *returnCode = TIGL_STRING_TRUNCATED;
        }
    }
}

void tiglWingGetIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            char* uidNamePtr,
                            int* wingIndex,
                            TiglReturnCode* returnCode,
                            int lengthString1)
{
    char* namePtr = makeCString(uidNamePtr, lengthString1);
    *returnCode = tiglWingGetIndex(*cpacsHandle, namePtr, wingIndex);
    free(namePtr);
}


void tiglWingGetSegmentUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* wingIndex,
                            int* segmentIndex,
                            char* uidNamePtr,
                            TiglReturnCode* returnCode,
                            int lengthString1)
{
    const char* namePtr = 0;
    *returnCode = tiglWingGetSegmentUID(*cpacsHandle, *wingIndex, *segmentIndex, &uidNamePtr);

    if (*returnCode == TIGL_SUCCESS)
    {
        copyToFortranString(namePtr, lengthString1, uidNamePtr);
        if ((size_t) lengthString1 < strlen(namePtr) + 1)
        {
            *returnCode = TIGL_STRING_TRUNCATED;
        }
    }
}

void tiglWingGetSegmentIndex(TiglCPACSConfigurationHandle* cpacsHandle,
                            char* uidNamePtr,
                            int* segmentIndex,
                            int* wingIndex,
                            TiglReturnCode* returnCode,
                            int lengthString1)
{
    char * namePtr = makeCString(uidNamePtr, lengthString1);

    *returnCode = tiglWingGetSegmentIndex(*cpacsHandle, namePtr, segmentIndex, wingIndex);
    free(namePtr);
}


void tiglWingGetSectionUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* wingIndex,
                            int* sectionIndex,
                            char* uidNamePtr,
                            TiglReturnCode* returnCode,
                            int lengthString1)
{
    const char* namePtr = 0;
    *returnCode = tiglWingGetSectionUID(*cpacsHandle, *wingIndex, *sectionIndex, &uidNamePtr);

    if (*returnCode == TIGL_SUCCESS)
    {
        copyToFortranString(namePtr, lengthString1, uidNamePtr);
        if ((size_t) lengthString1 < strlen(namePtr) + 1)
        {
            *returnCode = TIGL_STRING_TRUNCATED;
        }
    }
}


void tiglWingGetSymmetry_f(TiglCPACSConfigurationHandle *cpacsHandle,
                            int *wingIndex,
                            TiglSymmetryAxis *symmetryAxisPtr,
                            TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetSymmetry(
        *cpacsHandle,
        *wingIndex,
        symmetryAxisPtr);
}

void tiglWingComponentSegmentGetPoint_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                        char * csUID_f,
                                        double * eta,
                                        double * xsi,
                                        double * x,
                                        double * y,
                                        double * z,
                                        TiglReturnCode* returnCode,
                                        int lengthStringCsUID)
{
    char * csUID_c = makeCString(csUID_f, lengthStringCsUID);
    
    *returnCode = tiglWingComponentSegmentGetPoint(*cpacsHandle, csUID_c, *eta, *xsi, x, y, z);
    
    free(csUID_c);
}



//########################### GetPoint Functions #####################################

void tiglFuselageGetPointOnXPlane_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                   int *fuselageIndex,
                                   int *segmentIndex,
                                   double *eta,
                                   double *xpos,
                                   int *pointIndex,
                                   double* pointXPtr,
                                   double* pointYPtr,
                                   double* pointZPtr,
                                   TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetPointOnXPlane(*cpacsHandle,
                                               *fuselageIndex,
                                               *segmentIndex,
                                               *eta,
                                               * xpos,
                                               *pointIndex,
                                               pointXPtr,
                                               pointYPtr,
                                               pointZPtr);
}


void tiglFuselageGetPointOnYPlane_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                   int *fuselageIndex,
                                   int *segmentIndex,
                                   double *eta,
                                   double *xpos,
                                   int *pointIndex,
                                   double* pointXPtr,
                                   double* pointYPtr,
                                   double* pointZPtr,
                                   TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetPointOnYPlane(*cpacsHandle,
                                               *fuselageIndex,
                                               *segmentIndex,
                                               *eta,
                                               * xpos,
                                               *pointIndex,
                                               pointXPtr,
                                               pointYPtr,
                                               pointZPtr);
}


void tiglFuselageGetNumPointsOnYPlane(TiglCPACSConfigurationHandle *cpacsHandle,
                                       int *fuselageIndex,
                                       int *segmentIndex,
                                       double *eta,
                                       double *ypos,
                                       int* numPointsPtr,
                                       TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetNumPointsOnYPlane(*cpacsHandle,
                                                   *fuselageIndex,
                                                   *segmentIndex,
                                                   *eta,
                                                   *ypos,
                                                   numPointsPtr);
}

void tiglFuselageGetNumPointsOnXPlane(TiglCPACSConfigurationHandle *cpacsHandle,
                                       int *fuselageIndex,
                                       int *segmentIndex,
                                       double *eta,
                                       double *xpos,
                                       int* numPointsPtr,
                                       TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetNumPointsOnXPlane(*cpacsHandle,
                                                   *fuselageIndex,
                                                   *segmentIndex,
                                                   *eta,
                                                   *xpos,
                                                   numPointsPtr);
}


void tiglFuselageGetCircumference_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                    int *fuselageIndex,
                                    int *segmentIndex,
                                    double *eta,
                                    double *circumferencePtr,
                                    TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetCircumference(*cpacsHandle,
                                               *fuselageIndex,
                                               *segmentIndex,
                                               *eta,
                                               circumferencePtr);
}


void tiglFuselageGetPoint_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* fuselageIndex,
                            int* segmentIndex,
                            double* eta,
                            double* zeta,
                            double* pointXPtr,
                            double* pointYPtr,
                            double* pointZPtr,
                            TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetPoint(
        *cpacsHandle,
        *fuselageIndex,
        *segmentIndex,
        *eta,
        *zeta,
        pointXPtr,
        pointYPtr,
        pointZPtr);
}



void tiglFuselageGetPointAngle_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                int* fuselageIndex,
                                int* segmentIndex,
                                double* eta,
                                double* alpha,
                                double* pointXPtr,
                                double* pointYPtr,
                                double* pointZPtr,
                                TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetPointAngle(*cpacsHandle,
                                            *fuselageIndex,
                                            *segmentIndex,
                                            *eta,
                                            *alpha,
                                            pointXPtr,
                                            pointYPtr,
                                            pointZPtr);
}


void tiglFuselageGetPointAngleTranslated_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                            int* fuselageIndex,
                                            int* segmentIndex,
                                            double* eta,
                                            double* alpha,
                                            double* x_cs,
                                            double* z_cs,
                                            double* pointXPtr,
                                            double* pointYPtr,
                                            double* pointZPtr,
                                            TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetPointAngleTranslated(*cpacsHandle,
                                                      *fuselageIndex,
                                                      *segmentIndex,
                                                      *eta,
                                                      *alpha,
                                                      *x_cs,
                                                      *z_cs,
                                                      pointXPtr,
                                                      pointYPtr,
                                                      pointZPtr);
}


//##########################################################################################

void tiglGetFuselageCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* fuselageCountPtr,
                            TiglReturnCode* returnCode)
{
    *returnCode = tiglGetFuselageCount(*cpacsHandle, fuselageCountPtr);
}


void tiglFuselageGetSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                   int* fuselageIndex,
                                   int* segmentCountPtr,
                                   TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetSegmentCount(*cpacsHandle, *fuselageIndex, segmentCountPtr);
}



void tiglFuselageGetStartConnectedSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                                 int* fuselageIndex,
                                                 int* segmentIndex,
                                                 int* segmentCountPtr,
                                                 TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetStartConnectedSegmentCount(
        *cpacsHandle,
        *fuselageIndex,
        *segmentIndex,
        segmentCountPtr);
}


void tiglFuselageGetEndConnectedSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                               int* fuselageIndex,
                                               int* segmentIndex,
                                               int* segmentCountPtr,
                                               TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetEndConnectedSegmentCount(
        *cpacsHandle,
        *fuselageIndex,
        *segmentIndex,
        segmentCountPtr);
}


void tiglFuselageGetStartConnectedSegmentIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                                 int* fuselageIndex,
                                                 int* segmentIndex,
                                                 int* n,
                                                 int* connectedIndexPtr,
                                                 TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetStartConnectedSegmentIndex(
        *cpacsHandle,
        *fuselageIndex,
        *segmentIndex,
        *n,
        connectedIndexPtr);
}


void tiglFuselageGetEndConnectedSegmentIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                               int* fuselageIndex,
                                               int* segmentIndex,
                                               int* n,
                                               int* connectedIndexPtr,
                                               TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetEndConnectedSegmentIndex(
        *cpacsHandle,
        *fuselageIndex,
        *segmentIndex,
        *n,
        connectedIndexPtr);
}


void tiglFuselageGetStartSectionAndElementUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                                  int* fuselageIndex,
                                                  int* segmentIndex,
                                                  char* sectionIndexPtr,
                                                  char* elementIndexPtr,
                                                  TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetStartSectionAndElementUID(
        *cpacsHandle,
        *fuselageIndex,
        *segmentIndex,
        &sectionIndexPtr,
        &elementIndexPtr);
}

void tiglFuselageGetEndSectionAndElementUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                                int* fuselageIndex,
                                                int* segmentIndex,
                                                char* sectionIndexPtr,
                                                char* elementIndexPtr,
                                                TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetEndSectionAndElementUID(*cpacsHandle, *fuselageIndex, *segmentIndex, &sectionIndexPtr, &elementIndexPtr);
}

void tiglFuselageGetSymmetry_f(TiglCPACSConfigurationHandle *cpacsHandle,
                            int *fuselageIndex,
                            TiglSymmetryAxis *symmetryAxisPtr,
                            TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetSymmetry(*cpacsHandle, *fuselageIndex, symmetryAxisPtr);
}


/*
 * Volume Functions
 */

void tiglFuselageGetVolume_f(TiglCPACSConfigurationHandle *cpacsHandle,
                             int *fuselageIndex,
                             double *volumePtr,
                             TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetVolume(*cpacsHandle, *fuselageIndex, volumePtr);
}


void tiglWingGetVolume_f(TiglCPACSConfigurationHandle *cpacsHandle,
                             int *wingIndex,
                             double *volumePtr,
                             TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetVolume(*cpacsHandle, *wingIndex, volumePtr);
}


void tiglWingGetSegmentVolume_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                int *wingIndex,
                                int *segmentIndex,
                                double *volumePtr,
                                TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetSegmentVolume(*cpacsHandle, *wingIndex, *segmentIndex, volumePtr);
}


void tiglFuselageGetSegmentVolume_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                int *fuselageIndex,
                                int *segmentIndex,
                                double *volumePtr,
                                TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetSegmentVolume(*cpacsHandle, *fuselageIndex, *segmentIndex, volumePtr);
}



/*
 * Surface Area Functions
 */
void tiglFuselageGetSurfaceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                 int *fuselageIndex,
                                 double *surfaceAreaPtr,
                                 TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetSurfaceArea(*cpacsHandle, *fuselageIndex, surfaceAreaPtr);
}


void tiglWingGetSurfaceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                 int *wingIndex,
                                 double *surfaceAreaPtr,
                                 TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetSurfaceArea(*cpacsHandle, *wingIndex, surfaceAreaPtr);
}


void tiglWingGetSegmentSurfaceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                        int *wingIndex,
                                        int *segmentIndex,
                                        double *surfaceAreaPtr,
                                        TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetSegmentSurfaceArea(*cpacsHandle, *wingIndex, *segmentIndex, surfaceAreaPtr);
}


void tiglFuselageGetSegmentSurfaceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                    int *fuselageIndex,
                                    int *segmentIndex,
                                    double *surfaceAreaPtr,
                                    TiglReturnCode* returnCode)
{
    *returnCode = tiglFuselageGetSegmentSurfaceArea(*cpacsHandle, *fuselageIndex, *segmentIndex, surfaceAreaPtr);
}


void tiglWingGetReferenceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                 int *wingIndex,
                                 double *referenceAreaPtr,
                                 TiglReturnCode* returnCode)
{
    //*returnCode = tiglWingGetRefenceArea(*cpacsHandle, *wingIndex, referenceAreaPtr);
}


void tiglWingGetWettedArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                 char *wingUID,
                                 double *referenceAreaPtr,
                                 TiglReturnCode* returnCode)
{
    *returnCode = tiglWingGetWettedArea(*cpacsHandle, wingUID, referenceAreaPtr);
}

/*****************************************************************************************************/
/*                    VTK EXPORT                                                                     */
/*****************************************************************************************************/

void tiglExportMeshedWingVTKByIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                       int* wingIndex,
                                       char* filenamePtr,
                                       double* deflection,
                                       TiglReturnCode* returnCode,
                                       int lengthString1)
{
  char *cString = NULL;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportMeshedWingVTKByIndex(*cpacsHandle, *wingIndex, cString, *deflection);
  free(cString);
}


void tiglExportMeshedWingVTKByUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                       char* wingUID,
                                       char* filenamePtr,
                                       double* deflection,
                                       TiglReturnCode* returnCode,
                                       int lengthString1,
                                       int lengthString2)
{
  char *cString = NULL;
  char *cString2 = NULL;
  cString = makeCString(wingUID, lengthString1);
  cString = makeCString(filenamePtr, lengthString2);
  *returnCode = tiglExportMeshedWingVTKByUID(*cpacsHandle, cString, cString, *deflection);
  free(cString);
  free(cString2);
}



void tiglExportMeshedFuselageVTKByIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                           int* fuselageIndex,
                                           char* filenamePtr,
                                           double* deflection,
                                           TiglReturnCode* returnCode,
                                           int lengthString1)
{
  char *cString = NULL;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportMeshedFuselageVTKByIndex(*cpacsHandle, *fuselageIndex, cString, *deflection);
  free(cString);
}



void tiglExportMeshedFuselageVTKByUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                        char* fuselageUID,
                                        char* filenamePtr,
                                        double* deflection,
                                        TiglReturnCode* returnCode,
                                        int lengthString1,
                                        int lengthString2)
{
  char *cString = NULL;
  char *cString2 = NULL;
  cString = makeCString(fuselageUID, lengthString1);
  cString = makeCString(filenamePtr, lengthString2);
  *returnCode = tiglExportMeshedFuselageVTKByUID(*cpacsHandle, cString, cString, *deflection);
  free(cString);
  free(cString2);
}


void tiglExportMeshedGeometryVTK_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                   char* filenamePtr,
                                   double* deflection,
                                   TiglReturnCode* returnCode,
                                   int lengthString1)
{
  char *cString = NULL;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportMeshedGeometryVTK(*cpacsHandle, cString, *deflection);
  free(cString);
}



void tiglExportMeshedWingVTKSimpleByUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                          char* wingUID,
                                          char* filenamePtr,
                                          double* deflection,
                                          TiglReturnCode* returnCode,
                                          int lengthString1,
                                          int lengthString2)
{
  char *cString = NULL;
  char *cString2 = NULL;
  cString = makeCString(wingUID, lengthString1);
  cString = makeCString(filenamePtr, lengthString2);
  *returnCode = tiglExportMeshedWingVTKSimpleByUID(*cpacsHandle, cString, cString, *deflection);
  free(cString);
  free(cString2);
}


void tiglExportMeshedFuselageVTKSimpleByUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                            char* fuselageUID,
                                            char* filenamePtr,
                                            double* deflection,
                                            TiglReturnCode* returnCode,
                                            int lengthString1,
                                            int lengthString2)
{
  char *cString = NULL;
  char *cString2 = NULL;
  cString = makeCString(fuselageUID, lengthString1);
  cString = makeCString(filenamePtr, lengthString2);
  *returnCode = tiglExportMeshedFuselageVTKSimpleByUID(*cpacsHandle, cString, cString, *deflection);
  free(cString);
  free(cString2);
}


void tiglExportMeshedGeometryVTKSimple_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                       char* filenamePtr,
                                       double* deflection,
                                       TiglReturnCode* returnCode,
                                       int lengthString1)
{
  char *cString = NULL;
  cString = makeCString(filenamePtr, lengthString1);
  *returnCode = tiglExportMeshedGeometryVTKSimple(*cpacsHandle, cString, *deflection);
  free(cString);
}

/*****************************************************************************************************/
/*                    MATERIAL FUNCTIONS                                                             */
/*****************************************************************************************************/

void tiglWingComponentSegmentGetMaterialUIDs_f(TiglCPACSConfigurationHandle* cpacsHandle,
                               char* uID,
                               double* eta,
                               double* xsi,
                               const char*** materialUIDs,
                               int * nuids,
                               TiglReturnCode* returnCode,
                               int lengthString1)
{
  /*char *cString = NULL;
  char *matUID = NULL;
  cString = makeCString(uID, lengthString1);
  *returnCode = tiglGetMaterialUID(*cpacsHandle, cString, *eta, *xsi, &matUID);
  copyToFortranString(matUID, strlen(matUID), materialUID);
  free(cString);
  free(matUID);*/
    
  
  //@todo: provide proper implementation. how to pass array of string to fortran???
  *returnCode = TIGL_ERROR;
}



/*****************************************************************************************************/
/*                     UTILITY FUNCTIONS                                                             */
/*****************************************************************************************************/
void tiglComponentGetHashCode(TiglCPACSConfigurationHandle* cpacsHandle,
                               char* uID,
                               int* hashCode,
                               TiglReturnCode* returnCode,
                               int lengthString1)
{
  char *cString = NULL;
  cString = makeCString(uID, lengthString1);
  *returnCode = tiglComponentGetHashCode(*cpacsHandle, cString, hashCode);
  free(cString);
}


void tiglFuselageGetMinumumDistanceToGround_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             char *fuselageUID,
                                             double* axisPntX,
                                             double* axisPntY,
                                             double* axisPntZ,
                                             double* axisDirX,
                                             double* axisDirY,
                                             double* axisDirZ,
                                             double* angle,
                                             double* pointXPtr,
                                             double* pointYPtr,
                                             double* pointZPtr,
                                             TiglReturnCode* returnCode,
                                             int lengthString1)
{
      char *cString = NULL;
      cString = makeCString(fuselageUID, lengthString1);
      *returnCode = tiglFuselageGetMinumumDistanceToGround(*cpacsHandle,
                                                                                     cString,
                                                                                     *axisPntX,
                                                                                     *axisPntY,
                                                                                     *axisPntZ,
                                                                                     *axisDirX,
                                                             *axisDirY,
                                                             *axisDirZ,
                                                             *angle,
                                                             pointXPtr,
                                                             pointYPtr,
                                                             pointZPtr);
      free(cString);
}

void tiglGetErrorString_f(TiglReturnCode code, char * strpointer, int lengthStr){
    const char * str = tiglGetErrorString(code);
    copyToFortranString(str, lengthStr, strpointer);
}

