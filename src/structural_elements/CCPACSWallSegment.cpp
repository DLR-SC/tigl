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

#include "CCPACSWallSegment.h"
#include "generated/CPACSWallSegments.h"
#include "generated/CPACSWalls.h"
#include "CCPACSFuselageStructure.h"
#include "CCPACSFuselage.h"
#include "CCPACSHull.h"
#include "CCPACSHullStructure.h"
#include "CCPACSFuselageSegment.h"
#include "ITiglWallUtils.h"
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

namespace
{

// Given a CCPACSWallposition with a defined shape, project the point pnt along the direction dir onto the
// plane, in which the shape lies. The shape must be convertable to a planar TopoDS_Face.
void FlushPointAlongVec(tigl::CCPACSWallPosition const& p, gp_Vec const dir, double extents, gp_Pnt& pnt)
{
    if (!p.GetShape()) {
        throw tigl::CTiglError("Cannot flush point for wall position without shape definition.");
    }
    gp_Pln pln;
    TopoDS_Shape shape        = *p.GetShape();
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
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(pnt.Translated(dir * extents), pnt.Translated(-dir * extents)).Edge();
    GetIntersectionPoint(face, edge, pnt);
}

} // namespace

namespace tigl
{

CCPACSWallSegment::CCPACSWallSegment(CCPACSWallSegments* parent, tigl::CTiglUIDManager* uidMgr)
    : generated::CPACSWallSegment(parent, uidMgr)
{
}

TopoDS_Compound CCPACSWallSegment::GetCutPlanes() const
{
    // Makes sure, that the cut planes are already computed
    if (_cutPlanes.IsNull()) {
        BuildLoft();
    }
    return _cutPlanes;
}

void CCPACSWallSegment::SetPhi(const double& value)
{
    CPACSWallSegment::SetPhi(value);
    Invalidate();
}

void CCPACSWallSegment::SetDoubleSidedExtrusion(const boost::optional<bool>& value)
{
    CPACSWallSegment::SetDoubleSidedExtrusion(value);
    Invalidate();
}

void CCPACSWallSegment::InvalidateImpl(const boost::optional<std::string>& source) const
{
    loft.clear();
}

const CCPACSWalls& CCPACSWallSegment::GetWalls() const
{
    return tigl::GetWalls(this);
}

const CCPACSFuselage& CCPACSWallSegment::GetFuselage() const
{
    return tigl::GetFuselage(this);
}

const CCPACSHull& CCPACSWallSegment::GetHull() const
{
    return tigl::GetHull(this);
}

PNamedShape CCPACSWallSegment::BuildLoft() const
{
    // A bounding box is created to use its diagonal as reference for the
    // extrusion length (edge length). This ensures that these always go
    //  beyond the model, but do not reach into infinity.
    Bnd_Box boundingBox;
    TopoDS_Shape parentShape;
    CTiglTransformation transformationMatrix;

    bool isFuselage = GetParent()->GetParent()->IsParent<CCPACSFuselageStructure>();
    bool isHull     = GetParent()->GetParent()->IsParent<CCPACSHullStructure>();

    if (isFuselage) {
        const CCPACSFuselage& fuselage = GetFuselage();
        parentShape                    = fuselage.GetLoft()->Shape();
        transformationMatrix           = fuselage.GetTransformationMatrix();
    }
    else if (isHull) {
        const CCPACSHull& hull = GetHull();
        parentShape            = hull.GetLoft()->Shape();
        transformationMatrix   = hull.GetTransformationMatrix();
    }
    else {
        throw CTiglError("Parent of CCPACSWallSegment is neither a fuselage nor a hull.");
    }

    BRepBndLib::Add(parentShape, boundingBox);
    double bboxSize = sqrt(boundingBox.SquareExtent());

    size_t nWallPositions = GetWallPositionUIDs().GetWallPositionUIDs().size();

    // list with base points:
    std::vector<gp_Pnt> basePoints;
    basePoints.reserve(nWallPositions);

    // extrusion in negative direction is realized by setting uNeg to
    // -bboxSize:
    bool negativeExtrusion = GetDoubleSidedExtrusion().value_or(false);
    double uPositive       = bboxSize;
    double uNegative       = 0.;
    if (negativeExtrusion) {
        uNegative = -bboxSize;
    }

    gp_Pnt upper, lower; // upper and lower extrusion points for each base point
    gp_Vec xVector; // vector from one base point to the next

    // build for untrimmed wall
    TopoDS_Builder builderWall, builderCutter;
    TopoDS_Compound wall;
    builderWall.MakeCompound(wall);
    builderCutter.MakeCompound(_cutPlanes);

    size_t count      = 0;
    const auto& walls = GetWalls();
    for (auto wallPositionUID : GetWallPositionUIDs().GetWallPositionUIDs()) {
        const CCPACSWallPosition& p = walls.GetWallPosition(wallPositionUID);

        // get current base point and x_vec pointing from previous base point to current
        gp_Pnt base_point = p.GetBasePoint();
        if (count > 0) {
            xVector = gp_Vec(basePoints.back(), base_point);
        }

        // extrusion vector
        double phiRad = Radians(GetPhi());
        gp_Vec extVec(0., -sin(phiRad), cos(phiRad));
        extVec = transformationMatrix.Transform(extVec);

        if (count == 1) {
            // flush first position to shape
            if (p.GetShape() && GetFlushConnectionStart().value_or(false)) {
                std::string uidPrev             = GetWallPositionUIDs().GetWallPositionUIDs().front();
                const CCPACSWallPosition& pPrev = walls.GetWallPosition(uidPrev);
                FlushPointAlongVec(pPrev, xVector, bboxSize, upper);
                FlushPointAlongVec(pPrev, xVector, bboxSize, lower);
            }
        }

        gp_Pnt upperNew = base_point.Translated(uPositive * extVec);
        gp_Pnt lowerNew = base_point.Translated(uNegative * extVec);

        if (count == nWallPositions - 1) {
            // flush last position to shape
            if (p.GetShape() && GetFlushConnectionEnd().value_or(false)) {
                FlushPointAlongVec(p, xVector, bboxSize, upperNew);
                FlushPointAlongVec(p, xVector, bboxSize, lowerNew);
            }
        }

        // create the wall segment from the four corner points lower, upper, lower_new, upper_new
        if (count > 0) {
            TopoDS_Face face = BuildFace(lower, lowerNew, upper, upperNew);
            builderWall.Add(wall, face);

            //first and last face are enlarged for the cutting tool
            gp_Pnt lowerCut    = lower;
            gp_Pnt upperCut    = upper;
            gp_Pnt lowerNewCut = lowerNew;
            gp_Pnt upperNewCut = upperNew;
            if (count == 1) {
                lowerCut = lower.Translated(-bboxSize * xVector);
                upperCut = upper.Translated(-bboxSize * xVector);
            }
            if (count == nWallPositions - 1) {
                lowerNewCut = lowerNew.Translated(bboxSize * xVector);
                upperNewCut = upperNew.Translated(bboxSize * xVector);
            }
            face = BuildFace(lowerCut, lowerNewCut, upperCut, upperNewCut);
            builderCutter.Add(_cutPlanes, face);
        }

        // store base_pnts and remember current lower and upper point for next position
        basePoints.push_back(base_point);
        upper = upperNew;
        lower = lowerNew;
        ++count;
    }

    // trim the wall
    // Step 1/2: Trim the wall by the fuselage / hull
    {
        TopoDS_Compound cutWall;
        builderWall.MakeCompound(cutWall);

        TopoDS_Shape result = SplitShape(wall, parentShape);
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(result, TopAbs_FACE, faceMap);
        for (int i = 0; i < faceMap.Extent(); ++i) {
            // check if face center is on the interior of the fuselage / hull
            TopoDS_Face face  = TopoDS::Face(faceMap(i + 1));
            gp_Pnt faceCenter = GetCentralFacePoint(face);

            BRepClass3d_SolidClassifier clas3d(parentShape);
            clas3d.Perform(faceCenter, Precision::Confusion());

            if (clas3d.State() == TopAbs_IN) {
                builderWall.Add(cutWall, face);
            }
        }
        wall = cutWall;
    }

    // Step 2/2: Cut the wall with bounding elements:
    if (GetBoundingElementUIDs()) {
        for (std::string boundingElementUid : GetBoundingElementUIDs()->GetBoundingElementUIDs()) {
            const CCPACSWallSegment& boundingElement = GetWalls().GetWallSegment(boundingElementUid);
            TopoDS_Compound boundingCutPlane         = boundingElement.GetCutPlanes();
            TopoDS_Shape result                      = SplitShape(wall, boundingCutPlane);

            TopoDS_Compound cutWall;
            builderWall.MakeCompound(cutWall);
            TopTools_IndexedMapOfShape faceMap;
            TopExp::MapShapes(result, TopAbs_FACE, faceMap);

            // Loop over the split faces and check weather one of the
            // base points belongs to the face.
            for (int i = 0; i < faceMap.Extent(); ++i) {
                TopoDS_Face face = TopoDS::Face(faceMap(i + 1));
                for (auto point : basePoints) {
                    if (IsPointInsideFace(face, point)) {
                        builderWall.Add(cutWall, face);
                        break;
                    }
                }
            }
            wall = cutWall;
        } // loop over bounding elements
    } // if has bounding elements

    return PNamedShape(new CNamedShape(wall, GetDefaultedUID()));
}

} // namespace tigl
