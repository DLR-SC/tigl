
#include "CCPACSTransformation.h"

namespace tigl
{
    void CCPACSTransformation::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        generated::CPACSTransformation::ReadCPACS(tixiHandle, xpath);
        BuildMatrix();
    }

    const CTiglTransformation& CCPACSTransformation::AsTransformation() const
    {
        return m_transformation;
    }

    ECPACSTranslationType CCPACSTransformation::GetTranslationType() const
    {
        return m_translation->GetRefType();
    }

    CTiglPoint CCPACSTransformation::GetTranslation() const
    {
        return m_translation ? m_translation->AsPoint() : CTiglPoint(0, 0, 0);
    }

    CTiglPoint CCPACSTransformation::GetScaling() const
    {
        return m_scaling ? m_scaling->AsPoint() : CTiglPoint(1, 1, 1);
    }

    CTiglPoint CCPACSTransformation::GetRotation() const
    {
        return m_rotation?  m_rotation->AsPoint() : CTiglPoint(0, 0, 0);
    }

    void CCPACSTransformation::SetTranslation(const CTiglPoint& translation)
    {
        if (!m_translation) {
            m_translation = boost::in_place();
        }
        m_translation->SetAsPoint(translation);
        BuildMatrix();
    }

    void CCPACSTransformation::SetRotation(const CTiglPoint& rotation)
    {
        if (!m_rotation) {
            m_rotation = boost::in_place();
        }
        m_rotation->SetAsPoint(rotation);
        BuildMatrix();
    }

    void CCPACSTransformation::SetScaling(const CTiglPoint& scaling)
    {
        if (!m_scaling) {
            m_scaling = boost::in_place();
        }
        m_scaling->SetAsPoint(scaling);
        BuildMatrix();
    }

    void CCPACSTransformation::BuildMatrix()
    {
        m_transformation.SetIdentity();
        if (m_scaling) {
            const auto& s = m_scaling->AsPoint();
            m_transformation.AddScaling(s.x, s.y, s.z);
        }
        if (m_rotation) {
            const auto& r = m_rotation->AsPoint();
            m_transformation.AddRotationZ(r.z);
            m_transformation.AddRotationY(r.y);
            m_transformation.AddRotationX(r.x);
        }
        if (m_translation) {
            const auto& t = m_translation->AsPoint();
            m_transformation.AddTranslation(t.x, t.y, t.z);
        }
    }

    void CCPACSTransformation::Reset()
    {
        m_transformation.SetIdentity();
        if (m_scaling) {
            m_scaling->SetAsPoint(CTiglPoint(1, 1, 1));
        }
        if (m_rotation) {
            m_rotation->SetAsPoint(CTiglPoint(0, 0, 0));
        }
        if (m_translation) {
            m_translation->SetAsPoint(CTiglPoint(0, 0, 0));
        }
    }
}