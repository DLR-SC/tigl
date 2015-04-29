/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-27 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSExternalObject.h"

#include "CCPACSConfiguration.h"
#include "tiglcommonfunctions.h"

namespace 
{

    std::string getPathRelativeToApp(TixiDocumentHandle handle, const std::string& pathInCPACS)
    {
        if (IsPathRelative(pathInCPACS)) {
            // the path is relative to the xml file
            // we need the path relative or absolute to the
            // executable that opened the cpacs file
            
            char* tixiDocPath = NULL;
            if (tixiGetDocumentPath(handle, &tixiDocPath) == SUCCESS && tixiDocPath != NULL ) {
                
                // get directory of cpacs file
                // TODO: check in case of only filename without path
                std::string cpacsPath = tixiDocPath;
                size_t pos = cpacsPath.find_last_of("/\\");
                if (pos == std::string::npos) {
                    // no separator found, current directory
                    return pathInCPACS;
                }
                std::string dirPath = cpacsPath.substr(0,pos);
                
                // create path of file
                return dirPath + "/" + pathInCPACS;
            }
            else {
                return pathInCPACS;
            }
        }
        else {
            return pathInCPACS;
        }
    }


}

namespace tigl
{

CCPACSExternalObject::CCPACSExternalObject(CCPACSConfiguration* config)
    : _config(config)
{
    
}

// Read CPACS wing element
void CCPACSExternalObject::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& objectXPath)
{
    Reset();

    std::string tempString;

    // Get attribute "uid"
    char* cUID = NULL;
    tempString   = "uID";
    if (tixiGetTextAttribute(tixiHandle, objectXPath.c_str(), tempString.c_str(), &cUID) == SUCCESS) {
        SetUID(cUID);
    }

    // Get subelement "parent_uid"
    char* cParentUID = NULL;
    tempString         = objectXPath + "/parentUID";
    if (tixiCheckElement(tixiHandle, tempString.c_str()) == SUCCESS && tixiGetTextElement(tixiHandle, tempString.c_str(), &cParentUID) == SUCCESS) {
        SetParentUID(cParentUID);
    }

    // Get Transformation
    transformation.ReadCPACS(tixiHandle, objectXPath);

    // Get File Path, and type
    std::string fileXPath  = objectXPath + "/file";
    if (tixiCheckElement(tixiHandle, fileXPath.c_str()) == SUCCESS) {
        char* cFilePath = NULL;
        if (tixiGetTextElement(tixiHandle, fileXPath.c_str(), &cFilePath) == SUCCESS) {
            _filePath = getPathRelativeToApp(tixiHandle, cFilePath);
        }
        else {
            throw tigl::CTiglError("No file specified in " + objectXPath + " !");
        }
        
        char* cFileType = NULL;
        if (tixiGetTextAttribute(tixiHandle, fileXPath.c_str(), "type", &cFileType) == SUCCESS) {
            _fileType = cFileType;
        }
        else {
            throw tigl::CTiglError("No file type attribute specified in " + fileXPath + " !");
        }
    }

    // Register ourself at the unique id manager
    if (_config) {
        _config->GetUIDManager().AddUID(cUID, this);
    }\

    // Get symmetry axis attribute, has to be done, when segments are build
    char* ptrSym = NULL;
    tempString   = "symmetry";
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(objectXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS) {
        SetSymmetryAxis(ptrSym);
    }
    
    
}

std::string CCPACSExternalObject::GetFilePath() const
{
    return _filePath;
}

TiglGeometricComponentType CCPACSExternalObject::GetComponentType()
{
    return TIGL_COMPONENT_PHYSICAL;
}

CCPACSExternalObject::~CCPACSExternalObject()
{
    
}

PNamedShape CCPACSExternalObject::BuildLoft()
{
    return PNamedShape();
}

} // namespace tigl

