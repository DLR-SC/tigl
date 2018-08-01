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

namespace tigl
{

CCPACSTransformation::CCPACSTransformation(CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
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
        m_translation->SetRefType(boost::optional<ECPACSTranslationType>());
    }
    _transformationMatrix.clear();
}

void CCPACSTransformation::setTranslation(const CTiglPoint & translation)
{
    GetTranslation(CreateIfNotExists).SetAsPoint(translation);
    _transformationMatrix.clear();
}

void CCPACSTransformation::setTranslation(const CTiglPoint& translation, ECPACSTranslationType type)
{
    CCPACSPointAbsRel& t = GetTranslation(CreateIfNotExists);
    t.SetAsPoint(translation);
    t.SetRefType(type);
    _transformationMatrix.clear();
}

void CCPACSTransformation::setRotation(const CTiglPoint& rotation)
{
    if (!m_rotation) {
        m_rotation = boost::in_place(m_uidMgr);
    }
    m_rotation->SetAsPoint(rotation);
    _transformationMatrix.clear();
}

void CCPACSTransformation::setScaling(const CTiglPoint& scale)
{
    if (!m_scaling) {
        m_scaling = boost::in_place(m_uidMgr);
    }
    m_scaling->SetAsPoint(scale);
    _transformationMatrix.clear();
}

void CCPACSTransformation::setTransformationMatrix(const CTiglTransformation& matrix)
{
    // TODO(bgruber): implement matrix decomposition into m_rotation, m_scaling and m_translation
    *_transformationMatrix.writeAccess() = matrix;
}

void CCPACSTransformation::updateMatrix(CTiglTransformation& cache) const
{
    cache.SetIdentity();
    if (m_scaling) {
        const CTiglPoint& s = m_scaling->AsPoint();
        cache.AddScaling(s.x, s.y, s.z);
    }
    if (m_rotation) {
        const CTiglPoint& r = m_rotation->AsPoint();
        cache.AddRotationZ(r.z);
        cache.AddRotationY(r.y);
        cache.AddRotationX(r.x);
    }
    if (m_translation) {
        const CTiglPoint& t = m_translation->AsPoint();
        cache.AddTranslation(t.x, t.y, t.z);
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
    if (m_translation) {
        return m_translation->GetRefDefaultedType();
    }
    else {
        return CCPACSPointAbsRel::defaultTranslationType;
    }
}

CTiglTransformation CCPACSTransformation::getTransformationMatrix() const
{
    return *_transformationMatrix;
}

void CCPACSTransformation::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& transformationXPath)
{
    generated::CPACSTransformation::ReadCPACS(tixiHandle, transformationXPath);
}

} // namespace tigl
