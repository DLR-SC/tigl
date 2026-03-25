/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-03-25 Marko Alder <marko.alder@dlr.de>
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

#include "CCPACSTransformationPlanar.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSTransformationPlanar::CCPACSTransformationPlanar(CCPACSDeckComponent2DBase* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTransformationPlanar(parent, uidMgr)
    , _transformationMatrix(*this, &CCPACSTransformationPlanar::updateMatrix)
{
}

void CCPACSTransformationPlanar::reset()
{
    if (m_scaling) {
        m_scaling->SetX(1.);
        m_scaling->SetY(1.);
        m_scaling->SetZ(1.);
    }

    if (m_rotation) {
        m_rotation->SetZ(0.);
    }

    if (m_translation) {
        m_translation->SetX(0.);
        m_translation->SetY(0.);
    }
}

CCPACSTransformationPlanar& CCPACSTransformationPlanar::operator=(const CCPACSTransformationPlanar& trafo)
{
    if (trafo.m_scaling) {
        if (!m_scaling) {
            m_scaling = boost::in_place(this, m_uidMgr);
        }
        m_scaling->SetX(trafo.m_scaling->GetX());
        m_scaling->SetY(trafo.m_scaling->GetY());
        m_scaling->SetZ(trafo.m_scaling->GetZ());
    }
    else {
        m_scaling = boost::none;
    }

    if (trafo.m_rotation) {
        if (!m_rotation) {
            m_rotation = boost::in_place(this, m_uidMgr);
        }
        m_rotation->SetZ(trafo.m_rotation->GetZ());
    }
    else {
        m_rotation = boost::none;
    }

    if (trafo.m_translation) {
        if (!m_translation) {
            m_translation = boost::in_place(this, m_uidMgr);
        }
        m_translation->SetX(trafo.m_translation->GetX());
        m_translation->SetY(trafo.m_translation->GetY());
    }
    else {
        m_translation = boost::none;
    }

    return *this;
}

void CCPACSTransformationPlanar::setScaling(const CTiglPoint& scale)
{
    if (!m_scaling) {
        m_scaling = boost::in_place(this, m_uidMgr);
    }

    m_scaling->SetX(scale.x);
    m_scaling->SetY(scale.y);
    m_scaling->SetZ(scale.z);
}

void CCPACSTransformationPlanar::setRotation(const CTiglPoint& rotation)
{
    if (!m_rotation) {
        m_rotation = boost::in_place(this, m_uidMgr);
    }

    m_rotation->SetZ(rotation.z);
}

void CCPACSTransformationPlanar::setTranslation(const CTiglPoint& translation)
{
    if (!m_translation) {
        m_translation = boost::in_place(this, m_uidMgr);
    }

    m_translation->SetX(translation.x);
    m_translation->SetY(translation.y);
}

void CCPACSTransformationPlanar::setTransformationMatrix(const CTiglTransformation& matrix)
{
    double scale[3];
    double rotation[3];
    double translation[3];
    matrix.Decompose(&scale[0], &rotation[0], &translation[0]);

    if (!m_scaling) {
        m_scaling = boost::in_place(this, m_uidMgr);
    }
    m_scaling->SetX(scale[0]);
    m_scaling->SetY(scale[1]);
    m_scaling->SetY(scale[2]);

    if (!m_rotation) {
        m_rotation = boost::in_place(this, m_uidMgr);
    }
    m_rotation->SetZ(rotation[2]);

    if (!m_translation) {
        m_translation = boost::in_place(this, m_uidMgr);
    }
    m_translation->SetX(translation[0]);
    m_translation->SetY(translation[1]);

    if (m_uidMgr) {
        Invalidate();
    }
}

void CCPACSTransformationPlanar::updateMatrix(CTiglTransformation& cache) const
{
    cache.SetIdentity();

    if (m_scaling) {
        cache.AddScaling(m_scaling->GetX(), m_scaling->GetY(), m_scaling->GetZ());
    }

    if (m_rotation) {
        cache.AddRotationZ(m_rotation->GetZ());
    }

    if (m_translation) {
        cache.AddTranslation(m_translation->GetX(), m_translation->GetY(), 0.);
    }
}

CTiglPoint CCPACSTransformationPlanar::getScaling() const
{
    return m_scaling ? CTiglPoint(m_scaling->GetX(), m_scaling->GetY(), m_scaling->GetZ()) : CTiglPoint(1., 1., 1.);
}

CTiglPoint CCPACSTransformationPlanar::getRotation() const
{
    return m_rotation ? CTiglPoint(0., 0., m_rotation->GetZ()) : CTiglPoint(0., 0., 0.);
}

CTiglPoint CCPACSTransformationPlanar::getTranslationVector() const
{
    return m_translation ? CTiglPoint(m_translation->GetX(), m_translation->GetY(), 0.) : CTiglPoint(0., 0., 0.);
}

CTiglTransformation CCPACSTransformationPlanar::getTransformationMatrix() const
{
    return *_transformationMatrix;
}

void CCPACSTransformationPlanar::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& transformationXPath)
{
    generated::CPACSTransformationPlanar::ReadCPACS(tixiHandle, transformationXPath);
}

void CCPACSTransformationPlanar::InvalidateImpl(const boost::optional<std::string>& source) const
{
    _transformationMatrix.clear();
    // invalidate parent
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate(GetUID());
    }
}

} // namespace tigl
