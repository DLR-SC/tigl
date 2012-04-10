#!/usr/bin/python
# Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)

# Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
# Changed: $Id$ 

# Version: $Revision$

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @file
# @brief Demo Python script for TIGL, demonstrates how to use the TIGL routines
#

from ctypes import *
from os import *

# define handles
cpacsHandle = c_int(0)
tixiHandle = c_int(0)
filename   = "./cpacs_example.xml"
exportName = "./cpacs_example.iges"
segmentCount = c_int(0)
segmentIndex = c_int(0)
wingCount = c_int(0)
fuselageCount = c_int(0)
innerSegmentCount = c_int(0)
outerSegmentCount = c_int(0)
connectedSegmentIndex = c_int(0)
startSegmentCount = c_int(0)
endSegmentCount = c_int(0)
sectionIndex = c_int(0)
elementIndex = c_int(0)
sectionUID = c_char_p()
elementUID = c_char_p()
xsi = c_double(0)
eta = c_double(0)
zeta = c_double(0)
x = c_double(0)
y = c_double(0)
z = c_double(0)
elementIndex = c_int(0)
sectionIndex = c_int(0)



# open TIXI and TIGL shared libraries
import sys
if sys.platform == 'win32':
    TIXI = cdll.TIXI
    TIGL = cdll.TIGL
else:
    TIXI = CDLL("libTIXI.so")
    TIGL = CDLL("libTIGL.so")

# Open a CPACS configuration file. First open the CPACS-XML file
# with TIXI to get a tixi handle and then use this handle to open
# and read the CPACS configuration.
tixiReturn = TIXI.tixiOpenDocument(filename, byref(tixiHandle))
if tixiReturn != 0:
    print 'Error: tixiOpenDocument failed for file: ' + filename
    exit(1)

tiglReturn = TIGL.tiglOpenCPACSConfiguration(tixiHandle, "VFW-614", byref(cpacsHandle))
if tiglReturn != 0:
    TIXI.tixiCloseDocument(tixiHandle)
    print "Error: tiglOpenCPACSConfiguration failed for file: " + filename
    exit(1)

# Set the wire algorithm to be used in interpolations. By default this is the
# bspline interpolation algorithm. The algorithm is used globally for all
# open CPACS configurations.

#tiglReturn = TIGL.tiglUseAlgorithm(TIGL_INTERPOLATE_BSPLINE_WIRE)
tiglReturn = TIGL.tiglUseAlgorithm(1)
if tiglReturn != 0:
    TIXI.tixiCloseDocument(tixiHandle)
    print "Error: tiglUseAlgorithm failed"
    exit(1)


#
# Output some CPACS statistics.
#
print "File: " + filename

TIGL.tiglGetWingCount(cpacsHandle, byref(wingCount))
print "Number of wings: %d" % wingCount.value
# Loop over all wings and get the number of wing segments
for wingIndex in range(1, wingCount.value + 1):
    TIGL.tiglWingGetSegmentCount(cpacsHandle, wingIndex, byref(segmentCount))
    print "Number of segments of wing number %d: %d" % (wingIndex, segmentCount.value)
print


TIGL.tiglGetFuselageCount(cpacsHandle, byref(fuselageCount))
print "Number of fuselages: %d" % fuselageCount.value
# Loop over all fuselages and get the number of fuselage segments
for fuselageIndex in range(1, fuselageCount.value + 1):
    TIGL.tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, byref(segmentCount))
    print "Number of segments of fuselage number %d: %d" % (fuselageIndex, segmentCount.value)
print

#--------------------------------------------
# Loop over all wings and output some points.
#--------------------------------------------
for wingIndex in range(1, wingCount.value + 1):
    print "Wing number %d" % wingIndex
    TIGL.tiglWingGetSegmentCount(cpacsHandle, wingIndex, byref(segmentCount))
    for segmentIndex in range(1, segmentCount.value + 1):
        print "  Segment number %d" % segmentIndex
        for eta in range(0, 11, 5):
            for xsi in range(0, 11, 5):
                d_eta = c_double(eta/10.0)
                d_xsi = c_double(xsi/10.0)
                TIGL.tiglWingGetUpperPoint(cpacsHandle, wingIndex, segmentIndex, d_eta, d_xsi, byref(x), byref(y), byref(z))
                print "     eta = %f, xsi = %f: upper point = (%f, %f, %f)" % (d_eta.value, d_xsi.value, x.value, y.value, z.value)
                
                TIGL.tiglWingGetLowerPoint(cpacsHandle, wingIndex, segmentIndex, d_eta, d_xsi, byref(x), byref(y), byref(z))
                print "     eta = %f, xsi = %f: lower point = (%f, %f, %f)" % (d_eta.value, d_xsi.value, x.value, y.value, z.value)




#--------------------------------------------
# Loop over all fuselages and output some points.
#--------------------------------------------
print 
for fuselageIndex in range(1, fuselageCount.value + 1):
    print
    print "Fuselage number %d" % fuselageIndex
    TIGL.tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, byref(segmentCount))
    for segmentIndex in range(1, segmentCount.value + 1):
        print "  Segment number %d" % segmentIndex
        for eta in range(0, 11, 5):
            for xsi in range(0, 11, 5):
                d_eta = c_double(eta/10.0)
                d_xsi = c_double(xsi/10.0)
                TIGL.tiglFuselageGetPoint(cpacsHandle, fuselageIndex, segmentIndex, d_eta, d_xsi, byref(x), byref(y), byref(z))
                print "     eta = %f, xsi = %f: upper point = (%f, %f, %f)" % (d_eta.value, d_xsi.value, x.value, y.value, z.value)


print
print



#----------------------------------------------------------------------------------------
# Loop over all wings and output for every segment the connected inner and outer segments.
#----------------------------------------------------------------------------------------
for wingIndex in range(1, wingCount.value + 1):
    print "Wing number %d" % wingIndex
    TIGL.tiglWingGetSegmentCount(cpacsHandle, wingIndex, byref(segmentCount))
    for segmentIndex in range(1, segmentCount.value + 1):
        print "  Segment number %d" % segmentIndex
        
        # Inner connected segments
        TIGL.tiglWingGetInnerConnectedSegmentCount(cpacsHandle, wingIndex, segmentIndex, byref(innerSegmentCount))
        print "    Count of inner connected segments: %d"% innerSegmentCount.value
        for n in range(1, innerSegmentCount.value + 1):
            TIGL.tiglWingGetInnerConnectedSegmentIndex(cpacsHandle, wingIndex, segmentIndex, n, byref(connectedSegmentIndex))
            print "    Index of the %d-th inner connected segment: %d" % (n, connectedSegmentIndex.value)

        # Outer connected segments
        TIGL.tiglWingGetOuterConnectedSegmentCount(cpacsHandle, wingIndex, segmentIndex, byref(outerSegmentCount))
        print "    Count of outer connected segments: %d"% outerSegmentCount.value
        for n in range(1, outerSegmentCount.value + 1):
            TIGL.tiglWingGetOuterConnectedSegmentIndex(cpacsHandle, wingIndex, segmentIndex, n, byref(connectedSegmentIndex))
            print "    Index of the %d-th outer connected segment: %d" % (n, connectedSegmentIndex.value)

print



#----------------------------------------------------------------------------------
# Loop over all wings and output for every segment the section and element UIDs.
#----------------------------------------------------------------------------------
for wingIndex in range(1, wingCount.value + 1):
    print "Wing number %d" % wingIndex
    TIGL.tiglWingGetSegmentCount(cpacsHandle, wingIndex, byref(segmentCount))
    for segmentIndex in range(1, segmentCount.value + 1):
        print "  Segment number %d:"% segmentIndex

        TIGL.tiglWingGetInnerSectionAndElementUID(cpacsHandle, wingIndex, segmentIndex, byref(sectionUID), byref(elementUID))
        print "    Inner section UID: " + str(sectionUID.value)
        print "    Inner element UID: " + str(elementUID.value)

        TIGL.tiglWingGetOuterSectionAndElementUID(cpacsHandle, wingIndex, segmentIndex, byref(sectionUID), byref(elementUID))
        print "    Outer section UID: " + str(sectionUID.value)
        print "    Outer element UID: " + str(elementUID.value)
print




#----------------------------------------------------------------------------------
# Loop over all wings and output for every segment the section and element indices.
#----------------------------------------------------------------------------------
for wingIndex in range(1, wingCount.value + 1):
    print "Wing number %d" % wingIndex
    TIGL.tiglWingGetSegmentCount(cpacsHandle, wingIndex, byref(segmentCount))
    for segmentIndex in range(1, segmentCount.value + 1):
        print "  Segment number %d:"% segmentIndex

        TIGL.tiglWingGetInnerSectionAndElementIndex(cpacsHandle, wingIndex, segmentIndex, byref(sectionIndex), byref(elementIndex))
        print "    Inner section index: " + str(sectionIndex.value)
        print "    Inner element index: " + str(elementIndex.value)

        TIGL.tiglWingGetOuterSectionAndElementIndex(cpacsHandle, wingIndex, segmentIndex, byref(sectionIndex), byref(elementIndex))
        print "    Outer section index: " + str(sectionIndex.value)
        print "    Outer element index: " + str(elementIndex.value)
print
#----------------------------------------------------------------------------------
# Loop over all fuselages and output for every segment the segments connected
# to the start and end of the current segment.
#----------------------------------------------------------------------------------
for fuselageIndex in range(1, fuselageCount.value + 1):
    print "Fuselage number %d" % fuselageIndex
    TIGL.tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, byref(segmentCount))
    for segmentIndex in range(1, segmentCount.value + 1):
        print "  Segment number %d:"% segmentIndex
        
        #To the start section connected segments#
        TIGL.tiglFuselageGetStartConnectedSegmentCount(cpacsHandle, fuselageIndex, segmentIndex, byref(startSegmentCount))
        print "    Count of to the start section connected segments: %d" % startSegmentCount.value
        for n in range(1, startSegmentCount.value + 1):
            TIGL.tiglFuselageGetStartConnectedSegmentIndex(cpacsHandle, fuselageIndex, segmentIndex, n, byref(connectedSegmentIndex))
            print "    Index of the %d-th to the start section connected segment: %d" % (n, connectedSegmentIndex.value)
        
        # To the end connected segments
        TIGL.tiglFuselageGetEndConnectedSegmentCount(cpacsHandle, fuselageIndex, segmentIndex, byref(endSegmentCount))
        print "    Count of to the start section connected segments: %d" % endSegmentCount.value
        for n in range(1, endSegmentCount.value + 1):
            TIGL.tiglFuselageGetEndConnectedSegmentIndex(cpacsHandle, fuselageIndex, segmentIndex, n, byref(connectedSegmentIndex))
            print "    Index of the %d-th to the end section connected segment: %d" % (n, connectedSegmentIndex.value)


print



#--------------------------------------------------------------------------------------
# Loop over all fuselages and output for every segment the section and element indices.
#--------------------------------------------------------------------------------------
for fuselageIndex in range(1, fuselageCount.value + 1):
    print "Fuselage number %d" % fuselageIndex
    TIGL.tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, byref(segmentCount))
    for segmentIndex in range(1, segmentCount.value + 1):
        print "  Segment number %d:"% segmentIndex
        
        TIGL.tiglFuselageGetStartSectionAndElementIndex(cpacsHandle, fuselageIndex, segmentIndex, byref(sectionIndex), byref(elementIndex))
        print "    Start section index: %d" % sectionIndex.value
        print "    Start element index: %d" % elementIndex.value
        
        TIGL.tiglFuselageGetEndSectionAndElementIndex(cpacsHandle, fuselageIndex, segmentIndex, byref(sectionIndex), byref(elementIndex))
        print "    End section index: %d" % sectionIndex.value
        print "    End element index: %d" % elementIndex.value

print


#-----------------------------------------
# Testing intersection line computing
#-----------------------------------------
intersectionLineCount = c_int(0)
TIGL.tiglFuselageWingSurfaceIntersectionLineCount(cpacsHandle, 1, 1, byref(intersectionLineCount))
print "Number of intersection lines between Wing=1 and Fuselage=1: %d" % intersectionLineCount.value

print



#------------------------------------
# Export CPACS geometry as IGES file.
#------------------------------------
print "Exporting CPACS geometry as IGES file..."
TIGL.tiglExportIGES(cpacsHandle, exportName)



#---------------------------------------------------------------------------------------------
# Cleanup: First(!) close the CPACS configuration and then the corresponding tixi file handle.
#---------------------------------------------------------------------------------------------
TIGL.tiglCloseCPACSConfiguration(cpacsHandle)
TIXI.tixiCloseDocument(tixiHandle)
























