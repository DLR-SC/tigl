#pragma once

#include <vector>
#include "generated/CPACSStringVectorBase.h"

namespace tigl {
	class CCPACSStringVector : private generated::CPACSStringVectorBase {
	public:
		TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string &xpath);
		TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

		const std::vector<double>& GetVector() const;
		std::vector<double>& GetVector();

	private:
		std::vector<double> m_vec;
	};
}