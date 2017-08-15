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
#include "CTiglLogging.h"
#include "TixiSaveExt.h"

namespace tigl
{

CCPACSMaterial::CCPACSMaterial()
{
    Cleanup();
}

void CCPACSMaterial::Cleanup()
{
    uid = "UID_NOTSET";
    thickness = -1.;
    thicknessScaling = 1.;
    isvalid = false;
    is_composite = false;
#if CPACS_VERSION >= VERSION_HEX_CODE(2,3,0)
    orthotropyDirection = 0.;
#else
    orthotropyDirection = CTiglPoint();
#endif
}

void CCPACSMaterial::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &materialXPath)
{
    Cleanup();
    
    // check path
    if (tixiCheckElement(tixiHandle, materialXPath.c_str()) != SUCCESS) {
        LOG(ERROR) << "Material definition" << materialXPath << " not found in CPACS file!" << std::endl;
        return;
    }
    
    // test whether composite or normal material
    char * matUID = NULL;
    if (tixiGetTextElement(tixiHandle, (materialXPath + "/materialUID").c_str(), &matUID) == SUCCESS){
        uid = matUID;
        is_composite = false;
    }
    else if (tixiGetTextElement(tixiHandle, (materialXPath + "/compositeUID").c_str(), &matUID) == SUCCESS){
        uid = matUID;
        is_composite = true;
    }
    else {
        throw CTiglError("Neither Material UID nor Composite UID  specified in " + materialXPath, TIGL_ERROR);
    }
    
    // get thickness (not mandatory)
    std::string path = materialXPath + "/thickness";
    if (tixiCheckElement(tixiHandle, path.c_str())== SUCCESS) {
       if (tixiGetDoubleElement(tixiHandle, path.c_str(), &thickness) != SUCCESS) {
           LOG(ERROR) << "Invalid material thickness in " << materialXPath;
       }
    }
    else if (tixiCheckElement(tixiHandle, (materialXPath + "/thicknessScaling").c_str())== SUCCESS) {
       if (tixiGetDoubleElement(tixiHandle, (materialXPath + "/thicknessScaling").c_str(), &thicknessScaling) != SUCCESS) {
           LOG(ERROR) << "Invalid composite thickness scaling in " << materialXPath;
       }
    }
    else {
        if (!isComposite()) {
            LOG(INFO) << "Thickness of Material " << materialXPath << " not set.";
        }
        else {
            LOG(INFO) << "Thickness scaling of Composite Material " << materialXPath << " not set.";
        }
    }
    
    if (isComposite()) {
        // handle orthotropy direction
        path = materialXPath + "/orthotropyDirection";
        if (tixiCheckElement(tixiHandle, path.c_str()) == SUCCESS) {
#if CPACS_VERSION >= VERSION_HEX_CODE(2,3,0)
            if (tixiGetDoubleElement(tixiHandle, (materialXPath + "/orthotropyDirection").c_str(), &orthotropyDirection) != SUCCESS) {
                LOG(ERROR) << "Invalid composite orthotropy direction in " << materialXPath;
            }
#else
            if (tixiGetPoint(tixiHandle, path.c_str(), &(orthotropyDirection.x), &(orthotropyDirection.y), &(orthotropyDirection.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <orthotropyDirection/> in CCPACSMaterial::ReadCPACS", TIGL_XML_ERROR);
            }
#endif
        }
    }

    isvalid = true;
}

void CCPACSMaterial::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    if (!isComposite()) {
        TixiSaveExt::TixiSaveTextElement(tixiHandle, xpath.c_str(), "materialUID", GetUID().c_str());
        if (thickness > 0) {
            TixiSaveExt::TixiSaveDoubleElement(tixiHandle, xpath.c_str(), "thickness", thickness, NULL);
        }
    }
    else {
        TixiSaveExt::TixiSaveTextElement(tixiHandle, xpath.c_str(), "compositeUID", GetUID().c_str());
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, xpath.c_str(), "thicknessScaling", GetThicknessScaling(), NULL);
#if CPACS_VERSION >= VERSION_HEX_CODE(2,3,0)
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, xpath.c_str(), "orthotropyDirection", orthotropyDirection, NULL);
#else
        TixiSaveExt::TixiSaveElement(tixiHandle, xpath.c_str(), "orthotropyDirection");
        const std::string subpath = xpath + "/orthotropyDirection";
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, subpath.c_str(), "x", orthotropyDirection.x, NULL);
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, subpath.c_str(), "y", orthotropyDirection.y, NULL);
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, subpath.c_str(), "z", orthotropyDirection.z, NULL);
#endif
    }
}

void CCPACSMaterial::Invalidate()
{
    isvalid = false;
}

bool CCPACSMaterial::isComposite() const
{
    return is_composite;
}

void CCPACSMaterial::SetComposite(bool composite)
{
    is_composite = composite;
}

bool CCPACSMaterial::IsValid() const
{
    return isvalid;
}

const std::string& CCPACSMaterial::GetUID() const
{
    return uid;
}

double CCPACSMaterial::GetThickness() const
{
    return thickness;
}

double CCPACSMaterial::GetThicknessScaling() const
{
    return thicknessScaling;
}

#if CPACS_VERSION >= VERSION_HEX_CODE(2,3,0)
void CCPACSMaterial::SetOrthotropyDirection(double direction)
#else
void CCPACSMaterial::SetOrthotropyDirection(tigl::CTiglPoint direction)
#endif
{
    orthotropyDirection = direction;
}

#if CPACS_VERSION >= VERSION_HEX_CODE(2,3,0)
double CCPACSMaterial::GetOrthotropyDirection() const
#else
const CTiglPoint& CCPACSMaterial::GetOrthotropyDirection() const
#endif
{
    return orthotropyDirection;
}

void CCPACSMaterial::SetUID(const std::string& uid)
{
    this->uid = uid;
}

void CCPACSMaterial::SetThickness(double thickness)
{
    this->thickness = thickness;
}

void CCPACSMaterial::SetThicknessScaling(double thicknessScaling)
{
    this->thicknessScaling = thicknessScaling;
}

} // namespace tigl
