/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSMaterial.h"

#include "CTiglError.h"
#include "CTiglLogger.h"

namespace tigl {

CCPACSMaterial::CCPACSMaterial()
{
    Cleanup();
}

void CCPACSMaterial::Cleanup(){
    uid = "UID_NOTSET";
    thickness = -1.;
    thicknessScaling = 1.;
    isvalid = false;
    is_composite = false;
}

void CCPACSMaterial::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &materialXPath){
    Cleanup();
    
    // check path
    if( tixiCheckElement(tixiHandle, materialXPath.c_str()) != SUCCESS){
        LOG(ERROR) << "Material definition" << materialXPath << " not found in CPACS file!" << std::endl;
        return;
    }
    
    // test whether composite or normal material
    std::string tempstring = materialXPath + "/materialUID";
    char * matUID = NULL;
    if (tixiGetTextElement(tixiHandle, tempstring.c_str(), &matUID) == SUCCESS){
        uid = matUID;
        is_composite = false;
    }
    else if (tixiGetTextElement(tixiHandle, std::string(materialXPath + "/compositeUID").c_str(), &matUID) == SUCCESS){
        uid = matUID;
        is_composite = true;
    }
    else {
        throw CTiglError("Neither Material UID nor Composite UID  specified in " + materialXPath, TIGL_ERROR);
    }
    
    // get thickness (not mandatory)
    tempstring = materialXPath + "/thickness";
    if(tixiCheckElement(tixiHandle, tempstring.c_str())== SUCCESS){
       if(tixiGetDoubleElement(tixiHandle, tempstring.c_str(), &thickness) != SUCCESS)
           LOG(ERROR) << "Invalid material thickness in " << materialXPath;
    }
    else if(tixiCheckElement(tixiHandle, std::string(materialXPath + "/thicknessScaling").c_str())== SUCCESS){
       if(tixiGetDoubleElement(tixiHandle, std::string(materialXPath + "/thicknessScaling").c_str(), &thicknessScaling) != SUCCESS)
           LOG(ERROR) << "Invalid composite thickness scaling in " << materialXPath;
    }
    else {
        if(!isComposite())
            LOG(INFO) << "Thickness of Material " << materialXPath << " not set.";
        else
            LOG(INFO) << "Thickness scaling of Composite Material " << materialXPath << " not set.";
    }
    
    isvalid = true;
}

void CCPACSMaterial::Invalidate()
{
    isvalid = false;
}

bool CCPACSMaterial::isComposite() const
{
    return is_composite;
}

bool CCPACSMaterial::IsValid() const
{
    return isvalid;
}

const std::string& CCPACSMaterial::GetUID() const
{
    return uid;
}

} // namespace tigl
