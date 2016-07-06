#include <sstream>

#include "CCPACSStringVector.h"

namespace tigl {
	namespace {
		const char sep = ';';

		std::vector<double> toDoubleVec(const std::string& s) {
			std::stringstream ss(s);
			std::vector<double> r;
			std::string d;
			while (std::getline(ss, d, sep)) {
				r.push_back(std::stod(d));
			}
			return r;
		}

		std::string toString(const std::vector<double>& v) {
			std::stringstream ss;
			for (const double& d : v) {
				ss << d;
				if (&d != &v.back())
					ss << sep;
			}
			return ss.str();
		}
	}

	void CCPACSStringVector::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string &xpath) {
		generated::CPACSStringVectorBase::ReadCPACS(tixiHandle, xpath);
		m_vec = toDoubleVec(m_simpleContent);

	}

	void CCPACSStringVector::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {
		const_cast<std::string&>(m_simpleContent) = toString(m_vec); // TODO: this is a terrible hack, but WriteCPACS() has to be const
		generated::CPACSStringVectorBase::WriteCPACS(tixiHandle, xpath);
	}

	const std::vector<double>& CCPACSStringVector::GetVector() const {
		return m_vec;
	}

	std::vector<double>& CCPACSStringVector::GetVector() {
		return m_vec;
	}
}
