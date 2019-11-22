/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-11-22 Martin Siggel <martin.siggel@dlr.de>

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


#include "CCPACSCurveParamPointMap.h"
#include <algorithm>
#include "to_string.h"
#include "CTiglLogging.h"

namespace tigl
{

CCPACSCurveParamPointMap::CCPACSCurveParamPointMap(CCPACSCurvePointListXYZ* parent)
    : generated::CPACSCurveParamPointMap(parent)
{
}

void CCPACSCurveParamPointMap::ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath)
{

    generated::CPACSCurveParamPointMap::ReadCPACS(tixiHandle, xpath);

    m_pointIndices.clear();
    m_pointParams.clear();

    const auto& indexAsDouble = GetPointIndex().AsVector();
    const auto& params = GetParamOnCurve().AsVector();

    if (params.size() != indexAsDouble.size()) {
        throw CTiglError("index and paramOnCurve in CCPACSCurveParamPointMap must all have the same number of elements");
    }

    m_pointParams = params;
    std::transform(std::begin(indexAsDouble), std::end(indexAsDouble), std::back_inserter(m_pointIndices), [](double v) {
        // TODO: check if really an unsigned int
        return static_cast<unsigned int>(v);
    });
}

std::vector<unsigned int> CCPACSCurveParamPointMap::GetPointIndexAsVector() const
{
    return m_pointIndices;
}

std::vector<double> CCPACSCurveParamPointMap::GetParamAsVector() const
{
    return m_pointParams;
}

void CCPACSCurveParamPointMap::Reverse(double minAllowedParam, double maxAllowedParam, size_t nPoints)
{
    unsigned int nPointsInt = static_cast<unsigned int>(nPoints);
    std::transform(std::begin(m_pointIndices), std::end(m_pointIndices), std::begin(m_pointIndices), [=](unsigned int oldIdx) {
        return nPointsInt - 1 - oldIdx;
    });

    std::transform(std::begin(m_pointParams), std::end(m_pointParams), std::begin(m_pointParams), [=](double oldParm) {
        return minAllowedParam + maxAllowedParam - oldParm;
    });

    auto pointIndexDouble = GetPointIndex().AsVector();
    std::transform(std::begin(m_pointIndices), std::end(m_pointIndices), std::begin(pointIndexDouble), [](int val) {
        return val;
    });
    GetPointIndex().SetAsVector(pointIndexDouble);

    GetParamOnCurve().SetAsVector(m_pointParams);
}

} // namespace tigl
