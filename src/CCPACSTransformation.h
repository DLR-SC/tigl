#pragma once

#include "generated/CPACSTransformation.h"

#include "CTiglTransformation.h"

namespace tigl {
	class CCPACSTransformation : private generated::CPACSTransformation {
	public:
		TIGL_EXPORT CCPACSTransformation();

		TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
		TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

		TIGL_EXPORT CTiglTransformation GetTransformation() const;

		TIGL_EXPORT const CTiglPoint& GetScaling() const;
		TIGL_EXPORT const CTiglPoint& GetRotation() const;
		TIGL_EXPORT const CTiglPoint& GetTranslation() const;

		TIGL_EXPORT void SetScaling(const CTiglPoint& scaling);
		TIGL_EXPORT void SetRotation(const CTiglPoint& rotation);
		TIGL_EXPORT void SetTranslation(const CTiglPoint& translation);

		TIGL_EXPORT void Reset();

	private:
		TIGL_EXPORT void BuildMatrix();

		CTiglTransformation m_transformation;
		CTiglPoint m_t;
		CTiglPoint m_s;
		CTiglPoint m_r;
	};
}
