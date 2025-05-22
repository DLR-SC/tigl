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
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSTransformation::CCPACSTransformation(CCPACSEnginePosition* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSFuselage* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSDuct* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSDuctAssembly* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSFuselageSectionElement* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSFuselageSection* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSExternalObject* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSGenericSystem* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSGenericGeometryComponent* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSLandingGearBase* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSVessel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSFuelTank* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSNacelleSection* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSRotor* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSRotorHinge* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSWing* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSWingSectionElement* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CCPACSWingSection* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{
}

CCPACSTransformation::CCPACSTransformation(CTiglUIDManager* uidMgr)
    : generated::CPACSTransformation((CCPACSWingSection*)nullptr, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformation::updateMatrix)
{}


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
}

CCPACSTransformation &CCPACSTransformation::operator =(const CCPACSTransformation &trafo)
{
    m_scaling = trafo.m_scaling;
    m_rotation = trafo.m_rotation;
    m_translation = trafo.m_translation;

    _transformationMatrix.clear();

    return *this;
}

void CCPACSTransformation::setTranslation(const CTiglPoint & translation)
{
    GetTranslation(CreateIfNotExists).SetAsPoint(translation);
}

void CCPACSTransformation::setTranslation(const CTiglPoint& translation, ECPACSTranslationType type)
{
    CCPACSPointAbsRel& t = GetTranslation(CreateIfNotExists);
    t.SetAsPoint(translation);
    t.SetRefType(type);
}

void CCPACSTransformation::setRotation(const CTiglPoint& rotation)
{
    if (!m_rotation) {
        m_rotation = boost::in_place(this, m_uidMgr);
    }
    m_rotation->SetAsPoint(rotation);
}

void CCPACSTransformation::setRotationType(ECPACSTranslationType rotationType)
{
    _rotationType = rotationType;
}

void CCPACSTransformation::setScaling(const CTiglPoint& scale)
{
    if (!m_scaling) {
        m_scaling = boost::in_place(this, m_uidMgr);
    }
    m_scaling->SetAsPoint(scale);
}

void CCPACSTransformation::setScalingType(ECPACSTranslationType scalingType)
{
    _scalingType = scalingType;
}

void CCPACSTransformation::setTransformationMatrix(const CTiglTransformation& matrix)
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


    if (!m_scaling) {
        m_scaling = boost::in_place(this, m_uidMgr);
    }
    m_scaling->SetX(scale[0]);
    m_scaling->SetY(scale[1]);
    m_scaling->SetZ(scale[2]);

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

ECPACSTranslationType CCPACSTransformation::getScalingType() const
{
    return _scalingType;
}

ECPACSTranslationType CCPACSTransformation::getRotationType() const
{
    return _rotationType;
}

CTiglTransformation CCPACSTransformation::getTransformationMatrix() const
{
    return *_transformationMatrix;
}

void CCPACSTransformation::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& transformationXPath)
{
    generated::CPACSTransformation::ReadCPACS(tixiHandle, transformationXPath);
}

void CCPACSTransformation::InvalidateImpl(const boost::optional<std::string>& source) const
{
    _transformationMatrix.clear();
    // invalidate parent
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate(GetUID());
    }
}


void CCPACSTransformation::Init(const std::string& baseUID)
{
    if (GetUIDManager().IsUIDRegistered(baseUID)) {
        throw CTiglError(" CCPACSTransformation::Init: Impossible to initialize this transformation with the uid \"" +
                         baseUID + "\". This uid is already present in the file. Choose another uid.");
    }

    SetUID(baseUID);
    setTransformationMatrix(CTiglTransformation());
    GetTranslation(CreateIfNotExists).SetUID(GetUIDManager().MakeUIDUnique(baseUID + "Transl"));
    GetRotation(CreateIfNotExists).SetUID(GetUIDManager().MakeUIDUnique(baseUID + "Rot"));
    GetScaling(CreateIfNotExists).SetUID(GetUIDManager().MakeUIDUnique(baseUID + "Scal"));
}

} // namespace tigl
