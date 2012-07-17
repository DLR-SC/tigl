#! /usr/bin/env python
#############################################################################
# Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
#
# Created: 2010-08-13 Arne Bachmann <Arne.Bachmann@dlr.de>
# Changed: $Id: dbms.h 4577 20xx-xx-xx 09:27:39Z litz_ma $ 
#
# Version: $Revision: 3978 $
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################

import sys
from ctypes import *

%s# generated enums are placed before this text

class TiglException(Exception):
    ''' The exception encapsulates the error return code of the library and arguments that were provided for the function. '''
    def __init__(self, code, *args, **kwargs):
        Exception.__init__(self)
        self.code = code
        if "error" in kwargs:
            self.error = str(kwargs["error"])
        elif code in TiglReturnCode._names:
            self.error = TiglReturnCode._names[code]
        else:
            self.error = "UNDEFINED"
        self.args = tuple(args)
        self.kwargs = dict(kwargs)
    def __str__(self):
        return self.error + " (" + str(self.code) + ") " + str(list(self.args)) + " " + str(self.kwargs)

class Tigl(object):
    def __init__(self):
        ''' The constructor initializes the TIGL library '''
        self._handle = c_int(-1)
        
        if sys.platform == 'win32':
            self.TIGL = cdll.TIGL
        else:
            self.TIGL = CDLL("libTIGL.so")
        self.version = c_char_p()
        self.version.value = self.TIXI.tixiGetVersion()
        self.version = self.version.value
            
    def __del__(self):
        ''' The destructor cleans up the library '''
        if hasattr(self, "TIGL"):
            if self.TIGL != None:
                self.close()
                self.TIGL = None

    def _validateReturnValue(self, tixiReturn, *args, **kwargs):
        ''' Helper function to raise an exception if return value is not SUCCESS '''
        if tiglReturn != ReturnCode.TIGL_SUCCESS:
            raise TiglException(tiglReturn, args, kwargs)

    def open(self, tixi, configuration):
        ''' Open a document from a file. Tixi is from tixiwrapper and should have already opened a data set. '''
        if tixi._handle.value == -1:
            self._validateReturnValue(TiglReturnCode.TIGL_OPEN_FAILED) # raise a generic exception
        uuid = c_char_p(configuration)
        tiglReturn = self.TIGL.tiglOpenCPACSConfiguration(tixi._handle, uuid, byref(self._handle))
        if tiglReturn != ReturnCode.SUCCESS:
            self._handle.value = -1
        self._validateReturnValue(tiglReturn, configuration)
    
    def close(self):
        ''' Closes the current document.
            Test cases contained in other functions already.
        '''
        if self._handle.value != -1:
            tiglReturn = self.TIGL.tiglCloseCPACSConfiguration(self._handle)
            self._handle.value = -1
            self._validateReturnValue(tiglReturn)
    
%s# the method definitions are placed above