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

#include "CCPACSPointListXYZVector.h"

#include "CTiglLogging.h"
#include "CTiglError.h"
#include "CTiglUIDObject.h"

namespace tigl
{

CCPACSPointListXYZVector::CCPACSPointListXYZVector(CCPACSProfileGeometry* parent)
: generated::CPACSPointListXYZVector(parent) {}


void CCPACSPointListXYZVector::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    generated::CPACSPointListXYZVector::ReadCPACS(tixiHandle, xpath);

    // create cached representation from CPACS fields
    const std::vector<double>& xs = m_x.AsVector();
    const std::vector<double>& ys = m_y.AsVector();
    const std::vector<double>& zs = m_z.AsVector();
    if (xs.size() != ys.size() || ys.size() != zs.size()) {
        throw CTiglError("component vectors in CCPACSPointListXYZ must all have the same number of elements");
    }
    m_vec.clear();
    for (std::size_t i = 0; i < xs.size(); i++) {
        m_vec.push_back(CTiglPoint(xs[i], ys[i], zs[i]));
    }
}

const std::vector<CTiglPoint>& CCPACSPointListXYZVector::AsVector() const
{
    return m_vec;
}

void CCPACSPointListXYZVector::SetValue(int index, const CTiglPoint& point)
{
    m_x.SetValue(index, point.x);
    m_y.SetValue(index, point.y);
    m_z.SetValue(index, point.z);
    // no invalidation necessary, done by m_x, m_y, m_z
}

void CCPACSPointListXYZVector::SetAsVector(const std::vector<CTiglPoint>& points)
{
    m_vec = points;
    std::vector<double> x, y, z;

    for (std::vector<CTiglPoint>::const_iterator it = m_vec.begin(); it != m_vec.end(); ++it) {
        x.push_back(it->x);
        y.push_back(it->y);
        z.push_back(it->z);
    }
    m_x.SetAsVector(x);
    m_y.SetAsVector(y);
    m_z.SetAsVector(z);
    // no invalidation necessary, done by m_x, m_y, m_z
}

} // namespace tigl
