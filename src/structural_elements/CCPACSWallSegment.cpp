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
#include <BRepClass3d_SolidClassifier.hxx>

#include <algorithm>

namespace {

// Given a CCPACSWallposition with a defined shape, project the point pnt along the direction dir onto the
// plane, in which the shape lies. The shape must be convertable to a planar TopoDS_Face.
void FlushPointAlongVec(tigl::CCPACSWallPosition const& p, gp_Vec const dir, double extents, gp_Pnt& pnt)
{
    if (!p.GetShape()) {
        throw tigl::CTiglError("Cannot flush point for wall position without shape definition.");
    }
    gp_Pln pln;
    TopoDS_Shape shape = *p.GetShape();
    Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(*p.GetShape()));
    GeomLib_IsPlanarSurface surf_check(surf);
    if (surf_check.IsPlanar()) {
        pln = surf_check.Plan();
    }
    else {
        throw tigl::CTiglError("Cannot flush point at non-planar surface");
    }

    pln.SetLocation(p.GetBasePoint());
    TopoDS_Face face = BRepBuilderAPI_MakeFace(pln);
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(pnt.Translated( dir*extents),
                                               pnt.Translated(-dir*extents)).Edge();
    GetIntersectionPoint(face, edge, pnt);
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

void CCPACSFuselageWallSegment::SetPhi(const double& value)
{
    CPACSWallSegment::SetPhi(value);
    Invalidate();
}

void CCPACSFuselageWallSegment::SetDoubleSidedExtrusion(const boost::optional<bool>& value)
{
    CPACSWallSegment::SetDoubleSidedExtrusion(value);
    Invalidate();
}

void CCPACSFuselageWallSegment::InvalidateImpl(const boost::optional<std::string>& source) const
{
    loft.clear();
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
    if (!fuselage) {
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
    bool negativeExtrusion = GetDoubleSidedExtrusion().value_or(false);
    double u_pos = bboxSize;
    double u_neg = 0.;
    if (negativeExtrusion) {
        u_neg = -bboxSize;
    }

    gp_Pnt upper, lower;   // upper and lower extrusion points for each base point
    gp_Vec x_vec;          // vector from one base point to the next

    // build for untrimmed wall
    TopoDS_Builder builder_wall, builder_cutter;
    TopoDS_Compound wall;
    builder_wall.MakeCompound(wall);
    builder_cutter.MakeCompound(m_cutPlanes);

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
        ext_vec = GetFuselage().GetTransformationMatrix().Transform(ext_vec);

        if (count==1) {
            // flush first position to shape
            if( p.GetShape() && GetFlushConnectionStart().value_or(false) ) {
                std::string uid_prev = GetWallPositionUIDs().GetWallPositionUIDs().front();
                const CCPACSWallPosition& p_prev = walls.GetWallPosition(uid_prev);
                FlushPointAlongVec(p_prev, x_vec, bboxSize, upper);
                FlushPointAlongVec(p_prev, x_vec, bboxSize, lower);
            }
        }

        gp_Pnt upper_new = base_point.Translated(u_pos*ext_vec);
        gp_Pnt lower_new = base_point.Translated(u_neg*ext_vec);

        if (count==nWallPositions-1) {
            // flush last position to shape
            if( p.GetShape() && GetFlushConnectionEnd().value_or(false) ) {
                FlushPointAlongVec(p, x_vec, bboxSize, upper_new);
                FlushPointAlongVec(p, x_vec, bboxSize, lower_new);
            }
        }

        // create the wall segment from the four corner points lower, upper, lower_new, upper_new
        if (count > 0 ) {
            TopoDS_Face face = BuildFace(lower, lower_new, upper, upper_new);
            builder_wall.Add(wall, face);

            //first and last face are enlarged for the cutting tool
            gp_Pnt lower_cut = lower;
            gp_Pnt upper_cut = upper;
            gp_Pnt lower_new_cut = lower_new;
            gp_Pnt upper_new_cut = upper_new;
            if (count==1) {
                lower_cut = lower.Translated(-bboxSize*x_vec);
                upper_cut = upper.Translated(-bboxSize*x_vec);
            }
            if (count==nWallPositions-1) {
                lower_new_cut = lower_new.Translated(bboxSize*x_vec);
                upper_new_cut = upper_new.Translated(bboxSize*x_vec);
            }
            face = BuildFace(lower_cut, lower_new_cut, upper_cut, upper_new_cut);
            builder_cutter.Add(m_cutPlanes, face);
        }

        // store base_pnts and remember current lower and upper point for next position
        base_pnts.push_back(base_point);
        upper = upper_new;
        lower = lower_new;
        ++count;
    }

    // trim the wall
    // Step 1/2: Trim the wall by the fuselage
    {
        TopoDS_Compound cut_wall;
        builder_wall.MakeCompound(cut_wall);

        TopoDS_Shape result = SplitShape(wall, fuselageShape);
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(result, TopAbs_FACE, faceMap);
        for (int i = 0; i < faceMap.Extent(); ++i) {
            // check if face center is on the interior of the fuselage
            TopoDS_Face face = TopoDS::Face(faceMap(i+1));
            gp_Pnt faceCenter = GetCentralFacePoint(face);

            BRepClass3d_SolidClassifier clas3d(fuselageShape);
            clas3d.Perform(faceCenter, Precision::Confusion());

            if (clas3d.State() == TopAbs_IN) {
                builder_wall.Add(cut_wall, face);
            }
        }
        wall = cut_wall;
    }

    // Step 2/2: Cut the wall with bounding elements:
    if (GetBoundingElementUIDs()) {
        for (std::string bounding_element_uid : GetBoundingElementUIDs()->GetBoundingElementUIDs()) {
            const CCPACSFuselageWallSegment& bounding_element = GetWalls().GetWallSegment(bounding_element_uid);
            TopoDS_Compound bounding_cutPlane = bounding_element.GetCutPlanes();
            TopoDS_Shape result = SplitShape(wall, bounding_cutPlane);
            
            TopoDS_Compound cut_wall;
            builder_wall.MakeCompound(cut_wall);
            TopTools_IndexedMapOfShape faceMap;
            TopExp::MapShapes(result, TopAbs_FACE, faceMap);

            // Loop over the split faces and check weather one of the
            // base points belongs to the face.
            for (int i = 0; i <faceMap.Extent(); ++i) {
                TopoDS_Face face = TopoDS::Face(faceMap(i+1));
                for (auto point : base_pnts) {
                    if ( IsPointInsideFace(face, point) ) {
                        builder_wall.Add(cut_wall,face);
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
