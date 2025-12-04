/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSControlSurfaces.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSLeadingEdgeDevice.h"

namespace tigl
{

CCPACSControlSurfaces::CCPACSControlSurfaces(CCPACSWingComponentSegment* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaces(parent, uidMgr)
{
}

size_t CCPACSControlSurfaces::ControlSurfaceCount() const
{
    size_t count = 0;
    if (GetTrailingEdgeDevices()) {
        count += GetTrailingEdgeDevices()->GetTrailingEdgeDevices().size();
    }
    if (GetLeadingEdgeDevices()) {
        count += GetLeadingEdgeDevices()->GetLeadingEdgeDevices().size();
    }

    return count;
}

void CCPACSControlSurfaces::GetFusedControlSurfaceCutOutShape(PNamedShape& fusedBoxes) const
{
    bool first = false;
    if (!fusedBoxes)
    {
        first = true;
    }

    if (GetTrailingEdgeDevices()) {


        for (size_t ted_index = 0; ted_index < GetTrailingEdgeDevices()->GetTrailingEdgeDevices().size(); ted_index++) {
            CCPACSTrailingEdgeDevice& TrailingEdgeDevice = *GetTrailingEdgeDevices()->GetTrailingEdgeDevices().at(ted_index);

            PNamedShape TrailingEdgeDevicePrism = TrailingEdgeDevice.GetCutOutShape();
                if (!first) {
                    ListPNamedShape childs;
                    childs.push_back(TrailingEdgeDevicePrism);
                    fusedBoxes = CFuseShapes(fusedBoxes, childs);
                }
                else {
                    first      = false;
                    fusedBoxes = TrailingEdgeDevicePrism;
                }
            // trigger build of the flap
            TrailingEdgeDevice.GetLoft();
        }
    }
    if (GetLeadingEdgeDevices()) {

        for (size_t led_index = 0; led_index < GetLeadingEdgeDevices()->GetLeadingEdgeDevices().size(); led_index++) {
            CCPACSLeadingEdgeDevice& LeadingEdgeDevice = *GetLeadingEdgeDevices()->GetLeadingEdgeDevices().at(led_index);

            PNamedShape LeadingEdgeDevicePrism = LeadingEdgeDevice.GetCutOutShape();

            if (!first) {
                ListPNamedShape childs;
                childs.push_back(LeadingEdgeDevicePrism);
                fusedBoxes = CFuseShapes(fusedBoxes, childs);
            }
            else {
                first      = false;
                fusedBoxes = LeadingEdgeDevicePrism;
            }
            
            // trigger build of the flap
            LeadingEdgeDevice.GetLoft();
        }
    }
}

void CCPACSControlSurfaces::GetFlapsShapes(ListPNamedShape& flapsAndWingShapes) const
{
    if (GetTrailingEdgeDevices()) {
        const auto& TED                 = GetTrailingEdgeDevices()->GetTrailingEdgeDevices();
        for (size_t ted_index = 0; ted_index < TED.size(); ted_index++) {

            const auto& TrailingEdgeDevice = *TED.at(ted_index);
            auto TEDShape                  = TrailingEdgeDevice.GetTransformedFlapShape();
            flapsAndWingShapes.push_back(TEDShape);
        }
    }
    if (GetLeadingEdgeDevices()) {
        const auto& LED                = GetLeadingEdgeDevices()->GetLeadingEdgeDevices();
        for (size_t led_index = 0; led_index < LED.size(); led_index++) {

            const auto& LeadingEdgeDevice = *LED.at(led_index);
            auto LEDShape                 = LeadingEdgeDevice.GetTransformedFlapShape();
            flapsAndWingShapes.push_back(LEDShape);
        }
    }
}
} // namespace tigl
