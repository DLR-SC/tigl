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

#ifndef CTIGLAPPROXIMATEBSPLINEWIRE_H
#define CTIGLAPPROXIMATEBSPLINEWIRE_H

#include "tigl_internal.h"
#include "ITiglWireAlgorithm.h"
#include <variant>

namespace tigl 
{

class CTiglApproximateBsplineWire : public ITiglWireAlgorithm
{

public:
    // Constructor
    TIGL_EXPORT CTiglApproximateBsplineWire(int nrControlPoints, const std::string approxErrStr="RMSE",
                                            std::vector<double> interpolatedPointsIndices=std::vector<double>{});

    TIGL_EXPORT CTiglApproximateBsplineWire(double tolerance, const std::string approxErrStr="RMSE",
                                            std::vector<double> interpolatedPointsIndices=std::vector<double>{});

    TIGL_EXPORT CTiglApproximateBsplineWire(int nrControlPoints, const std::string approxErrStr,
                                            std::vector<double> interpolatedPointsIndices,
                                            std::vector<double> initialParams);

    TIGL_EXPORT CTiglApproximateBsplineWire(double tolerance, const std::string approxErrStr,
                                            std::vector<double> interpolatedPointsIndices,
                                            std::vector<double> initialParams);

    // Destructor
    TIGL_EXPORT ~CTiglApproximateBsplineWire() override;

    // Builds the wire from the given points
    TIGL_EXPORT TopoDS_Wire BuildWire(const CPointContainer& points, bool forceClosed = false) const override;

    TIGL_EXPORT int GetUsedNrPoles() const;

    TIGL_EXPORT std::string GetApproxErrStrLong() const;

    TIGL_EXPORT double GetApproxErr() const;

    // Returns the algorithm code identifier for an algorithm
    TIGL_EXPORT TiglAlgorithmCode GetAlgorithmCode() const override;

    // Returns the point on the wire with the smallest x value
    TIGL_EXPORT gp_Pnt GetPointWithMinX(const CPointContainer& points) const override;

    // Returns the point on the wire with the biggest x value
    TIGL_EXPORT gp_Pnt GetPointWithMaxX(const CPointContainer& points) const override;

    // Returns the point on the wire with the smallest y value
    TIGL_EXPORT gp_Pnt GetPointWithMinY(const CPointContainer& points) const override;

    // Returns the point on the wire with the biggest y value
    TIGL_EXPORT gp_Pnt GetPointWithMaxY(const CPointContainer& points) const override;

private:
    // Copy constructor
    CTiglApproximateBsplineWire(const CTiglApproximateBsplineWire& ) { /* Do nothing */ }

    // Assignment operator
    void operator=(const CTiglApproximateBsplineWire& ) { /* Do nothing */ }

    ETiglContinuity continuity;

    std::variant<std::monostate, int, double> m_approximationSettings;

    const std::string* m_profileUID;

    bool m_interpStartEnd;

    bool m_interpFarestPntFromStartEnd; // Defines whether the farest point from the averaged starting and end point should be interpolated

    const std::string m_approxErrStr; // Currently allowed entries are "RMSE", "MaxError"

    mutable std::string m_approxErrStrLong; // Currently allowed entries are "root mean square error", "maximum error"

    mutable int m_usedNrPoles; // Number of actually used poles (might differ from user input due to interpolated points)

    mutable double m_approxErr; // Approximation error according to chosen computation method

    std::vector<double> m_interpolatedPointsIndices; // Contains indices of points that should still be interpolated

    std::vector<double> m_initialParams; // Initial parameter values for approximation
};

} // end namespace tigl

#endif // CTIGLAPPROXIMATEBSPLINEWIRE_H
