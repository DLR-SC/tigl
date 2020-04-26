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

#include "CCPACSEtaIsoLine.h"

#include "CTiglUIDManager.h"
#include "tigletaxsifunctions.h"
#include "CCPACSWingCellPositionSpanwise.h"
#include "generated/CPACSControlSurfaceAirfoil.h"
#include "CCPACSControlSurfaceBorderTrailingEdge.h"
#include "generated/CPACSControlSurfaceSkinCutOutBorder.h"
#include "generated/CPACSControlSurfaceTrackType.h"
#include "generated/CPACSCutOutProfile.h"
#include "generated/CPACSSparCell.h"

namespace tigl
{
CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSWingCellPositionSpanwise* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEtaIsoLine(parent, uidMgr)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSControlSurfaceAirfoil* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEtaIsoLine(parent, uidMgr)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSControlSurfaceBorderTrailingEdge* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEtaIsoLine(parent, uidMgr)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSControlSurfaceSkinCutOutBorder* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEtaIsoLine(parent, uidMgr)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSControlSurfaceTrackType* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEtaIsoLine(parent, uidMgr)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSCutOutProfile* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEtaIsoLine(parent, uidMgr)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSSparCell* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEtaIsoLine(parent, uidMgr)
{
}

void CCPACSEtaIsoLine::SetEta(const double& value)
{
    generated::CPACSEtaIsoLine::SetEta(value);
    InvalidateParent();
}

void CCPACSEtaIsoLine::SetReferenceUID(const std::string& value)
{
    generated::CPACSEtaIsoLine::SetReferenceUID(value);
    InvalidateParent();
}

//double CCPACSEtaIsoLine::ComputeCSOrTEDEta() const {
//    return transformEtaXsiToCSOrTed({ m_eta, 0 }, m_referenceUID, std::declval<CTiglUIDManager>()).eta; // TODO
//}

void CCPACSEtaIsoLine::InvalidateParent() const
{
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate();
    }
}

} // namespace tigl
