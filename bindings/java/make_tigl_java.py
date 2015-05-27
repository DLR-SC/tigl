# -*-# coding: utf-8 -*-
"""
Created on Wed May 01 11:44:49 2013

@author: Martin Siggel <martin.siggel@dlr.de>
"""

import sys, os 
from datetime import date

tiglpath = os.path.dirname(os.path.realpath(__file__)) + '/../..'
sys.path.append(tiglpath + '/bindings')
import bindings_generator.java_generator as PG
import bindings_generator.cheader_parser   as CP

apache = \
'''/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-10-21 Martin Siggel <martin.siggel@dlr.de>
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

'''
apache += \
'/* \n\
* This file is automatically created from tigl.h on %s.\n'\
    % date.today() + \
'* If you experience any bugs please contact the authors\n\
*/\n\n'


postconstr = '''
self.version = self.getVersion()
'''

blacklist = ['tiglLogToFileStreamEnabled']

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
    pg = PG.JavaGenerator(name_prefix = 'tigl', libname = 'TIGL', package = "de.dlr.sc.tigl")
    pg.license = apache
    pg.blacklist = blacklist
    pg.postconstr = postconstr

    
    print 'Creating python interface... ',
    wrapper = pg.create_native_interface(parser)
    print 'done'
    
    # write file
    directory = 'src/de/dlr/sc/tigl'
    filename = directory + '/TiglNativeInterface.java'
    print 'Write tigl java interface to file "%s" ... ' % filename, 
    fop = open(filename, 'w')
    fop.write(wrapper)
    print 'done'
    
    pg.write_enums(parser, directory)
    
    
