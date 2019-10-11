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
#include "generated/CPACSWallPosition.h"
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

// The x coordinate is determined by an intersection of a line parallel
// to the x-axes and a given shape.
double GetXCoord(const TopoDS_Shape& shape, double cy, double cz, double bboxSize);

// fill gap between extrusion vector and shape by projection of
// extrusion vector
//
// TODO: I have a hunch that this can be replaced by something of the lines of
//   gp_Vec CalcExtrusionVector(...)
// and we do not explicitly need to build a vec of ext_vecs...
void CloseGap(gp_Vec& ext_vec, gp_Pnt base_pnt, gp_Vec norm_vec, gp_Vec x_vec, const TopoDS_Shape& shape);

}

namespace tigl
{

CCPACSFuselageWallSegment::CCPACSFuselageWallSegment(CCPACSWallSegments* parent, tigl::CTiglUIDManager *uidMgr)
    : generated::CPACSWallSegment(parent, uidMgr)
{
}

TopoDS_Compound CCPACSFuselageWallSegment::GetCutPlanes() const
{
    // TODO: this should be improved
    // Makes sure, that the cut planes are already computed
    GetLoft();
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
    bool flushConnectionStart = GetFlushConnectionStart().value_or(false);
    bool flushConnectionEnd = GetFlushConnectionEnd().value_or(false);
    bool negativeExtrusion = GetNegativeExtrusion().value_or(false);
    
    const CCPACSFuselage& fuselage = GetFuselage();
    
    // A bounding box is created to use its diagonal as reference for the
    // extrusion length (edge length). This ensures that these always go
    //  beyond the model, but do not reach into infinity.
    Bnd_Box boundingBox;
    TopoDS_Shape fuselageShape = fuselage.GetLoft()->Shape();
    BRepBndLib::Add(fuselageShape, boundingBox);
    double bboxSize = sqrt(boundingBox.SquareExtent());
    
    const auto& walls = GetWalls();

 
    // Base points and vectors

    // extrusion vectors:
    //   The extrusion vectors are equal at each base point. However,
    //   since the first and last vector can be modified later on, each
    //   point is assigned its own vector.
    double phiRad = Radians(GetPhi());
    size_t nWallPositionUIDs = GetWallPositionUIDs().GetWallPositionUIDs().size();
    std::vector<gp_Vec> ext_vecs(nWallPositionUIDs, gp_Vec(0., -sin(phiRad), cos(phiRad)));

    // list with base points:
    std::vector<gp_Pnt> base_pnts;
    std::vector<TopoDS_Shape> shapes;
    base_pnts.reserve(nWallPositionUIDs);
    shapes.reserve(nWallPositionUIDs);

    for (auto wallPositionUID : GetWallPositionUIDs().GetWallPositionUIDs()) {
        // TODO: move logic into wall position class
        const CCPACSWallPosition& p = walls.GetWallPosition(wallPositionUID);
        double x = 0;
        TopoDS_Shape shape;
        double y = p.GetY();
        double z = p.GetZ();
        if (p.GetX_choice4()) {
            x = p.GetX_choice4().value();
        }
        else if(p.GetBulkheadUID_choice1()) {
            shape = GetUIDManager().GetGeometricComponent(p.GetBulkheadUID_choice1().value()).GetLoft()->Shape();
            x = GetXCoord(shape, y, z, bboxSize);
        }
        else if(p.GetWallSegmentUID_choice2()) {
            const CCPACSFuselageWallSegment& wall = walls.GetWallSegment(p.GetWallSegmentUID_choice2().value());
            if ( wall.GetDefaultedUID() == p.GetWallSegmentUID_choice2().value() ){
                throw CTiglError("Fuselage wall references itself");
            }
            shape = wall.GetLoft()->Shape();
            x = GetXCoord(shape, y, z, bboxSize);
        }
        else if (p.GetFuselageSectionUID_choice3()) {
            shape = fuselage.GetSectionFace(p.GetFuselageSectionUID_choice3().value());
            x = GetXCoord(shape, y, z, bboxSize);
        }
        else {
            throw CTiglError("Cannot determine x coordinate of wall position.");
        }
        base_pnts.push_back(gp_Pnt(x,y,z));
        shapes.push_back(shape);
    }
    size_t n_base_pnts = base_pnts.size();
 
    // vectors along the base points and normal vectors of the all segments:
    std::vector<gp_Vec> x_vecs, norm_vecs;
    for (size_t i = 0; i < n_base_pnts - 1; ++i) {
        gp_Vec x_vec = gp_Vec(base_pnts[i], base_pnts[i+1]);
        x_vecs.push_back(x_vec);
        norm_vecs.push_back(x_vec.Crossed(ext_vecs[i]));
        
    }

    // TODO: Here it is very strange, that the vectors norm_vecs and x_vecs are 1 smaller than the others
    // This could be a bug
    if (flushConnectionStart && !shapes.front().IsNull()) {
        CloseGap(ext_vecs.front(), base_pnts.front(), norm_vecs.front(), x_vecs.front(), shapes.front());
    }
    if (flushConnectionEnd && !shapes.back().IsNull()) {
        CloseGap(ext_vecs.back(), base_pnts.back(), norm_vecs.back(), x_vecs.back(), shapes.back());
    }

    // lines
    std::vector<gp_Lin> v_lines;
    for (size_t i = 0; i < base_pnts.size(); ++i) {
        v_lines.push_back(gp_Lin(base_pnts[i], gp_Dir(ext_vecs[i])));
    }
    size_t n_vlines = v_lines.size();

    // edges 

    // extrusion in negative direction is realized by setting u_neg to
    // -bboxSize:
    double u_pos = bboxSize;
    double u_neg = 0.;
    if (negativeExtrusion) {
        u_neg = -bboxSize;
    }

   // list with vertical (v) edges:
    std::vector<TopoDS_Edge> v_edges;
    // list with start and end points of the edges: 
    std::vector<std::pair<gp_Pnt, gp_Pnt>> edge_pnts;
    for (auto v_line : v_lines) {
        BRepBuilderAPI_MakeEdge builder(v_line, u_neg, u_pos);
        v_edges.push_back(builder.Edge());
        edge_pnts.push_back(std::make_pair(BRep_Tool::Pnt(builder.Vertex1()), BRep_Tool::Pnt(builder.Vertex2())));
    }

    // list with horizontal (h) edges connecting the start points (lwr):
    std::vector<TopoDS_Edge> h_edges_lwr;
    // the same with the upper (upr) edges: 
    std::vector<TopoDS_Edge> h_edges_upr;
    for (size_t i = 0; i < n_vlines - 1; ++i) {
        TopoDS_Edge edgeLwr = BRepBuilderAPI_MakeEdge(edge_pnts[i].first, edge_pnts[i+1].first).Edge();
        TopoDS_Edge edgeUpr = BRepBuilderAPI_MakeEdge(edge_pnts[i].second, edge_pnts[i+1].second).Edge();
        h_edges_lwr.push_back(edgeLwr);
        h_edges_upr.push_back(edgeUpr);
    }
    
    // Construction of a wire for each wall segment
    std::vector<TopoDS_Wire> wires;
    for (size_t i = 0; i < n_vlines -1; ++i) {
        TopoDS_Wire w = BRepBuilderAPI_MakeWire(v_edges[i], h_edges_lwr[i], v_edges[i+1], h_edges_upr[i]).Wire();
        wires.push_back(w);
    }

    // faces
    std::vector<TopoDS_Face> faces;
    for (const TopoDS_Wire& wire : wires) {
        TopoDS_Face face = BRepBuilderAPI_MakeFace(wire, false).Face();
        faces.push_back(face);
    }
    
    TopoDS_Builder builder;
    TopoDS_Compound wall;
    builder.MakeCompound(wall);
    for (const TopoDS_Face& face : faces) {
        builder.Add(wall, face);
    }
    
    // trimming of the wall 
    // the wall will first be cut at the fuselage:
    TopoDS_Compound cut_wall;
    builder.MakeCompound(cut_wall);
    
    
    TopoDS_Shape result = SplitShape(wall, fuselageShape);
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(result, TopAbs_FACE, faceMap);
    for (int i = 0; i < faceMap.Extent(); ++i) {
        TopoDS_Face face = TopoDS::Face(faceMap.FindKey(i+1));
        gp_Pnt faceCenter = GetCentralFacePoint(face);
        if (!boundingBox.IsOut(faceCenter)) {
            builder.Add(cut_wall, face);
        }
    }
    
    wall = cut_wall;

   // cut the wall with bounding elements:
   auto additional = base_pnts;

    if (GetBoundingElementUIDs()) {
        size_t count = 1;
        for (std::string bounding_element_uid : GetBoundingElementUIDs()->GetBoundingElementUIDs()) {
            const CCPACSFuselageWallSegment& bounding_element = GetWalls().GetWallSegment(bounding_element_uid);
            count += 1;
            // TODO: Check: The order of whoch the walls are evaulated seem to matter here!
            TopoDS_Compound bounding_cutPlane = bounding_element.GetCutPlanes();
            TopoDS_Shape result = SplitShape(wall, bounding_cutPlane);
            
            TopoDS_Compound cut_wall;
            builder.MakeCompound(cut_wall);
            TopTools_IndexedMapOfShape faceMap;
            TopExp::MapShapes(result, TopAbs_FACE, faceMap);

            // Loop over the split faces and check weather one of the
            // base points belongs to the face.
            //
            // TODO: You should use
            // BRepClass_FaceClassifier (!!!) for this test. Since I couldn't find
            // it in pythonOCC I made a workaround by looping over the edges
            // and check weather a point is on the edge.
            for (int i = 0; i <faceMap.Extent(); ++i) {
                bool test = false;
                TopoDS_Shape shape = faceMap.FindKey(i+1);
                TopoDS_Face face = TopoDS::Face(shape);
                TopExp_Explorer s_expl;
                s_expl.Init(shape, TopAbs_EDGE);
                // check, wheather one of the base points is on the face edges
                while (s_expl.More()) {
                    for (size_t j = 0; j < base_pnts.size(); ++j) {
                        // check, whather pref is on current edge
                        // TODO: replace with proper function
                        gp_Pnt pref = base_pnts[j];
                        TopoDS_Edge edge = TopoDS::Edge(s_expl.Current());
                        gp_Pnt p1 = BRep_Tool::Pnt(TopExp::FirstVertex(edge));
                        gp_Pnt p2 = BRep_Tool::Pnt(TopExp::LastVertex(edge));
                        double d1 = gp_Vec(pref,p1).Magnitude();
                        double d2 = gp_Vec(pref,p2).Magnitude();
                        double dist = d1+d2-gp_Vec(p1,p2).Magnitude();
                        double eps = 1e-10;
                        if (dist < eps) {
                            test = true;
                        }
                    }
                    s_expl.Next();
                }
                // if the base points are on the current face, add it to the cut wall
                if (test) {
                    builder.Add(cut_wall,face);
                }
            }
            // TODO: check this. It seems strange to me
            // That only the last bounding element writes to wall
            wall = cut_wall;
        } // loop over bounding elements
    } // if has bounding elements
    
    TopoDS_Compound cutPlanes;
    builder.MakeCompound(cutPlanes);

    // cutting faces 
    //   In addition to the wall geometry cutting faces are build which
    //   exceed the fuselage.
    
    // define cutting planes:
    std::vector<gp_Pln> cut_plns;
    for (size_t i = 0; i < n_base_pnts -1; ++i) {
        gp_Pln plane(gp_Ax3(base_pnts[i], gp_Dir(norm_vecs[i]), gp_Dir(x_vecs[i])));
        cut_plns.push_back(plane);
    }

    // deduct cutting faces:
    std::vector<TopoDS_Face> cut_faces;
    for (auto pln : cut_plns) {
        TopoDS_Face face = BRepBuilderAPI_MakeFace(pln,-bboxSize,bboxSize, -bboxSize,bboxSize).Face();
        cut_faces.push_back(face);
    }
    
    if (n_base_pnts > 2) {
        for (size_t i = 0; i < n_base_pnts-1; ++i) {
            TopoDS_Face cut_face = cut_faces[i];
            double size = 2.*bboxSize;
            if (i == 0) {
                TopoDS_Face cut_cut_face = BRepBuilderAPI_MakeFace(cut_plns[i+1],-size,size,-size,size).Face();
                TopoDS_Shape result = SplitShape(cut_face, cut_cut_face);
                TopTools_IndexedMapOfShape faceMap;
                TopExp::MapShapes(result, TopAbs_FACE, faceMap);
                builder.Add(cutPlanes,faceMap.FindKey(1));
            }
            else if (i == n_base_pnts-2) {
                // TODO: Code duplication from above
                TopoDS_Face cut_cut_face = BRepBuilderAPI_MakeFace(cut_plns[i-1],-size,size,-size,size).Face();
                TopoDS_Shape result = SplitShape(cut_face, cut_cut_face);
                TopTools_IndexedMapOfShape faceMap;
                TopExp::MapShapes(result, TopAbs_FACE, faceMap);
                builder.Add(cutPlanes,faceMap.FindKey(2));
            }
            else {
                TopoDS_Face cut_cut_face1 = BRepBuilderAPI_MakeFace(cut_plns[i-1],-size,size,-size,size).Face();
                TopoDS_Face cut_cut_face2 = BRepBuilderAPI_MakeFace(cut_plns[i+1],-size,size,-size,size).Face();
                TopoDS_Compound cut_compound;
                builder.MakeCompound(cut_compound);
                builder.Add(cut_compound,cut_cut_face1);
                builder.Add(cut_compound,cut_cut_face2);
                TopoDS_Shape result = SplitShape(cut_face, cut_compound);
                TopTools_IndexedMapOfShape faceMap;
                TopExp::MapShapes(result, TopAbs_FACE, faceMap);
                builder.Add(cutPlanes,faceMap.FindKey(2));
            }
        }
    }
    else {
        builder.Add(cutPlanes,cut_faces[0]);
    }
    
    m_cutPlanes = cutPlanes;

    return PNamedShape(new CNamedShape(wall, GetDefaultedUID()));
}

} // namespace tigl

namespace {

// The x coordinate is determined by an intersection of a line parallel
// to the x-axes and a given shape.
double GetXCoord(const TopoDS_Shape& shape, double cy, double cz, double bboxSize)
{
    gp_Pnt xy_pnt(0.,cy,cz);
    gp_Lin x_line (xy_pnt, gp_Dir(1.,0.,0.));
    IntCurvesFace_ShapeIntersector intersector;
    intersector.Load(shape,1e-5);
    // ToDo: not sure if I interpret PInf and PSup correctly:
    intersector.Perform(x_line,-bboxSize, bboxSize);
    int NbPnt = intersector.NbPnt();
    if (NbPnt == 1) {
        gp_Pnt pnt = intersector.Pnt(1);
        return pnt.Coord(1);
    }
    else {
        throw tigl::CTiglError("Number of intersection points in GetXCoord is not 1. Instead it is " + tigl::std_to_string(NbPnt));
    }
}


// fill gap between extrusion vector and shape by projection of
// extrusion vector
void CloseGap(gp_Vec& ext_vec, gp_Pnt base_pnt, gp_Vec norm_vec, gp_Vec x_vec, const TopoDS_Shape& shape)
{
    gp_Pln pln1(gp_Ax3(base_pnt,gp_Dir(norm_vec),gp_Dir(x_vec)));
    Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(shape));
    GeomLib_IsPlanarSurface surf_check(surf);
    if (surf_check.IsPlanar()) {
        gp_Pln pln2 = surf_check.Plan();

        gp_Dir n1 = pln1.Axis().Direction();
        gp_Dir n2 = pln2.Axis().Direction();
        gp_Vec proj_vec = gp_Vec(n1.Crossed(n2));
        if (proj_vec.Angle(ext_vec) > 0.5*M_PI) {
            proj_vec.Reverse();
        }
        ext_vec = proj_vec;
    }
    else {
        throw tigl::CTiglError("Cannot fill gap at non-planar surface");
    }

}

}
