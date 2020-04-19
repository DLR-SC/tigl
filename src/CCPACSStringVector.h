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
#include "generated/CPACSStringVectorBase.h"

namespace tigl
{

TIGL_EXPORT std::vector<double> stringToDoubleVec(const std::string& s);
TIGL_EXPORT std::string doubleVecToString(const std::vector<double>& v);

class CCPACSStringVector : private generated::CPACSStringVectorBase
{
public:
    TIGL_EXPORT CCPACSStringVector(CCPACSWingProfileCST* parent);
    TIGL_EXPORT CCPACSStringVector(CCPACSEmissivityMap* parent);
    TIGL_EXPORT CCPACSStringVector(CCPACSPointListRelXYZVector* parent);
    TIGL_EXPORT CCPACSStringVector(CCPACSPointListXYVector* parent);
    TIGL_EXPORT CCPACSStringVector(CCPACSPointListXYZVector* parent);
    TIGL_EXPORT CCPACSStringVector(CCPACSRotorBladeAttachment* parent);
    TIGL_EXPORT CCPACSStringVector(CCPACSSpecificHeatMap* parent);

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;
    TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const override;

    using generated::CPACSStringVectorBase::GetMapType;
    using generated::CPACSStringVectorBase::SetMapType;

    TIGL_EXPORT const std::vector<double>& AsVector() const;
    TIGL_EXPORT void SetValue(int index, double value);
    TIGL_EXPORT void SetAsVector(const std::vector<double>& vec);

private:
    void InvalidateParent() const;

    // cache
    std::vector<double> m_vec;
};

} // namespace tigl
