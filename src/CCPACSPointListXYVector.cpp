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

#include "CCPACSPointListXYVector.h"

#include "CTiglLogging.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSPointListXYVector::CCPACSPointListXYVector(CCPACSNacelleProfile* parent)
: generated::CPACSPointListXYVector(parent)
{}


void CCPACSPointListXYVector::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    generated::CPACSPointListXYVector::ReadCPACS(tixiHandle, xpath);

    // create cached representation from CPACS fields
    const std::vector<double>& xs = m_x.AsVector();
    const std::vector<double>& ys = m_y.AsVector();
    if (xs.size() != ys.size() ) {
        throw CTiglError("component vectors in CCPACSPointListXYZ must all have the same number of elements");
    }
    m_vec.clear();
    for (std::size_t i = 0; i < xs.size(); i++) {
        m_vec.push_back(CTiglPoint(xs[i], ys[i], 0.0));
    }
}

const std::vector<CTiglPoint>& CCPACSPointListXYVector::AsVector() const
{
    return m_vec;
}

void CCPACSPointListXYVector::SetAsVector(const std::vector<CTiglPoint>& points)
{
    m_vec = points;
    std::vector<double> x, y;
    for (std::vector<CTiglPoint>::const_iterator it = m_vec.begin(); it != m_vec.end(); ++it) {
        x.push_back(it->x);
        y.push_back(it->y);
    }
    m_x.SetAsVector(x);
    m_y.SetAsVector(y);
    // no invalidation necessary, done by m_x and m_y

}

} // namespace tigl
