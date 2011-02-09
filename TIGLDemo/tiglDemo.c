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
* @brief Demo program for TIGL, demonstrates how to use the TIGL routines
*/

#include "tigl.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{
    TixiDocumentHandle           tixiHandle;
    ReturnCode                   tixiReturn;
    TiglCPACSConfigurationHandle cpacsHandle;
    TiglReturnCode               tiglReturn;
    
    char* filename   = "./TestData/cpacs_example.xml";
    char* exportName = "./TestData/cpacs_example.iges";

    int wingCount;
    int wingIndex;
    int fuselageCount;
    int fuselageIndex;
    int segmentCount;
    int segmentIndex;
    int innerSegmentCount;
    int outerSegmentCount;
    int startSegmentCount;
    int endSegmentCount;
    int n;
    int connectedSegmentIndex;
	char* elementUID = NULL;
	char* sectionUID = NULL;
	int sectionIndex;
	int elementIndex;
    double xsi;
    double eta;
    double zeta;
    double x, y, z;
    char *sectionIndexUID;
	char *elementIndexUID;


    /** 
    Open a CPACS configuration file. First open the CPACS-XML file
    with TIXI to get a tixi handle and then use this handle to open
    and read the CPACS configuration.
    */
	tixiReturn = tixiOpenDocument(filename, &tixiHandle);
    if (tixiReturn != SUCCESS) 
    {
        fprintf(stdout, "\nError: tixiOpenDocument failed for file %s\n", filename);
        exit(1);
    }
    
    tiglReturn = tiglOpenCPACSConfiguration(tixiHandle, "VFW-614", &cpacsHandle);
    if (tiglReturn != TIGL_SUCCESS) 
    {
        tixiCloseDocument(tixiHandle);
        fprintf(stdout, "\nError: tiglOpenCPACSConfiguration failed for file %s\n", filename);
        exit(1);
    }

    /**
    Set the wire algorithm to be used in interpolations. By default this is the
    bspline interpolation algorithm. The algorithm is used globally for all
    open CPACS configurations.
    */
    tiglReturn = tiglUseAlgorithm(TIGL_INTERPOLATE_BSPLINE_WIRE);
    if (tiglReturn != TIGL_SUCCESS) 
    {
        tixiCloseDocument(tixiHandle);
        fprintf(stdout, "\nError: tiglUseAlgorithm failed\n");
        exit(1);
    }

    /**
    * Output some CPACS statistics.
    */
    fprintf(stdout, "\nFile %s:\n", filename);

    tiglGetWingCount(cpacsHandle, &wingCount);
    fprintf(stdout, "Number of wings: %d\n", wingCount);
    /* Loop over all wings and get the number of wing segments */
    for (wingIndex = 1; wingIndex <= wingCount; wingIndex++) 
    {
        tiglWingGetSegmentCount(cpacsHandle, wingIndex, &segmentCount);        
        fprintf(stdout, "Number of segments of wing number %d: %d\n", wingIndex, segmentCount);
    }

    fprintf(stdout, "\n");

    tiglGetFuselageCount(cpacsHandle, &fuselageCount);
    fprintf(stdout, "Number of fuselages: %d\n", fuselageCount);
    /* Loop over all fuselages and get the number of fuselage segments */
    for (fuselageIndex = 1; fuselageIndex <= fuselageCount; fuselageIndex++) 
    {
        tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, &segmentCount);        
        fprintf(stdout, "Number of segments of fuselage number %d: %d\n", fuselageIndex, segmentCount);
    }

    fprintf(stdout, "\n");

    /**
    * Loop over all wings and output some points.
    */
    for (wingIndex = 1; wingIndex <= wingCount; wingIndex++) 
    {
        fprintf(stdout, "Wing number %d:\n", wingIndex);
        tiglWingGetSegmentCount(cpacsHandle, wingIndex, &segmentCount);
        for (segmentIndex = 1; segmentIndex <= segmentCount; segmentIndex++) 
        {
            fprintf(stdout, "  Segment number %d:\n", segmentIndex);
            for (eta = 0.0; eta <= 1.0; eta += 0.5) 
            {
                for (xsi = 0.0; xsi <= 1.0; xsi += 0.5) 
                {
                    tiglWingGetUpperPoint(cpacsHandle, wingIndex, segmentIndex, eta, xsi, &x, &y, &z);
                    fprintf(stdout, "    eta = %f, xsi = %f: upper point = (%f, %f, %f)\n", eta, xsi, x, y, z);

                    tiglWingGetLowerPoint(cpacsHandle, wingIndex, segmentIndex, eta, xsi, &x, &y, &z);
                    fprintf(stdout, "    eta = %f, xsi = %f: lower point = (%f, %f, %f)\n", eta, xsi, x, y, z);
                }
            }
        }
        fprintf(stdout, "\n");
    }

    /**
    * Loop over all fuselages and output some points.
    */
    for (fuselageIndex = 1; fuselageIndex <= fuselageCount; fuselageIndex++) 
    {
        fprintf(stdout, "Fuselage number %d:\n", fuselageIndex);
        tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, &segmentCount);
        for (segmentIndex = 1; segmentIndex <= segmentCount; segmentIndex++) 
        {
            fprintf(stdout, "  Segment number %d:\n", segmentIndex);
            for (eta = 0.0; eta <= 1.0; eta += 0.5) 
            {
                for (zeta = 0.0; zeta <= 1.0; zeta += 0.5) 
                {
                    tiglFuselageGetPoint(cpacsHandle, fuselageIndex, segmentIndex, eta, zeta, &x, &y, &z);
                    fprintf(stdout, "    eta = %f, zeta = %f: point = (%f, %f, %f)\n", eta, zeta, x, y, z);
                }
            }
        }
        fprintf(stdout, "\n");
    }

    /**
    * Loop over all wings and output for every segment the connected inner and outer segments.
    */
    for (wingIndex = 1; wingIndex <= wingCount; wingIndex++) 
    {
        fprintf(stdout, "Wing number %d:\n", wingIndex);
        tiglWingGetSegmentCount(cpacsHandle, wingIndex, &segmentCount);
        for (segmentIndex = 1; segmentIndex <= segmentCount; segmentIndex++) 
        {
            fprintf(stdout, "  Segment number %d:\n", segmentIndex);

            /** Inner connected segments */
            tiglWingGetInnerConnectedSegmentCount(cpacsHandle, wingIndex, segmentIndex, &innerSegmentCount);
            fprintf(stdout, "    Count of inner connected segments: %d\n", innerSegmentCount);
            for (n = 1; n <= innerSegmentCount; n++)
            {
                tiglWingGetInnerConnectedSegmentIndex(cpacsHandle, wingIndex, segmentIndex, n, &connectedSegmentIndex);
                fprintf(stdout, "    Index of the %d-th inner connected segment: %d\n", n, connectedSegmentIndex);
            }

            /** Outer connected segments */
            tiglWingGetOuterConnectedSegmentCount(cpacsHandle, wingIndex, segmentIndex, &outerSegmentCount);
            fprintf(stdout, "    Count of outer connected segments: %d\n", outerSegmentCount);
            for (n = 1; n <= outerSegmentCount; n++)
            {
                tiglWingGetOuterConnectedSegmentIndex(cpacsHandle, wingIndex, segmentIndex, n, &connectedSegmentIndex);
                fprintf(stdout, "    Index of the %d-th outer connected segment: %d\n", n, connectedSegmentIndex);
            }
        }
        fprintf(stdout, "\n");
    }

    /**
    * Loop over all wings and output for every segment the section and element UIDs.
    */
    for (wingIndex = 1; wingIndex <= wingCount; wingIndex++) 
    {
        fprintf(stdout, "Wing number %d:\n", wingIndex);
        tiglWingGetSegmentCount(cpacsHandle, wingIndex, &segmentCount);
        for (segmentIndex = 1; segmentIndex <= segmentCount; segmentIndex++) 
        {
            fprintf(stdout, "  Segment number %d:\n", segmentIndex);

			tiglWingGetInnerSectionAndElementUID(cpacsHandle, wingIndex, segmentIndex, &sectionUID, &elementIndexUID);
            fprintf(stdout, "    Inner section UID: %s\n", sectionUID);
            fprintf(stdout, "    Inner element UID: %s\n", elementIndexUID);
			free(sectionUID);
			free(elementIndexUID);

            tiglWingGetOuterSectionAndElementUID(cpacsHandle, wingIndex, segmentIndex, &sectionUID, &elementIndexUID);
            fprintf(stdout, "    Outer section UID: %s\n", sectionUID);
            fprintf(stdout, "    Outer element UID: %s\n", elementIndexUID);
			free(sectionUID);
			free(elementIndexUID);
        }
        fprintf(stdout, "\n");
    }



    /**
    * Loop over all wings and output for every segment the section and element indices.
    */
    for (wingIndex = 1; wingIndex <= wingCount; wingIndex++) 
    {
        fprintf(stdout, "Wing number %d:\n", wingIndex);
        tiglWingGetSegmentCount(cpacsHandle, wingIndex, &segmentCount);
        for (segmentIndex = 1; segmentIndex <= segmentCount; segmentIndex++) 
        {
            fprintf(stdout, "  Segment number %d:\n", segmentIndex);

			tiglWingGetInnerSectionAndElementIndex(cpacsHandle, wingIndex, segmentIndex, &sectionIndex, &elementIndex);
            fprintf(stdout, "    Inner section index: %d\n", sectionIndex);
            fprintf(stdout, "    Inner element index: %d\n", elementIndex);

            tiglWingGetOuterSectionAndElementIndex(cpacsHandle, wingIndex, segmentIndex, &sectionIndex, &elementIndex);
            fprintf(stdout, "    Outer section index: %d\n", sectionIndex);
            fprintf(stdout, "    Outer element index: %d\n", elementIndex);
        }
        fprintf(stdout, "\n");
    }


    /**
    * Loop over all fuselages and output for every segment the segments connected
    * to the start and end of the current segment.
    */
    for (fuselageIndex = 1; fuselageIndex <= fuselageCount; fuselageIndex++) 
    {
        fprintf(stdout, "Fuselage number %d:\n", fuselageIndex);
        tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, &segmentCount);
        for (segmentIndex = 1; segmentIndex <= segmentCount; segmentIndex++) 
        {
            fprintf(stdout, "  Segment number %d:\n", segmentIndex);

            /** To the start section connected segments */
            tiglFuselageGetStartConnectedSegmentCount(cpacsHandle, fuselageIndex, segmentIndex, &startSegmentCount);
            fprintf(stdout, "    Count of to the start section connected segments: %d\n", startSegmentCount);
            for (n = 1; n <= startSegmentCount; n++)
            {
                tiglFuselageGetStartConnectedSegmentIndex(cpacsHandle, fuselageIndex, segmentIndex, n, &connectedSegmentIndex);
                fprintf(stdout, "    Index of the %d-th to the start section connected segment: %d\n", n, connectedSegmentIndex);
            }

            /** To the end connected segments */
            tiglFuselageGetEndConnectedSegmentCount(cpacsHandle, fuselageIndex, segmentIndex, &endSegmentCount);
            fprintf(stdout, "    Count of to the end section connected segments: %d\n", endSegmentCount);
            for (n = 1; n <= endSegmentCount; n++)
            {
                tiglFuselageGetEndConnectedSegmentIndex(cpacsHandle, fuselageIndex, segmentIndex, n, &connectedSegmentIndex);
                fprintf(stdout, "    Index of the %d-th to the end section connected segment: %d\n", n, connectedSegmentIndex);
            }
        }
        fprintf(stdout, "\n");
    }

    /**
    * Loop over all fuselages and output for every segment the section and element UID.
    */
    for (fuselageIndex = 1; fuselageIndex <= fuselageCount; fuselageIndex++) 
    {
        fprintf(stdout, "Fuselage number %d:\n", fuselageIndex);
        tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, &segmentCount);
        for (segmentIndex = 1; segmentIndex <= segmentCount; segmentIndex++) 
        {
            fprintf(stdout, "  Segment number %d:\n", segmentIndex);

            tiglFuselageGetStartSectionAndElementUID(cpacsHandle, fuselageIndex, segmentIndex, &sectionIndexUID, &elementIndexUID);
            fprintf(stdout, "    Start section UID: %s\n", sectionIndexUID);
            fprintf(stdout, "    Start element UID: %s\n", elementIndexUID);

            tiglFuselageGetEndSectionAndElementUID(cpacsHandle, fuselageIndex, segmentIndex, &sectionIndexUID, &elementIndexUID);
            fprintf(stdout, "    End section UID: %s\n", sectionIndexUID);
            fprintf(stdout, "    End element UID: %s\n", elementIndexUID);
        }
        fprintf(stdout, "\n");
    }


	/**
    * Loop over all fuselages and output for every segment the section and element indicies.
    */
    for (fuselageIndex = 1; fuselageIndex <= fuselageCount; fuselageIndex++) 
    {
        fprintf(stdout, "Fuselage number %d:\n", fuselageIndex);
        tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, &segmentCount);
        for (segmentIndex = 1; segmentIndex <= segmentCount; segmentIndex++) 
        {
            fprintf(stdout, "  Segment number %d:\n", segmentIndex);

            tiglFuselageGetStartSectionAndElementIndex(cpacsHandle, fuselageIndex, segmentIndex, &sectionIndex, &elementIndex);
            fprintf(stdout, "    Start section index: %d\n", sectionIndex);
            fprintf(stdout, "    Start element index: %d\n", elementIndex);

            tiglFuselageGetEndSectionAndElementIndex(cpacsHandle, fuselageIndex, segmentIndex, &sectionIndex, &elementIndex);
            fprintf(stdout, "    End section index: %d\n", sectionIndex);
            fprintf(stdout, "    End element index: %d\n", elementIndex);
        }
        fprintf(stdout, "\n");
    }

    /**
    * Export CPACS geometry as IGES file.
    */
    fprintf(stdout, "\nExporting CPACS geometry as IGES file...\n");
    tiglExportIGES(cpacsHandle, exportName);

    /**
    * Cleanup: First(!) close the CPACS configuration and then the corresponding tixi file handle.
    */
    tiglCloseCPACSConfiguration(cpacsHandle);
    tixiCloseDocument(tixiHandle);
    return 0;
}
