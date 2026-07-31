#! /usr/bin/env python
# -*- coding: cp1252 -*-
#############################################################################
# Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
#
# Created: 2014-01-17 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *
import sys

# this function produces (console_logging_level=3 (TILOG_INFO)):
# WRN 01/17 09:19:00 tigl.cpp:108] CPACS dataset version is higher than TIGL library version!
# INF 01/17 09:19:00 CCPACSFarField.cpp:65] No far-field defined.
# ERR 01/17 09:19:00 tigl.cpp:1445] Error: Invalid uid in tiglWingComponentSegmentPointGetSegmentEtaXsi
def test_logging(console_logging_level):
    tixi = Tixi3()
    tigl = Tigl3()

    # set logging
    tigl.logSetFileEnding("log")
    tigl.logSetTimeInFilenameEnabled(False)
    tigl.logToFileEnabled("test_logging")
    tigl.logSetVerbosity(console_logging_level)
    tixi.open('TestData/simple_test_logging.xml')
    tigl.open(tixi, 'LoggingTestModel')

    # trigger an info by vtk export
    try: 
        tigl.exportMeshedWingVTKByIndex(1, "tmp.vtk", 0.2)
    except Tigl3Exception as err:
        pass

    # get error by requesting non-existent wing component segment
    try:
        tigl.wingComponentSegmentPointGetSegmentEtaXsi('nonexistent', 0.29029029, 0.75)
    except Tigl3Exception as err:
        pass


if __name__ == '__main__':
    console_logging_level=0
    try:
        console_logging_level=int(sys.argv[1])
    except:
        print("Error in logging_helper.py: Call with one argument: Console logging level")
    test_logging(console_logging_level)

