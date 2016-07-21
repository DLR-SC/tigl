
#include "CCPACSTransformation.h"

namespace tigl {
	TIGL_EXPORT void CCPACSTransformation::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {
		generated::CPACSTransformation::ReadCPACS(tixiHandle, xpath);
		BuildMatrix();
	}

	TIGL_EXPORT const CTiglTransformation& CCPACSTransformation::AsTransformation() const {
		return m_transformation;
	}

	TIGL_EXPORT ECPACSTranslationType CCPACSTransformation::GetTranslationType() const {
		return m_translation->GetRefType();
	}

	TIGL_EXPORT CTiglPoint CCPACSTransformation::GetTranslation() const {
		if (m_translation.isValid())
			return m_translation->AsPoint();
		else
			return CTiglPoint(0, 0, 0);
	}

	TIGL_EXPORT CTiglPoint CCPACSTransformation::GetScaling() const {
		if (m_scaling.isValid())
			return m_scaling->AsPoint();
		else
			return CTiglPoint(1, 1, 1);
	}

	TIGL_EXPORT CTiglPoint CCPACSTransformation::GetRotation() const {
		if (m_rotation.isValid())
			return m_rotation->AsPoint();
		else
			return CTiglPoint(0, 0, 0);
	}

	TIGL_EXPORT void CCPACSTransformation::SetTranslation(const CTiglPoint& translation) {
		if (!m_translation.isValid())
			m_translation.construct();
		m_translation->SetAsPoint(translation);
		BuildMatrix();
	}

	TIGL_EXPORT void CCPACSTransformation::SetRotation(const CTiglPoint& rotation) {
		if (!m_rotation.isValid())
			m_rotation.construct();
		m_rotation->SetAsPoint(rotation);
		BuildMatrix();
	}

	TIGL_EXPORT void CCPACSTransformation::SetScaling(const CTiglPoint& scaling) {
		if (!m_scaling.isValid())
			m_scaling.construct();
		m_scaling->SetAsPoint(scaling);
		BuildMatrix();
	}

	TIGL_EXPORT void CCPACSTransformation::BuildMatrix() {
		m_transformation.SetIdentity();
		if (m_scaling.isValid()) {
			const auto& s = m_scaling->AsPoint();
			m_transformation.AddScaling(s.x, s.y, s.z);
		}
		if (m_rotation.isValid()) {
			const auto& r = m_rotation->AsPoint();
			m_transformation.AddRotationZ(r.z);
			m_transformation.AddRotationY(r.y);
			m_transformation.AddRotationX(r.x);
		}
		if (m_translation.isValid()) {
			const auto& t = m_translation->AsPoint();
			m_transformation.AddTranslation(t.x, t.y, t.z);
		}
	}

	TIGL_EXPORT void CCPACSTransformation::Reset() {
		m_transformation.SetIdentity();
		if (m_scaling.isValid())
			m_scaling->SetAsPoint(CTiglPoint(1, 1, 1));
		if (m_rotation.isValid())
			m_rotation->SetAsPoint(CTiglPoint(0, 0, 0));
		if (m_translation.isValid())
			m_translation->SetAsPoint(CTiglPoint(0, 0, 0));
	}
}