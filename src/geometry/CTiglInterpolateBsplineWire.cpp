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
*         of points by a BSpline interpolation.
*/

#include "CTiglInterpolateBsplineWire.h"
#include "CTiglError.h"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "TopoDS_Edge.hxx"
#include "TColgp_HArray1OfPnt.hxx"
#include "CTiglPointsToBSplineInterpolation.h"
#include "Precision.hxx"
#include "math.h"
#include <algorithm>
#include <iostream>

namespace tigl 
{

// Constructor
CTiglInterpolateBsplineWire::CTiglInterpolateBsplineWire()
{
    continuity = _C0;
}

// Destructor
CTiglInterpolateBsplineWire::~CTiglInterpolateBsplineWire()
{
}

// Builds the wire from the given points
TopoDS_Wire CTiglInterpolateBsplineWire::BuildWire(const CPointContainer& points, bool forceClosed) const
{
    bool endTangency = (continuity == _C1);

    if (points.size() < 2) {
        throw CTiglError("To less points to build a curve in CTiglInterpolateBsplineWire::BuildCurve", TIGL_ERROR);
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


    // Test if we have to build a closed bspline curve from the remaining
    // points. This is true if the start and end point are very close to each other.
    int pointCount  = static_cast<Standard_Integer>(usedPoints.size());
    gp_Pnt startPnt = usedPoints[0];
    gp_Pnt endPnt   = usedPoints[pointCount - 1];

    if (forceClosed  && startPnt.Distance(endPnt) > Precision::Confusion()) {
        // do to compatibilty, tangency is only allowed if start and endpoint are specified the same
        endTangency = false;
    }

    if (pointCount < 2) {
        throw CTiglError("To less points to build a curve in CTiglInterpolateBsplineWire::BuildCurve", TIGL_ERROR);
    }

    Handle(TColgp_HArray1OfPnt) hpoints = new TColgp_HArray1OfPnt(1, pointCount);
    for (int j = 0; j < pointCount; j++) {
        hpoints->SetValue(j + 1, usedPoints[j]);
    }

    CTiglPointsToBSplineInterpolation interpol(hpoints, 3, endTangency);
    Handle(Geom_BSplineCurve) hcurve = interpol.Curve();

    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hcurve);
    BRepBuilderAPI_MakeWire wireBuilder(edge);
    if (!wireBuilder.IsDone()) {
        throw CTiglError("Wire construction failed in CTiglInterpolateBsplineWire::BuildWire", TIGL_ERROR);
    }

    TopoDS_Wire wire = wireBuilder.Wire();

    if (forceClosed && !hcurve->IsClosed()) {
        edge = BRepBuilderAPI_MakeEdge(endPnt, startPnt);
        wire = BRepBuilderAPI_MakeWire(wire, edge).Wire();
        if (!wire.Closed()) {
            throw CTiglError("Wire closing failed in CTiglInterpolateBsplineWire::BuildWire", TIGL_ERROR);
        }
    }

    return wire;
}

// Returns the algorithm code identifier for an algorithm
TiglAlgorithmCode CTiglInterpolateBsplineWire::GetAlgorithmCode() const 
{
    return TIGL_INTERPOLATE_BSPLINE_WIRE;
}

// Returns the point on the wire with the smallest x value
gp_Pnt CTiglInterpolateBsplineWire::GetPointWithMinX(const CPointContainer& points) const
{        
    if (points.size() == 0) {
        throw CTiglError("To less points in CTiglInterpolateBsplineWire::GetPointWithMinX", TIGL_ERROR);
    }

    gp_Pnt minXPnt = points[0];
    for (CPointContainer::size_type i = 1; i < points.size(); i++) {
        if (points[i].X() < minXPnt.X()) {
            minXPnt = points[i];
        }
    }
    return minXPnt;
}

// Returns the point on the wire with the biggest x value
gp_Pnt CTiglInterpolateBsplineWire::GetPointWithMaxX(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("To less points in CTiglInterpolateBsplineWire::GetPointWithMaxX", TIGL_ERROR);
    }

    gp_Pnt maxXPnt = points[0];
    for (CPointContainer::size_type i = 1; i < points.size(); i++) {
        if (points[i].X() > maxXPnt.X()) {
            maxXPnt = points[i];
        }
    }
    return maxXPnt;
}

// Returns the point on the wire with the smallest y value
gp_Pnt CTiglInterpolateBsplineWire::GetPointWithMinY(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("To less points in CTiglInterpolateBsplineWire::GetPointWithMinY", TIGL_ERROR);
    }

    gp_Pnt minYPnt = points[0];
    for (CPointContainer::size_type i = 1; i < points.size(); i++) {
        if (points[i].Y() < minYPnt.Y()) {
            minYPnt = points[i];
        }
    }
    return minYPnt;
}

// Returns the point on the wire with the biggest y value
gp_Pnt CTiglInterpolateBsplineWire::GetPointWithMaxY(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("To less points in CTiglInterpolateBsplineWire::GetPointWithMaxY", TIGL_ERROR);
    }

    gp_Pnt maxYPnt = points[0];
    for (CPointContainer::size_type i = 1; i < points.size(); i++) {
        if (points[i].Y() > maxYPnt.Y()) {
            maxYPnt = points[i];
        }
    }
    return maxYPnt;
}

} // end namespace tigl
