/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-01-18 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLARCLENGTHREPARAMETERIZATION_H
#define CTIGLARCLENGTHREPARAMETERIZATION_H

#include <Geom_BSplineCurve.hxx>

namespace tigl
{

/**
 * @brief This provides an algorithm for an arc-length
 * reparameterization of a bspline curve
 *
 * This class uses the techniques from the paper
 * Peterson J. Arc Length Parameterization of Spline Curves
 */
class CTiglArcLengthReparameterization
{
public:
    CTiglArcLengthReparameterization();

    void init(const Handle(Geom_BSplineCurve) curve, double accuracy=1e-5);

    bool isInitialized() const;

    /// Returns the curve parameter for the given arc length
    double parameter(double arcLength) const;

    double totalLength() const;

private:
    Handle(Geom_BSplineCurve) _reparCurve;

    bool _isInitialized;
    double _totalLength;
};

} // namespace tigl

#endif // CTIGLARCLENGTHREPARAMETERIZATION_H
