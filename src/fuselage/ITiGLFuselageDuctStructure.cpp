/*
* Copyright (C) 2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-16 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include "ITiglFuselageDuctStructure.h"
#include "tiglcommonfunctions.h"
#include "Debugging.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CNamedShape.h"

#include "TopTools_ListIteratorOfListOfShape.hxx"
#include "IntCurvesFace_Intersector.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "BRepProj_Projection.hxx"
#include "TopExp_Explorer.hxx"
#include "gp_Lin.hxx"

namespace tigl {

ITiglFuselageDuctStructure::ITiglFuselageDuctStructure(CTiglRelativelyPositionedComponent const* parent)
 : m_parent(parent)
 , m_loft(*this, &ITiglFuselageDuctStructure::StoreLoft)
{}

void ITiglFuselageDuctStructure::Invalidate() const
{
    m_loft.clear();
}

void ITiglFuselageDuctStructure::StoreLoft(TopoDS_Shape& cache) const
{
    cache = m_parent->GetTransformationMatrix().Inverted().Transform(m_parent->GetLoft()->DeepCopy()->Shape());
}

TopoDS_Shape const& ITiglFuselageDuctStructure::GetLoft() const
{
        return *m_loft;
}

gp_Lin ITiglFuselageDuctStructure::Intersection(CCPACSFuselageStringerFramePosition const& pos) const
{
    const gp_Pnt pRef        = pos.GetRefPoint();
    const double angleRef = (M_PI / 180.) * pos.GetReferenceAngle();
    return Intersection(pRef, angleRef);
}

gp_Lin ITiglFuselageDuctStructure::Intersection(gp_Pnt pRef, double angleRef) const
{
    // to have a left-handed coordinates system for the intersection computation (see documentation)
    const gp_Ax1 xAxe(pRef, gp_Dir(1, 0, 0));
    const gp_Dir zReference(0, 0, 1);
    const gp_Dir angleDir = zReference.Rotated(xAxe, angleRef);

    // build a line to position the intersection with the fuselage shape
    gp_Lin line(pRef, angleDir);

    // get the list of shape from the fuselage shape
    TopExp_Explorer exp;
    for (exp.Init(*m_loft, TopAbs_FACE); exp.More(); exp.Next()) {
        IntCurvesFace_Intersector intersection(TopoDS::Face(exp.Current()), 0.1); // intersection builder
        intersection.Perform(line, 0, std::numeric_limits<Standard_Real>::max());
        if (intersection.IsDone() && intersection.NbPnt() > 0) {
            gp_Lin result(intersection.Pnt(1), line.Direction());
            // return the line with the point on the fuselage as the origin, and the previous line's direction
            return result;
        }
    }

    TRACE_POINT(debug);
    debug.dumpShape(*m_loft, "loft");
    debug.dumpShape(BRepBuilderAPI_MakeEdge(pRef, pRef.XYZ() + angleDir.XYZ() * 1000), "line");

    throw std::logic_error("Error computing intersection line");
}

namespace
{
    TopoDS_Wire project(TopoDS_Shape wireOrEdge, BRepProj_Projection& proj, DebugScope& debug)
    {
        BRepBuilderAPI_MakeWire wireBuilder;
        for (; proj.More(); proj.Next())
            wireBuilder.Add(proj.Current());

        TopTools_ListOfShape wireList;
        BuildWiresFromConnectedEdges(proj.Shape(), wireList);

        if (wireList.Extent() == 0) {
            debug.addShape(proj.Shape(), "projection");

            throw CTiglError("Projection returned no wires");
        }
        if (wireList.Extent() == 1)
            return TopoDS::Wire(wireList.First());

        // select the wire which is closest to the wire we projected
        for (TopTools_ListIteratorOfListOfShape it(wireList); it.More(); it.Next()) {
            const TopoDS_Wire w                = TopoDS::Wire(it.Value());
            const gp_Pnt wStart     = GetFirstPoint(w);
            const gp_Pnt wEnd       = GetLastPoint(w);
            const gp_Pnt inputStart = GetFirstPoint(wireOrEdge);
            const gp_Pnt inputEnd   = GetLastPoint(wireOrEdge);

            const double pointEqualEpsilon = 1e-4;
            if ((wStart.IsEqual(inputStart, pointEqualEpsilon) && wEnd.IsEqual(inputEnd, pointEqualEpsilon)) ||
                (wEnd.IsEqual(inputStart, pointEqualEpsilon) && wStart.IsEqual(inputEnd, pointEqualEpsilon))) {
                return w;
            }
        }

        TopoDS_Compound c;
        TopoDS_Builder b;
        b.MakeCompound(c);
        for (TopTools_ListIteratorOfListOfShape it(wireList); it.More(); it.Next()) {
            b.Add(c, it.Value());
        }
        debug.addShape(proj.Shape(), "projection");
        debug.addShape(c, "wireList");

        // give up
        throw CTiglError("Failed to project wire/edge onto fuselage");
    }
}

TopoDS_Wire ITiglFuselageDuctStructure::projectConic(TopoDS_Shape wireOrEdge, gp_Pnt origin) const
{
    DEBUG_SCOPE(debug);
    debug.addShape(wireOrEdge, "wireOrEdge");
    debug.addShape(*m_loft, "loft");
    debug.addShape(BRepBuilderAPI_MakeVertex(origin), "origin");

    BRepProj_Projection proj(wireOrEdge, *m_loft, origin);
    return project(wireOrEdge, proj, debug);
}

TopoDS_Wire ITiglFuselageDuctStructure::projectParallel(TopoDS_Shape wireOrEdge, gp_Dir direction) const
{
    const TopoDS_Shape directionLine = BRepBuilderAPI_MakeEdge(
        BRepBuilderAPI_MakeVertex(gp_Pnt(0, 0, 0)).Vertex(),
        BRepBuilderAPI_MakeVertex(gp_Pnt(direction.XYZ() * 1000)).Vertex()
    ).Shape();

    DEBUG_SCOPE(debug);
    debug.addShape(wireOrEdge, "wireOrEdge");
    debug.addShape(*m_loft, "loft");
    debug.addShape(directionLine, "direction");

    BRepProj_Projection proj(wireOrEdge, *m_loft, direction);
    return project(wireOrEdge, proj, debug);
}

} //namespace tigl
