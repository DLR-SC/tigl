# -*- coding: utf-8 -*-
"""
Created on Wed Sep 16 13:57:10 2015

@author: Martin Siggel <martin.siggel@dlr.de>
@author: Melven Roehrig-Zoellner <Melven.Roehrig-Zoellner@DLR.de>
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
        return self.value


indent = 2*' '

class Fortran03Generator(object):
    '''
    Generates Fortran 2003 code wrappers from the parsed semantics
    '''

    def __init__(self):
        self.license = None
        self.userdeclarations = None
        self.userfunctions = None
        self.blacklist = []
        self.basic_types = {
                'int'    : 'integer(kind=C_INT)',
                'long'   : 'integer(kind=C_LONG)',
                'float'  : 'real(kind=C_FLOAT)',
                'double' : 'real(kind=C_DOUBLE)',
                'char'   : 'character(kind=C_CHAR,len=1)'}
        self.internal_suffix = '_c'
        self.helper_methods = {
'c_f_stringptr':
'''
subroutine c_f_stringptr(str_c, str_f)
  use, intrinsic :: iso_c_binding
  type(C_PTR), intent(in) :: str_c
  character(kind=C_CHAR), pointer, intent(inout) :: str_f(:)
  integer :: i

  if( .not. c_associated(str_c) ) return
  i = 1
  call c_f_pointer(str_c,str_f,(/i/))
  do while(str_f(i) .ne. C_NULL_CHAR)
    i = i + 1
    call c_f_pointer(str_c,str_f,(/i/))
  end do
  call c_f_pointer(str_c,str_f,(/i-1/))

end subroutine c_f_stringptr
''',
'f_c_strarrayptr':
'''
subroutine f_c_strarrayptr(strarray_f,tmp_str,tmp_ptr,strarray_c)
  use, intrinsic :: iso_c_binding
  character(kind=C_CHAR,len=*), intent(in) :: strarray_f(:)
  character(kind=C_CHAR), target, allocatable, intent(inout) :: tmp_str(:,:)
  type(C_PTR), target, allocatable, intent(inout) :: tmp_ptr(:)
  type(C_PTR), intent(inout) :: strarray_c
  integer :: i, j

  allocate(tmp_str(1+len(strarray_f),size(strarray_f)))
  allocate(tmp_ptr(size(strarray_f)))
  do i = 1, size(strarray_f), 1
    do j = 1, len(strarray_f), 1
      tmp_str(j,i) = strarray_f(i)(j:j)
    end do
    tmp_str(len(strarray_f)+1,i) = C_NULL_CHAR
    tmp_ptr(i) = c_loc( tmp_str(1,i) )
  end do

  strarray_c = c_loc(tmp_ptr)
end subroutine
''',
'c_f_strarrayptr':
'''
! WARNING this function is written such that it
!         circumvents triggering old GCC gfortran bugs
subroutine c_f_strarrayptr(n,strarray_c,strarray_f)
  use, intrinsic :: iso_c_binding
  integer, intent(in) :: n
  type(C_PTR), intent(in) :: strarray_c(n)
  type(CStringPtr), target, intent(inout) :: strarray_f(n)
  type(CStringPtr), pointer :: tmp => null()
  integer :: i

  do i = 1, n, 1
    tmp => strarray_f(i)
    call c_f_stringptr(strarray_c(i),tmp%str)
  end do
end subroutine
''',}
        self.helper_classes = '''
! we need a wrapper type to create arrays of C-strings as there is no pointer of a pointer in Fortran
type CStringPtr
  character(kind=C_CHAR), pointer :: str(:)
end type
'''


    def add_alias(self, oldname, newname):
        self.aliases[oldname] = newname
            

    def create_wrapper(self, cparser):
        string = ''
        if self.license:
            for line in self.license.splitlines():
                string += '! ' + line + '\n'
        
        # module declaration
        string += '\n\n'
        string += 'module tixi\n'
        string += indent + 'use, intrinsic :: iso_c_binding, only: C_CHAR\n'
        string += indent + 'implicit none\n'
        string += indent + 'public\n'

        # enum declarations
        string += '\n\n'
        for enumname, values in cparser.enums.iteritems():
            string += self.create_enum(enumname, values) + '\n\n'

        # additional declarations
        if self.userdeclarations:
            string += '\n\n'
            string += self.userdeclarations
            string += '\n\n'

        string += '\n\n'
        # start interface block
        string += 'interface\n'

        # add all declarations, some of them private (that require additional wrapper code)
        private_methods = list()
        for dec in cparser.declarations:
            method_name = dec.method_name
            if self.requires_method_wrapper(dec):
                method_name = method_name + self.internal_suffix
                private_methods.append(method_name)
            if not method_name in self.blacklist:
                try:
                    header, footer = self.create_method_declaration(method_name, dec, 'C')
                    string += header + footer + '\n\n'
                except GeneratorException as e:
                    e.value += 'Function was: %s\n' % dec.method_name
                    raise
            
        # end interface block
        string += 'end interface\n'

        # mark private methods
        string += '\n\n'
        for method_name in private_methods:
            string += indent + 'private :: %s\n' % method_name
        string += '\n'
        for method_name in self.helper_methods:
            string += indent + 'private :: %s\n' % method_name

        string += '\n'
        string += self.helper_classes

        string += '\n\n'
        string += 'contains\n'
        for method_code in self.helper_methods.values():
            for line in method_code.splitlines():
                string += indent + line + '\n'
        string += '\n\n'

        # functions for internal conversions
        if self.userfunctions:
            string += '\n\n'
            for line in self.userfunctions.splitlines():
                string += indent + line + '\n'
            string += '\n\n'
        
        # wrapper functions for more complicated cases
        for dec in cparser.declarations:
            if self.requires_method_wrapper(dec):
                method_name = dec.method_name
                if not method_name in self.blacklist:
                    string += '\n\n'
                    try:
                        string += self.create_method_wrapper(method_name, dec)
                    except GeneratorException as e:
                        e.value += 'Function was: %s\n' % dec.method_name
                        raise


        # end of module
        string += '\n\n'
        string += 'end module\n'

        return string
        

    def create_enum(self, enumname, values):
        '''
        Generates the Fortran 2003 equivalent of a C enum
        '''
        string = '! enum ' + enumname + '\n'
        string += 'enum, bind(C)\n'

        for index, val in values.iteritems():
            string += indent + 'enumerator :: %s = %d\n' % (val, index)
        string += 'end enum'
        
        return string
    
    
    def create_method_declaration(self, method_name, fun_dec, language_binding):
        '''
        Generates the Fortran 2003 interface of a c function
        '''
        
        header = ''
        # raw C code
        for line in fun_dec.raw_string.splitlines():
            header += '! ' + line + '\n'
        # raw annotation
        if fun_dec.raw_annotation:
            for line in fun_dec.raw_annotation.splitlines():
                header += '! ' + line + '\n'

        # begin subroutine/function block
        if fun_dec.return_value:
            header += indent + 'function %s'   % method_name
        else:
            header += indent + 'subroutine %s' % method_name
        # argument names
        header += '(' + (', &\n'+4*indent).join((arg.name for arg in fun_dec.arguments)) + ')'
        # return value name
        if fun_dec.return_value:
            header += ' &\n' + 3*indent + 'result(%s)' % fun_dec.return_value.name

        # bind C
        if language_binding == 'C':
            header += ' &\n' + 3*indent + 'bind(C,name=\'%s\')' % fun_dec.method_name
        header += '\n'

        # use iso_c_binding
        header += 2*indent + 'use, intrinsic :: iso_c_binding\n'

        # argument declarations of non-arrays (before arrays, as these may use non-arrays as dimensions)
        for arg in fun_dec.arguments:
            if not arg.arrayinfos['is_array']:
                header += 2*indent + self.create_argument_decl(arg, language_binding, False, fun_dec) + '\n'

        # argument declarations of non-arrays (before arrays, as these may use non-arrays as dimensions)
        for arg in fun_dec.arguments:
            if arg.arrayinfos['is_array']:
                header += 2*indent + self.create_argument_decl(arg, language_binding, False, fun_dec) + '\n'

        # return value declaration
        if fun_dec.return_value:
            header += 2*indent + self.create_argument_decl(fun_dec.return_value, language_binding, True, fun_dec) + '\n'

        # end subroutine/function block
        if fun_dec.return_value:
            footer = indent + 'end function %s\n'   % method_name
        else:
            footer = indent + 'end subroutine %s\n' % method_name

        return header, footer


    def requires_method_wrapper(self, fun_dec):
        '''
        Checks wether there are subroutine/function arguments (or return values) that require a wrapper function
        '''

        for arg in fun_dec.arguments:
            if arg.is_string:
                return True
            if arg.type == 'void' and arg.npointer > 0:
                return True

        if fun_dec.return_value:
            if fun_dec.return_value.is_string:
                return True
            if fun_dec.return_value.arrayinfos['is_array']:
                return True
            if fun_dec.return_value.type == 'void' and fun_dec.return_value.npointer > 0:
                return True

        return False


    def create_method_wrapper(self, method_name, fun_dec):
        '''
        Generates the Fortran 2003 wrapper code around a c function call
        '''
        
        string = ''
        header, footer = self.create_method_declaration(method_name, fun_dec, 'F')

        # generate conversion stuff
        var_alias = dict()
        declarations = list()
        pre_code = list()
        post_code = list()
        # return value
        if fun_dec.return_value:
            var, decl, pre, post = self.create_variable_wrapper(fun_dec.return_value, True, fun_dec)
            if var :
                var_alias[fun_dec.return_value.name] = var
                declarations.append(decl)
                pre_code.append(pre)
                post_code.append(post)
            else:
                var_alias[fun_dec.return_value.name] = fun_dec.return_value.name
        # arguments
        for arg in fun_dec.arguments:
            var, decl, pre, post = self.create_variable_wrapper(arg, False, fun_dec)
            if var :
                var_alias[arg.name] = var
                declarations.append(decl)
                pre_code.append(pre)
                post_code.append(post)
            else:
                var_alias[arg.name] = arg.name


        # declarations
        string += header
        for decl in declarations:
            if decl:
                string += decl + '\n'

        # pre conversion
        for pre in pre_code:
            if pre:
                string += pre + '\n'

        # call the C function
        string += '\n'
        if fun_dec.return_value:
            string += 2*indent + '%s = ' % var_alias[fun_dec.return_value.name]
        else:
            string += 2*indent + 'call '
        string += '%s(' % (method_name+self.internal_suffix)
        string += (', &\n'+4*indent).join((var_alias[arg.name] for arg in fun_dec.arguments)) + ')\n'
        string += '\n'

        # post conversion
        for post in post_code:
            if post:
                string += post + '\n'

        # end of block
        string += footer

        return string


    def create_variable_wrapper(self, arg_dec, function_result, fun_dec):
        '''
        Generates code for the conversion from C arguments to Fortran and vice vers
        '''

        # generate both C and F declarations
        C_decl = self.create_argument_decl(arg_dec, 'C', function_result, fun_dec)#, dummy_arg=True)
        F_decl = self.create_argument_decl(arg_dec, 'F', function_result, fun_dec)#, dummy_arg=True)

        # nothing to do, if they match
        if C_decl == F_decl:
            return None, None, None, None
        
        # ignore if they only differ by value / intent(in)
        if C_decl == F_decl.replace('intent(in)','value'):
            return None, None, None, None

        # so we need a C declaration with a different name
        var = arg_dec.name + self.internal_suffix
        try:
            decl = 2*indent + self.create_argument_decl(arg_dec, 'C',
                                                        function_result, 
                                                        fun_dec,
                                                        arg_name=var, 
                                                        dummy_arg=False) + '\n'
        except:
            decl = None
        # handle strings
        if arg_dec.is_string:
            if function_result or arg_dec.is_outarg:
                if arg_dec.arrayinfos['is_array']:
                    pre = 2*indent + 'allocate(%s(size(%s)))\n' % (var, arg_dec.name)
                    pre += 2*indent + '%s = C_NULL_PTR\n' % var
                    post = 2*indent + 'call c_f_strarrayptr(size(%s),%s, %s)\n' % (arg_dec.name, var, arg_dec.name)
                else:
                    pre = None
                    post = 2*indent + 'call c_f_stringptr(%s, %s)\n' % (var, arg_dec.name)
            else:
                if arg_dec.arrayinfos['is_array']:
                    var_tmp1 = var+'tmp1'
                    decl += 2*indent + 'character(kind=C_CHAR), target, allocatable :: %s(:,:)\n' % var_tmp1
                    var_tmp2 = var+'tmp2'
                    decl += 2*indent + 'type(C_PTR), target, allocatable :: %s(:)\n' % var_tmp2
                    pre = 2*indent + 'call f_c_strarrayptr(%s, %s, %s, %s)\n' % (
                            arg_dec.name, var_tmp1, var_tmp2, var)
                else:
                    pre = None
                    var = arg_dec.name + ' // C_NULL_CHAR'
                    decl = None
                post = None
        elif arg_dec.type in self.basic_types:
            if not arg_dec.is_outarg:
                raise NotImplementedError;
            pre = None
            dim_name = fun_dec.arguments[arg_dec.arrayinfos['arraysizes'][0]].name
            post = 2*indent + 'call c_f_pointer(%s, %s, (/%s/))\n' % (var, arg_dec.name, dim_name)
        else:
            raise GeneratorException('Unhandled argument type', arg_dec)

        return var, decl, pre, post


    def create_argument_decl(self, arg_dec, language_binding, function_result, fun_dec, arg_name=None, dummy_arg=True):
        '''
        Generates the Fortran 2003 declaration for an argument of a function/subroutine call
        '''

        if not language_binding in ('C','F'):
            raise ValueError('language_binding must be "C" or "F"')
        if not arg_name:
            arg_name = arg_dec.name
        if function_result:
            dummy_arg = False

        string = ''
        if arg_dec.is_string:
            if arg_dec.arrayinfos['is_array']:
                if arg_dec.npointer > 2:
                    raise GeneratorException('Unhandled argument type', arg_dec)
                dim_name = None
                if arg_dec.arrayinfos['arraysizes']:
                    if len(arg_dec.arrayinfos['arraysizes']) > 1:
                        raise GeneratorException('Unhandled argument type', arg_dec)
                    if arg_dec.arrayinfos['arraysizes']:
                        dim_name = fun_dec.arguments[arg_dec.arrayinfos['arraysizes']].name

                if arg_dec.is_outarg:
                    if arg_dec.arrayinfos['autoalloc']:
                        raise GeneratorException('Unhandled argument type', arg_dec)
                    if language_binding == 'C':
                        if dummy_arg:
                            string = 'type(C_PTR), intent(out) :: %s(%s)' % (
                                    arg_name, dim_name if dim_name else '*' )
                        else:
                            string = 'type(C_PTR), allocatable :: %s(:)' % arg_name
                    else: #language_binding == 'C':
                        if dummy_arg:
                            string = 'type(CStringPtr), intent(out) :: %s(%s)' % (
                                    arg_name, dim_name if dim_name else ':')
                        else:
                            raise RuntimeError('Shouldnt trigger this situation')
                else:
                    if language_binding == 'C':
                        if dummy_arg:
                            string = 'type(C_PTR), value :: ' + arg_name
                        else:
                            string = 'type(C_PTR) :: ' + arg_name
                    else: #language_binding == 'F':
                        if dummy_arg:
                            string = 'character(kind=C_CHAR,len=*), intent(in) :: %s(%s)' % (
                                    arg_name, dim_name if dim_name else ':')
                        else:
                            raise RuntimeError('Shouldnt trigger this situation')

            else:
                if language_binding == 'C':
                    if function_result:
                        string = 'type(C_PTR) :: %s' % arg_name
                    elif dummy_arg:
                        if arg_dec.is_outarg:
                            string = 'type(C_PTR), intent(inout) :: %s' % arg_name
                        else:
                            string = 'character(kind=C_CHAR), intent(in) :: %s(*)' % arg_name
                    else:
                        string = 'type(C_PTR) :: %s = C_NULL_PTR' % arg_name
                else: #language_binding == 'F'
                    if function_result or arg_dec.is_outarg:
                        string = 'character(kind=C_CHAR), pointer :: %s(:)' % arg_name
                    elif dummy_arg:
                        string = 'character(kind=C_CHAR,len=*), intent(in) :: %s' % arg_name
                    else:
                        raise RuntimeError('Shouldnt trigger this situation')

        elif arg_dec.type in self.basic_types:
            if arg_dec.arrayinfos['is_array']:
                if arg_dec.npointer > 2:
                    raise GeneratorException('Unhandled argument type', arg_dec)
                if arg_dec.npointer == 2 and not arg_dec.is_outarg:
                    raise GeneratorException('Unhandled argument type', arg_dec)

                if arg_dec.npointer > 1:
                    if language_binding == 'C':
                        string = 'type(C_PTR)'
                    else:
                        string = self.basic_types[arg_dec.type] + ', pointer'
                        #if arg_dec.arrayinfos['autoalloc']:
                        #    string += ', pointer'
                else:
                    string = self.basic_types[arg_dec.type]

                if dummy_arg:
                    if arg_dec.is_const:
                        string += ', intent(in)'
                    elif arg_dec.is_outarg:
                        string += ', intent(out)'
                string += ' :: ' + arg_name

                if not arg_dec.arrayinfos['arraysizes']:
                    if not dummy_arg:
                        raise GeneratorException('Unhandled argument type', arg_dec)
                    #if language_binding == 'C':
                    string += '(*)'
                    #else:
                    #    string += '(:)'
                elif arg_dec.npointer == 1:
                    string += '(%s)' %','.join((fun_dec.arguments[i].name for i in reversed(arg_dec.arrayinfos['arraysizes'])))
                elif language_binding == 'F':
                    string += '(:)'

            else:
                string = self.basic_types[arg_dec.type]
                if dummy_arg:
                    if arg_dec.is_outarg:
                        string += ', intent(out)'
                    elif arg_dec.npointer > 0:
                        string += ', intent(inout)'
                    else:
                        string += ', value' if language_binding == 'C' else ', intent(in)'
                string += ' :: ' + arg_name

        elif arg_dec.type == 'void' and arg_dec.npointer > 0 and language_binding == 'C':
            string = 'type(C_PTR)'
            if dummy_arg:
                if arg_dec.is_outarg:
                    string += ', intent(out)'
                elif arg_dec.npointer > 1:
                    string += ', intent(inout)'
                else:
                    string += ', value'
            string += ' :: ' + arg_name
        else:
            raise GeneratorException('Unhandled argument type', arg_dec)


        return string
