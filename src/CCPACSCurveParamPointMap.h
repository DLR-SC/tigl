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

#ifndef CCPACSCURVEPARAMPOINTMAP_H
#define CCPACSCURVEPARAMPOINTMAP_H

#include "generated/CPACSCurveParamPointMap.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSCurveParamPointMap : public generated::CPACSCurveParamPointMap
{
public:
    TIGL_EXPORT CCPACSCurveParamPointMap(CCPACSCurvePointListXYZ* parent);

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath) override;

    TIGL_EXPORT std::vector<unsigned int> GetPointIndexAsVector() const;
    TIGL_EXPORT std::vector<double> GetParamAsVector() const;

    TIGL_EXPORT void Reverse(double minAllowedParam, double maxAllowedParam, size_t nPoints);

private:
    std::vector<unsigned int> m_pointIndices;
    std::vector<double> m_pointParams;
};

} // namespace tigl

#endif // CCPACSCURVEPARAMPOINTMAP_H
