/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
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

#ifndef CTIGLINTERPOLATEPOINTSWITHKINKS_H
#define CTIGLINTERPOLATEPOINTSWITHKINKS_H

#include "tigl_internal.h"

#include <TColgp_HArray1OfPnt.hxx>
#include <Geom_BSplineCurve.hxx>
#include <map>
#include <vector>
#include <Cache.h>

namespace tigl
{

using ParamMap = std::map<unsigned int, double>;

/**
 * @brief This algorithm allows to interpolate a list of points
 *
 * Compared to CTiglPointsToBSplineInterpolation, this algorithm allows
 * to define kinks for points. Also, the interpolation parameter
 * can be set for in a map.
 */

class CTiglInterpolatePointsWithKinks
{
public:
    TIGL_EXPORT CTiglInterpolatePointsWithKinks(const Handle(TColgp_HArray1OfPnt) & points,
                                                const std::vector<unsigned int>& kinkIndices,
                                                const ParamMap& parameters,
                                                double alpha = 0.5,
                                                unsigned int maxDegree=3);


    TIGL_EXPORT Handle(Geom_BSplineCurve) Curve() const;
    TIGL_EXPORT std::vector<double> Parameters() const;


private:
    const Handle(TColgp_HArray1OfPnt) & m_pnts;
    std::vector<unsigned int> m_kinks;
    ParamMap m_params;
    double m_alpha;
    unsigned int m_maxDegree;

    struct Result {
        Handle(Geom_BSplineCurve) curve;
        std::vector<double> parameters;
    };

    void ComputeResult(Result& result) const;
    Cache<Result, CTiglInterpolatePointsWithKinks> m_result;

};

} // namespace tigl

#endif // CTIGLINTERPOLATEPOINTSWITHKINKS_H
