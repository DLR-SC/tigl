# -*- coding: utf-8 -*-
"""
Created on Sat May  4 19:28:45 2013

@author: Martin Sigel <martin.siggel@dlr.de>
"""

import os
from datetime import date

class MatlabGenerator(object):
    
    def __init__(self, cparser, prefix, includefile):
        self.cparser = cparser
        self.prefix  = prefix
        self.libinclude = includefile
        self.blacklist  = []
        self.mex_body = '(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])'
        
    def create_m_file(self, fun_dec):
        '''creates an matlab .m file for the function fun_dec'''
        
        name = fun_dec.method_name
        filename = name + '.m'

        # if file already exist, don't overwrite - it might be handwritten
        if os.path.isfile(filename):
            return
        
        fop = open(filename, 'w')
        
        string = 'function '
        # get number of outargs
        outargs = [arg for arg in fun_dec.arguments if arg.is_outarg]
        inargs = [arg for arg in fun_dec.arguments if not arg.is_outarg]
        inargs = [arg for arg in inargs if not arg.is_sizearg or fun_dec.arguments[arg.size_ref].is_outarg]
        
        if not fun_dec.returns_error and fun_dec.return_value.type != 'void':
            outargs.insert(0, fun_dec.return_value)
        
        outarrays = [arg for arg in fun_dec.arguments if arg.is_outarg and arg.arrayinfos['is_array']]
        outstr = ''
        if len(outargs) > 1:
            outstr += '['
            for arg in outargs:
                outstr += arg.name + ', '
            outstr = outstr[0:-2] + '] = '
        elif len(outargs) == 1:
            outstr += outargs[0].name + ' = '
            
        string += outstr + fun_dec.method_name + '('
        
        instr = ''

        for arg in inargs:
            instr += ', ' + arg.name
            
        count = 0
        for arg in outarrays:
            if len(arg.arrayinfos['arraysizes']) == 0:
                instr += ', _' + arg.name + '_size'
                count += 1
        
        if len(instr) > 0:
            string += instr[2:]
            
        string += ')\n'
        
        indent = ' '*4
        string += indent \
                + '%% this file was automatically generated from %s on %s\n\n'\
                % (self.libinclude, date.today())
        
        # make some checks
        for arg in inargs:
            myfun = 'not(ischar' if arg.is_string else '(ischar'
            string += indent + 'if %s(%s))\n' % (myfun, arg.name)
            string += 2*indent + 'error(\'Invalid type of argument "%s"\');\n'\
                    % arg.name
            string += indent + 'end\n\n'
                
        # call the mex interface
        string += indent + 'try\n'
        string += 2*indent + outstr + self.prefix + '_matlab(\'%s\'%s);\n' \
                % (name, instr)
        indent = ' '*4
        string += indent + 'catch err\n'
        string += 2*indent + 'error(err.message)\n'
        string += indent + 'end\n'
        string += 'end\n\n'
        
        fop.write(string)
        fop.close()
    
    def convert_c_to_mx(self, arg, index):
        string = ''
        if not arg.arrayinfos['is_array']:
            if arg.is_string and arg.npointer <= 2:
                string = 'plhs[%d] = mxCreateString(%s);\n' % (index, arg.name)
            elif arg.type == 'double' and arg.npointer <= 1:
                string = 'plhs[%d] = mxCreateDoubleMatrix(1, 1, mxREAL);\n' \
                    % index
                string +='*mxGetPr(plhs[%d]) = %s;\n' % (index, arg.name)
            elif (arg.type == 'int' or arg.type == 'size_t') and arg.npointer == 1:
                string = 'plhs[%d] = mxCreateDoubleMatrix(1, 1, mxREAL);\n' \
                    % index
                string +='*mxGetPr(plhs[%d]) = (double)%s;\n' % (index, arg.name)
            else:
                #pass
                print arg.name, arg.type
                raise Exception('Conversion from "%s, %s" to mx not yet implemented' \
                    % (arg.type + '*'*arg.npointer, arg.name))    
        else:
            if arg.type == 'double':
                string += 'plhs[%d] = dArrayToMx(%s, _%s_size);\n' % (index, arg.name, arg.name)
            elif arg.type == 'int':
                string += 'plhs[%d] = iArrayToMx(%s, _%s_size);\n' % (index, arg.name, arg.name) 
            elif arg.is_string:
                string += 'plhs[%d] = mxCreateCharMatrixFromStrings(_%s_size, %s);\n' % (index, arg.name, arg.name)
            else:
                #pass
                raise Exception('Conversion from "%s, %s" to mx not yet implemented' \
                    % (arg.type + '*'*arg.npointer, arg.name))
                
        return string
        
    
    def create_mex_inarraysizes(self, func):
        string = ''
        inargs = [arg for arg in func.arguments if not arg.is_outarg]
        inargs_sizes = [arg for arg in inargs if arg.is_sizearg and not func.arguments[arg.size_ref].is_outarg]
        
        for arg in inargs_sizes:
            string += 4*' ' + arg.name + ' = mxGetArraySize(prhs[%d]);\n' \
                % (arg.size_ref + 1)
                
        return string
        
    def create_mex_array_malloced_sizes(self, func):
        string = ''
        outarrays = [arg for arg in func.arguments if arg.is_outarg and \
            arg.arrayinfos['is_array'] and not arg.arrayinfos['autoalloc']]
        for arg in outarrays:   
            if len(arg.arrayinfos['arraysizes']) > 0:
                sizestr = '_%s_size = 1' % arg.name
                for sizeindex in arg.arrayinfos['arraysizes']:
                    sizestr += ' * ' + func.arguments[sizeindex].name
                sizestr += ';\n'
                string += 4*' ' + sizestr
                
        return string
        
        
    def create_mex_array_auto_sizes(self, func):
        string = ''
        outarrays = [arg for arg in func.arguments if arg.is_outarg and \
            arg.arrayinfos['is_array'] and arg.arrayinfos['autoalloc']]
        # calc output array sizes
        for arg in outarrays:
            if len(arg.arrayinfos['arraysizes']) > 0:
                sizestr = '_%s_size = 1' % arg.name
                for sizeindex in arg.arrayinfos['arraysizes']:
                    sizestr += ' * ' + func.arguments[sizeindex].name
                sizestr += ';\n'
                string += 4*' ' + sizestr
                
        return string
        
        
    def create_mex_allocs(self, func):
        string = ''
        outarrays = [arg for arg in func.arguments if arg.is_outarg and \
            arg.arrayinfos['is_array'] and not arg.arrayinfos['autoalloc']]
        for arg in outarrays:
            string += 4*' ' + '%s = mxMalloc(sizeof(%s) * %s);\n' \
                % (arg.name, arg.type + (arg.npointer-1)*'*', '_' + arg.name+'_size')
                
        return string
        
    
    def create_mex_function(self, func):
        string = 'void mex_%s%s\n{\n' % (func.method_name, self.mex_body)
        #declare variables
        inargs = [arg for arg in func.arguments if not arg.is_outarg]
        inargs_wo_sizes = [arg for arg in inargs if not arg.is_sizearg or func.arguments[arg.size_ref].is_outarg]
        if len(inargs) > 0:
            string += 4*' ' + '/* input arguments */\n'
            for arg in inargs:
                string += 4*' ' + arg.rawtype + '*'*arg.npointer + ' ' + arg.name
                if arg.npointer > 0:
                    string += ' = NULL'
                string += ';\n'
            string += '\n'
            
        outargs = [arg for arg in func.arguments if arg.is_outarg]
        if len(outargs) > 0:
            string += 4*' ' + '/* output variables */\n'
            for arg in outargs:
                assert(arg.npointer > 0)
                if arg.arrayinfos['is_array'] and not arg.arrayinfos['autoalloc']:
                    string += 4*' ' + arg.rawtype + '*'*(arg.raw_npointer) + ' ' + arg.name + ' = NULL'
                else:
                    string += 4*' ' + arg.rawtype + '*'*(arg.raw_npointer-1) + ' ' + arg.name
                    if arg.npointer > 1:
                        string += ' = NULL'
                string += ';\n'
            string += '\n'
            
         # output array sizes
        outarrays = [arg for arg in func.arguments if arg.is_outarg and arg.arrayinfos['is_array']]
        if len(outarrays) > 0:
            sizestr = 'int '
            _count = 0
            for arg in outarrays:
                sizestr += '_%s_size, ' % arg.name
                _count += 1
            sizestr = sizestr[0:-2]
            if _count > 0:
                string += 4*' ' + sizestr + ';\n'
            
        if func.return_value.rawtype != 'void':
            string += 4*' '  + func.return_value.rawtype \
                + '*'*func.return_value.raw_npointer + ' ' + func.return_value.name
            if func.return_value.npointer > 0:
                string += ' = NULL'
            string += ';\n\n'

        pseudocall = func.method_name + '('
        for arg in inargs_wo_sizes:
            pseudocall += arg.name + ', '
        
        # create arg of manual size
        add_arg_count = 0
        for arg in outarrays:
            if len(arg.arrayinfos['arraysizes']) == 0:
                pseudocall += '_%s_size, ' % arg.name
                add_arg_count += 1
            
        if len(inargs) > 0:
            pseudocall = pseudocall[0:-2]
        pseudocall += ')'
        
        #check correct number in inargs
        string += 4*' ' + '/* check for corect number of in and out args */\n'
        string += 4*' ' + 'if (nrhs != %d) {\n' % (len(inargs_wo_sizes) + 1 + add_arg_count)
        
        string += 4*' ' + '    mexErrMsgTxt("%s: Wrong number of arguments\\n");\n' % pseudocall
        string += 4*' ' + '}\n'
        
        #check correct bumber of outargs
        noutargs = len(outargs)
        if not func.returns_error and func.return_value.type != 'void':
            noutargs += 1
            
        string += 4*' ' + 'if (nlhs != %d) {\n' % (noutargs)
        string += 4*' ' + '    mexErrMsgTxt("%s: Wrong number of output values. \
This function returns %d value(s)\\n");\n' % (pseudocall, noutargs)
        string += 4*' ' + '}\n'

        string += '\n'
        
        #input arg checks
        for index,arg in enumerate(inargs_wo_sizes):
            if not arg.arrayinfos['is_array']:
                if arg.type == 'int' or arg.type == 'double':
                    string += 4*' ' + 'if (!isscalar(prhs[%s])) {\n' \
                        % (index + 1)
                    string += 4*' ' + '    mexErrMsgTxt("Argument \'%s\' must not be an array.\\n");\n' \
                        % (arg.name)
                    string += 4*' ' + '}\n\n'
                elif arg.is_string:
                    string += 4*' ' + 'if (!mxIsChar(prhs[%s])) {\n' \
                        % (index + 1)
                    string += 4*' ' + '    mexErrMsgTxt("Argument \'%s\' must be a string.\\n");\n' \
                        % (arg.name)
                    string += 4*' ' + '}\n\n'
                    
        # checks for manual size arguments
        count = 0
        ninargs = len(inargs)
        for arg in outarrays:
            if len(arg.arrayinfos['arraysizes']) == 0:
                string += 4*' ' + 'if (!isscalar(prhs[%s])) {\n' \
                        % (count + ninargs + 1)
                string += 4*' ' + '    mexErrMsgTxt("Argument \'%s\' must not be an array.\\n");\n' \
                        % ('_' + arg.name + '_size')
                string += 4*' ' + '}\n\n'
                count += 1
        
        # input arg conversion
        for index,arg in enumerate(inargs_wo_sizes):
            if not arg.arrayinfos['is_array']:
                if arg.type == 'int':
                    string += 4*' ' + '%s = mxToInt(prhs[%d]);\n' \
                        % (arg.name, index+1)
                elif arg.type == 'size_t':
                    string += 4*' ' + '%s = mxToSize_t(prhs[%d]);\n' \
                        % (arg.name, index+1)
                elif arg.is_string:
                    string += 4*' ' + 'mxToString(prhs[%d], &%s);\n' \
                        % (index+1, arg.name)
                elif arg.type == 'double':
                    string += 4*' ' + '%s = *mxGetPr(prhs[%d]);\n' \
                        % (arg.name, index + 1)
                else:
                    raise Exception('Code wrapper for type "%s" currently not supported' % arg.type)
            else:
                if arg.type == 'double' and arg.npointer == 1:
                    string += 4*' ' + '%s = mxGetPr(prhs[%d]);\n' \
                        % (arg.name, index + 1)
                elif arg.type == 'int' and arg.npointer == 1:
                    string += 4*' ' + 'mxToIntArray(prhs[%d], &%s);\n' \
                        % (index + 1, arg.name)
                else:
                   raise Exception('Code wrapper for "%s array" currently not supported' % arg.type) 
        
        string += self.create_mex_inarraysizes(func)
        
        count = 0
        for arg in outarrays:
            if len(arg.arrayinfos['arraysizes']) == 0:
                string += 4*' ' + '%s = mxToInt(prhs[%d]);\n' \
                    % ('_' + arg.name + '_size', count + ninargs +1)
                count += 1
                    
        string += '\n'
        
        # manual allocation if necessary
        string += self.create_mex_array_malloced_sizes(func)
        string += self.create_mex_allocs(func)    
        
        # function call
        string += 4*' '
        if func.return_value.rawtype != 'void':
            string +=  func.return_value.name + ' = '
                
        string += func.method_name + '('
        
        for arg in func.arguments:
            if arg.is_outarg and (arg.arrayinfos['autoalloc'] or not arg.arrayinfos['is_array']):
                string += '&'
            string += arg.name + ', '
        
        if len(func.arguments) > 0:
            string = string[0:-2]
        
        string += ');\n'
        # validate return value
        if func.returns_error:
            string += 4*' ' + 'handleError("%s", %s);\n' % (func.method_name, func.return_value.name)
        
            
        string += '\n'
        
        string += self.create_mex_array_auto_sizes(func)
                
        # output arg conversion      
        retargs = outargs
        if not func.returns_error and func.return_value.type != 'void':
            retargs.insert(0, func.return_value)
            
        for index, arg in enumerate(retargs):
            for line in self.convert_c_to_mx(arg, index).splitlines():
                string += 4*' ' + line + '\n'
                
        
        string += '}\n\n'
        return string

    def create_mex_dispatcher(self):
        '''crates the main entry point for the mex file'''        
        
        indent = 4*' '   
        string = '''
#ifdef __cplusplus
extern "C" {
#endif


'''
        string += '/** main entry point of MATLAB, deals as a function dispatcher */\n'
        string += 'void mexFunction%s\n{\n' % self.mex_body
        
        string += '''
    char * functionName = NULL;
    if (nrhs < 1) {
        mexErrMsgTxt("No function argument given!");
        return;
    }

    mxToString(prhs[0],&functionName);

''' 
        call = ''
        defs = (fun for fun in self.cparser.declarations if fun.method_name not in self.blacklist)
        for index, func in enumerate(defs):
            if index > 0:
                call += 'else '
            call += 'if (strcmp(functionName, "%s") == 0) {\n' % func.method_name
            call += '    mex_%s(nlhs, plhs, nrhs, prhs);\n' % func.method_name
            call += '}\n'
        
        call += '''else {
    // this is only executed if the function could not be identified
    char text[255];
'''
        call += '    snprintf(text, 250, "%%s is not a valid %s function!\\n", functionName);\n' % self.prefix
        call += '''    mxFree(functionName);
    mexErrMsgTxt(text);
}
mxFree(functionName);
''' 
        
        for line in call.splitlines():
            string += indent + line + '\n'
            
        string += '}\n'
        string += '''
#ifdef __cplusplus
} /* extern C */
#endif

'''   
        return string
       
    def create_mex_error_handler(self):
        string = ''
        mycodename = self.cparser.returncode_str if self.cparser.returncode_str != "" else 'int'
        
        string += 'void handleError(const char * funcname, %s code)\n{\n' % (mycodename)
     
        if self.cparser.returncode_str in self.cparser.enums:
            successcode = '%s' % (self.cparser.enums[self.cparser.returncode_str][0])
        else:
            successcode = '0'

        string += '    if (code != %s) {\n' % successcode
        string += '        char errormsg[1024];\n'
        string += '        sprintf(errormsg, "Error: \\"%s\\" returned code %d", funcname, code);\n'
        string += '        mexErrMsgTxt(errormsg);\n'
        string += '    }\n'
        string += '}'

              
        return string
        
    def create_cmex_file(self):
        '''creates the .c file, which then is compiled to the matlab mex library'''        
        
        string = '/* This file was automatically generated from %s on %s */\n\n' % (self.libinclude, date.today())        
        
        incpath = os.path.dirname(os.path.realpath(__file__)) + '/matlab_cheader.in'
        
        #read header
        fop = open(incpath, 'r')
        string += fop.read() + '\n\n'
        fop.close()
        
        string += '#include <%s>\n\n' % self.libinclude
        string += self.create_mex_error_handler() + '\n\n'
        
        defs = (fun for fun in self.cparser.declarations if fun.method_name not in self.blacklist)
        for function in defs:
            string += self.create_mex_function(function)
        
        string += self.create_mex_dispatcher()
        filename = self.prefix + 'matlab.c'
        fop = open(filename, 'w')
        fop.write(string)
        fop.close()
        
    def create_wrapper(self):
        defs = (fun for fun in self.cparser.declarations if fun.method_name not in self.blacklist)
        for fun_dec in defs:
            self.create_m_file(fun_dec)
            
        self.create_cmex_file()
            
            