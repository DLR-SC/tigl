/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
/**
* @file 
* @brief  Implementation of routines for building a wire from a std::vector
*         of points by linear interpolation.
*/

#include "CTiglInterpolateLinearWire.h"
#include "CTiglError.h"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "TopoDS_Edge.hxx"
#include "Precision.hxx"
#include "ShapeFix_Wire.hxx"


namespace tigl 
{

// Constructor
CTiglInterpolateLinearWire::CTiglInterpolateLinearWire()
{
}

// Destructor
CTiglInterpolateLinearWire::~CTiglInterpolateLinearWire()
{
}

// Builds the wire from the given points
TopoDS_Wire CTiglInterpolateLinearWire::BuildWire(const CPointContainer& points, bool forceClosed) const
{
    if (points.size() < 2) {
        throw CTiglError("To less points to build a wire in CTiglInterpolateLinearWire::BuildWire", TIGL_ERROR);
    }

    // If first and last point are identical always force wire closure independently of given forceClosed flag.
    if (points[0].Distance(points[points.size() - 1]) <= Precision::Confusion()) {
        forceClosed = true;
    }

    // Remove points which are to close to each other.
    gp_Pnt prevPnt = points[0];
    CPointContainer usedPoints;
    usedPoints.push_back(prevPnt);
    for (CPointContainer::size_type i = 1; i < points.size(); i++) {
        gp_Pnt nextPnt = points[i];
        if (prevPnt.Distance(nextPnt) <= Precision::Confusion()) {
            continue;
        }
        usedPoints.push_back(nextPnt);
        prevPnt = nextPnt;
    }

    // Find an end point of the wire, which is not to close to the
    // wire start point.
    int pointCount  = static_cast<Standard_Integer>(usedPoints.size());
    gp_Pnt startPnt = usedPoints[0];
    gp_Pnt endPnt   = usedPoints[pointCount - 1];
    while (pointCount > 1 && startPnt.Distance(endPnt) <= Precision::Confusion()) {
        pointCount--;
        forceClosed = true;
        endPnt = usedPoints[pointCount - 1];
    }

    if (pointCount < 2) {
        throw CTiglError("To less points to close wire in CTiglInterpolateLinearWire::BuildWire", TIGL_ERROR);
    }

    BRepBuilderAPI_MakeWire wireBuilder;
    for (int j = 1; j < pointCount; j++) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(usedPoints[j - 1], usedPoints[j]);
        wireBuilder.Add(edge);
        if (wireBuilder.IsDone() != Standard_True) {
            throw CTiglError("Wire construction failed in CTiglInterpolateLinearWire::BuildWire", TIGL_ERROR);
        }
    }

    TopoDS_Wire wire = wireBuilder.Wire();

    if (forceClosed && !wire.Closed()) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(endPnt, startPnt);
        wire = BRepBuilderAPI_MakeWire(wire, edge).Wire();
        if (!wire.Closed()) {
            throw CTiglError("Wire closing failed in CTiglInterpolateLinearWire::BuildWire", TIGL_ERROR);
        }
    }

    return wire;
}

// Returns the algorithm code identifier for an algorithm
TiglAlgorithmCode CTiglInterpolateLinearWire::GetAlgorithmCode() const
{
    return TIGL_INTERPOLATE_LINEAR_WIRE;
}

// Returns the point on the wire with the smallest x value
gp_Pnt CTiglInterpolateLinearWire::GetPointWithMinX(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("To less points in CTiglInterpolateBsplineWire::GetPointWithMinX", TIGL_ERROR);
    }

    gp_Pnt minXPnt = points[0];
    for (CPointContainer::size_type i = 0; i < points.size(); i++) {
        if (points[i].X() < minXPnt.X()) {
            minXPnt = points[i];
        }
    }
    return minXPnt;
}

// Returns the point on the wire with the biggest x value
gp_Pnt CTiglInterpolateLinearWire::GetPointWithMaxX(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("To less points in CTiglInterpolateBsplineWire::GetPointWithMaxX", TIGL_ERROR);
    }

    gp_Pnt maxXPnt = points[0];
    for (CPointContainer::size_type i = 0; i < points.size(); i++) {
        if (points[i].X() > maxXPnt.X()) {
            maxXPnt = points[i];
        }
    }
    return maxXPnt;
}

// Returns the point on the wire with the smallest y value
gp_Pnt CTiglInterpolateLinearWire::GetPointWithMinY(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("To less points in CTiglInterpolateBsplineWire::GetPointWithMinY", TIGL_ERROR);
    }

    gp_Pnt minYPnt = points[0];
    for (CPointContainer::size_type i = 0; i < points.size(); i++) {
        if (points[i].Y() < minYPnt.Y()) {
            minYPnt = points[i];
        }
    }
    return minYPnt;
}

// Returns the point on the wire with the biggest y value
gp_Pnt CTiglInterpolateLinearWire::GetPointWithMaxY(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("To less points in CTiglInterpolateBsplineWire::GetPointWithMaxY", TIGL_ERROR);
    }

    gp_Pnt maxYPnt = points[0];
    for (CPointContainer::size_type i = 0; i < points.size(); i++) {
        if (points[i].Y() > maxYPnt.Y()) {
            maxYPnt = points[i];
        }
    }
    return maxYPnt;
}


} // end namespace tigl
