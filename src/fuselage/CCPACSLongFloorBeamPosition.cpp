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

#include "CCPACSLongFloorBeamPosition.h"

#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include "CCPACSCrossBeamAssemblyPosition.h"
#include "CCPACSLongFloorBeam.h"
#include "CCPACSFuselage.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"

namespace tigl
{
CCPACSLongFloorBeamPosition::CCPACSLongFloorBeamPosition(CCPACSLongFloorBeam* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSLongFloorBeamPosition(parent, uidMgr)
    , m_cutGeometryForCrossBeam(*this, &CCPACSLongFloorBeamPosition::BuildCutGeometryForCrossBeam)
{
}

void CCPACSLongFloorBeamPosition::SetPositionY(const double& value) {
    generated::CPACSLongFloorBeamPosition::SetPositionY(value);
    Invalidate();
}

void CCPACSLongFloorBeamPosition::Invalidate()
{
    m_cutGeometryForCrossBeam.clear();
    m_parent->Invalidate();
}

gp_Pnt CCPACSLongFloorBeamPosition::GetCrossBeamIntersection(TiglCoordinateSystem cs) const {
    CCPACSCrossBeamAssemblyPosition& crossBeam =
        m_uidMgr->ResolveObject<CCPACSCrossBeamAssemblyPosition>(m_crossBeamUID);

    const TopoDS_Wire crossBeamWire = TopoDS::Wire(crossBeam.GetGeometry(true, FUSELAGE_COORDINATE_SYSTEM));

    gp_Pnt intersection;
    if (!GetIntersectionPoint(GetCutGeometryForCrossBeam(), crossBeamWire, intersection)) {
        throw tigl::CTiglError("Error: long floor beam position y is not on cross beam", TIGL_XML_ERROR);
    }

    if (cs == GLOBAL_COORDINATE_SYSTEM)
        return m_parent->GetParent()->GetParent()->GetParent()->GetTransformationMatrix().Transform(intersection);
    else
        return intersection;
}

TopoDS_Face CCPACSLongFloorBeamPosition::GetCutGeometryForCrossBeam(TiglCoordinateSystem cs) const
{
    const TopoDS_Face face = *m_cutGeometryForCrossBeam;
    if (cs == GLOBAL_COORDINATE_SYSTEM)
        return TopoDS::Face(m_parent->GetParent()->GetParent()->GetParent()->GetTransformationMatrix().Transform(face));
    else
        return face;
}

void CCPACSLongFloorBeamPosition::BuildCutGeometryForCrossBeam(TopoDS_Face& cache) const
{
    const gp_Pln yCutPlane(gp_Ax3(gp_Pnt(0., m_positionY, 0.), gp_Vec(0., -1., 0.), gp_Vec(1., 0., 0.)));
    cache = BRepBuilderAPI_MakeFace(yCutPlane).Face();
}
} // namespace tigl
