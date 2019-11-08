/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-10-16 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#pragma once

#include "CNamedShape.h"
#include "CTiglUIDManager.h"
#include "CCPACSWallPosition.h"
#include "CCPACSFuselageWallSegment.h"
#include "CCPACSFuselageStructure.h"
#include "CCPACSFuselage.h"
#include "CTiglError.h"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "gp_Lin.hxx"
#include "IntCurvesFace_ShapeIntersector.hxx"

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

}

namespace tigl
{

CCPACSWallPosition::CCPACSWallPosition(CCPACSWallPositions* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWallPosition(parent, uidMgr)
{}

void CCPACSWallPosition::CalcBasePointAndShape() const
{
    Bnd_Box boundingBox;
    CCPACSFuselage const& fuselage = GetFuselage();
    TopoDS_Shape fuselageShape = fuselage.GetLoft()->Shape();
    BRepBndLib::Add(fuselageShape, boundingBox);
    double bboxSize = sqrt(boundingBox.SquareExtent());

    // calculate base point and optional shape
    double x = 0;
    double y = GetY();
    double z = GetZ();
    if (GetX_choice4()) {
        x = GetX_choice4().value();
    }
    else if(GetBulkheadUID_choice1()) {
        shape = GetUIDManager().GetGeometricComponent(GetBulkheadUID_choice1().value()).GetLoft()->Shape();
        x = GetXCoord(*shape, y, z, bboxSize);
    }
    else if(GetWallSegmentUID_choice2()) {
        const CCPACSFuselageWallSegment& wall = GetUIDManager().ResolveObject<CCPACSFuselageWallSegment>(GetWallSegmentUID_choice2().value());
        if ( wall.GetDefaultedUID() == GetWallSegmentUID_choice2().value() ){
            throw CTiglError("Fuselage wall references itself");
        }
        shape = wall.GetLoft()->Shape();
        x = GetXCoord(*shape, y, z, bboxSize);
    }
    else if (GetFuselageSectionUID_choice3()) {
        shape = fuselage.GetSectionFace(GetFuselageSectionUID_choice3().value());
        x = GetXCoord(*shape, y, z, bboxSize);
    }
    else {
        throw CTiglError("Cannot determine x coordinate of wall position.");
    }
    base_point = gp_Pnt(x,y,z);
    isBuilt = true;
}

gp_Pnt& CCPACSWallPosition::GetBasePoint()
{
    return const_cast<gp_Pnt&>(const_cast<const CCPACSWallPosition*>(this)->GetBasePoint());
}


gp_Pnt const& CCPACSWallPosition::GetBasePoint() const
{
    if (!isBuilt) {
        CalcBasePointAndShape();
    }
    return base_point;
}

boost::optional<TopoDS_Shape>& CCPACSWallPosition::GetShape()
{
    return const_cast<boost::optional<TopoDS_Shape>&>(const_cast<const CCPACSWallPosition*>(this)->GetShape());
}

boost::optional<TopoDS_Shape> const& CCPACSWallPosition::GetShape() const
{
    if (!isBuilt) {
        CalcBasePointAndShape();
    }
    return shape;
}

const CCPACSWalls& CCPACSWallPosition::GetWalls() const
{
    const CCPACSWallPositions* wallPositions = GetParent();
    if (!wallPositions) {
        throw CTiglError("Error in CCPACSWallPosition::GetWalls. Null pointer returned.", TIGL_NULL_POINTER);
    }

    const CCPACSWalls* walls = wallPositions->GetParent();
    if (!walls) {
        throw CTiglError("Error in CCPACSWallPosition::GetWalls. Null pointer returned.", TIGL_NULL_POINTER);
    }

    return *walls;
}

const CCPACSFuselage &CCPACSWallPosition::GetFuselage() const
{
    const CCPACSWalls& walls = GetWalls();

    const CCPACSFuselageStructure* fuselageStructure = walls.GetParent();
    if (!fuselageStructure) {
        throw CTiglError("Cannot get fuselage in CCPACSWallPosition::GetFuselage. Null pointer parent.", TIGL_NULL_POINTER);
    }

    const CCPACSFuselage* fuselage = fuselageStructure->GetParent();
    if (!fuselageStructure) {
        throw CTiglError("Cannot get fuselage in CCPACSWallPosition::GetFuselage. Null pointer parent.", TIGL_NULL_POINTER);
    }

    return *fuselage;
}

}
