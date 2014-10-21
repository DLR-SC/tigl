# -*- coding: utf-8 -*-
"""
Created on Wed May 01 11:43:11 2013

@author: Martin Siggel <martin.siggel@dlr.de>
"""

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
    Generates Python code wrappers from the parsed semantics
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
        
    def create_error_handler(self, cparser):
        string = ''
        indent = 4*' '
        string += 'class %sException(Exception):\n' % self.classname
        string += '    \'\'\' The exception encapsulates the error return code of the library and arguments that were provided for the function. \'\'\'\n'
        string += '    def __init__(self, code, *args, **kwargs):\n'
        string += '        Exception.__init__(self)\n'
        string += '        self.code = code\n'
        string += '        if "error" in kwargs:\n'
        string += '            self.error = str(kwargs["error"])\n'
        string += '        elif code in %s._names:\n' % cparser.returncode_str
        string += '            self.error = %s._names[code]\n' % cparser.returncode_str
        string += '        else:\n'
        string += '            self.error = "UNDEFINED"\n'
        string += '        self.args = tuple(args)\n'
        string += '        self.kwargs = dict(kwargs)\n'
        string += '    def __str__(self):\n'
        string += '        return self.error + " (" + str(self.code) + ") " + str(list(self.args)) + " " + str(self.kwargs)\n'      
        string += '\n\n'
        
        if cparser.returncode_str in cparser.enums:
            successcode = '%s.%s' % (cparser.returncode_str, cparser.enums[cparser.returncode_str][0])
        else:
            successcode = '0'
        string += 'def catch_error(returncode, *args, **kwargs):\n'
        string += indent + 'if returncode != %s:\n' % successcode
        string += indent + '    raise %sException(returncode, args, kwargs)\n' \
            % self.classname        
        return string
                

    def create_constructor(self):
        indent = 4*' '
        string = ''
        string += indent + 'def __init__(self):\n'
        indent +=  4*' '
        string += indent + 'self.%s = ctypes.c_int(-1)\n\n' % self.handle_str
               
        string += indent + '# We only support python2.5 - 3.0 now\n'
        string += indent + 'if sys.version_info>(3,0,0):\n'
        string += indent + '    print("Python3 not supported in %sWrapper.")\n'\
            % self.prefix
        string += indent + '    sys.exit()\n'
        string += indent + 'elif sys.version_info<(2,5,0):\n'
        string += indent + '    print("At least python 2.5 is needed from %sWrapper.")\n\n' \
            % self.prefix
        
        string += indent + 'if sys.platform == \'win32\':\n'
        string += indent + '    self.lib = ctypes.cdll.%s\n' % self.libname
        string += indent + 'elif sys.platform == \'darwin\':\n'
        string += indent + '    self.lib = ctypes.CDLL("lib%s.dylib")\n' % self.libname
        string += indent + 'else:\n'
        string += indent + '    self.lib = ctypes.CDLL("lib%s.so")\n' % self.libname
        
        return string
        
    def create_destructor(self):
        if not self.closefunction or self.closefunction == '':
            return ' '
        
        indent = 4*' '
        string = indent + 'def __del__(self):\n'
        indent += indent
        string += indent + 'if hasattr(self, "lib"):\n'
        string += indent + '    if self.lib != None:\n'
        string += indent + '        self.%s()\n' % self.closefunction
        string += indent + '        self.lib = None\n'
        return string

    def write_enums(self, cparser, directory):
        for enumname, values in cparser.enums.iteritems():
            string = self.create_enum(enumname, values)
            filename = directory + '/' + enumname + '.java'
            print 'Write java enum %s to file "%s" ... ' % (enumname, filename), 
            fop = open(filename, 'w')
            fop.write(string)
            fop.close()
            print 'done'
        

    def create_enum(self, enumname, values):
        string = ""
        string += self.license
        string += "package " + self.package + ';\n\n'
        string += 'import java.util.ArrayList;\n\n'
        string += 'public enum %s {\n' % enumname
        indent = 4*' '
        for index, val in values.iteritems():
            string += indent + '%s(%d),\n' % (val, index)
            
        if len(values) > 0:
            string = string[:-2] + ';\n'
        
        string += '\n'
        
        string += indent + 'private static ArrayList<%s> codes = new ArrayList<>();\n\n' % enumname

        string += indent + 'static {\n'
        for index, val in values.iteritems():
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
    
    def create_header(self, fun_dec, indention_depth):
        '''Creates the method header as "def myFunc(self, myvalue):"'''
        
        string = ''
        indent = 4*indention_depth*' '
        
        # generate simplified method name, e.g.: tixiGetValue -> getValue
        name = fun_dec.method_name
        if name in self.aliases:
            name = self.aliases[name]
        else:
            if name.startswith(self.prefix):
                name = name[len(self.prefix)].lower() + name[len(self.prefix)+1:]
        
        # position of the handle variable
        handle_index = -1
        
        num_inargs  = 0
        num_outargs = 0
        
        # create method header
        string += indent + 'def %s(self' % name
        for index, arg in enumerate(fun_dec.arguments):
            if arg.is_handle and fun_dec.uses_handle:
                # we dont use the handle as an function argument
                handle_index = index
            elif not arg.is_outarg:
                string += ', %s' % arg.name
                num_inargs += 1
            elif arg.is_outarg:
                num_outargs += 1
                
        for index, arg in enumerate(fun_dec.arguments):
            # create aditional size argument for manually allocated arrays
            if arg.arrayinfos['is_array'] and arg.is_outarg and not arg.arrayinfos['autoalloc'] and len(arg.arrayinfos['arraysizes']) == 0:
                string += ', %s_len' % arg.name
            
        string += '):\n'
        return (string, num_inargs, num_outargs, handle_index)
    

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
            raise GeneratorException('Cannot create python to c conversion ' +
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