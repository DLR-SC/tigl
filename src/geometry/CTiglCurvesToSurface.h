/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2018-07-18 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include "tigl_internal.h"

#include <vector>

#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>

#include <Geom_RectangularTrimmedSurface.hxx>

namespace tigl {

class CTiglCurvesToSurface
{
public:
    /**
     * @brief Surface skinning algorithm
     *
     * Creates a surface by interpolation of B-spline curves. The direction of the input curves
     * is treated as u direction. The skinning will be performed in v direction. The interpolation
     * parameters will be determined automatically.
     *
     * By default, the curves are skinned continuously. This can be changed by setting the maximum degree
     * of the interpolation in v-direction using ::SetMaxDegree.
     *
     * @param splines_vector Curves to be interpolated.
     * @param continuousIfClosed Make a C2 continous surface at the start/end junction if the first and last curve are the same
     */
    TIGL_EXPORT explicit CTiglCurvesToSurface(const std::vector<Handle(Geom_Curve) >& splines_vector,
                                              bool continuousIfClosed = false);

    /**
     * @brief Surface skinning algorithm
     *
     * Creates a surface by interpolation of B-spline curves. The direction of the input curves
     * is treated as u direction. The skinning will be performed in v direction.
     *
     * @param splines_vector Curves to be interpolated.
     * @param parameters Parameters of v-direction at which the resulting surface should interpolate the input curves.
     * @param continuousIfClosed Make a C2 continous surface at the start/end junction if the first and last curve are the same
     */
    TIGL_EXPORT explicit CTiglCurvesToSurface(const std::vector<Handle(Geom_Curve) >& splines_vector,
                                              const std::vector<double>& parameters,
                                              bool continuousIfClosed = false);
    /**
     * @brief sets the maximum interpolation degree of the splines in skinning direction
     *
     * @param maxDegree maximum degree of the splines in skinning direction
     */
    TIGL_EXPORT void SetMaxDegree(int degree);

    /**
     * @brief returns the parameters at the profile curves
     */
    TIGL_EXPORT std::vector<double> GetParameters() const
    {
        return _parameters;
    }

    /**
      * @brief returns the skinned surface
      */
    Handle(Geom_BSplineSurface) Surface() {
        if ( !_hasPerformed ) {
            Perform();
        }
        return _skinnedSurface;
    }

private:
    TIGL_EXPORT void CalculateParameters(std::vector<Handle(Geom_BSplineCurve)> const& splines_vector);
    TIGL_EXPORT void Perform();
    TIGL_EXPORT void Invalidate() { _hasPerformed = false; }

    Handle(Geom_BSplineSurface) _skinnedSurface = nullptr;

    std::vector<Handle(Geom_BSplineCurve) > _inputCurves;
    std::vector<Handle(Geom_BSplineCurve) > _compatibleSplines;
    std::vector<double> _parameters;
    bool _continuousIfClosed = false;
    bool _hasPerformed = false;
    int _maxDegree = 3;
};

}
