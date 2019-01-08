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
    *_transformationMatrix.writeAccess() = matrix;

    // decompose matrix into scaling, rotation and translation
    if (!m_translation) {
        m_translation = boost::in_place(m_uidMgr);
    }
    m_translation->SetX(matrix.GetValue(0,3));
    m_translation->SetY(matrix.GetValue(1,3));
    m_translation->SetZ(matrix.GetValue(2,3));

    // CPACS transformation order is scale before translate, this means scale
    // is length of the columns in matrices upper 3x3 block
    double m_rot[3][3];
    double scale[3] = {0., 0., 0.};
    for (int i = 0; i<3; ++i ) {
        for (int j=0; j<3; ++j ) {
            m_rot[i][j] = matrix.GetValue(i,j);
            scale[j] += m_rot[i][j]*m_rot[i][j];
        }
    }
    for (int i=0; i<3; ++i) {
        scale[i] = sqrt(scale[i]);
    }
    if (!m_scaling) {
        m_scaling = boost::in_place(m_uidMgr);
    }
    m_scaling->SetX(scale[0]);
    m_scaling->SetY(scale[1]);
    m_scaling->SetZ(scale[2]);

    // CPACS rotation is Euler xyz.
    // This implementation is based on http://www.gregslabaugh.net/publications/euler.pdf

    // remove scaling from upper 3x3 block to obtain orthonormal matrix
    for (int i = 0; i<3; ++i ) {
        for (int j=0; j<3; ++j ) {
            m_rot[i][j] /= scale[j];
        }
    }
    double rot[3] = {0., 0., 0.};
    if( fabs( fabs(m_rot[2][0]) - 1) > 1e-10 ){
        rot[1] = -asin(m_rot[2][0]);
        double cosTheta = cos(rot[2]);
        rot[0] = atan2(m_rot[2][1]/cosTheta, m_rot[2][2]/cosTheta);
        rot[2] = atan2(m_rot[1][0]/cosTheta, m_rot[0][0]/cosTheta);
    }
    else {
        if ( fabs(m_rot[2][0] + 1) > 1e-10 ) {
            rot[0] = rot[2] + atan2(m_rot[0][1], m_rot[0][2]);
            rot[1] = M_PI/2;
        }
        else{
            rot[0] = -rot[2] + atan2(-m_rot[0][1], -m_rot[0][2]);
            rot[1] = -M_PI/2;
        }
    }
    if (!m_rotation) {
        m_rotation = boost::in_place(m_uidMgr);
    }
    m_rotation->SetX(Degrees(rot[0]));
    m_rotation->SetY(Degrees(rot[1]));
    m_rotation->SetZ(Degrees(rot[2]));
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
