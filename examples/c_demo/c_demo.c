/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-14 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "tigl.h"
#include "tixi.h"
#include <stdio.h>

void printUsage()
{
    printf("Usage: c-demo <cpacsfile>\n\n");
}

int main(int argc, char* argv[])
{
    TixiDocumentHandle tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    int nWings = 0;
    int nFuselages = 0;
    int i = 0;
    double x, y, z;
    
    if (argc != 2) {
        printUsage();
        return 1;
    }
    
    // read in cpacs xml file
    if (tixiOpenDocument(argv[1], &tixiHandle) != SUCCESS) {
        return 2;
    }
    
    // enable logging of errors and warnings into file
    tiglLogSetFileEnding("txt");
    tiglLogSetTimeInFilenameEnabled(TIGL_FALSE);
    tiglLogToFileEnabled("demolog");
    
    // now open cpacs file with tigl
    if (tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle) != TIGL_SUCCESS) {
        printf("Error reading in cpacs file with TiGL\n");
        return 3;
    }
    
    // query number of wings and fuselages and their names
    tiglGetWingCount(tiglHandle, &nWings);
    tiglGetFuselageCount(tiglHandle, &nFuselages);
    for (i = 1; i <= nWings; ++i) {
        char * wingUID = NULL;
        tiglWingGetUID(tiglHandle, i, &wingUID);
        printf("Wing %d name: %s\n", i, wingUID);
    }
    
    for (i = 1; i <= nFuselages; ++i) {
        char * fuselageUID = NULL;
        tiglFuselageGetUID(tiglHandle, i, &fuselageUID);
        printf("Fuselage %d name: %s\n", i, fuselageUID);
    }
    
    // query point on the upper wing surface
    if (nWings > 0 && tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.5, 0.5, &x, &y, &z) == TIGL_SUCCESS) {
        printf("Point on upper wing surface of wing 1/segment 1: (x,y,z) = (%g, %g, %g)\n", x, y, z);
    }
    
    // compute intersection of wing with a x-z plane
    if (nWings > 0) {
        char * wingUID = NULL;
        char* int_id = NULL;
        int nlines = 0;
        tiglWingGetUID(tiglHandle, 1, &wingUID);
        // do the intersection with a plane at p = (0,0.5,0) and normal vector n = (0,1,0)
        tiglIntersectWithPlane(tiglHandle, wingUID, 0, 0.5, 0, 0, 1, 0, &int_id);
        
        // get number of intersection wires
        tiglIntersectGetLineCount(tiglHandle, int_id, &nlines);
        
        // query points on the first line
        if (nlines > 0) {
            double zeta = 0.;
            printf("\nIntersection points of a plane with first wing:\n");
            while ( zeta < 1) {
                tiglIntersectGetPoint(tiglHandle, int_id, 1, zeta, &x, &y, &z);
                printf("zeta = %g\tp=(%g, %g, %g)\n", zeta, x, y, z);
                zeta += 0.1;
            }
            printf("\n");
        }
    }
    
    // Export geometry to iges file
    if (tiglExportIGES(tiglHandle, "igesexport.igs") != TIGL_SUCCESS) {
        printf("Error exporting geometry to IGES!\n");
    }
    
    // close everything
    tiglCloseCPACSConfiguration(tiglHandle);
    tixiCloseDocument(tixiHandle);
    tixiCleanup();
    
    return 0;
}
