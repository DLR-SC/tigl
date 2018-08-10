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

#include "CCPACSPointListXYZ.h"

#include "CTiglLogging.h"
#include "CTiglError.h"

namespace tigl
{

void CCPACSPointListXYZ::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
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

    OrderPoints();
}

void CCPACSPointListXYZ::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
{
    // write back to CPACS fields
    CCPACSPointListXYZ* self =
        const_cast<CCPACSPointListXYZ*>(this); // TODO: ugly hack, but WriteCPACS() has to be const, fix this
    std::vector<double>& xs = self->m_x.AsVector();
    std::vector<double>& ys = self->m_y.AsVector();
    std::vector<double>& zs = self->m_z.AsVector();
    xs.clear();
    ys.clear();
    zs.clear();
    for (std::vector<CTiglPoint>::const_iterator it = m_vec.begin(); it != m_vec.end(); ++it) {
        xs.push_back(it->x);
        ys.push_back(it->y);
        zs.push_back(it->z);
    }

    generated::CPACSPointListXYZVector::WriteCPACS(tixiHandle, xpath);
}

const std::vector<CTiglPoint>& CCPACSPointListXYZ::AsVector() const
{
    return m_vec;
}

std::vector<CTiglPoint>& CCPACSPointListXYZ::AsVector()
{
    return m_vec;
}

void CCPACSPointListXYZ::OrderPoints()
{
    // points with maximal/minimal y-component
    std::size_t minZIndex = 0;
    std::size_t maxZIndex = 0;
    for (std::size_t i = 1; i < m_vec.size(); i++) {
        if (m_vec[i].z < m_vec[minZIndex].z) {
            minZIndex = i;
        }
        if (m_vec[i].z > m_vec[maxZIndex].z) {
            maxZIndex = i;
        }
    }

    // check if points with maximal/minimal z-component were calculated correctly
    if (maxZIndex == minZIndex) {
        throw CTiglError("CCPACSPointListXYZ::OrderPoints: Unable to separate upper and lower wing profile from point list", TIGL_XML_ERROR);
    }
    // force order of points to run through the lower profile first and then through the upper profile
    if (minZIndex > maxZIndex) {
        LOG(WARNING) << "The points don't seem to be ordered in a mathematical positive sense.";
        std::reverse(m_vec.begin(), m_vec.end());
    }
}

} // namespace tigl
