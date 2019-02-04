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

#ifndef CCPACSETAISOLINE
#define CCPACSETAISOLINE

#include "generated/CPACSEtaIsoLine.h"

namespace tigl
{

class CCPACSEtaIsoLine : public generated::CPACSEtaIsoLine
{
public:
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSWingCellPositionSpanwise* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSControlSurfaceAirfoil* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSControlSurfaceBorderTrailingEdge* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSControlSurfaceSkinCutOutBorder* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSControlSurfaceTrackType* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSCutOutProfile* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSSparCell* parent);

    TIGL_EXPORT virtual void SetEta(const double& value);
    TIGL_EXPORT virtual void SetReferenceUID(const std::string& value);

    //TIGL_EXPORT double ComputeCSOrTEDEta() const;

private:
    void InvalidateParent();
};

} // namespace tigl

#endif // CCPACSETAISOLINE
