/* 
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-05-14 Martin Siggel <Martin.Siggel@dlr.de> with Marko Alder <Marko.Alder@dlr.de>
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

#include "CCPACSFuselageWallSegment.h"
#include "generated/CPACSWallSegments.h"
#include "generated/CPACSWalls.h"
#include "CCPACSFuselageStructure.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageSegment.h"
#include "CNamedShape.h"
#include "CCPACSWallPosition.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"
#include "to_string.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>

#include <algorithm>

namespace {

// get trimming plane for start and end connection of a wall segment.
// Makes sure that the normal of the plane always has a positive x-component.
gp_Pln GetConnectionPlane(tigl::CCPACSWallPosition const& p, bool flushConnection)
{
    gp_Pln cutter_pln;
    if (flushConnection && p.GetShape()) {
        // trimming plane is the plane of the shape defined at the wall position
        TopoDS_Shape shape = *p.GetShape();
        Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(shape));
        GeomLib_IsPlanarSurface surf_check(surf);
        if (surf_check.IsPlanar()) {
            cutter_pln = surf_check.Plan();
            cutter_pln.SetLocation(p.GetBasePoint());
            // make sure normal points in positive x-direction
            gp_Ax1 normal = cutter_pln.Axis();
            if( normal.Direction().X() < 0 ) {
                normal.Reverse();
                cutter_pln.SetAxis(normal);
            }
        }
        else {
            throw tigl::CTiglError("Cannot fill gap at non-planar surface");
        }
    }
    else {
        // trimming plane is YZ-plane at the base point
        cutter_pln = gp_Pln(gp_Ax3(p.GetBasePoint(), gp_Dir(1., 0., 0.), gp_Dir(0., 1., 0.)));
    }
    return cutter_pln;
}

}

namespace tigl
{

CCPACSFuselageWallSegment::CCPACSFuselageWallSegment(CCPACSWallSegments* parent, tigl::CTiglUIDManager *uidMgr)
    : generated::CPACSWallSegment(parent, uidMgr)
{
}

TopoDS_Compound CCPACSFuselageWallSegment::GetCutPlanes() const
{
    // Makes sure, that the cut planes are already computed
    if (m_cutPlanes.IsNull()) {
        BuildLoft();
    }
    return m_cutPlanes;
}

const CCPACSWalls& CCPACSFuselageWallSegment::GetWalls() const
{
    const CCPACSWallSegments* wallSegments = GetParent();
    if (!wallSegments) {
        throw CTiglError("Error in CCPACSFuselageWallSegment::GetWalls. Null pointer returned.", TIGL_NULL_POINTER);
    }
    
    const CCPACSWalls* walls = wallSegments->GetParent();
    if (!walls) {
        throw CTiglError("Error in CCPACSFuselageWallSegment::GetWalls. Null pointer returned.", TIGL_NULL_POINTER);
    }
    
    return *walls;
}

const CCPACSFuselage &CCPACSFuselageWallSegment::GetFuselage() const
{
    const CCPACSWalls& walls = GetWalls();
    
    const CCPACSFuselageStructure* fuselageStructure = walls.GetParent();
    if (!fuselageStructure) {
        throw CTiglError("Cannot get fuselage in CCPACSFuselageWallSegment::GetFuselage. Null pointer parent.", TIGL_NULL_POINTER);
    }

    const CCPACSFuselage* fuselage = fuselageStructure->GetParent();
    if (!fuselageStructure) {
        throw CTiglError("Cannot get fuselage in CCPACSFuselageWallSegment::GetFuselage. Null pointer parent.", TIGL_NULL_POINTER);
    }

    return *fuselage;
}

PNamedShape CCPACSFuselageWallSegment::BuildLoft() const
{    
    // A bounding box is created to use its diagonal as reference for the
    // extrusion length (edge length). This ensures that these always go
    //  beyond the model, but do not reach into infinity.
    Bnd_Box boundingBox;
    const CCPACSFuselage& fuselage = GetFuselage();
    TopoDS_Shape fuselageShape = fuselage.GetLoft()->Shape();
    BRepBndLib::Add(fuselageShape, boundingBox);
    double bboxSize = sqrt(boundingBox.SquareExtent());
    
    size_t nWallPositions = GetWallPositionUIDs().GetWallPositionUIDs().size();

    // list with base points:
    std::vector<gp_Pnt> base_pnts;
    base_pnts.reserve(nWallPositions);

    // extrusion in negative direction is realized by setting u_neg to
    // -bboxSize:
    bool negativeExtrusion = GetNegativeExtrusion().value_or(false);
    double u_pos = bboxSize;
    double u_neg = 0.;
    if (negativeExtrusion) {
        u_neg = -bboxSize;
    }

    gp_Pnt upper, lower;   // upper and lower extrusion points for each base point
    gp_Vec x_vec;          // vector from one base point to the next

    // build for untrimmed wall
    TopoDS_Builder builder;
    TopoDS_Compound wall;
    builder.MakeCompound(wall);

    size_t count = 0;
    const auto& walls = GetWalls();
    for (auto wallPositionUID : GetWallPositionUIDs().GetWallPositionUIDs()) {
        const CCPACSWallPosition& p = walls.GetWallPosition(wallPositionUID);

        // get current base point and x_vec pointing from previous base point to current
        gp_Pnt base_point = p.GetBasePoint();
        if ( count > 0 ) {
            x_vec = gp_Vec(base_pnts.back(), base_point);
        }

        // extrusion vector
        double phiRad = Radians(GetPhi());
        gp_Vec ext_vec(0., -sin(phiRad), cos(phiRad));

        if (count==1) {
            // make first wall segment larger in negative x-direction
            upper.Translate(-x_vec*bboxSize);
            lower.Translate(-x_vec*bboxSize);
        }

        gp_Pnt upper_new = base_point.Translated(u_pos*ext_vec);
        gp_Pnt lower_new = base_point.Translated(u_neg*ext_vec);
        if (count==nWallPositions-1) {
            // make last wall segment larger in positive x-drection
            upper_new.Translate(x_vec*bboxSize);
            lower_new.Translate(x_vec*bboxSize);
        }

        // create the wall segment from the four corner points
        if (count > 0 ) {
            TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(lower, lower_new).Edge();
            TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(lower_new, upper_new).Edge();
            TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(upper_new, upper).Edge();
            TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(upper, lower).Edge();
            TopoDS_Wire w = BRepBuilderAPI_MakeWire(e1, e2, e3, e4).Wire();
            TopoDS_Face face = BRepBuilderAPI_MakeFace(w, false).Face();
            builder.Add(wall, face);
        }

        base_pnts.push_back(base_point);
        upper = upper_new;
        lower = lower_new;
        ++count;
    }

    // store the untrimmed wall as the cutting tool
    // (to be used in the construction of walls bound by this)
    m_cutPlanes = wall;

    // trimming of the wall

    // Step 1/3: Trim start and end connections
    {
        double extents = 2*bboxSize; // make sure the cutting plane does not miss anything
        bool flushConnection = GetFlushConnectionStart().value_or(false);
        auto wallPositionUID = GetWallPositionUIDs().GetWallPositionUIDs().front();
        const CCPACSWallPosition& p_start = walls.GetWallPosition(wallPositionUID);
        gp_Pln pln_start = GetConnectionPlane(p_start, flushConnection);
        TopoDS_Face cutter_start = BRepBuilderAPI_MakeFace(pln_start,-extents, extents, -extents, extents).Face();

        flushConnection = GetFlushConnectionEnd().value_or(false);
        wallPositionUID = GetWallPositionUIDs().GetWallPositionUIDs().back();
        const CCPACSWallPosition& p_end = walls.GetWallPosition(wallPositionUID);
        gp_Pln pln_end = GetConnectionPlane(p_end, flushConnection);
        TopoDS_Face cutter_end = BRepBuilderAPI_MakeFace(pln_end,-extents, extents, -extents, extents).Face();

        TopoDS_Shape result =SplitShape(wall, cutter_end);
        result = SplitShape(result, cutter_start);

        TopoDS_Compound cut_wall;
        builder.MakeCompound(cut_wall);

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(result, TopAbs_FACE, faceMap);
        for (int i = 0; i < faceMap.Extent(); ++i) {
            TopoDS_Face face = TopoDS::Face(faceMap(i+1));
            gp_Pnt faceCenter = GetCentralFacePoint(face);
            // if the face Center is to the right of pln_start and to the left of pln_end, keep the face.
            bool RightOfPlnStart = IsPointAbovePlane(pln_start, faceCenter);
            bool LeftOfPlnEnd    = !IsPointAbovePlane(pln_end, faceCenter);
            if (RightOfPlnStart && LeftOfPlnEnd) {
                builder.Add(cut_wall, face);
            }
        }
        wall = cut_wall;
    }
    
    // Step 2/3: Trim the wall by the fuselage
    {
        TopoDS_Compound cut_wall;
        builder.MakeCompound(cut_wall);

        TopoDS_Shape result = SplitShape(wall, fuselageShape);
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(result, TopAbs_FACE, faceMap);
        for (int i = 0; i < faceMap.Extent(); ++i) {
            TopoDS_Face face = TopoDS::Face(faceMap(i+1));
            gp_Pnt faceCenter = GetCentralFacePoint(face);
            // TODO: This is fast but could be potentially dangerous.
            // Maybe we should check if the point is outside the fuselage
            // shape rather than the bounding box.
            if (!boundingBox.IsOut(faceCenter)) {
                builder.Add(cut_wall, face);
            }
        }
        wall = cut_wall;
    }

    // Step 3/3: Cut the wall with bounding elements:
    if (GetBoundingElementUIDs()) {
        for (std::string bounding_element_uid : GetBoundingElementUIDs()->GetBoundingElementUIDs()) {
            const CCPACSFuselageWallSegment& bounding_element = GetWalls().GetWallSegment(bounding_element_uid);
            // TODO: Check: The order of whoch the walls are evaulated seem to matter here!
            TopoDS_Compound bounding_cutPlane = bounding_element.GetCutPlanes();
            TopoDS_Shape result = SplitShape(wall, bounding_cutPlane);
            
            TopoDS_Compound cut_wall;
            builder.MakeCompound(cut_wall);
            TopTools_IndexedMapOfShape faceMap;
            TopExp::MapShapes(result, TopAbs_FACE, faceMap);

            // Loop over the split faces and check weather one of the
            // base points belongs to the face.
            for (int i = 0; i <faceMap.Extent(); ++i) {
                TopoDS_Face face = TopoDS::Face(faceMap(i+1));
                for (auto point : base_pnts) {
                    if ( IsPointInsideFace(face, point) ) {
                        builder.Add(cut_wall,face);
                        break;
                    }
                }
            }
            wall = cut_wall;
        } // loop over bounding elements
    } // if has bounding elements
    
    return PNamedShape(new CNamedShape(wall, GetDefaultedUID()));
}

} // namespace tigl
