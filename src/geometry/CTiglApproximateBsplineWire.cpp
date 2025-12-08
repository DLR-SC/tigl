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
*         of points by a 3D BSpline curve which approximates the set of points.
*         The resulting curve may not pass through the given points, so that's
*         the difference to the BSpline interpolation algorithm.
*/

#include "CTiglApproximateBsplineWire.h"
#include "CTiglBSplineApproxInterp.h"
#include "CTiglError.h"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "TopoDS_Edge.hxx"
#include "TColgp_Array1OfPnt.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "Geom_BSplineCurve.hxx"
#include "GeomAbs_Shape.hxx"
#include "Precision.hxx"
#include "CTiglLogging.h"
            
namespace tigl 
{

// Constructor
CTiglApproximateBsplineWire::CTiglApproximateBsplineWire()
{
    continuity = _C0;
    m_isWingProfile = false;
}

// Destructor
CTiglApproximateBsplineWire::~CTiglApproximateBsplineWire()
{
}

CTiglApproximateBsplineWire::CTiglApproximateBsplineWire(int nrControlPoints, const std::string& profileUID, bool isWingProfile)
{
    continuity = _C0;
    m_profileUID = &profileUID;
    m_approximationSettings = nrControlPoints;
    m_isWingProfile = isWingProfile;
}

CTiglApproximateBsplineWire::CTiglApproximateBsplineWire(double tolerance, const std::string& profileUID, bool isWingProfile)
{
    continuity = _C0;
    m_profileUID = &profileUID;
    m_approximationSettings = tolerance;
    m_isWingProfile = isWingProfile;
}

// Builds the wire from the given points
TopoDS_Wire CTiglApproximateBsplineWire::BuildWire(const CPointContainer& points, bool forceClosed) const
{
    bool endTangency = (continuity == _C1);

    if (points.size() < 2) {
        throw CTiglError("Too few points to build a curve in CTiglApproximateBsplineWire::BuildWire", TIGL_ERROR);
    }

    // If first and last point are identical always force wire closure independently of given forceClosed flag.
    if (points[0].Distance(points[points.size() - 1]) <= Precision::Confusion()) {
        forceClosed = true;
    }

    // Remove points which are too close to each other.
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
        // due to compatibility, tangency is only allowed if start and endpoint are specified the same
        endTangency = false;
    }

    if (pointCount < 2) {
        throw CTiglError("Too few points to build a curve in CTiglApproximateBsplineWire::BuildWire", TIGL_ERROR);
    }

    Handle(TColgp_HArray1OfPnt) hpoints = new TColgp_HArray1OfPnt(1, pointCount);
    for (int j = 0; j < pointCount; j++) {
        hpoints->SetValue(j + 1, usedPoints[j]);
    }

    Handle(Geom_BSplineCurve) hcurve;
    if (std::holds_alternative<int>(m_approximationSettings)) {
        int nrControlPoints = std::get<int>(m_approximationSettings);
        double errApproxCalc = -1.;

        if (nrControlPoints < 3) {
            throw CTiglError("CTiglApproximateBsplineWire::BuildWire: controlPointNumber must be 3 or larger");
        }

        CTiglBSplineApproxInterp approx(*hpoints, nrControlPoints, 3, endTangency);

        if (m_isWingProfile) {
            // Make sure that the first and last point is still interpolated to ensure a closed wing profile
            approx.InterpolatePoint(0);
            approx.InterpolatePoint(hpoints->Length()-1);
        }

        // Potentially use `CTiglBSplineApproxInterp::FitCurveOptimal`? Do we want to allow 'optimizing' the parameters?
        // IDEA: Choice for error calc could be also implemented in CPACS? Guess, thats no big deal... -> More user flexibility
        CTiglApproxResult approxResult = approx.FitCurve(std::vector<double>(), calcPointVecErrorRMSE);
        LOG(WARNING) << "I am approximated" << std::endl;
        hcurve = approxResult.curve;
        errApproxCalc = approxResult.error;
        LOG(WARNING) << "#Poles: " << hcurve->NbPoles();
        LOG(WARNING) << "The profile with uID '" << *m_profileUID << "' is created by approximating the point list. This leads to a root mean square error of " << errApproxCalc << "." << std::endl;
    }
    else if (std::holds_alternative<double>(m_approximationSettings)) {
        double maxErrorApprox = std::get<double>(m_approximationSettings);
        throw CTiglError("CTiglApproximateBsplineWire::BuildWire: 'Max Error' open for implementation");
    }
    else {
        throw CTiglError("Invalid defintion of approximationSettings");
    }

    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hcurve);
    BRepBuilderAPI_MakeWire wireBuilder(edge);
    if (!wireBuilder.IsDone()) {
        throw CTiglError("Wire construction failed in CTiglApproximateBsplineWire::BuildWire", TIGL_ERROR);
    }

    TopoDS_Wire wire = wireBuilder.Wire();

    if (forceClosed && !hcurve->IsClosed()) {
        edge = BRepBuilderAPI_MakeEdge(endPnt, startPnt);
        wire = BRepBuilderAPI_MakeWire(wire, edge).Wire();
        if (!wire.Closed()) {
            throw CTiglError("Wire closing failed in CTiglApproximateBsplineWire::BuildWire", TIGL_ERROR);
        }
    }

    return wire;
}

// Returns the algorithm code identifier for an algorithm
TiglAlgorithmCode CTiglApproximateBsplineWire::GetAlgorithmCode() const
{
    return TIGL_APPROXIMATE_BSPLINE_WIRE;
}

// Returns the point on the wire with the smallest x value
gp_Pnt CTiglApproximateBsplineWire::GetPointWithMinX(const CPointContainer& points) const
{
    if (points.size() == 0) {
        throw CTiglError("Too few points in CTiglApproximateBsplineWire::GetPointWithMinX", TIGL_ERROR);
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
        throw CTiglError("Too few points in CTiglApproximateBsplineWire::GetPointWithMaxX", TIGL_ERROR);
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
        throw CTiglError("Too few points in CTiglApproximateBsplineWire::GetPointWithMinY", TIGL_ERROR);
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
        throw CTiglError("Too few points in CTiglApproximateBsplineWire::GetPointWithMaxY", TIGL_ERROR);
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
