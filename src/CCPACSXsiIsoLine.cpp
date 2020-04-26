// Copyright (c) 2018 RISC Software GmbH
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "CCPACSXsiIsoLine.h"

#include "CTiglUIDManager.h"
#include "tigletaxsifunctions.h"

#include "CCPACSControlSurfaceBorderTrailingEdge.h"

namespace tigl
{
CCPACSXsiIsoLine::CCPACSXsiIsoLine(CCPACSControlSurfaceBorderTrailingEdge* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSXsiIsoLine(parent, uidMgr)
{
}

void CCPACSXsiIsoLine::SetXsi(const double& value)
{
    generated::CPACSXsiIsoLine::SetXsi(value);
    InvalidateParent();
}

void CCPACSXsiIsoLine::SetReferenceUID(const std::string& value)
{
    generated::CPACSXsiIsoLine::SetReferenceUID(value);
    InvalidateParent();
}

//double CCPACSXsiIsoLine::ComputeCSOrTEDXsi() const {
//    return transformEtaXsiToCSOrTed({ 0, m_xsi}, m_referenceUID, std::declval<CTiglUIDManager>()).xsi; // TODO
//}

void CCPACSXsiIsoLine::InvalidateParent() const
{
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate();
    }
}

} // namespace tigl
