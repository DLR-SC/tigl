
#include "CCPACSPointListXYZ.h"

namespace tigl {
	void CCPACSPointListXYZ::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string &xpath) {
		generated::CPACSPointListXYZVector::ReadCPACS(tixiHandle, xpath);
		
		// create cached representation from CPACS fields
		const auto& xs = GetX().AsVector();
		const auto& ys = GetY().AsVector();
		const auto& zs = GetZ().AsVector();
		if (xs.size() != ys.size() || ys.size() != zs.size())
			throw std::runtime_error("component vectors in CCPACSPointListXYZ must all have the same number of elements");
		m_vec.clear();
		for (std::size_t i = 0; i < xs.size(); i++)
			m_vec.emplace_back(xs[i], ys[i], zs[i]);
	}

	void CCPACSPointListXYZ::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {
		// write back to CPACS fields
		auto self = const_cast<CCPACSPointListXYZ*>(this); // TODO: ugly hack, but WriteCPACS() has to be const, fix this
		auto& xs = self->GetX().AsVector();
		auto& ys = self->GetY().AsVector();
		auto& zs = self->GetZ().AsVector();
		xs.clear();
		ys.clear();
		zs.clear();
		for (const auto& p : m_vec) {
			xs.push_back(p.x);
			ys.push_back(p.y);
			zs.push_back(p.z);
		}

		generated::CPACSPointListXYZVector::WriteCPACS(tixiHandle, xpath);
	}

	const std::vector<CTiglPoint>& CCPACSPointListXYZ::AsVector() const {
		return m_vec;
	}

	std::vector<CTiglPoint>& CCPACSPointListXYZ::AsVector() {
		return m_vec;
	}
}
