/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
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
/**
* @file
* @brief  Implementation of CPACS fuelTank handling routines.
*/

#include "CCPACSFuelTank.h"
#include "CCPACSVessel.h"
#include "CTiglError.h"
#include "CGroupShapes.h"
#include "CNamedShape.h"

namespace tigl
{

CCPACSFuelTank::CCPACSFuelTank(CCPACSFuelTanks* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuelTank(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
{
}

CCPACSConfiguration const& CCPACSFuelTank::GetConfiguration() const
{
    return GetParent()->GetParent()->GetConfiguration();
}

std::string CCPACSFuelTank::GetDefaultedUID() const
{
    return generated::CPACSFuelTank::GetUID();
}

TiglGeometricComponentType CCPACSFuelTank::GetComponentType() const
{
    return TIGL_COMPONENT_TANK;
}

TiglGeometricComponentIntent CCPACSFuelTank::GetComponentIntent() const
{
    // needs to be physical, so that transformation relative to parent works
    return TIGL_INTENT_PHYSICAL;
}

PNamedShape CCPACSFuelTank::BuildLoft() const
{
    const auto& vessels = GetVessels().GetVessels();
    ListPNamedShape shapes;

    for (const auto& vessel : vessels) {
        auto loft = vessel->GetLoft();
        shapes.push_back(loft);
    }

    PNamedShape groupedShape = CGroupShapes(shapes);
    groupedShape->SetName(GetUID().c_str());
    groupedShape->SetShortName(GetShortShapeName().c_str());

    return groupedShape;
}

std::string CCPACSFuelTank::GetShortShapeName() const
{
    unsigned int findex = 0;
    unsigned int i      = 0;

    for (auto& t : GetParent()->GetFuelTanks()) {
        ++i;
        if (GetUID() == t->GetUID()) {
            findex = i;
            std::stringstream shortName;
            shortName << "T" << findex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

void CCPACSFuelTank::RegisterInvalidationCallback(std::function<void()> const& fn){
    invalidationCallbacks.push_back(fn);
}

void CCPACSFuelTank::InvalidateImpl(const boost::optional<std::string>&) const
{
    CTiglAbstractGeometricComponent::Reset();
    for (auto const& invalidator: invalidationCallbacks) {
        invalidator();
    }
}

} //namespace tigl
