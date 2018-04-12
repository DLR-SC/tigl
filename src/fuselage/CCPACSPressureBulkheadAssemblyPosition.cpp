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

#include "CCPACSPressureBulkheadAssemblyPosition.h"

#include <gp_Pln.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopExp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Ax3.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include "CCPACSFuselage.h"
#include "CCPACSFrame.h"
#include "CCPACSPressureBulkhead.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"

namespace tigl
{
CCPACSPressureBulkheadAssemblyPosition::CCPACSPressureBulkheadAssemblyPosition(CCPACSPressureBulkheadAssembly* parent,
                                                                               CTiglUIDManager* uidMgr)
    : generated::CPACSPressureBulkheadAssemblyPosition(parent, uidMgr)
    , invalidated(true)
{
}

void CCPACSPressureBulkheadAssemblyPosition::SetFrameUID(const std::string& value)
{
    generated::CPACSPressureBulkheadAssemblyPosition::SetFrameUID(value);
    Invalidate();
}

void CCPACSPressureBulkheadAssemblyPosition::SetPressureBulkheadElementUID(const std::string& value)
{
    generated::CPACSPressureBulkheadAssemblyPosition::SetPressureBulkheadElementUID(value);
    Invalidate();
}

void CCPACSPressureBulkheadAssemblyPosition::Invalidate()
{
    m_geometry      = boost::none;
}

TopoDS_Shape CCPACSPressureBulkheadAssemblyPosition::GetGeometry()
{
    if (!m_geometry)
        BuildGeometry();
    return m_geometry.value();
}

void CCPACSPressureBulkheadAssemblyPosition::BuildGeometry()
{
    CCPACSFuselage& fuselage = *m_parent->GetParent()->GetParent();
    CCPACSFrame& frame = m_uidMgr->ResolveObject<CCPACSFrame>(m_frameUID);

    if (frame.GetFramePositions().size() == 1) {
        // one frame position

        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(frame.GetGeometry(true), TopAbs_EDGE, edgeMap);

        if (edgeMap.Extent() != 1) {
            throw CTiglError("1D frame geometry should have exactly one edge");
        }

        TopoDS_Edge edge = TopoDS::Edge(edgeMap(1));
        TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge); //turn the edge into wire
        TopoDS_Face Face = BRepBuilderAPI_MakeFace(wire); //make the face from the wire
        m_geometry       = Face;
    } else if (frame.GetFramePositions().size() >= 2) {
        // two ore more frame positions

        //BRep_Builder builder;
        //TopoDS_Compound compound;
        //builder.MakeCompound(compound);

        std::vector<TopoDS_Edge> edges;
        std::vector<gp_Pnt> refPoints;
        for (int i = 0; i < frame.GetFramePositions().size(); i++) {
            const CCPACSFuselageStringerFramePosition& framePosition = *frame.GetFramePositions()[i];
            const gp_Pnt refPoint = framePosition.GetRefPoint();
            const gp_Pnt intersectionPoint = fuselage.Intersection(framePosition).Location();

            //TopoDS_Vertex vertex;
            //builder.MakeVertex(vertex, intersectionPoint, precision);
            //builder.Add(compound, vertex);

            const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(refPoint, intersectionPoint);
            //builder.Add(compound, edge);

            edges.push_back(edge);
            refPoints.push_back(refPoint);
        }

        // we have to make the wires in the order of construction (clockwise)
        // so we turn the first edge into a wire, then the edge-frame and then the last edge
        BRepBuilderAPI_MakeWire wireMaker;

        wireMaker.Add(edges[0]); // first edge

        // create all the edges of the frame (which is made of several edges)
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(frame.GetGeometry(true), TopAbs_EDGE, edgeMap);
        for (int i = 1; i <= edgeMap.Extent(); i++)
            wireMaker.Add(TopoDS::Edge(edgeMap(i)));

        wireMaker.Add(edges[edges.size() - 1]); // last edge

        // check if X coords of all ref points are the same
        for (int i = 0; i < refPoints.size() - 1; i++) {
            if (refPoints[i].X() != refPoints[i + 1].X()) {
                throw CTiglError("X coordinates of reference points are different (which is not available)");
            }
        }

        const double precision = 1e-6;

        // check if 2 reference points are the same for adding edgeRef
        int diffFramePosition = 0; // contains the number of different frame positions, to be able to create the "edgesRef" vector with the appropiate size(edgesRef are the edges between the reference points).
        for (int i = 0; i < refPoints.size() - 1; i++) {
            if (!refPoints[i].IsEqual(refPoints[i + 1], precision)) {
                diffFramePosition++;
            }
        }

        // generate edges between non-equal points
        std::vector<TopoDS_Edge> edgesRef;
        for (int i = 0; i < refPoints.size() - 1; i++) {
            if (!refPoints[i].IsEqual(refPoints[i + 1], precision)) {
                edgesRef.push_back(BRepBuilderAPI_MakeEdge(refPoints[i + 1], refPoints[i]));
            }
        }

        // add edges to wire in reverse order
        for (int i = 0; i < edgesRef.size(); i++) {
            wireMaker.Add(edgesRef[edgesRef.size() - 1 - i]);
        }

        TopoDS_Wire wire = wireMaker.Wire();
        //builder.Add(compound, wire);
        //m_geometry = compound;

        if (!wireMaker.IsDone()) {
            throw CTiglError("Wire generation failed");
        }

        m_geometry = BRepBuilderAPI_MakeFace(wire);
    }
}
} // namespace tigl
