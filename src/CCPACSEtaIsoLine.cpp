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
#include "CCPACSControlSurfaceSkinCutOutBorder.h"
#include "CCPACSControlSurfaceTrackType.h"
#include "generated/CPACSCutOutProfile.h"
#include "generated/CPACSSparCell.h"

namespace tigl
{
CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSWingCellPositionSpanwise* parent)
    : generated::CPACSEtaIsoLine(parent)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSControlSurfaceAirfoil* parent)
    : generated::CPACSEtaIsoLine(parent)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSControlSurfaceBorderTrailingEdge* parent)
    : generated::CPACSEtaIsoLine(parent)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSControlSurfaceSkinCutOutBorder* parent)
    : generated::CPACSEtaIsoLine(parent)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSControlSurfaceTrackType* parent)
    : generated::CPACSEtaIsoLine(parent)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSCutOutProfile* parent)
    : generated::CPACSEtaIsoLine(parent)
{
}

CCPACSEtaIsoLine::CCPACSEtaIsoLine(CCPACSSparCell* parent)
    : generated::CPACSEtaIsoLine(parent)
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

void CCPACSEtaIsoLine::InvalidateParent()
{
    //if (IsParent<CCPACSWingCellPositionSpanwise        >()) GetParent<CCPACSWingCellPositionSpanwise        >()->Invalidate();
    //if (IsParent<CCPACSControlSurfaceAirfoil           >()) GetParent<CCPACSControlSurfaceAirfoil           >()->Invalidate();
    //if (IsParent<CCPACSControlSurfaceBorderTrailingEdge>()) GetParent<CCPACSControlSurfaceBorderTrailingEdge>()->Invalidate();
    //if (IsParent<CCPACSControlSurfaceSkinCutOutBorder  >()) GetParent<CCPACSControlSurfaceSkinCutOutBorder  >()->Invalidate();
    //if (IsParent<CCPACSControlSurfaceTrackType         >()) GetParent<CCPACSControlSurfaceTrackType         >()->Invalidate();
    //if (IsParent<CCPACSCutOutProfile                   >()) GetParent<CCPACSCutOutProfile                   >()->Invalidate();
    //if (IsParent<CCPACSSparCell                        >()) GetParent<CCPACSSparCell                        >()->Invalidate();
}

} // namespace tigl
