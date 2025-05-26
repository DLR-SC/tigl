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

#include "CCPACSTransformationSE3.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSTransformationSE3::CCPACSTransformationSE3(CCPACSComponent* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformationSE3(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformationSE3::updateMatrix)
{
}

CCPACSTransformationSE3::CCPACSTransformationSE3(CCPACSElementGeometryAddtionalPart* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformationSE3(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformationSE3::updateMatrix)
{
}

void CCPACSTransformationSE3::reset()
{
    if (m_rotation) {
        m_rotation->SetAsPoint(CTiglPoint(0, 0, 0));
    }
    if (m_translation) {
        m_translation->SetAsPoint(CTiglPoint(0, 0, 0));
        m_translation->SetRefType(boost::optional<ECPACSTranslationType>());
    }
}

CCPACSTransformationSE3& CCPACSTransformationSE3::operator=(const CCPACSTransformationSE3& trafo)
{
    m_rotation    = trafo.m_rotation;
    m_translation = trafo.m_translation;

    _transformationMatrix.clear();

    return *this;
}

void CCPACSTransformationSE3::setTranslation(const CTiglPoint& translation)
{
    GetTranslation(CreateIfNotExists).SetAsPoint(translation);
}

void CCPACSTransformationSE3::setTranslation(const CTiglPoint& translation, ECPACSTranslationType type)
{
    CCPACSPointAbsRel& t = GetTranslation(CreateIfNotExists);
    t.SetAsPoint(translation);
    t.SetRefType(type);
}

void CCPACSTransformationSE3::setRotation(const CTiglPoint& rotation)
{
    if (!m_rotation) {
        m_rotation = boost::in_place(this, m_uidMgr);
    }
    m_rotation->SetAsPoint(rotation);
}

void CCPACSTransformationSE3::setRotationType(ECPACSTranslationType rotationType)
{
    _rotationType = rotationType;
}

void CCPACSTransformationSE3::setTransformationMatrix(const CTiglTransformation& matrix)
{
    // decompose matrix into scaling, rotation and translation
    double scale[3];
    double rotation[3];
    double translation[3];
    matrix.Decompose(&scale[0], &rotation[0], &translation[0]);

    if (!m_translation) {
        m_translation = boost::in_place(this, m_uidMgr);
    }
    m_translation->SetX(translation[0]);
    m_translation->SetY(translation[1]);
    m_translation->SetZ(translation[2]);

    if (!m_rotation) {
        m_rotation = boost::in_place(this, m_uidMgr);
    }
    m_rotation->SetX(rotation[0]);
    m_rotation->SetY(rotation[1]);
    m_rotation->SetZ(rotation[2]);
    if (m_uidMgr) {
        Invalidate();
    }
}

void CCPACSTransformationSE3::updateMatrix(CTiglTransformation& cache) const
{
    cache.SetIdentity();
    cache.AddScaling(1.0, 1.0, 1.0);

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

CTiglPoint CCPACSTransformationSE3::getTranslationVector() const
{
    return m_translation ? m_translation->AsPoint() : CTiglPoint(0, 0, 0);
}

CTiglPoint CCPACSTransformationSE3::getRotation() const
{
    return m_rotation ? m_rotation->AsPoint() : CTiglPoint(0, 0, 0);
}

ECPACSTranslationType CCPACSTransformationSE3::getTranslationType() const
{
    if (m_translation) {
        return m_translation->GetRefDefaultedType();
    }
    else {
        return CCPACSPointAbsRel::defaultTranslationType;
    }
}

ECPACSTranslationType CCPACSTransformationSE3::getRotationType() const
{
    return _rotationType;
}

CTiglTransformation CCPACSTransformationSE3::getTransformationMatrix() const
{
    return *_transformationMatrix;
}

void CCPACSTransformationSE3::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& transformationXPath)
{
    generated::CPACSTransformationSE3::ReadCPACS(tixiHandle, transformationXPath);
}

void CCPACSTransformationSE3::InvalidateImpl(const boost::optional<std::string>& source) const
{
    _transformationMatrix.clear();
    // invalidate parent
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate(GetUID());
    }
}

} // namespace tigl
