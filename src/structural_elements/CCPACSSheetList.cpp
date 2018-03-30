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

#include "CCPACSSheetList.h"

#include <TopoDS_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <GeomAPI.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>

#include "generated/CPACSSheet.h"
#include "CCPACSPointXY.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"

namespace tigl
{
CCPACSSheetList::CCPACSSheetList(CCPACSStructuralProfile* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSheetList(parent, uidMgr)
{
}

TopoDS_Wire CCPACSSheetList::CreateProfileWire(const gp_Pln& profilePlane) const
{
    TopoDS_Compound c;
    TopoDS_Builder builder;
    builder.MakeCompound(c);

    for (int i = 0; i < m_sheets.size(); i++) {
        const CCPACSSheet& sheet = *m_sheets[i];

        // get the points to link
        const gp_Pnt2d& p1 = m_uidMgr->ResolveObject<CCPACSPointXY>(sheet.GetFromPointUID()).As_gp_Pnt();
        const gp_Pnt2d& p2 = m_uidMgr->ResolveObject<CCPACSPointXY>(sheet.GetToPointUID()).As_gp_Pnt();

        // create the 2D segment and tranform into 3D curve
        TopoDS_Edge segment =
            BRepBuilderAPI_MakeEdge(GeomAPI::To3d(GCE2d_MakeSegment(p1, p2).Value(), profilePlane)).Edge();

        builder.Add(c, segment);
    }

    return BuildWireFromEdges(c);
}

} // namespace tigl
