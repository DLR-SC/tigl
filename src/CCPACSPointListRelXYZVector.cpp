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


#include "CCPACSPointListRelXYZVector.h"

namespace tigl
{

CCPACSPointListRelXYZVector::CCPACSPointListRelXYZVector(CCPACSGuideCurveProfile* parent)
: generated::CPACSPointListRelXYZVector(parent)
{}

void CCPACSPointListRelXYZVector::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
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

const std::vector<CTiglPoint>& CCPACSPointListRelXYZVector::AsVector() const
{
    return m_vec;
}

void CCPACSPointListRelXYZVector::SetAsVector(const std::vector<CTiglPoint>& points)
{
    m_vec = points;

    std::vector<double> x, y, z;
    for (std::vector<CTiglPoint>::const_iterator it = m_vec.begin(); it != m_vec.end(); ++it) {
        x.push_back(it->x);
        y.push_back(it->y);
        z.push_back(it->z);
    }
    GetRX().SetAsVector(x);
    GetRY().SetAsVector(y);
    GetRZ().SetAsVector(z);
    // no invalidation necessary, done by m_rX, m_rY, m_rZ
}

} // namespace tigl
