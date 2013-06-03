# -*- coding: utf-8 -*-
"""
Created on Sat Apr 27 22:38:05 2013

@author: Martin Siggel <martin.siggel@dlr.de>
"""

import re
import copy

class CHeaderFileParser(object):
    '''
    Parses C-Header file code and creates a semantic
    This generated semantic can passed to wrapper generator
    classes in order to create bindings for the code
    '''

    def __init__(self):
        self.declarations = []
        
        # blacklist of methods not to be parsed
        self.blacklist = ()
        
        self.lines = []
        self.decoration = 'DLL_EXPORT'
        self.enums = {}
        self.typedefs = {}
        self.handle_str = None
        self.returncode_str = None
    
    def parse_header_file(self, filename):
        self.read_header(filename)
        self.parse_defines()
        self.parse_methods()
        print 'Parsed %d functions, %d enums, and %d typedefs'\
            % (len(self.declarations), len(self.enums), len(self.typedefs))
    
    def read_header(self, filename):
        fop = open( filename, 'r')
        self.lines = fop.readlines()
        
        # remove // style comments
        for index, line in enumerate(self.lines):
            pos = line.find('//')
            if pos >= 0:
                self.lines[index] = line[:pos]
    
    def parse_enum_string(self, enumstr):
        # simplify
        thestr = copy.copy(enumstr).strip()
        thestr = thestr.replace('\n',' ')

        # remove comments
        match = re.compile(r'/\*(?P<comment>.*?)\*/')  
        #comments = match.findall(thestr)   
        thestr = match.sub('', thestr)
        
        # remove multpiple whitespaces
        thestr = re.compile(r'\s+').sub(' ', thestr)
        
        #extract name and value
        match = re.compile(r'enum\s+(?P<name>\w+)\s*?{(?P<values>.*?)}')
        res = match.search(thestr)
        
        values = {}
        names = res.group('values').split(',')
        for index, val in enumerate(names):
            name = val.strip()
            match = re.search(r'(?P<id>.+)=(?P<value>.+)', name)
            if match:
                value = int(match.group('value').strip())
                values[value] = match.group('id').strip()
            else:
                values[index] = name
        
        
        print 'Parsed enum %s' % res.group('name')
            
        return {'name': res.group('name'), 'values': values}
    
    def parse_methods(self):
        '''
        Parse methods and their annotations
        '''
        dec_pattern = re.compile(self.decoration + 
            r'\s+(?P<fundec>.*?\([\w\s,*]*(\);)?)')
            
        anno_pattern = re.compile('#annotate.*?#')
        
        endterm = ');'        
        
        fundec_dict = []
        anno_dict = []
        
        # find functions
        continuing = False
        fundec = ''
        for index, line in enumerate(self.lines):
            if not continuing:
                match = dec_pattern.match(line.strip())
                if match:
                    fundec = match.group('fundec').strip()
                    if not endterm in line:
                        continuing = True
                    else:
                        fundec_dict.append({'line': index, 'declaration': fundec})
            else:
                fundec += ' ' + line.strip()
                if endterm in line:
                    continuing = False
                    fundec_dict.append({'line': index, 'declaration': fundec})
        
        for index, line in enumerate(self.lines):
            match = anno_pattern.search(line)
            if match:
                anno_dict.append({'line': index, 'annotation': match.group()})
        
        # match annotation with functions
        anno_pos = 0
        for function in fundec_dict:
            dec = CFunctionDec()
            dec.parse_method_header(function['declaration'], self.typedefs, self.enums, self.handle_str, self.returncode_str)
            if anno_pos < len(anno_dict) and anno_dict[anno_pos]['line'] < function['line']:
                anno = Annotation(anno_dict[anno_pos]['annotation'])
                dec.apply_annotation(anno)
                anno_pos += 1
                
            self.declarations.append(dec)
            dec.deduce_arg_roles()    
    
    def add_alias(self, newname, basictype):
        self.typedefs[newname] = basictype
    
    
    def parse_defines(self):
        '''
        Parse typedefs, enums...
        '''
        
        enum_pattern = re.compile(r'\s*enum\s+(?P<name>\w+)')
        continuing = False
        enum_str = ''
        for line in self.lines:
            if not continuing:
                match = enum_pattern.match(line)
                if match:
                    continuing = True
                    enum_str = line
                    if "};" in line:
                        enum = self.parse_enum_string(enum_str)
                        self.enums[enum['name']] = enum['values']
            else:
                enum_str += line
                if "};" in line:
                    enum = self.parse_enum_string(enum_str)
                    self.enums[enum['name']] = enum['values']
                    continuing = False
                    
        # parse typedefs
        typedef_pattern = re.compile('typedef\s+(?P<type>[\w\s*]+)\s+(?P<name>\w+)\s*?;')
        for line in self.lines:
            match = typedef_pattern.search(line)
            if match:
                self.typedefs[match.group('name')] =  match.group('type')
        

class Annotation(object):
    """Helper class to parse the function annotation if present"""
    
    regex = r'(?P<index>\d)((?P<array>A)(?P<alloc>M)?(\((?P<indexlist>[\d,\s]+)\))?)?'
    
    def __init__(self, string = None):
        self.inargs = {}
        self.outargs = {}
        self.uses_handle = True
        self.returns_error = True
        
        if string:
            self.parse_string(string)

    def parse_string(self, string):
        """
        Parses an annotion string for input and output arguments        
        #annotate in: 1,2 out: 3A(4), 5A(M) nohandle returns: error|value
        
        the number in the annotation specifies the index of an argument 
        (counting from 0).
        An "A" states, that the argument is an array
        Brackets after an Array like 4A(1,2) mean, that the size of an
        array is determinind by the product of the given arguments values.
        In this case the array4 had a size arg1.value*arg2.value.
        An M means, that the array is not allocated inside the wrapped function,
        but has to be preallocated. The normally requires an additional argument
        stating the size of the array. 
        """

        #search output args
        self.parse_param_group('out', string, self.outargs)

        #search input args
        self.parse_param_group('in', string, self.inargs)

        #search if to use handle
        res = re.search(r'\bnohandle\b|\bhandle\b', string)
        if res:
            self.uses_handle = res.group() != 'nohandle'
            
        #search if function returns status error (or value)
        res = re.search(r'\bnoerror\b', string)
        if res:
            self.returns_error = res.group() != 'noerror'
        else:
            self.returns_error = True
            
        #check correctness
        for inarg in self.inargs:
            if inarg in self.outargs:
                raise Exception('Input argument can not be an output ' +
                                'argument at the same time')

    @staticmethod
    def parse_param_group(inout, text, params):
        """
        parses the index values of each in and output argument group,
        i.e. it tries to decode something like: 1, 4A(5)
        """
        if not inout in text:
            return

        # find the appropriate section in the annotation string
        res = re.search(inout + r':\s((' + Annotation.regex + 
                        r'|(,[\s]*))+)($|\s|#)', text)
        if not res:
            raise Exception('Cannot understand annotation "%s"' % text)
        
        ins = res.group()
        # parse each input parameter
        while ins:
            # find paramter
            tmp = re.search(Annotation.regex, ins)
            if not tmp:
                break
            
            arg_index =  int(tmp.group('index'))
            # parse array size arguments
            indexlist = []
            if tmp.group('indexlist'):
                tmpstr = tmp.group('indexlist')
                indexlist = [int(val) for val in tmpstr.split(',')]
            
            params[arg_index]  = {'isarray':  tmp.group('array') is 'A', 
                                  'arraysizes': indexlist,
                                  'autoalloc': tmp.group('alloc') is None,
                                  'index': arg_index}
            
            ins = ins.replace(tmp.group(),'',1)

class CFunctionArg(object):
    '''
    Stores the type, name, number of pointers
    and constness of a function argument
    '''    
    
    def __init__(self):
        self.type = ''
        self.rawtype = ''
        self.npointer = 0
        self.raw_npointer = 0
        self.name = ''
        self.is_const   = False
        self.is_handle  = False
        self.is_string  = False
        self.arrayinfos = {'is_array': False, 'arraysizes': None, 'autoalloc': True}
        self.is_outarg  = False
        self.is_annotated = False
        self.is_sizearg   = False
        # if arg is sizearg, store argument that is referenced by the sizearg
        self.size_ref     = 0
        
        
    def parse_arg(self, string, typedeflist, enumlist, handle_str):
        ''' Parse the function argument.
            param: type definition from the signature
            Attributes:
              .name = name of the parameter in the signature
              .type = type of the parameter in the signature
              .is_const = True if there was a const
              .npointer = number of "*" stars
              
             This is the place, if you want to add some automagical
             detection features
        '''

        regex = r'((?P<const>const)[\s]+)?(?P<type>[\w]+)[\s]*' + \
                r'(?P<pointer>[\*]*)[\s]*(?P<name>[\w]+)?[\s]*'
        res = re.search(regex, string)
        
        cst = res.group('const') + ' ' if res.group('const') else ''
        pnt = res.group('pointer') if res.group('pointer') else ''
        self.name = res.group('name')
        self.rawtype   = res.group('type')
        self.raw_npointer  = len(pnt)         
        
        thetype = cst + res.group('type') + pnt
        
        newstring = self.resolv_type(thetype, enums = enumlist, typedefs = typedeflist)
        
        regex = r'((?P<const>const)[\s]+)?(?P<type>[\w]+)[\s]*' + \
                r'(?P<pointer>[\*]*)'
        res = re.search(regex, newstring)
        
        self.is_const  = res.group('const') == 'const'
        self.npointer  = len(res.group('pointer'))  
        self.is_handle = self.rawtype == handle_str
        self.type = res.group('type')        
        self.is_string = self.type == 'char' and self.npointer > 0
        
        # reserved keywords and types that are not allowed as names
        keywords = ['const', 'while', 'for']
        
        # resolve type
        #self.type = self.resolv_type(self.rawtype, enums = enumlist, typedefs = typedeflist)

        
        if self.name in keywords:
            raise Exception('"%s" is an invalid argument name' % (self.name))
            
       
    def resolv_type(self, mytype, enums, typedefs):
        '''
        Tries to deduce the type of a arguments, in particular
        if the type is not native (typedef, enum)        
        '''        
        
        regex = r'(?P<const>const\s)?(?P<name>[\w\s]+)(?P<pointer>[*]+)?'
        
        basictypes    = ['int', 'long', 'float', 'double', 'char', 'void']

        match = re.search(regex, mytype)
        name = match.group('name')
        ptr = match.group('pointer') if match.group('pointer') else ''
        cst = match.group('const') if match.group('const') else ''
        
        if match and name in basictypes:
            return mytype
        elif match and name.startswith('enum'):
            return 'int'
        elif match and name in typedefs:
            return cst + self.resolv_type(typedefs[name], enums, typedefs) + ptr
        else:
            raise Exception('"%s" is not a valid type name' % (name))

             
class CFunctionDec(object):
    def __init__(self):
        self.arguments = []
        self.return_value = None
        self.returns_error = True
        self.method_name = ''
        self.uses_handle = True
        
    def parse_method_header(self, string, typedeflist = None, enumlist = None,  handle_str = None, returncode_str = None):
        '''
        Parses a typical function header like
            int getMyValue(double * p, int index)
            
        Scans for function name, arguments and return value
        '''        
        
        regex = r'(?P<retval>(const )?[\w*]+([\s]+[\*]+)?)[\s]+(?P<name>[\w]+)' + \
                r'[\s]*\((?P<args>[\w\s,*\[\]]*)\)'
        res = re.search(regex, string)
        if not res:
            raise Exception('Cannot parse function declaration "%s"' % string)
        
        self.return_value = CFunctionArg()
        self.return_value.parse_arg(res.group('retval'), typedeflist, enumlist, handle_str)
        self.return_value.name = 'ret'
        self.returns_error =  returncode_str == self.return_value.rawtype
        self.method_name  = res.group('name') 
        
        print 'Parsed function %s' % self.method_name    
        
        arg_string = res.group('args')
        if not arg_string:
            return
        
        arg_str_list = arg_string.split(',')
        for index, arg_str in enumerate(arg_str_list):
            arg = CFunctionArg()
            arg.parse_arg(arg_str, typedeflist, enumlist, handle_str)
                    
            if not arg.name:
                arg.name = 'arg%d' % index
            
            self.arguments.append(arg)
            
    def deduce_arg_roles(self):
        ''' should be called after applying annotations'''
        not_annotated_args = (arg for arg in self.arguments if not arg.is_annotated)
        for index, arg in enumerate(not_annotated_args):
            # apply some default behaviour
               
            if arg.is_const:
                # can not be an output argument
                if arg.is_string and arg.npointer == 2:
                    arg.arrayinfos['is_array'] = True
                elif not arg.is_string and arg.npointer == 1:
                    arg.arrayinfos['is_array'] = True
                arg.is_outarg = False
        
            elif arg.npointer == 1 and not arg.is_string:
                # arg is assumed to output variable
                arg.is_outarg = True
            elif arg.npointer == 1 and arg.is_string:
                # normal input string
                pass
            elif arg.npointer == 2 and arg.is_string:
                # outpur string, malloced by lib
                arg.is_outarg = True
            elif arg.npointer == 2 and not arg.is_string:
                arg.is_outarg = True
                arg.arrayinfos['is_array'] = True
                if arg.arraysinfos['arraysizes'] is None:
                    raise Exception('Cannot continue without annotation for argument "%s" in "%s"' % (arg.name, self.method_name))
            
        
    def apply_annotation(self, annotation):
        '''
        Uses the information parsed from the annotation to identify
        the roles of the arguments (in, outs, arrays, arraysizes...)
        '''
        
        if annotation.returns_error:
            self.returns_error = True
        else:
            self.returns_error = False
        
        # apply outpt args
        for index, outarg in annotation.outargs.iteritems():
            if index >= len(self.arguments):
                raise Exception('annotation index is too large for function %s'\
                    % self.method_name)
                
            self.arguments[index].is_outarg = True
            self.arguments[index].arrayinfos['is_array']  = outarg['isarray']
            self.arguments[index].arrayinfos['autoalloc'] = outarg['autoalloc']
            self.arguments[index].is_annotated = True
            if outarg['isarray']:
                self.arguments[index].arrayinfos['arraysizes'] = outarg['arraysizes']
                if not outarg['autoalloc']:
                    continue
                
                for sizeindex in outarg['arraysizes']:
                    self.arguments[sizeindex].is_sizearg = True
                    self.arguments[sizeindex].size_ref = index
            
        # apply input args
        for index, inarg in annotation.inargs.iteritems():
            if index >= len(self.arguments):
                raise Exception('annotation index is too large for function %s'\
                    % self.method_name)
                
            self.arguments[index].is_outarg = False
            self.arguments[index].arrayinfos['is_array']  = inarg['isarray']
            self.arguments[index].arrayinfos['autoalloc'] = inarg['autoalloc']
            self.arguments[index].is_annotated = True
            if inarg['isarray']:
                self.arguments[index].arrayinfos['arraysizes'] = inarg['arraysizes']
                if not inarg['autoalloc']:
                    continue
                
                for sizeindex in inarg['arraysizes']:
                    self.arguments[sizeindex].is_sizearg = True
                    self.arguments[sizeindex].size_ref = index
            
            
        self.uses_handle = annotation.uses_handle
        if not self.uses_handle and len(self.arguments) > 0:
            self.arguments[0].is_handle = False
            
if __name__ == '__main__':
    arg = CFunctionArg()

    typedefs = {'MyString': 'MyString3', 'MyString3': 'char*', 'ErrorCode': 'enum ErrorCode'}
    enums = ['TYPE','ERRORCODE']    
    
    assert(arg.resolv_type('const char*',[],[]) == 'const char*')
    assert(arg.resolv_type('MyString', [], typedefs) == 'char*')
    assert(arg.resolv_type('const MyString*', [], typedefs) == 'const char**')
    
    assert(arg.resolv_type('ErrorCode', enums, typedefs) == 'int')
    
    #arg.resolv_type('const MyString2*', [], typedefs)
    
    
    