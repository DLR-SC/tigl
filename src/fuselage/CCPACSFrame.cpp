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

#include "CCPACSFrame.h"

#include <gp_Pnt.hxx>
#include <gp_Pln.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRepAlgoAPI_Section.hxx>

#include "tiglcommonfunctions.h"
#include "CNamedShape.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSProfileBasedStructuralElement.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CCPACSCrossBeamAssemblyPosition.h"
#include "CCPACSCrossBeamStrutAssemblyPosition.h"
#include "CCPACSFuselageStringer.h"

namespace tigl
{
CCPACSFrame::CCPACSFrame(CCPACSFramesAssembly* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFrame(parent, uidMgr)
{
}

void CCPACSFrame::Invalidate()
{
    for (auto& c : m_geomCache)
        c = boost::none;
}

TopoDS_Shape CCPACSFrame::GetGeometry(bool just1DElements, TiglCoordinateSystem cs)
{
    if (!m_geomCache[just1DElements])
        BuildGeometry(just1DElements);

    auto shape = m_geomCache[just1DElements].value();
    if (cs == TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM) {
        CTiglTransformation trafo = m_parent->GetParent()->GetParent()->GetTransformationMatrix();
        return trafo.Transform(shape);
    }
    else
        return shape;
}


void CCPACSFrame::BuildGeometry(bool just1DElements)
{
    if (m_framePositions.empty())
        throw CTiglError("Cannot build frame geometry, no frame positions defined in XML", TIGL_XML_ERROR);

    // this function build the frame in 2 steps :
    // 1) path definition (projection on the fuselage)
    // 2) if not just 1D element, build and sweep the profile all along the path

    CCPACSFuselage& fuselage        = *m_parent->GetParent()->GetParent();
    const TopoDS_Shape fuselageLoft = fuselage.GetLoft()->Shape();

    // initialize object needed for the frame build
    gp_Pln profilePlane;
    TopoDS_Wire path;

    if (m_framePositions.size() == 1) {
        // if there is 1 position ==> the path is around the fuselage, in an X normal orientated plane
        const auto& fp = *m_framePositions[0];

        TopoDS_Shape section = BRepAlgoAPI_Section(fuselageLoft, gp_Pln(fp.GetRefPoint(), gp_Dir(1, 0, 0))).Shape();
        path                 = BuildWireFromEdges(section);

        if (!just1DElements) {
            // -1) place the point and the plane (X Axis as normal vector)
            const gp_Lin iAxe = fuselage.Intersection(fp);
            const gp_Ax1 xAxe(iAxe.Location(),
                              gp_Dir(1, 0, 0)); // parallel to x Axis => used to rotate the profile plane
            profilePlane = gp_Pln(gp_Ax3(iAxe.Location(), iAxe.Rotated(xAxe, M_PI / 2.).Direction(), gp_Dir(1, 0, 0)));
        }
    }
    else {
        // if there is more than 1 position ==> the path is projected segment by segment on the fuselage
        // create a frame in the fuselage, following the path described by all the positions in frame/frameposition[]

        // -1) place every points in the fuselage loft
        std::vector<gp_Lin> pointList;
        for (int i = 0; i < m_framePositions.size(); i++)
            pointList.push_back(fuselage.Intersection(*m_framePositions[i]));

        // with a ShapeExtend_WireData, the individual segment orientation is not crucial during the wire building
        Handle(ShapeExtend_WireData) wirePath = new ShapeExtend_WireData;
        for (int i = 0; i < pointList.size() - 1; i++) {
            const gp_Pnt p1 = pointList.at(i + 0).Location();
            const gp_Pnt p3 = pointList.at(i + 1).Location();

            // first, we cut the initial segment in 2 parts
            const gp_Pnt midPnt = (p1.XYZ() + p3.XYZ()) / 2;

            // draw the intermediate point.
            const gp_Pnt p2 = fuselage.Intersection(midPnt, M_PI / 2.).Location();

            // second, we get the projection points
            // build the point in the middle of the edge previously created
            const gp_Pnt midPnt1 = (p1.XYZ() + p2.XYZ()) / 2; // get the middle point of the segment 1
            const gp_Pnt midPnt2 = (p2.XYZ() + p3.XYZ()) / 2; // get the middle point of the segment 2

            const gp_Lin proj1 = fuselage.Intersection(midPnt1, -M_PI / 2.); // define the projection point
            const gp_Lin proj2 = fuselage.Intersection(midPnt2, -M_PI / 2.); // define the projection point

            // third, we project the sub-segment on the fuselage, according to the projections points,
            // and add it to the path add the current part in the path builder
            wirePath->Add(fuselage.projectConic(BRepBuilderAPI_MakeEdge(p1, p2).Edge(), proj1.Location()));
            wirePath->Add(fuselage.projectConic(BRepBuilderAPI_MakeEdge(p2, p3).Edge(), proj2.Location()));
        }

        // finally, we can create the path with all the projection obtained
        // TODO : throw exception if the frame's path creation fail
        // the ShapeFix_Wire classe will create a proper wire (direction, order) with all the data in the ShapeExtend_WireData
        ShapeFix_Wire fixWire;
        fixWire.Load(wirePath);
        fixWire.Perform();
        fixWire.FixReorder();
        fixWire.FixConnected();
        path = fixWire.WireAPIMake();
        path.Orientation(TopAbs_FORWARD);

        if (!just1DElements) {
            // creation of the profile plane
            // the plan axis system is defined by : the location, the normal vector, the x vector. It's a right-handed system
            // the rotation is necessary to have a normal vector tangent to the fuselage
            const gp_Ax1 xAxe(pointList.at(0).Location(),
                              gp_Dir(1, 0, 0)); // parallel to x Axis => used to rotate the profile plane
            profilePlane = gp_Pln(gp_Ax3(pointList.at(0).Location(),
                                         pointList.at(0).Rotated(xAxe, M_PI / 2.).Direction(), gp_Dir(1, 0, 0)));
        }
    }

    if (just1DElements)
        m_geomCache[true] = path;
    else {
        const auto& pbse = m_uidMgr->ResolveObject<CCPACSProfileBasedStructuralElement>(
            m_framePositions.front()->GetStructuralElementUID());
        m_geomCache[false] = pbse.makeFromWire(path, profilePlane);
    }
}
} // namespace tigl
