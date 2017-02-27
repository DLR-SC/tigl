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

CCPACSTransformation::CCPACSTransformation() {
    _transformationMatrix.SetIdentity();
}

void CCPACSTransformation::reset()
{
    if (m_scaling) {
        m_scaling->SetAsPoint(CTiglPoint(1, 1, 1));
    }
    if (m_rotation) {
        m_rotation->SetAsPoint(CTiglPoint(0, 0, 0));
    }
    if (m_translation) {
        m_translation->SetAsPoint(CTiglPoint(0, 0, 0));
        if (m_translation->HasRefType())
            m_translation->SetRefType(ABS_LOCAL);
    }
    _transformationMatrix.SetIdentity();
}

void CCPACSTransformation::setTranslation(const CTiglPoint& translation, ECPACSTranslationType type)
{
    if (!m_translation) {
        m_translation = boost::in_place();
    }
    m_translation->SetAsPoint(translation);
    m_translation->SetRefType(type);
}

void CCPACSTransformation::setRotation(const CTiglPoint& rotation)
{
    if (!m_rotation) {
        m_rotation = boost::in_place();
    }
    m_rotation->SetAsPoint(rotation);
}

void CCPACSTransformation::setScaling(const CTiglPoint& scale)
{
    if (!m_scaling) {
        m_scaling = boost::in_place();
    }
    m_scaling->SetAsPoint(scale);
}

void CCPACSTransformation::updateMatrix()
{
    _transformationMatrix.SetIdentity();
    if (m_scaling) {
        const auto& s = m_scaling->AsPoint();
        _transformationMatrix.AddScaling(s.x, s.y, s.z);
    }
    if (m_rotation) {
        const auto& r = m_rotation->AsPoint();
        _transformationMatrix.AddRotationZ(r.z);
        _transformationMatrix.AddRotationY(r.y);
        _transformationMatrix.AddRotationX(r.x);
    }
    if (m_translation) {
        const auto& t = m_translation->AsPoint();
        _transformationMatrix.AddTranslation(t.x, t.y, t.z);
    }
}

CTiglPoint CCPACSTransformation::getTranslationVector() const
{
    return m_translation ? m_translation->AsPoint() : CTiglPoint(0, 0, 0);
}

CTiglPoint CCPACSTransformation::getRotation() const
{
    return m_rotation ? m_rotation->AsPoint() : CTiglPoint(0, 0, 0);
}

CTiglPoint CCPACSTransformation::getScaling() const
{
    return m_scaling ? m_scaling->AsPoint() : CTiglPoint(1, 1, 1);
}

ECPACSTranslationType CCPACSTransformation::getTranslationType() const
{
    return m_translation->GetRefType();
}

CTiglTransformation CCPACSTransformation::getTransformationMatrix() const
{
    return _transformationMatrix;
}

void CCPACSTransformation::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& transformationXPath)
{
    generated::CPACSTransformation::ReadCPACS(tixiHandle, transformationXPath);
    updateMatrix();
}

} // namespace tigl
