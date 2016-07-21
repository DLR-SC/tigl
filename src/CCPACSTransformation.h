#pragma once

#include "generated/CPACSTransformation.h"

#include "CTiglTransformation.h"
#include "ECPACSTranslationType.h"

namespace tigl {
	class CCPACSTransformation : public generated::CPACSTransformation {
	public:
		TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

		TIGL_EXPORT const CTiglTransformation& AsTransformation() const;

		TIGL_EXPORT ECPACSTranslationType GetTranslationType() const;

		TIGL_EXPORT CTiglPoint GetScaling() const;
		TIGL_EXPORT CTiglPoint GetRotation() const;
		TIGL_EXPORT CTiglPoint GetTranslation() const;

		TIGL_EXPORT void SetScaling(const CTiglPoint& scaling);
		TIGL_EXPORT void SetRotation(const CTiglPoint& rotation);
		TIGL_EXPORT void SetTranslation(const CTiglPoint& translation);

		TIGL_EXPORT void Reset();

	private:
		void BuildMatrix();

		// caches the transformation created from scaling, rotation and translation
		CTiglTransformation m_transformation;
	};
}
