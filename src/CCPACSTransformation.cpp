
#include "CCPACSTransformation.h"

namespace tigl {
	namespace {
		CTiglPoint convertPoint(const tigl::generated::CPACSPoint& p) {
			return CTiglPoint(p.GetX(), p.GetY(), p.GetZ());
		}

		CTiglPoint convertPoint(const tigl::generated::CPACSPointAbsRel& p) {
			return CTiglPoint(p.GetX(), p.GetY(), p.GetZ());
		}
	}

	TIGL_EXPORT CCPACSTransformation::CCPACSTransformation() {
		Reset();
	}

	TIGL_EXPORT void CCPACSTransformation::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {
		generated::CPACSTransformation::ReadCPACS(tixiHandle, xpath);

		m_s = convertPoint(m_scaling.get());
		m_r = convertPoint(m_rotation.get());
		m_t = convertPoint(m_translation.get());
		BuildMatrix();
	}

	TIGL_EXPORT void CCPACSTransformation::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {
		generated::CPACSTransformation::WriteCPACS(tixiHandle, xpath);
	}

	TIGL_EXPORT CTiglTransformation CCPACSTransformation::GetTransformation() const {
		return m_transformation;
	}

	TIGL_EXPORT ECPACSTranslationType CCPACSTransformation::GetTranslationType() const {
		return generated::CPACSTransformation::GetTranslation().GetRefType();
	}

	TIGL_EXPORT const CTiglPoint& CCPACSTransformation::GetTranslation() const {
		return m_t;
	}

	TIGL_EXPORT const CTiglPoint& CCPACSTransformation::GetScaling() const {
		return m_s;
	}

	TIGL_EXPORT const CTiglPoint& CCPACSTransformation::GetRotation() const {
		return m_r;
	}

	TIGL_EXPORT void CCPACSTransformation::SetTranslation(const CTiglPoint& translation) {
		m_t = translation;
		BuildMatrix();
	}

	TIGL_EXPORT void CCPACSTransformation::SetRotation(const CTiglPoint& rotation) {
		m_r = rotation;
		BuildMatrix();
	}

	TIGL_EXPORT void CCPACSTransformation::SetScaling(const CTiglPoint& scaling) {
		m_s = scaling;
		BuildMatrix();
	}

	TIGL_EXPORT void CCPACSTransformation::BuildMatrix() {
		m_transformation.SetIdentity();
		m_transformation.AddScaling(m_s.x, m_s.y, m_s.z);
		m_transformation.AddRotationZ(m_r.z);
		m_transformation.AddRotationY(m_r.y);
		m_transformation.AddRotationX(m_r.x);
		m_transformation.AddTranslation(m_t.x, m_t.y, m_t.z);
	}

	TIGL_EXPORT void CCPACSTransformation::Reset() {
		m_transformation.SetIdentity();
		m_s = CTiglPoint(0, 0, 0);
		m_r = CTiglPoint(0, 0, 0);
		m_t = CTiglPoint(0, 0, 0);
	}
}