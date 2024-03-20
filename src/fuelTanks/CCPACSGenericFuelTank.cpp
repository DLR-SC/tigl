/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-04-06 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief  Implementation of CPACS genericFuelTank handling routines.
*/

#include "CCPACSGenericFuelTank.h"
#include "CCPACSHull.h"
#include "CTiglError.h"
#include "CGroupShapes.h"

namespace tigl {

CCPACSGenericFuelTank::CCPACSGenericFuelTank(CCPACSGenericFuelTanks* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSGenericFuelTank(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(GetParent()->GetParent()->GetParent(), &m_transformation)
{}

CCPACSConfiguration& CCPACSGenericFuelTank::GetConfiguration() const
{
    return GetParent()->GetParent()->GetParent()->GetConfiguration();
}

std::string CCPACSGenericFuelTank::GetDefaultedUID() const
{
    return generated::CPACSGenericFuelTank::GetUID();
}

TiglGeometricComponentType CCPACSGenericFuelTank::GetComponentType() const
{
    return TIGL_COMPONENT_FUSELAGE_TANK;
}

TiglGeometricComponentIntent CCPACSGenericFuelTank::GetComponentIntent() const
{
    // needs to be physical, so that transformation relative to parent works
    return TIGL_INTENT_PHYSICAL;
}

bool CCPACSGenericFuelTank::HasHulls() const
{
    if (GetHulls_choice1()->GetHullsCount()>0) {
        return true;
    }
    else {
        return false;
    }
}

boost::optional<CCPACSHulls>& CCPACSGenericFuelTank::GetHulls()
{
    if (HasHulls()) {
        return GetHulls_choice1();
    }
    else {
        throw CTiglError("No hulls defined.");
    }
}

const boost::optional<CCPACSHulls>& CCPACSGenericFuelTank::GetHulls() const
{
    if (HasHulls()) {
        return GetHulls_choice1();
    }
    else {
        throw CTiglError("No hulls defined.");
    }
}

PNamedShape CCPACSGenericFuelTank::BuildLoft() const
{
    const auto& hulls = GetHulls_choice1()->GetHulls();
    ListPNamedShape shapes;    

    for (const auto& hull : hulls) {
        auto loft = hull->GetLoft();
        shapes.push_back(loft);
    }

    PNamedShape groupedShape = CGroupShapes(shapes);
    
    return groupedShape;
}

std::string CCPACSGenericFuelTank::GetShortShapeName() const
{
    unsigned int findex = 0;
    unsigned int i = 0;

    for (auto& t: GetParent()->GetGenericFuelTanks()) {
        ++i;
        if (GetUID() == t->GetUID()) {
            findex = i;
            std::stringstream shortName;
            shortName << "D" << findex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

} //namespace tigl
