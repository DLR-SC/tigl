# -*- coding: utf-8 -*-
"""
Created on Wed Oct 21 10:31:30 2014

@author: Martin Siggel <martin.siggel@dlr.de>
"""

from __future__ import print_function

class GeneratorException(Exception):
    
    def __init__(self, errormsg, arg):
        msg = errormsg + '\n'
        msg += 'Argument was:\n'
        msg += '  type      : ' + arg.type + '\n'
        msg += '  name      : ' + arg.name + '\n'
        msg += '  is_outarg : ' + str(arg.is_outarg) + '\n'
        msg += '  is_const  : ' + str(arg.is_const) + '\n'
        msg += '  npointer  : ' + str(arg.npointer) + '\n'
        msg += '  is_string : ' + str(arg.is_string) + '\n'
        msg += '  arrayinfos: ' + str(arg.arrayinfos)+ '\n'
        msg += '  is_handle : ' + str(arg.is_handle) + '\n'
        self.value = msg
        
    def __str__(self):
        return repr(self.value)

        

class JavaGenerator(object):
    '''
    Generates Java code wrappers from the parsed semantics
    '''    
    
    def __init__(self, name_prefix, libname, package):
        self.prefix = name_prefix
        self.libname = libname
        self.classname = libname[0].upper() + libname[1:].lower() + 'NativeInterface'
        self.handle_str = '_handle'
        self.native_types = ['int', 'double', 'float', 'char', 'bool', 'size_t']
        self.license = None
        self.package = package
        self.userfunctions = None
        self.postconstr = None
        self.closefunction = None
        self.blacklist = []
        self.aliases = {}
            
    def add_alias(self, oldname, newname):
        self.aliases[oldname] = newname
            
    def create_native_interface(self, cparser):
        string = self.license
        string += "package " + self.package + ';\n\n'
        string += 'import com.sun.jna.*;\n'
        string += 'import com.sun.jna.ptr.*;\n\n\n'
        string += 'public class %s {\n\n' % self.classname 
        
        indent = 4*' '

        string += indent + 'static {\n'
        string += 2*indent +    'Native.register("%s");\n' % self.libname
        string += indent + '}\n\n'
        
        for dec in cparser.declarations:
            if dec.method_name not in self.blacklist:
                string += self.create_method_wrapper(dec) + '\n'        
        
        string += '};\n'
        
        return string
        

    def write_enums(self, cparser, directory):
        for enumname, values in cparser.enums.items():
            string = self.create_enum(enumname, values)
            filename = directory + '/' + enumname + '.java'
            print('Write java enum %s to file "%s" ... ' % (enumname, filename), end=' ') 
            fop = open(filename, 'w')
            fop.write(string)
            fop.close()
            print('done')
        

    def create_enum(self, enumname, values):
        string = ""
        string += self.license
        string += "package " + self.package + ';\n\n'
        string += 'import java.util.ArrayList;\n\n'
        string += 'public enum %s {\n' % enumname
        indent = 4*' '
        for index, val in values.items():
            string += indent + '%s(%d),\n' % (val, index)
            
        if len(values) > 0:
            string = string[:-2] + ';\n'
        
        string += '\n'
        
        string += indent + 'private static ArrayList<%s> codes = new ArrayList<>();\n\n' % enumname

        string += indent + 'static {\n'
        for index, val in values.items():
            string += 2*indent +    'codes.add(%s);\n' % val
        string += indent + '}\n\n'
        
        string += indent + 'private final int code;\n\n'
        
        string += indent + 'private %s(final int value) {\n' % enumname
        string += 2*indent +    ' code = value;\n'
        string += indent + '}\n\n'
        
        string += indent + 'public static %s getEnum(final int value) {\n' % enumname
        string += 2*indent +    'return codes.get(Integer.valueOf(value));\n'
        string += indent + '}\n\n'
        
        string += indent + 'public int getValue() {\n'
        string += 2*indent +   'return code;\n'
        string += indent + '}\n'
        
        string += '};'
        
        return string
    
    

    def get_type(self, arg, fun_dec):
        if arg.is_string and not arg.is_outarg:
            return "String"
        elif arg.is_string and arg.is_outarg:
            return "PointerByReference";
        elif arg.is_outarg and not arg.arrayinfos['is_array']:
            return arg.type[0].upper() + arg.type[1:].lower() + 'ByReference'
        elif arg.is_outarg and arg.arrayinfos['is_array'] and arg.npointer == 1 and not arg.arrayinfos['autoalloc']:
            return 'Pointer'
        elif not arg.is_outarg and arg.arrayinfos['is_array'] and arg.npointer == 1:
            return 'Pointer'
        elif not arg.arrayinfos['is_array']:
            return arg.type
        else:
            raise GeneratorException('Cannot create Java to c conversion ' +
                 'for output argument "%s" in %s' % (arg.name, fun_dec.method_name), arg )

    
    def create_method_wrapper(self, fun_dec):
        indent = 4*' '
        string = indent + 'public static native '
        
        # get return value type
        string += self.get_type(fun_dec.return_value, fun_dec) + ' '
        string += fun_dec.method_name + '('
        
        for arg in fun_dec.arguments:
            string += self.get_type(arg, fun_dec) + ' ' + arg.name + ', '
            
        if len(fun_dec.arguments) > 0:
            string = string[:-2]
            
        string += ');'
        
        return string