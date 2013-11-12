# -*-# coding: utf-8 -*-
"""
Created on Wed May 23 10:52:32 2013

@author: Martin Siggel <martin.siggel@dlr.de>
"""

import sys, os 

tiglpath = os.path.dirname(os.path.realpath(__file__)) + '/../..'
sys.path.append(tiglpath + '/bindings')
import bindings_generator.matlab_generator as MG
import bindings_generator.cheader_parser   as CP


blacklist = ['tiglLogToFileStream']

if __name__ == '__main__':
    # parse the file    
    parser = CP.CHeaderFileParser()
    
    # set the handle string that the parser can identify the handles
    parser.handle_str = 'TiglCPACSConfigurationHandle'
    parser.returncode_str  ='TiglReturnCode'
    parser.decoration = 'TIGL_COMMON_EXPORT'
    parser.add_alias('TixiDocumentHandle', 'int')
    parser.add_alias('FILE','void')
    parser.parse_header_file(tiglpath + '/src/tigl.h')
    
    # create the wrapper
    generator = MG.MatlabGenerator(parser, 'tigl', 'tigl.h')
    generator.blacklist = blacklist
    generator.create_wrapper()
    
    
