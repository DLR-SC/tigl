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

#pragma once

#include <vector>

#include "generated/CPACSPointListXYZVector.h"
#include "CTiglPoint.h"

namespace tigl
{

class CCPACSPointListXYZ : public generated::CPACSPointListXYZVector
{
public:
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) OVERRIDE;
    TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const OVERRIDE;

    TIGL_EXPORT const std::vector<CTiglPoint>& AsVector() const;
    TIGL_EXPORT std::vector<CTiglPoint>& AsVector(); // WARN: if you modify the point list, make sure, the points are ordered

    TIGL_EXPORT void OrderPoints();

private:
    // cache
    std::vector<CTiglPoint> m_vec;
};

} // namespace tigl
