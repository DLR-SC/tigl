/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#include "CCPACSLongFloorBeam.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakePrism.hxx>

#include "generated/CPACSStructuralProfile.h"
#include "CCPACSFuselage.h"
#include "CCPACSSheetList.h"
#include "CCPACSLongFloorBeamPosition.h"
#include "CCPACSProfileBasedStructuralElement.h"
#include "CTiglUIDManager.h"
#include "CNamedShape.h"

namespace tigl
{
CCPACSLongFloorBeam::CCPACSLongFloorBeam(CCPACSLongFloorBeamsAssembly* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSLongFloorBeam(parent, uidMgr)
    , m_geometry1D(*this, &CCPACSLongFloorBeam::BuildGeometry1D)
    , m_geometry3D(*this, &CCPACSLongFloorBeam::BuildGeometry3D)
{
}

std::string CCPACSLongFloorBeam::GetDefaultedUID() const
{
    return GetUID();
}

PNamedShape CCPACSLongFloorBeam::GetLoft() const
{
    return PNamedShape(new CNamedShape(GetGeometry(true), GetUID()));
}

TiglGeometricComponentType CCPACSLongFloorBeam::GetComponentType() const
{
    return TIGL_COMPONENT_LONG_FLOOR_BEAM;
}

TiglGeometricComponentIntent CCPACSLongFloorBeam::GetComponentIntent() const
{
    return TIGL_INTENT_PHYSICAL | TIGL_INTENT_INNER_STRUCTURE;
}

void CCPACSLongFloorBeam::Invalidate()
{
    m_geometry1D.clear();
    m_geometry3D.clear();
}

TopoDS_Shape CCPACSLongFloorBeam::GetGeometry(bool just1DElements, TiglCoordinateSystem cs) const
{
    TopoDS_Shape shape = just1DElements ? *m_geometry1D : *m_geometry3D;
    if (cs == TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM) {
        return m_parent->GetParent()->GetParent()->GetTransformationMatrix().Transform(shape);
    }
    else {
        return shape;
    }
}

void CCPACSLongFloorBeam::BuildGeometry1D(TopoDS_Shape& cache) const
{
    BuildGeometry(cache, true);
}

void CCPACSLongFloorBeam::BuildGeometry3D(TopoDS_Shape& cache) const
{
    BuildGeometry(cache, false);
}

void CCPACSLongFloorBeam::BuildGeometry(TopoDS_Shape& cache, bool just1DElements) const
{
    // get long floor beam position points on the cross beams
    std::vector<gp_Pnt> points;
    for (std::size_t i = 0; i < m_longFloorBeamPositions.size(); i++) {
        points.push_back(m_longFloorBeamPositions[i]->GetCrossBeamIntersection(FUSELAGE_COORDINATE_SYSTEM));
    }

    // connect points by edges
    BRepBuilderAPI_MakeWire wire;
    for (size_t i = 1; i < points.size(); i++) {
        wire.Add(BRepBuilderAPI_MakeEdge(points[i - 1], points[i]));
    }

    if (just1DElements) {
        cache = wire;
    }
    else {
        TopoDS_Compound compound;
        TopoDS_Builder builder;
        builder.MakeCompound(compound);

        // sweep the profiles between each pair of positions
        for (size_t i = 1; i < points.size(); i++) {
            const CCPACSProfileBasedStructuralElement& pbse =
                m_uidMgr->ResolveObject<CCPACSProfileBasedStructuralElement>(
                    m_longFloorBeamPositions[i - 1]->GetStructuralElementUID());
            const CCPACSSheetList& sheetList =
                m_uidMgr->ResolveObject<CCPACSStructuralProfile>(pbse.GetStructuralProfileUID_choice1().value())
                    .GetSheetList();

            const gp_Pnt& pnt1 = points[i - 1];
            const gp_Pnt& pnt2 = points[i - 0];

            const gp_Vec vector(pnt1, pnt2);
            const gp_Pln profilePlan(gp_Ax3(pnt1, vector, gp_Vec(0., 1., 0.)));

            BRepPrimAPI_MakePrism frameShell(sheetList.CreateProfileWire(profilePlan), vector);
            frameShell.Build();
            if (!frameShell.IsDone()) {
                throw CTiglError("Error during the frame sweeping", TIGL_XML_ERROR);
            }
            builder.Add(compound, frameShell.Shape());
        }
        cache = compound;
    }
}

} // namespace tigl
