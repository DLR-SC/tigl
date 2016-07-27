
#include "CCPACSPointListRelXYZ.h"

namespace tigl {
	void CCPACSPointListRelXYZ::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string &xpath) {
		generated::CPACSPointListRelXYZVector::ReadCPACS(tixiHandle, xpath);

		// create cached representation from CPACS fields
		const auto& xs = GetRX().AsVector();
		const auto& ys = GetRY().AsVector();
		const auto& zs = GetRZ().AsVector();
		if (xs.size() != ys.size() || ys.size() != zs.size())
			throw std::runtime_error("component vectors in CCPACSPointListRelXYZ must all have the same number of elements");
		m_vec.clear();
		for (std::size_t i = 0; i < xs.size(); i++)
			m_vec.emplace_back(xs[i], ys[i], zs[i]);
	}

	void CCPACSPointListRelXYZ::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {
		// write back to CPACS fields
		auto self = const_cast<CCPACSPointListRelXYZ*>(this); // TODO: ugly hack, but WriteCPACS() has to be const, fix this
		auto& xs = self->GetRX().AsVector();
		auto& ys = self->GetRY().AsVector();
		auto& zs = self->GetRZ().AsVector();
		xs.clear();
		ys.clear();
		zs.clear();
		for (const auto& p : m_vec) {
			xs.push_back(p.x);
			ys.push_back(p.y);
			zs.push_back(p.z);
		}

		generated::CPACSPointListRelXYZVector::WriteCPACS(tixiHandle, xpath);
	}

	const std::vector<CTiglPoint>& CCPACSPointListRelXYZ::AsVector() const {
		return m_vec;
	}

	std::vector<CTiglPoint>& CCPACSPointListRelXYZ::AsVector() {
		return m_vec;
	}
}
