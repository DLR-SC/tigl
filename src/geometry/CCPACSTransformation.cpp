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

#include "CCPACSTransformation.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"

namespace tigl
{

CCPACSTransformation::CCPACSTransformation()
    : _translation(0., 0., 0.),
      _scaling(1., 1., 1.),
      _rotation(0., 0., 0.)
{
    _transformationMatrix.SetIdentity();
    _translationType = ABS_LOCAL;
}

void CCPACSTransformation::reset()
{
    _translation = CTiglPoint(0, 0, 0);
    _scaling = CTiglPoint(1., 1., 1.);
    _rotation = CTiglPoint(0., 0., 0.);
    _transformationMatrix.SetIdentity();
    _translationType = ABS_LOCAL;
}

void CCPACSTransformation::setTranslation(const CTiglPoint& translation, ECPACSTranslationType type)
{
    _translation = translation;
    _translationType = type;
}

void CCPACSTransformation::setRotation(const CTiglPoint& rotation)
{
    _rotation = rotation;
}

void CCPACSTransformation::setScaling(const CTiglPoint& scale)
{
    _scaling = scale;
}

void CCPACSTransformation::updateMatrix()
{
    _transformationMatrix.SetIdentity();

    // Step 1: scale around the orign
    _transformationMatrix.AddScaling(_scaling.x, _scaling.y, _scaling.z);

    // Step 2: rotate
    // Step 2a: rotate around z (yaw   += right tip forward)
    _transformationMatrix.AddRotationZ(_rotation.z);
    // Step 2b: rotate around y (pitch += nose up)
    _transformationMatrix.AddRotationY(_rotation.y);
    // Step 2c: rotate around x (roll  += right tip up)
    _transformationMatrix.AddRotationX(_rotation.x);

    // Step 3: translate the rotated body into its position
    _transformationMatrix.AddTranslation(_translation.x, _translation.y, _translation.z);
}

CTiglPoint CCPACSTransformation::getTranslationVector() const
{
    return _translation;
}

CTiglPoint CCPACSTransformation::getRotation() const
{
    return _rotation;
}

CTiglPoint CCPACSTransformation::getScaling() const
{
    return _scaling;
}

ECPACSTranslationType CCPACSTransformation::getTranslationType() const
{
    return _translationType;
}

CTiglTransformation CCPACSTransformation::getTransformationMatrix() const
{
    return _transformationMatrix;
}

void CCPACSTransformation::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& transformationXPath)
{
    // Get Translation
    std::string tempString  = transformationXPath + "/transformation/translation";
    const char* elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(_translation.x), &(_translation.y), &(_translation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <translation/> in CCPACSTransformation::ReadCPACS", TIGL_XML_ERROR);
        }
    }
    
    // Get translation type (attribute of "/transformation/translation")
    if (tixiCheckAttribute(tixiHandle, elementPath, "refType") == SUCCESS) {
        char * refTypeVal = NULL;
        if (tixiGetTextAttribute(tixiHandle, elementPath, "refType", &refTypeVal) == SUCCESS) {
            std::string refTypeStr(refTypeVal);
            if (refTypeStr == "absGlobal") {
                _translationType = ABS_GLOBAL;
            }
            else if (refTypeStr == "absLocal") {
                _translationType = ABS_LOCAL;
            }
        }
    }

    // Get scaling
    tempString  = transformationXPath + "/transformation/scaling";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(_scaling.x), &(_scaling.y), &(_scaling.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <scaling/> in CCPACSTransformation::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get rotation
    tempString  = transformationXPath + "/transformation/rotation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(_rotation.x), &(_rotation.y), &(_rotation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <rotation/> in CCPACSTransformation::ReadCPACS", TIGL_XML_ERROR);
        }
    }
    
    updateMatrix();
}

void CCPACSTransformation::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string &transformationXPath)
{
    std::string elementPath, subelementPath;

    TixiSaveExt::TixiSaveElement(tixiHandle, transformationXPath.c_str(), "transformation");
    elementPath = transformationXPath + "/transformation";

    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "scaling");
    subelementPath = elementPath + "/scaling";
    TixiSaveExt::TixiSavePoint(tixiHandle, subelementPath.c_str(), _scaling.x, _scaling.y, _scaling.z, "%f");

    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "rotation");
    subelementPath = elementPath + "/rotation";
    TixiSaveExt::TixiSavePoint(tixiHandle, subelementPath.c_str(), _rotation.x, _rotation.y, _rotation.z, "%f");

    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "translation");
    subelementPath = elementPath + "/translation";

    if (_translationType == ABS_LOCAL) {
        TixiSaveExt::TixiSaveTextAttribute(tixiHandle, subelementPath.c_str(), "refType", "absLocal");
    }
    else {
        TixiSaveExt::TixiSaveTextAttribute(tixiHandle, subelementPath.c_str(), "refType", "absGlobal");
    }

    TixiSaveExt::TixiSavePoint(tixiHandle, subelementPath.c_str(), _translation.x, _translation.y, _translation.z, "%f");
}


CCPACSTransformation::~CCPACSTransformation()
{
}

} // namespace tigl
