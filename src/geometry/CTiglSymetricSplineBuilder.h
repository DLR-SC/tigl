/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-06-10 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLSYMETRICSPLINEBUILDER_H
#define CTIGLSYMETRICSPLINEBUILDER_H

#include "tigl_internal.h"

#include <gp_Pnt.hxx>
#include <vector>

class Handle_Geom_BSplineCurve;

namespace tigl
{

/**
 * @brief CTiglSymetricSplineBuilder creates an interpolating
 * BSpline, that is symmetric to a main axis plane.
 * 
 * The algorithm mirrors each point at the symmetry plane
 * and computes the bspline and symmetrizes it. Finally,
 * the BSpline is trimmed to the first half of the curve (i.e.
 * it matches the input data)
 * 
 * Currently, only splines symmetric to the x-z plane are supported.
 * The y-Coordinate of the first point must be exactly zero!
 */
class CTiglSymetricSplineBuilder
{
public:
    typedef std::vector<gp_Pnt> CPointContainer;

    TIGL_EXPORT CTiglSymetricSplineBuilder(const CPointContainer& points);

    TIGL_EXPORT Handle_Geom_BSplineCurve GetBSpline() const;

private:
    void checkInputData() const;

    const CPointContainer& _points;
};

} //namespace tigl

#endif // CTIGLSYMETRICSPLINEBUILDER_H
