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


#include "CCPACSPointListRelXYZ.h"

namespace tigl
{

void CCPACSPointListRelXYZ::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    generated::CPACSPointListRelXYZVector::ReadCPACS(tixiHandle, xpath);

    // create cached representation from CPACS fields
    const std::vector<double>& xs = GetRX().AsVector();
    const std::vector<double>& ys = GetRY().AsVector();
    const std::vector<double>& zs = GetRZ().AsVector();
    if (xs.size() != ys.size() || ys.size() != zs.size()) {
        throw CTiglError("component vectors in CCPACSPointListRelXYZ must all have the same number of elements");
    }
    m_vec.clear();
    for (std::size_t i = 0; i < xs.size(); i++) {
        m_vec.push_back(CTiglPoint(xs[i], ys[i], zs[i]));
    }
}

void CCPACSPointListRelXYZ::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
{
    // write back to CPACS fields
    CCPACSPointListRelXYZ* self =
        const_cast<CCPACSPointListRelXYZ*>(this); // TODO: ugly hack, but WriteCPACS() has to be const, fix this
    std::vector<double>& xs = self->GetRX().AsVector();
    std::vector<double>& ys = self->GetRY().AsVector();
    std::vector<double>& zs = self->GetRZ().AsVector();
    xs.clear();
    ys.clear();
    zs.clear();
    for (std::vector<CTiglPoint>::const_iterator it = m_vec.begin(); it != m_vec.end(); ++it) {
        xs.push_back(it->x);
        ys.push_back(it->y);
        zs.push_back(it->z);
    }

    generated::CPACSPointListRelXYZVector::WriteCPACS(tixiHandle, xpath);
}

const std::vector<CTiglPoint>& CCPACSPointListRelXYZ::AsVector() const
{
    return m_vec;
}

std::vector<CTiglPoint>& CCPACSPointListRelXYZ::AsVector()
{
    return m_vec;
}

} // namespace tigl
