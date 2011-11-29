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

import re
import sys
import types

fd = open('wrapperstub.py', 'r')
stub = fd.read()
fd.close()

class ParamType(object):
    ''' Type of a parameter, as determined by its signature. '''
    CHAR = 'CHAR'
    STRING = 'STRING'
    INT = 'INT'
    FLOAT = 'FLOAT'
    DOUBLE = 'DOUBLE'
    HANDLE = 'HANDLE'
    ENUM = 'ENUM_'
    UNKNOWN = 'UNKNOWN_'
    
    def __init__(self, param):
        ''' Determine the basic type of the parameter.
            Attributes:
              .name = name of type as defined in the enum above
              .pointer = direct (False) or pointer (True), still independent of input or output info.
            Note:
              For char_p the type is string
            
            >>> print ParamType('char *').name
            STRING
            >>> print ParamType('char *').pointer
            False
            >>> print ParamType('char**').name
            STRING
            >>> print ParamType('char**').pointer
            True
            >>> print ParamType('char **').name
            STRING
            >>> print ParamType('char **').pointer
            True
            >>> print ParamType(' int  *').name
            INT
            >>> print ParamType('double   *').name
            DOUBLE
            >>> print ParamType(' double *').pointer
            True
            >>> print ParamType('  int **').name
            INT
            >>> print ParamType('int**    ').pointer
            True
            >>> print ParamType(' TixiDocumentHandle ').name
            HANDLE
            >>> print ParamType(' TixiDocumentHandle ').name
            HANDLE
            >>> print ParamType(' TiglCPACSConfigurationHandle ').name
            HANDLE
        '''
        global allEnums
        tmp = param.replace('*', '').strip()
        if tmp == 'char':
            self.name = ParamType.CHAR
        elif tmp == 'int':
            self.name = ParamType.INT
        elif tmp == 'float':
            self.name = ParamType.FLOAT
        elif tmp == 'double':
            self.name = ParamType.DOUBLE
        elif tmp == 'TixiDocumentHandle':
            self.name = ParamType.HANDLE
        elif tmp == 'TiglCPACSConfigurationHandle':
            self.name = ParamType.HANDLE
        else:
            if tmp in allEnums:
                self.name = ParamType.ENUM + tmp
            else:
                self.name = ParamType.UNKNOWN + tmp
                print >> sys.stderr, "Found unknown type", tmp
        self.pointer = param.count("*")
        if param.count('*') >= 1 and self.name == ParamType.CHAR:
            self.name = ParamType.STRING
            self.pointer -= 1
        self.pointer = (self.pointer > 0) # assign a boolean here            

class Param(object):
    ''' A parameter of a method, containing a method name and a type describing field. '''
    def __init__(self, param):
        ''' Parse the parameter.
            param: type definition from the signature
            Attributes:
              .name = name of the parameter in the signature
              .type = type of the parameter in the signature
              .const = True if there was a const

            >>> print Param(' TixiDocumentHandle * myHandle ').name
            myHandle
            >>> print Param(' TixiDocumentHandle * myHandle ').type.name
            HANDLE
            >>> print Param(' TixiDocumentHandle * myHandle ').const
            False
            >>> print Param('const   char  **  myStrings').name
            myStrings
            >>> print Param('const   char  **  myStrings').type.name
            STRING
            >>> print Param('const   char  **  myStrings').const
            True
            >>> print Param(' const  TixiDocumentHandle tixiHandle ').name
            tixiHandle
            >>> print Param(' const  TixiDocumentHandle tixiHandle ').type.name
            HANDLE
            >>> print Param(' const  TixiDocumentHandle tixiHandle ').const
            True
            >>> print Param('char ** text').name
            text
            >>> print Param('char ** text').type.name
            STRING
            >>> print Param('char ** text').type.pointer
            True
            >>> print Param('int boolean').type.name
            INT
            >>> print Param('int boolean').type.pointer
            False
            >>> print Param('int *boolean').type.pointer
            True
        '''
        if 'const' in param:
            self.const = True
            param = param.replace("const", "").replace("  ", " ").strip()
        else:
            self.const = False
        if '**' in param:
            tmp = param.split('**', 1)
            self.name = tmp[-1].strip()
            tmp = tmp[0].strip() + '**'
        elif '*' in param:
            tmp = param.split('*', 1)
            self.name = tmp[-1].strip()
            tmp = tmp[0].strip() + '*'
        else:
            tmp = param.split(' ')
            self.name = tmp[-1]
            tmp = tmp[0]
        self.type = ParamType(tmp)
        
def getPartAfter(contents, terms):
    ''' Return only the lines after the search term.
        >>> print getPartAfter("One\\nTwo\\nThree\\nFour\\nFive\\n".split("\\n"), "hre")
        ['Four', 'Five']
        >>> print getPartAfter("One\\nTwo\\nThree\\nFour\\nFive\\n".split("\\n"), ["hre", "ee"]) # same line
        ['Four', 'Five']
        >>> print getPartAfter("One\\nTwo\\nThree\\nFour\\nFive\\n".split("\\n"), ["hre", "our"]) # different line
        ['Five']
    '''
    retValue = []
    if type(terms) in types.StringTypes:
        terms = [terms]
    foundCount = 0
    for i, line in enumerate(contents):
        if foundCount == len(terms): # all found: add rest and leave
            retValue += contents[i:]
            break
        else:
            if terms[foundCount] in line:
                while foundCount < len(terms) and terms[foundCount] in line:
                    foundCount += 1
            else:
                foundCount = 0
    return [x for x in retValue if x != '']

def getPartBefore(contents, term):
    ''' Iterator to read until a certain line.
        >>> for line in getPartBefore("One\\nTwo\\nThree\\nFour\\nFive\\n".split("\\n"), "hre"): print line
        One
        Two
    '''
    for line in contents:
        if term in line:
            return
        yield line

def findLinesWith(contents, term, endTerm = None):
    ''' Return all lines containing a certain sequence, and the line numbers where they were found.
        contents: array of lines of a file (e.g. tigl.h)
        return value is a tuple (nos, lines) where
          nos: n line numbers found
          lines: n text contents found

        >>> nos, lines = findLinesWith("One\\nTwo\\nThree\\nFour\\nFive\\n".split("\\n"), "e")
        >>> print nos
        [0, 2, 4]
        >>> print lines
        ['One', 'Three', 'Five']
    '''
    nos = []
    lines = []
    continuing = False
    for i, line in enumerate(contents):
        if continuing:
            lines[-1] += line.strip()
            if endTerm in line:
                continuing = False
                continue
        if term in line:
            nos.append(i)
            lines.append(line.strip())
            if endTerm != None and endTerm not in line:
                continuing = True
    return nos, lines

def parseEnumPart(contents, enum):
    ''' Find and reproduce the enums.
        >>> print parseEnumPart("  enum Test {  \\n   ONE, /*! bla */\\n  TWO   /*! blupp\\n  contd. */ \\n      };  ".split("\\n"), "Test")
        ['ONE', 'TWO']
    '''
    tmp = getPartAfter(contents, ["enum %s" % enum, "{"])
    part = []
    for line in getPartBefore(tmp, "};"):
        x = re.split(",", line)[0].replace("\t", "").replace(" ", "")
        x = re.split("=", x)[0]
        part += [x]
    print >> sys.stderr, "Found", len(part), "values for enum %s" % enum
    return part

def parseEnumParts(contents, enums):
    ''' Determine all enum values as lists.
        contents: The file contents
        enums: list of enum names to parse and return
    '''
    returnValues = {}
    for enum in enums:
        returnValues[enum] = parseEnumPart(contents, enum)
    return returnValues

def isPragma(line):
    ''' Check if this is a pragma line to ignore.
        >>> print isPragma("#bla")
        True
        >>> print isPragma("# bla")
        True
        >>> print isPragma("# bla ")
        True
        >>> print isPragma(" #bla")
        True
        >>> print isPragma(" #  bla  ")
        True
    '''
    return re.match("\\A\\s*#", line) != None

def parseTypes(params):
    ''' Parse the types from a comma separated method signature.
        return value: list of Param objects
        >>> print [x.type.name for x in parseTypes("double *, char** , int")]
        ['DOUBLE', 'STRING', 'INT']
    '''
    if params == '':
        return [] # no params
    tmp = params.split(",")
    return [Param(t.strip()) for t in tmp]

def matchLineNumbers(markers, methods, marks):
    ''' Return the matching line numbers of the given markers.
        markers is a list of line numbers of the #PY:# annotations
        methods is a list of line number of the method definitions to assosiate with them
        marks is a list of line contents of the #PY:' annotations
    '''
    markerIndex = 0
    returnValues = [] # size of methods, -1 = no marker, index = marker number x for the current method position
    for m in methods:
        if markerIndex >= len(markers):
            returnValues += [ None ]
            continue
        if m > markers[markerIndex]:
#            print >> sys.stderr, "Found marker match for method definition in line %d at line %d (%s)" % (m + 1, markers[markerIndex] + 1, marks[markerIndex])
            returnValues += [ marks[markerIndex] ]
            markerIndex += 1
        else:
            returnValues += [ None ]
    return returnValues

class Def1(object):
    def __init__(self, rettype, name, pstring, outs, outarr):
        self.return_type = rettype # string
        self.method_name = name # string
        self.params = parseTypes(pstring)
        self.out_vars = outs
        self.out_arrs = outarr
        
def findDefs(lines, markers):
    ''' Find all method definition lines.
        lines: list of method definition lines
        markers: list of #PY:# annotation markers assosiated with them
        return value: list of Def1 objects
    '''
    global blacklist
    returnValues = []
    for i, line in enumerate(lines):
        if isPragma(line):
            continue # ignore irrelevant lines
        m = re.search("\\s*DLL_EXPORT\\s+(\\S+)\\s+(\\S+)\\s*\\((.*)\\);", line) #1 = return type, 2 = method name, 3 = parameters
        if m == None:
            print >> sys.stderr, "Could not parse method definition", line
            continue
        if m.group(2) not in blacklist:
            marker = markers[i]
            returnsizes = []
            if marker != None:
                tmp = re.search("#PY:([\\d,]*)(:([\\d\\-,;]*))?#", marker)
                if tmp in [None, '']:
                    marker = None # meaning default = assume last as output
                    returnsizes = []
                else:
                    keys = tmp.group(1) # the out slot annotations
                    if keys in [None, '', []]:
                        marker = [] # meaning explicit no outputs!
                    else:
                        marker = [int(x) for x in keys.split(",")]
                    rets = tmp.group(3) # the out array type annotations (same number necessary, 0=no arr, <0 =user-spec slot -n, >0 = slot sum(n))
                    if tmp.group(3) not in [None, '']:
                        returnsizes = [[int(x) for x in y.split(";")] for y in rets.split(",")]
            returnValues.append(Def1(m.group(1), m.group(2), m.group(3), marker, returnsizes))
    return returnValues

def parseMethodHeaders(contents):
    ''' Parse and wrap all methods.
        contents: the tigl.h file.
    '''
    markerLineNos, markers = findLinesWith(contents, "#PY:") # find annotation lines
    lineNos, lines = findLinesWith(contents, "DLL_EXPORT", ");") # find header lines
    markers = matchLineNumbers(markerLineNos, lineNos, markers) # assosiate both
    defs = findDefs(lines, markers) # extract information from signatures, list of Def1
    return defs

def createParams(params, outparams, outarray):
    ''' Helper to create the strings that are used in the def: line of the python wrapper and that are passed to the TIGL call.
        params: list of Param objects
        outparams: list of Param objects (subset of params allowed)
        outarray: list of indices of array?
        return value: tuple (python string, c string, outarrs locations)
        
        >>> print createParams([Param("const int myInt")], [], [])
        ('myInt', '_c_myInt', [])
        
        >>> out = Param("char ** text")
        >>> print createParams([Param("const int myInt"), out], [out], [])
        ('myInt', '_c_myInt, byref(_c_text)', [])
        
        >>> print createParams([Param("const int myInt"), out], [out], [[0]])
        ('myInt', '_c_myInt, byref(_c_text)', [])
        
        >>> print createParams([Param("const int myInt"), out], [out], [[-1]])
        ('myInt, _num_0_', '_c_myInt, byref(_c_text)', ['_num_0_'])
    '''
    pythonParams = []
    cParams = []
    for param in params:
        if param not in outparams:
            pythonParams += [param.name]
        if param.type.pointer:
            if param in outparams:
                cParams += ["byref(_c_" + param.name + ")"]
            else: # is a input pointer
                cParams += ["_c_" + param.name]
        else: # not a pointer
            if param in outparams:
                print >> sys.stderr, "Output parameter not a pointer type:", param.name, param.type.name
            else:
                cParams += ["_c_" + param.name]
    pythonParams = ", ".join(pythonParams)
    # aggregate slots for manual parameters
    slots = []
    for outarr in outarray:
        for o in outarr:
            if o not in slots:
                slots.append(o)
    outarrs = []
    for i, k in enumerate(slots):
        if k == -1:
            outarrs += ["_num_%d_" % i]
    if len(outarrs) > 0:
        pythonParams += ", " + ", ".join(outarrs) # add more parameters for any manual "-1" one
    cParams = ", ".join(cParams)
    return (pythonParams, cParams, outarrs)

def createEnums():
    ''' Create the file part to place in the wrapper stub, containing all enum definitions '''
    global allEnums
    returnValue = ""
    for key, values in allEnums.iteritems():
        returnValue += "class %s(object):\n" % key
        for count, label in enumerate(values):
            returnValue += "    %s = %d\n" % (label, count)
        returnValue += "    _names = {}\n"
        for count, label in enumerate(values):
            returnValue += "    _names[%d] = '%s'\n" % (count, label)
        returnValue += "\n"
    return returnValue

def createMethodStubs(defs):
    ''' Create the stubs for the wrapper code.
        defs: list of Def1 objects
        return value: whole generated content
    '''
    returnValue = ""
    for d in defs:
        returnType = d.return_type
        methodName = d.method_name[4].lower() + d.method_name[5:] # remove "tigl" prefix and make first character lower case
        #methodName = d.name # for easier conversion from C-TIXI
        params = d.params
        outs = d.out_vars # only output parameter indexes returned
        outarray = d.out_arrs # list of lists of indexes of input slots whose len is multiplied for output array length determination
        
        # Determine output parameters
        if outs == None:
            if len(params) > 0:
                p = params[-1] # assume last as output parameter
                if not p.type.pointer and p.type.name == ParamType.STRING: # char * as output really means character
                    p.type.pointer = True
                    p.type.name = ParamType.CHAR
                outs = [p] # last value as output param
                if len(outarray) == 0:
                    outarray = [[0]]
            else: # no params whatsoever
                outs = []
                if len(outarray) == 0:
                    outarray = []
        else: # is a list
#            print methodName
            if len(params) > 0 and params[0].type.name == ParamType.HANDLE:
                outs = [params[outs[o] + 1] for o in range(0, len(outs))] # + 1 to ignore handle param as in other argument list
            else:
                outs = [params[outs[o]] for o in range(0, len(outs))] # list of params used as output params
            while len(outarray) < len(outs):
                outarray += [[0]]
            
        # Determine if we use a handle in the method
        useHandle = ""
        hasParams = ""
        if len(params) > 0 and params[0].type.name == ParamType.HANDLE:
            params = params[1:] # ignore handle parameter for following calculations
            useHandle = "self._handle"
        onlyInputs = list(params)
        for o in outs:
            if o in onlyInputs:
                onlyInputs.remove(o) # keep only input variables in onlyInputs
        if len(onlyInputs) > 0:
            hasParams = ", "
        if (len(onlyInputs) + len(outs)) > 1 or (useHandle != "" and (len(onlyInputs) + len(outs)) > 0):
            useHandle += ", "
            
        # Create stub header. python method header, c-call, array output names
        pString, cString, outnames = createParams(params, outs, outarray)

        # Create output array sizes, determined from provided annotations
        outsizes = []
        for oai, outarr in enumerate(outarray):
            t = "(1" # default return array size is one
            if len(outarr) > 0:
                for oa in outarr:
                    if oa < 0: # use user-provided size (was added to signature above)
                        t += " * %s" % outnames[-int(oa) - 1] # -1, -2, -3 -> 0, 1, 2
                    elif oa == 0: # is no array
                        pass
                    else: # > 0
                        t += " * " + onlyInputs[int(oa)].name # use input slot size
            t += ")"
            outsizes.append(t)

        # create method header and body
        outsizecount = 0
        returnValue += "    def %s(self%s%s):\n" % (methodName, hasParams, pString)
        for p in params:
            arr = p not in outs and p.type.pointer # is an array
            out = p in outs and outsizes[outsizecount] != '(1)' # is an output variable
            if p.type.name == ParamType.CHAR:
                returnValue += "        _c_%s = c_char()\n" % p.name
            elif p.type.name == ParamType.STRING:
                if out:
                    returnValue += "        _d_%s = c_char_p * %s\n" % (p.name, outsizes[outsizecount])
                    returnValue += "        _c_%s = _d_%s()\n" % (p.name, p.name)
                    outsizecount += 1
                elif arr:
                    returnValue += "        _d_%s = c_char_p * len(%s)\n" % (p.name, p.name)
                    returnValue += "        _c_%s = _d_%s(*%s)\n" % (p.name, p.name, p.name)
                else:
                    returnValue += "        _c_%s = c_char_p()\n" % p.name
            elif p.type.name == ParamType.INT:
                if out:
                    returnValue += "        _d_%s = c_int * %s\n" % (p.name, outsizes[outsizecount])
                    returnValue += "        _c_%s = _d_%s()\n" % (p.name, p.name)
                    outsizecount += 1
                elif arr:
                    returnValue += "        _d_%s = c_int * len(%s)\n" % (p.name, p.name)
                    returnValue += "        _c_%s = _d_%s(*%s)\n" % (p.name, p.name, p.name)
                else:
                    returnValue += "        _c_%s = c_int(0)\n" % p.name
            elif p.type.name == ParamType.FLOAT:
                if out:
                    returnValue += "        _d_%s = c_float * %s\n" % (p.name, outsizes[outsizecount])
                    returnValue += "        _c_%s = _d_%s()\n" % (p.name, p.name)
                    outsizecount += 1
                elif arr:
                    returnValue += "        _d_%s = c_float * len(%s)\n" % (p.name, p.name)
                    returnValue += "        _c_%s = _d_%s(*%s)\n" % (p.name, p.name, p.name)
                else:
                    returnValue += "        _c_%s = c_float(0)\n" % p.name 
            elif p.type.name == ParamType.DOUBLE:
                if out:
                    returnValue += "        _d_%s = c_double * %s\n" % (p.name, outsizes[outsizecount])
                    returnValue += "        _c_%s = _d_%s()\n" % (p.name, p.name)
                    outsizecount += 1
                elif arr:
                    returnValue += "        _d_%s = c_double * len(%s)\n" % (p.name, p.name)
                    returnValue += "        _c_%s = _d_%s(*%s)\n" % (p.name, p.name, p.name)
                else:
                    returnValue += "        _c_%s = c_double(0)\n" % p.name 
            elif p.type.name == ParamType.HANDLE:
                returnValue += "        _c_%s = c_int(0)\n" % p.name 
            elif p.type.name.startswith(ParamType.ENUM):
                returnValue += "        _c_%s = c_int(0)\n" % p.name
            else:
                print >> sys.stderr, "Cannot create wrapper code for type", p.type.name
            if p not in outs and not arr:
                returnValue += "        _c_%s.value = %s\n" % (p.name, p.name)
        returnValue += "        tiglReturn = self.TIGL.tigl%s(%s%s)\n" % (methodName[0].upper() + methodName[1:], useHandle, cString)
        returnValue += "        self._validateReturnValue(tiglReturn%s%s)\n" % (hasParams, pString)
        
        # create return line, either tuple or single value
        if len(outs) > 1:
            returnValue += "        return ("
            for o, s in zip(outs, outsizes):
                if s == "(1)": # is only one value
                    returnValue += "_c_" + o.name + ".value, "
                else:
                    returnValue +=" tuple([__x__ for __x__ in _c_" + o.name + "]), "
            returnValue += ")\n"
        elif len(outs) == 1:
            if outsizes[0] == "(1)":
                returnValue += "        return _c_" + o.name + ".value\n"
            else:
                returnValue += "        return tuple([__x__ for __x__ in _c_" + o.name + "])\n"
        returnValue += "\n"
    return returnValue

if __name__ == '__main__':
    ''' The main entry point, need to be called in the folder where tigl.h resides '''
    if len(sys.argv) > 1 and sys.argv[1] == "--test":
        global allEnums
        import doctest
        allEnums = {}
        doctest.testmod()
        sys.exit(0)
    
    fd = open("tigl.h", "r")
    tiglFile = fd.read().split("\n")
    fd.close()


    blacklist = ["tiglOpenCPACSConfiguration", "tiglCloseCPACSConfiguration",
                 "tiglGetCPACSTixiHandle", "tiglIsCPACSConfigurationHandleValid", "tiglGetVersion"]
    allEnums = parseEnumParts(tiglFile, ["TiglReturnCode", "TiglBoolean", "TiglSymmetryAxis", "TiglAlgorithmCode", "TiglGeometricComponentType", "TiglImportExportFormat"])
    defs = parseMethodHeaders(tiglFile) # gets a list of 5-tuples
    
    fd = open("tiglwrapper.py", "w")
    print >> fd, stub % (createEnums(), createMethodStubs(defs))
    fd.close()

    # Create distributions
    import os
    import functools
    from distutils.core import setup
    
    s = functools.partial(setup,
            name = 'tiglwrapper',
            version = '2011-01',
            py_modules = ['tiglwrapper'],
            url = 'http://code.google.com/p/tigl/',
            author = 'Arne Bachmann',
            author_email = 'arne.bachmann.dlr@googlemail.com',
            maintainer = 'DLR/SC',
            license = 'http://www.apache.org/licenses/LICENSE-2.0',
            description = 'Generated Python wrapper for the TIGL C library'
        )
    
    upload = "--upload" in sys.argv # to Google code

    # Source distribution
    print "\nBuilding source distribution"
    sys.argv = ['tiglwrapper.py', 'sdist'] # need an existing argv[0] here for some stupid reason
    s()
    
    # Windows distribution
    if 'win' in sys.platform:
        print "\nBuilding windows binary distribution"
        sys.argv = ['tiglwrapper.py', 'bdist_wininst']
        s()

    os.remove("MANIFEST")

    # Upload to google
    if upload:
        import getpass
        import googlecode_upload
        sys.stdout.write('Please enter your googlecode.com username: ')
        sys.stdout.flush()
        username = sys.stdin.readline().rstrip()
        print 'Please enter your googlecode.com password (not the gmail password).'
        password = getpass.getpass()
        path = os.path.join(".", "dist/tiglwrapper-2011-01.win32.exe")
        googlecode_upload.upload_find_auth(path, "tigl", "TIGL Python Wrapper Win32 Installer", user_name = username, password = password)
        path = os.path.join(".", "dist/tiglwrapper-2011-01.zip")
        googlecode_upload.upload_find_auth(path, "tigl", "TIGL Python Wrapper Source", user_name = username, password = password)
