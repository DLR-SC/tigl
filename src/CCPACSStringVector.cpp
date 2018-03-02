/*
* Copyright (c) 2018 RISC Software GmbH
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

#include <sstream>
#include <cstdlib>

#include "CCPACSStringVector.h"

namespace tigl
{

namespace
{
    const char sep = ';';
}

std::vector<double> stringToDoubleVec(const std::string& s)
{
    std::stringstream ss(s);
    std::vector<double> r;
    std::string d;
    while (std::getline(ss, d, sep)) {
        r.push_back(std::strtod(d.c_str(), NULL));
    }
    return r;
}

std::string doubleVecToString(const std::vector<double>& v)
{
    std::stringstream ss;
    for (std::vector<double>::const_iterator it = v.begin(); it != v.end(); ++it) {
        ss << *it;
        if (it != v.end() - 1) {
            ss << sep;
        }
    }
    return ss.str();
}

void CCPACSStringVector::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    generated::CPACSStringVectorBase::ReadCPACS(tixiHandle, xpath);
    m_vec = stringToDoubleVec(m_simpleContent);
}

void CCPACSStringVector::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
{
    const_cast<std::string&>(m_simpleContent) =
        doubleVecToString(m_vec); // TODO: this is a terrible hack, but WriteCPACS() has to be const
    generated::CPACSStringVectorBase::WriteCPACS(tixiHandle, xpath);
}

const std::vector<double>& CCPACSStringVector::AsVector() const
{
    return m_vec;
}

std::vector<double>& CCPACSStringVector::AsVector()
{
    return m_vec;
}

} // namespace tigl
