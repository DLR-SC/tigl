#!/usr/bin/python
#
# Copyright (c) 2009 NHN Inc. All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#    * Neither the name of NHN Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
import getopt
import os #@UnusedImport
import re
#import sys #@UnusedImport
import copy
import nsiqcppstyle_checker
import nsiqcppstyle_state
import nsiqcppstyle_rulemanager
import nsiqcppstyle_reporter
#import updateagent.agent
from nsiqcppstyle_util import * #@UnusedWildImport
try :
    set()
except NameError:
    from sets import Set as set

version = "0.2.2.13"   
####################################################################################################
title = "nsiqcppstyle : N'SIQ Cpp Style ver "+ version + "\n"

def ShowMessageAndExit(msg, usageOutput=True) :
    print >> sys.stderr, msg
    if usageOutput : Usage()  
    sys.exit(-1)
     
def Usage() :
    print \
"""
======================================================================================
Usage : nsiqcppstyle [Options]
           targetdirectory 

[Example]
   nsiqcppstyle .
   nsiqcppstyle targetdir
   nsiqcppstyle -f filefilterpath targetfilepath
  
[Options] 
  -h            Show this help
  -v            Show detail ouput(verbose mode)
  -r            Show rule list
  -o path       Set the output path. It's only applied when the output is csv or xml.
  -f path       Set the filefilter path. If not provided, it uses the default filterpath
                (target/filefilter.txt) 
                If you provide the file path(not folder path) for the target, 
                -f option should be provided.
  --var=key:value,key:value
                provide the variables to customize the rule behavior.
  --list-rules / -r  Show all rules available.
                Add file extensions to be counted as assigned languages.
  -s            Assign Filter scope name to be applied in this analysis
  --output=     output format 'emacs', 'vs7', 'csv', 'xml', 'qtcreator' and 'eclipse'. Default value is vs7
                emacs, vs7, eclipse output the result on the stdout in the form 
                that each tool recognizes.
                csv and xml outputs the result on the file "nsiqcppstyle_result.csv" 
                "nsiqcppstyle_result.xml" respectively, if you don't provide -o option.
  --ci          Continuous Integration mode. If this mode is on, this tool only report summary.
  
* nsiqcppstyle reports coding standard violations on C/C++ source code.
* In default, it doesn't apply any rules on the source. If you want to apply rule,
  they are should be provided in the filefilter.txt file in following form.
  ~ RULENAME

* You can customize the rule behavior by providing --var=key:value pair when executing
  tool and you can put it in the filefilter.txt. The format is following.
  % key:value
  
* If you want to filter in or out some source code files in the target directory
  please locate filefilter.txt file in the target directory in the form of

  * FILTER_SCOPE_NAME
  + INCLUDE_PATH_PATTERNS
  - EXCLUDE_PATH_PATTERNS
  = LANGUAGE_NAME:EXTENSION,LANGUAGE_NAME:EXTENSION
  
  The filter scope name is the identifier to selectively apply filter.
  In case of the quality, Maybe only main sources except test should be measured.
  Otherwise, to measure the productivity, the test code might be measured as well.
  To keep this information in the same file(filefilter.txt), you can provide the
  * file_scope_name before the filter configuration starts.
  You can define multiple filter scope name in the filefilter.txt. In addition, 
  you can run nsiqcollector with -s option to specify the filter scope name used.
  We recommend you to define at least two filter scopes (Productivity, Quality)

  The included(+)/excluded(-) paths are applied sequentially from up to down
  In default, all files under target directory but under /.cvs/ and /.svn/ 
  will be included for analysis.

* It the basefilelist.txt (pair of filename and filesize) is in the target directory,
  nsiqcppstyle recognizes it and check the file are modified or new.
  And It checks only new and modified file. Please refer the nsiqcollector 
  to generate basefilelist.txt.
  
"""
    sys.exit(0)  



def main(argv=None):
    global filename
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "o:s:m:hvrf:", ["help", "csv", "output=", "list_rules", "verbose=", "show-url", "no-update", "ci", "var=", "noBase"])
        except getopt.error, msg:
            raise ShowMessageAndExit(msg)
            return 0
        
        outputPath = ""
        _nsiqcppstyle_state.output_format = "vs7"
        filterScope = "default"
        filterPath = ""
        ciMode = False
        noBase = False
        varMap = {}
        extLangMap = {
            "Html":set(["htm", "html"]),
            "Java":set(["java"]),
            "Javascript/ActionScript":set(["js", "as"]),
            "JSP/PHP":set(["jsp", "php", "JSP", "PHP"]),
            "C/C++":set(["cpp", "h", "c", "hxx", "cxx", "hpp"])
        } 

        updateNsiqCppStyle = True
        for o, a in opts:
            if o in ("-h", "--help"):
                print title
                Usage()
            elif o in ("-r", "--list-rules") :
                ShowRuleList()
            elif o == "-o" :
                outputPath = a.strip().replace("\"", "")
            elif o == "--no-update" :
                updateNsiqCppStyle = False
            elif o == "-f" :
                filterPath = a.strip().replace("\"", "")   
            elif o == "-v" :
                EnableVerbose()             
            elif o == "-s" :
                filterScope = a
            elif o == "--show-url" :
                _nsiqcppstyle_state.showUrl = True
            elif o == '--output':
                if not a in ('emacs', 'vs7', 'csv', 'xml', 'eclipse', 'qtcreator'):
                    print title
                    ShowMessageAndExit('The only allowed output formats are emacs, vs7 and csv.')
                _nsiqcppstyle_state.output_format = a
            elif o == "--var" :
                varMap = GetCustomKeyValueMap(a, "--var="+a)
            elif o == "--ci" :
                ciMode = True
            elif o == "--noBase" :
                noBase = True
                
        print title
        runtimePath = GetRuntimePath()
        sys.path.append(runtimePath)
        if updateNsiqCppStyle :
            try :
                print "======================================================================================"
                #updateagent.agent.Update(version)
            except Exception, e:
                print e
                
        targetPaths = GetRealTargetPaths(args)
        multipleTarget = True
        if len(targetPaths) == 1 :
            multipleTarget = False
        
        #If multiple target
        if multipleTarget :
            if len(outputPath) == 0 :
                ShowMessageAndExit("OutputPath(-o) should be provided to analyze multiple targets")
        else :
            outputPath = GetOutputPath(targetPaths[0], outputPath)
        ruleManager = nsiqcppstyle_rulemanager.ruleManager
        
        cExtendstionSet = extLangMap.get("C/C++")

        nsiqcppstyle_reporter.PrepareReport(outputPath, _nsiqcppstyle_state.output_format)
        analyzedFiles = []
        for targetPath in targetPaths  :
            nsiqcppstyle_reporter.StartTarget(targetPath)
            extLangMapCopy = copy.deepcopy(extLangMap)
            targetName = os.path.basename(targetPath)
            print "======================================================================================"
            print "=  Analyzing %s " % targetName
            
            if filterPath != "" :
                filefilterPath= filterPath  
            elif os.path.isfile(targetPath) :
                filefilterPath = os.path.join(os.path.dirname(targetPath), "filefilter.txt")
            else :
                filefilterPath = os.path.join(targetPath, "filefilter.txt")    
            if (noBase) :
                basefilelist = NullBaseFileList()
            else :
                basefilelist = BaseFileList(targetPath)
                
            # Get Active Filter
            filterManager = FilterManager(filefilterPath, extLangMapCopy, varMap, filterScope) 

            if filterScope != filterManager.GetActiveFilter().filterName :
                print "\n%s filter scope is not available. Instead, use %s\n" % (filterScope, filterManager.GetActiveFilter().filterName)
                
            filter = filterManager.GetActiveFilter()          
            # Load Rule
            
            if len(filter.nsiqCppStyleRules) == 0 :
                ShowMessageAndExit("Error!. Rules must be set in %s" % filefilterPath, False)
                continue
            
            ruleManager.LoadRules(filter.nsiqCppStyleRules, not ciMode)
            _nsiqcppstyle_state.checkers = filter.nsiqCppStyleRules
            _nsiqcppstyle_state.varMap = filter.varMap
            nsiqcppstyle_reporter.ReportRules(ruleManager.availRuleNames, filter.nsiqCppStyleRules)
            if not ciMode :
                print filter.to_string()            
            print "======================================================================================"
        
            if VerboseMode() : print "* run nsiqcppstyle analysis on %s" % targetName
            
            # if the target is file, analyze it without condition
            if os.path.isfile(targetPath) :
                fileExtension = targetPath[targetPath.rfind('.') + 1:]
                if fileExtension in cExtendstionSet :
                    ProcessFile(ruleManager, targetPath, analyzedFiles, ciMode)
                    
            # if the target is directory, analyze it with filefilter and basefilelist
            else :
                for root, dirs, files in os.walk(targetPath) :
                    if '.cvs' in dirs :
                        dirs.remove('.cvs')
                    if '.svn' in dirs :
                        dirs.remove('.svn')
                    for fname in files :
                        fileExtension = fname[fname.rfind('.') + 1:]
                        eachFile = os.path.join(root, fname)
                        basePart = eachFile[len(targetPath):]
                        if fileExtension in cExtendstionSet and basefilelist.IsNewOrChanged(eachFile) and filter.CheckFileInclusion(basePart) :
                            nsiqcppstyle_reporter.StartFile(os.path.dirname(basePart), fname)
                            ProcessFile(ruleManager, eachFile, analyzedFiles, ciMode)
                            nsiqcppstyle_reporter.EndFile()
            ruleManager.RunProjectRules(targetPath)
            nsiqcppstyle_reporter.EndTarget()
            
        nsiqcppstyle_reporter.ReportSummaryToScreen(analyzedFiles, _nsiqcppstyle_state, filter, ciMode)      
        nsiqcppstyle_reporter.CloseReport(_nsiqcppstyle_state.output_format)  
        return 0
    
    except Usage, err:
        print >> sys.stderr, err.msg
        print >> sys.stderr, "for help use --help"
        sys.exit(-1)


#################################################################################################3

def ProcessFile(ruleManager, file, analyzedFiles, ciMode):
    if not ciMode : print "Processing : ", file 
    nsiqcppstyle_checker.ProcessFile(ruleManager, file)
    analyzedFiles.append(file)
         
def Update():
    updateagent.agent.Update("http://nsiqcppstyle.nsiq.nhncorp.com/update", version)

csvResult = []    
def GetOutputPath(outputBasePath, outputPath) :
    "Returns the LOC and complexity result path"
    if outputPath == "" :
        if os.path.isfile(outputBasePath) :
            outputPath = os.path.dirname(outputBasePath)
        else :
            outputPath = outputBasePath
    return os.path.realpath(outputPath)

    
def GetRealTargetPaths(args) :
    "extract real target path list from args" 
    if len(args) == 0 :
        ShowMessageAndExit("Error! : Target directory must be provided")
    targetPaths = []
    for eachTarget in args :
        realPath = os.path.realpath(eachTarget)
        targetPaths.append(realPath)                           
#       CheckPathPermission(realPath, "Target directory")
        if not os.path.exists(realPath) :
            ShowMessageAndExit("Error! : Target directory %s is not exists" % eachTarget)
    return targetPaths

#################################################################################################3

def EnableVerbose() :
    _nsiqcppstyle_state.verbose = True
    
def VerboseMode() :
    return _nsiqcppstyle_state.verbose



##############################################################################
# Filter Manager
# - Load Filter
##############################################################################

class FilterManager :
    defaultFilterName = "default"
    
    def __init__(self, fileFilterPath, extLangMap, varMap, activeFilterName) :
        self.fileFilterPath = fileFilterPath
        self.baseExtLangMap = extLangMap
        self.baseVarMap = varMap
        self.filterMap = {FilterManager.defaultFilterName: self.CreateNewFilter(FilterManager.defaultFilterName)}
        filter = self.GetFilter(self.defaultFilterName)
        self.activeFilterName = self.defaultFilterName
        
        f = self.GetFilterFile(fileFilterPath)
        if f == None :
            filter.AddExclude("/.svn/")
            filter.AddExclude("/.cvs/")    
            return;
        
        for line in f.readlines():
            line = line.strip()
            if line.startswith("#") or len(line) == 0 :
                continue
            if line.startswith("*") :
                if (len(line[1:].strip()) != 0) :
                    filterName = line[1:].strip()
                    filter = self.GetFilter(filterName)    
            elif line.startswith("=") :
                if (len(line[1:].strip()) != 0) :
                    filter.AddLangMap(line[1:].strip(), "\"" + line + "\" of filefilter.txt")
            elif line.startswith("~") :
                if (len(line[1:].strip()) != 0) : 
                    filter.AddCppChecker(line[1:].strip())
            elif line.startswith("+") :
                arg = line[1:].strip()
                if arg != "" : filter.AddInclude(arg)
            elif line.startswith("-") :
                arg = line[1:].strip()
                if arg != "" : filter.AddExclude(arg)
            elif line.startswith("%") :
                arg = line[1:].strip()
                if arg != "" : filter.AddVarMap(arg, "\"" + arg + "\" of filefilter.txt")
        f.close()
        for eachMapKey in self.filterMap.keys() :
            self.filterMap[eachMapKey].AddExclude("/.cvs/")
            self.filterMap[eachMapKey].AddExclude("/.svn/")
           
        if (self.filterMap.has_key(activeFilterName)) :
            self.activeFilterName = activeFilterName
         
    def CreateNewFilter(self, filterName):
        return Filter(filterName, copy.deepcopy(self.baseExtLangMap), copy.deepcopy(self.baseVarMap))
    
    def GetFilter(self, filterName):
        if self.filterMap.has_key(filterName) :
            return self.filterMap[filterName]
        else :
            self.filterMap[filterName] = self.CreateNewFilter(filterName)
            return self.filterMap[filterName]
    
    def GetActiveFilter(self):  
        return self.GetFilter(self.activeFilterName)

    def GetFilterFile(self, filterfile):
        if not os.path.exists(filterfile) :
            return None
        f = file(filterfile, 'r')
        return f

##############################################################################
# Filter 
# - Represent each Filter 
# - Check if the file is included or not
##############################################################################
class Filter :
    """
     Filter 
     - Represent each Filter 
     - Check if the file is included or not
    """
    def __init__(self, filterName, baseExtLangMap, baseVarMap):
        self.extLangMap = baseExtLangMap
        self.varMap = baseVarMap
        self.filterName = filterName
        self.filefilter = []
        self.match = re.compile("^(\\\\|//)")
        self.nsiqCppStyleRules = []
        
    def to_string(self) :
        template = """Filter Scope "%s" is applied.
Current Filter Setting (Following is applied sequentially)
%s
Current File extension and Language Settings
%s""" 
        s = ""
        count = 1
        for eachfilter in self.filefilter :
            filterment = ""
            if eachfilter[0] : filterment = "is included"
            else : filterment = "is excluded"
            s = s + ("  %s. %s %s\n" % (count, eachfilter[1], filterment))
            count = count + 1
        return template % (self.filterName, s, self.GetLangString()) 
    
    def NormalizePath(self, eachFilter):
        replacedpath = eachFilter.replace("/", os.path.sep)
        replacedpath = replacedpath.replace("\\\\", os.path.sep);
        return replacedpath.replace("\\", os.path.sep);
   
    def CheckExist(self, includeOrExclude, eachFilter, startwith):
        return (self.filefilter.count([includeOrExclude, eachFilter, startwith]) == 1)

    def AddInclude(self, eachFilter):
        self.AddFilter(True, eachFilter)    
             
    def AddExclude(self, eachFilter):
        self.AddFilter(False, eachFilter)
    
    def AddCppChecker(self, eachChecker):
        self.nsiqCppStyleRules.append(eachChecker)
        
    def AddFilter(self, inclusion, eachFilter):
        startwith = False
        if eachFilter.startswith("\\\\") or eachFilter.startswith("//") :
            eachFilter = self.match.sub("", eachFilter)
      
        filterString = self.NormalizePath(eachFilter)
        if self.CheckExist(inclusion, filterString, startwith) :
            self.filefilter.remove([inclusion, filterString, startwith])
        self.filefilter.append([inclusion, filterString, startwith])   
    
    def GetFileFilter(self):
        return self.filefilter
    
    def GetLangString(self) :
        s = ""
        for eachKey in self.extLangMap.keys():
            if eachKey == "C/C++" :
                s = s + "  " + eachKey + "="
                extSet = self.extLangMap.get(eachKey)
                setLen = len(extSet)
                count = 0
                for eachExt in extSet :
                    count = count + 1
                    s = s + eachExt
                    if count < setLen : s = s + ","
                    else : s = s + "\n"
        return s;
    
        
    def CheckFileInclusion(self, fileStr):
        eachfile = self.NormalizePath(fileStr)
        inclusion = True
        for eachfilter in self.filefilter :
            if eachfilter[2] == True :
                if eachfile.startswith(eachfilter[1]) :
                    inclusion = eachfilter[0]
            else :
                if eachfile.find(eachfilter[1]) != -1 :
                    inclusion = eachfilter[0] 
        return inclusion  
    
    def GetLangMap(self):
        return self.extLangMap;        

    def AddLangMap(self, langMapString, where):
        langExtList = langMapString.split(",")
        for eachExt in langExtList :
            extLangPair = eachExt.split(":")
            if len(extLangPair) != 2 : 
                ShowMessageAndExit("Error! : The extension and language pair (%s) is incorrect in %s, please use LANGUAGENAME:EXTENSION style" % (langMapString, where))
            lang, ext = extLangPair
            self.extLangMap.get(lang).add(ext)

    def AddVarMap(self, keyValuePairString, where):
        varMap = GetCustomKeyValueMap(keyValuePairString, where)
        for eachVar in varMap.keys() :
            if self.varMap.has_key(eachVar) :
                continue
            else :
                self.varMap[eachVar] = varMap[eachVar]
        
def GetCustomKeyValueMap(keyValuePair, where):
    varMap = {}
    customKeyValues = keyValuePair.split(",")
    for eachCustomKeyValue in customKeyValues :
        customKeyValuePair = eachCustomKeyValue.split(":")
        if len(customKeyValuePair) != 2 : 
            ShowMessageAndExit("Error! : The var key and value pair (%s) is incorrect in %s, please use KEY:VALUE style" % (keyValuePair, where))
        key, value = customKeyValuePair
        varMap[key] = value
    return varMap

##############################################################################
# BaseFileList 
##############################################################################
class BaseFileList(object):
    """
     - Represent  basefilelist.txt state
     - It check if the current file and size pair is in the basefilelist.
    """
    def __init__(self, targetDir):
        self.baseFileList = {}
        if os.path.isdir(targetDir) :
            fsrc = os.path.join(targetDir, "basefilelist.txt")
            if os.path.exists(fsrc) :
                f = file(fsrc)
                for line in f.readlines() :
                    self.baseFileList[line.strip()] = True
    def IsNewOrChanged(self, filename):
        item = os.path.basename(filename) + str(os.path.getsize(filename))
        return not self.baseFileList.get(item, False)

class NullBaseFileList(object):
    """
     - Represent  basefilelist.txt state
     - It check if the current file and size pair is in the basefilelist.
    """
    def __init__(self):
        pass
    
    def IsNewOrChanged(self, filename):
        return True


def ShowRuleList():
    nsiqcppstyle_rulemanager.ruleManager.availRuleNames.sort()
    for rule in  nsiqcppstyle_rulemanager.ruleManager.availRuleNames:
        print "~", rule
    sys.exit(1)
    
    

def CheckPathPermission(path, folderrole) :
    if not os.access(path, os.R_OK) and os.path.exists(path) :
        ShowMessageAndExit("Error! : %s  You should have read permission in %s." % (folderrole, path))
    return True

####################################################################################################

_nsiqcppstyle_state = nsiqcppstyle_state._nsiqcppstyle_state



if __name__ == "__main__":
    sys.path.append(GetRuntimePath())
    sys.exit(main())
    