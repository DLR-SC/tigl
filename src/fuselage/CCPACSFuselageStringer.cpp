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

#include "CCPACSFuselageStringer.h"

#include <gp_Pln.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <GeomLProp_SLProps.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include "tiglcommonfunctions.h"
#include "CCPACSProfileBasedStructuralElement.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CCPACSFrame.h"

namespace tigl
{
CCPACSFuselageStringer::CCPACSFuselageStringer(CCPACSStringersAssembly* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSStringer(parent, uidMgr)
{
}

void CCPACSFuselageStringer::Invalidate()
{
    for (int i = 0; i < 2; ++i) {
        m_geomCache[i] = boost::none;
    }
}

TopoDS_Shape CCPACSFuselageStringer::GetGeometry(bool just1DElements, TiglCoordinateSystem cs)
{
    if (!m_geomCache[just1DElements]) {
        BuildGeometry(just1DElements);
    }

    const TopoDS_Shape shape = m_geomCache[just1DElements].value();
    if (cs == TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM) {
        CTiglTransformation trafo = m_parent->GetParent()->GetParent()->GetTransformationMatrix();
        return trafo.Transform(shape);
    }
    else
        return shape;
}

void CCPACSFuselageStringer::BuildGeometry(bool just1DElements)
{
    if (m_stringerPositions.size() < 2)
        throw CTiglError("Cannot build stringer geometry, less than 2 stringer positions defined in XML",
                         TIGL_XML_ERROR);

    // Build the geometry of the inner structure in 2 steps:
    // 1) path definition (projection on the fuselage)
    // 2) if not just 1D element, build and sweep the profile all along the path

    // -1) place every points in the fuselage loft
    CCPACSFuselage& fuselage  = *m_parent->GetParent()->GetParent();
    std::vector<gp_Lin> pointList;
    for (size_t i = 0; i < m_stringerPositions.size(); i++) {
        pointList.push_back(fuselage.Intersection(*m_stringerPositions[i]));
    }

    // creation of the profile plane
    // the plan axis system is defined by : the location, the normal vector, the x vector. It's a right-handed system
    // the rotation is necessary to have a normal vector tangent to the fuselage
    const gp_Ax1 xAxe(pointList.front().Location(),
                      gp_Dir(1, 0, 0)); // parallel to x Axis => used to rotate the profile plane
    gp_Pln profilePlane = gp_Pln(
        gp_Ax3(pointList.front().Location(), pointList.front().Rotated(xAxe, M_PI / 2.).Direction(), gp_Dir(1, 0, 0)));
    profilePlane.Rotate(gp_Ax1(pointList.front().Location(), pointList.front().Direction()),
                        M_PI / 2.); // correct the orientation of the stringer plane (parallel to the Y-Z reference)

    TopoDS_Compound compound;
    TopoDS_Builder builder;
    builder.MakeCompound(compound);
    for (size_t i = 0; i < pointList.size() - 1; i++) {
        const gp_Pnt p1 = pointList.at(i + 0).Location();
        const gp_Pnt p3 = pointList.at(i + 1).Location();

        // determine the direction of the projection
        const gp_Pnt midPointOnStringer = (p1.XYZ() + p3.XYZ()) / 2;
        const gp_Pnt midPointRefs =
            (m_stringerPositions[i]->GetRefPoint().XYZ() + m_stringerPositions[i + 1]->GetRefPoint().XYZ()) / 2.;
        const gp_Dir interDir(0, midPointOnStringer.Y() - midPointRefs.Y(), midPointOnStringer.Z() - midPointRefs.Z());

        // then, we project the segment on the fuselage, and get the resulting wire
        const TopoDS_Wire path = fuselage.projectParallel(BRepBuilderAPI_MakeEdge(p1, p3).Edge(), interDir);

        if (just1DElements) {
            builder.Add(compound, path);
        }
        else {
            const CCPACSProfileBasedStructuralElement& pbse = m_uidMgr->ResolveObject<CCPACSProfileBasedStructuralElement>(
                m_stringerPositions.front()->GetStructuralElementUID());
            builder.Add(compound, pbse.makeFromWire(path, profilePlane));
        }
    }

    if (just1DElements)
        m_geomCache[just1DElements] = BuildWireFromEdges(compound); // make sure the geometry is a single wire
    else
        m_geomCache[just1DElements] = compound;
}
} // namespace tigl
