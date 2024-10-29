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
#include "CTiglError.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

namespace tigl
{

// Constructor
CCPACSGenericSystem::CCPACSGenericSystem(CCPACSGenericSystems* parent, CTiglUIDManager* uidMgr)
: generated::CPACSGenericSystem(parent, uidMgr)
, CTiglRelativelyPositionedComponent(static_cast<std::string*>(nullptr), &m_transformation, &m_symmetry)
{
}

// Destructor
CCPACSGenericSystem::~CCPACSGenericSystem()
{
}

std::string CCPACSGenericSystem::GetDefaultedUID() const {
    return GetUID();
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSGenericSystem::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

// build loft
PNamedShape CCPACSGenericSystem::BuildLoft() const
{
    TopoDS_Shape sysShape;
    if (m_geometricBaseType) {
        if (m_geometricBaseType.value() == ECPACSGenericSystem_geometricBaseType::cylinder) {
            BRepPrimAPI_MakeCylinder cyl(0.5, 1);
            gp_Vec vec(0, 0, -0.5);
            gp_Trsf trsf = gp_Trsf();
            trsf.SetTranslation(vec);
            TopLoc_Location loc(trsf);
            sysShape = cyl.Shape();
            sysShape.Location(loc);
        }
        else if (m_geometricBaseType.value() == ECPACSGenericSystem_geometricBaseType::sphere) {
            BRepPrimAPI_MakeSphere sph(0.5);
            sysShape = sph.Shape();
        }
        else if (m_geometricBaseType.value() == ECPACSGenericSystem_geometricBaseType::cone) {
            BRepPrimAPI_MakeCone cone(0.5, 0, 1);
            gp_Vec vec(0, 0, -0.5);
            gp_Trsf trsf = gp_Trsf();
            trsf.SetTranslation(vec);
            TopLoc_Location loc(trsf);
            sysShape = cone.Shape();
            sysShape.Location(loc);
        }
        else if (m_geometricBaseType.value() == ECPACSGenericSystem_geometricBaseType::cube) {
            gp_Pnt p1(-0.5, -0.5, -0.5);
            gp_Pnt p2(0.5, 0.5, 0.5);
            BRepPrimAPI_MakeBox cube(p1, p2);
            sysShape = cube.Shape();
        }
    }

    sysShape = GetTransformationMatrix().Transform(sysShape);
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(sysShape, loftName.c_str(), loftShortName.c_str()));

    return loft;
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
