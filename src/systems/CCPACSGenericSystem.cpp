/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-21 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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
* @brief  Implementation of CPACS wing handling routines.
*/

#include <iostream>
#include <algorithm>

#include "CCPACSGenericSystem.h"
#include "CCPACSComponent.h"
#include "CTiglError.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"
#include "CGroupShapes.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

namespace tigl
{

// Constructor
CCPACSGenericSystem::CCPACSGenericSystem(CCPACSGenericSystems* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSGenericSystem(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(static_cast<std::string*>(nullptr),
                                         static_cast<CCPACSTransformation*>(nullptr), &m_symmetry)
{
}

// Destructor
CCPACSGenericSystem::~CCPACSGenericSystem()
{
}

std::string CCPACSGenericSystem::GetDefaultedUID() const
{
    return GetUID();
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSGenericSystem::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

double CCPACSGenericSystem::GetMassAllComponents() const
{
    const auto& comps = GetComponents();
    const auto n      = comps.GetComponentCount();

    double total_mass = 0.0;
    for (size_t i = 1; i <= n; ++i) {
        const auto& c = comps.GetComponent(i);
        total_mass += c.GetMass().get_value_or(0);
    }
    return total_mass;
}

double CCPACSGenericSystem::GetMassPositionedComponents() const
{
    const auto& comps = GetComponents();
    const auto n      = comps.GetComponentCount();

    double total_mass = 0.0;
    for (size_t i = 1; i <= n; ++i) {
        const auto& c = comps.GetComponent(i);
        if (c.IsPositioned()) {
            total_mass += c.GetMass().get_value_or(0);
        }
    }
    return total_mass;
}

boost::optional<tigl::CTiglPoint> CCPACSGenericSystem::GetCenterOfGravity() const
{
    const auto& comps = GetComponents();
    const auto n      = comps.GetComponentCount();

    double mSum = 0.0;
    double xSum = 0.0, ySum = 0.0, zSum = 0.0;

    for (size_t i = 1; i <= n; ++i) {
        const auto& c = comps.GetComponent(i);

        // Only consider components which are positioned, have a proper mass description, and a global CoG
        if (!c.IsPositioned()) {
            continue;
        }

        const auto m = c.GetMass();
        if (!m || *m <= 0.0) {
            continue;
        }

        const auto cogG = c.GetCenterOfGravityGlobal();
        if (!cogG) {
            continue;
        }

        mSum += *m;
        xSum += (*m) * cogG->x;
        ySum += (*m) * cogG->y;
        zSum += (*m) * cogG->z;
    }

    if (mSum <= 0.0) {
        return boost::none;
    }

    return tigl::CTiglPoint(xSum / mSum, ySum / mSum, zSum / mSum);
}

// build loft
PNamedShape CCPACSGenericSystem::BuildLoft() const
{
    const auto& components = GetComponents().GetComponents();
    ListPNamedShape shapes;

    for (const auto& component : components) {
        auto loft = component->GetLoft();
        shapes.push_back(loft);
    }

    PNamedShape groupedShape = CGroupShapes(shapes);
    groupedShape->SetName(GetUID().c_str());
    groupedShape->SetShortName(GetShortShapeName().c_str());

    return groupedShape;
}

// get short name for loft
std::string CCPACSGenericSystem::GetShortShapeName() const
{
    unsigned int gsindex = 0;
    for (int i = 1; i <= GetConfiguration().GetGenericSystemCount(); ++i) {
        const tigl::CCPACSGenericSystem& gs = GetConfiguration().GetGenericSystem(i);
        if (GetUID() == gs.GetUID()) {
            gsindex = i;
            std::stringstream shortName;
            shortName << "GS" << gsindex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

} // end namespace tigl
