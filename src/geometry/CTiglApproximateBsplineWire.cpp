/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file 
* @brief  Implementation of routines for building a wire from a std::vector
*         of points by a 3D BSpline curve which approximates the set of points.
*         The resulting curve may not pass through the given points, so that's
*         the difference to the BSpline interpolation algorithm.
*/

#include "CTiglApproximateBsplineWire.h"
#include "CTiglError.h"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "TopoDS_Edge.hxx"
#include "TColgp_Array1OfPnt.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "Geom_BSplineCurve.hxx"
#include "GeomAbs_Shape.hxx"
#include "Precision.hxx"
            
namespace tigl 
{

// Constructor
CTiglApproximateBsplineWire::CTiglApproximateBsplineWire()
{
}

// Destructor
CTiglApproximateBsplineWire::~CTiglApproximateBsplineWire(void)
{
}

// Builds the wire from the given points
TopoDS_Wire CTiglApproximateBsplineWire::BuildWire(const CPointContainer& points, bool forceClosed) const
{
    if (points.size() < 2) {
        throw CTiglError("Error: To less points to build a wire in CTiglApproximateBsplineWire::BuildWire", TIGL_ERROR);
    }

    // If first and last point are identical always force wire closure independently of given forceClosed flag.
    if (points[0].Distance(points[points.size() - 1]) <= Precision::Confusion()) {
        forceClosed = true;
    }

    TColgp_Array1OfPnt pointsArray(1, static_cast<Standard_Integer>(points.size()));
    for (CPointContainer::size_type i = 0; i < points.size(); i++) {
        pointsArray.SetValue(static_cast<Standard_Integer>(i + 1), points[i]);
    }

    Handle(Geom_BSplineCurve) hcurve = GeomAPI_PointsToBSpline(
        pointsArray, 
        Geom_BSplineCurve::MaxDegree() - 6, 
        Geom_BSplineCurve::MaxDegree(), 
        GeomAbs_C2, 
        Precision::Confusion()).Curve();

    // This one works around a bug in OpenCascade if a curve is closed and
    // periodic. After calling this method, the curve is still closed but
    // no longer periodic, which leads to errors when creating the 3d-lofts
    // from the curves.
    hcurve->SetNotPeriodic();

    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hcurve);
    BRepBuilderAPI_MakeWire wireBuilder(edge);
    if (!wireBuilder.IsDone()) {
        throw CTiglError("Error: Wire construction failed in CTiglApproximateBsplineWire::BuildWire", TIGL_ERROR);
    }

    TopoDS_Wire wire = wireBuilder.Wire();

    if (forceClosed && !hcurve->IsClosed()) {
        gp_Pnt startPnt = hcurve->StartPoint();
        gp_Pnt endPnt   = hcurve->EndPoint();
        if (startPnt.Distance(endPnt) <= Precision::Confusion()) {
            throw CTiglError("Error: Can't find a valid start and end point for wire closing in"
                             "CTiglApproximateBsplineWire::BuildWire", TIGL_ERROR);
        }
        edge = BRepBuilderAPI_MakeEdge(endPnt, startPnt);
        wire = BRepBuilderAPI_MakeWire(wire, edge).Wire();
        if (!wire.Closed()) {
            throw CTiglError("Error: Wire closing failed in CTiglApproximateBsplineWire::BuildWire", TIGL_ERROR);
        }
    }

    return wire;
}

// Returns the algorithm code identifier for an algorithm
TiglAlgorithmCode CTiglApproximateBsplineWire::GetAlgorithmCode(void) const
{
    return TIGL_APPROXIMATE_BSPLINE_WIRE;
}

// Returns the point on the wire with the smallest x value
gp_Pnt CTiglApproximateBsplineWire::GetPointWithMinX(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("Error: To less points in CTiglInterpolateBsplineWire::GetPointWithMinX", TIGL_ERROR);
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
gp_Pnt CTiglApproximateBsplineWire::GetPointWithMaxX(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("Error: To less points in CTiglInterpolateBsplineWire::GetPointWithMaxX", TIGL_ERROR);
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
gp_Pnt CTiglApproximateBsplineWire::GetPointWithMinY(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("Error: To less points in CTiglInterpolateBsplineWire::GetPointWithMinY", TIGL_ERROR);
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
gp_Pnt CTiglApproximateBsplineWire::GetPointWithMaxY(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("Error: To less points in CTiglInterpolateBsplineWire::GetPointWithMaxY", TIGL_ERROR);
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
