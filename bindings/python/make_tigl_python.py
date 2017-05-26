# -*-# coding: utf-8 -*-
"""
Created on Wed May 01 11:44:49 2013

@author: Martin Siggel <martin.siggel@dlr.de>
"""

from __future__ import print_function
import sys, os 
from datetime import date

tiglpath = os.path.dirname(os.path.realpath(__file__)) + '/../..'
sys.path.append(tiglpath + '/bindings')
import bindings_generator.python_generator as PG
import bindings_generator.cheader_parser   as CP

apache = \
'''#############################################################################
# Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
#
# Created: 2013-05-01 Martin Siggel <Martin.Siggel@dlr.de>
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

'''
apache += \
'# This file is automatically created from tigl.h on %s.\n'\
    % date.today() + \
'# If you experience any bugs please contact the authors\n\n'

userfunctions = \
'''
def open(self, tixi, uid):
    if tixi._handle.value < 0:
        catch_error(TiglReturnCode.TIGL_OPEN_FAILED, 'open')
    self.openCPACSConfiguration(tixi._handle.value, uid)

def close(self):
    if self._handle.value >= 0:
        # call to native function
        errorCode = self.lib.tiglCloseCPACSConfiguration(self._handle)
        catch_error(errorCode, 'tiglCloseCPACSConfiguration')
        self._handle.value = -1

'''

postconstr = '''
self.version = self.getVersion()
'''

blacklist = ['tiglCloseCPACSConfiguration', 'tiglLogToFileStreamEnabled']

if __name__ == '__main__':
    # parse the file
    
    parser = CP.CHeaderFileParser()
    parser.decoration = 'TIGL_COMMON_EXPORT'
    parser.add_alias('TixiDocumentHandle','int')
    parser.add_alias('FILE','void')
    
    # set the handle string that the parser can identify the handles
    parser.handle_str = 'TiglCPACSConfigurationHandle'
    parser.returncode_str  ='TiglReturnCode'
    parser.parse_header_file(tiglpath + '/src/api/tigl.h')
    
    # create the wrapper
    pg = PG.PythonGenerator(name_prefix = 'tigl', libname = 'tigl3')
    pg.license = apache
    pg.userfunctions = userfunctions
    pg.blacklist = blacklist
    pg.postconstr = postconstr
    #pg.add_alias('tiglCloseCPACSConfiguration','close')
    pg.closefunction = 'close'
    
    print('Creating python interface... ', end=' ')
    wrapper = pg.create_wrapper(parser)
    print('done')
    
    # write file
    filename = 'tigl3wrapper.py'
    print('Write tigl python interface to file "%s" ... ' % filename, end=' ') 
    fop = open(filename, 'w')
    fop.write(wrapper)
    print('done')
    
    
