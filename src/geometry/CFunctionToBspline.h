/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-11-17 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef FUNCTIONTOBSPLINE_H
#define FUNCTIONTOBSPLINE_H

#include "tiglmathfunctions.h"
#include "tigl_internal.h"

#include <Geom_BSplineCurve.hxx>

namespace tigl
{

/**
 * @brief The FunctionToBspline class can be used to approximate an arbitary 3D function
 * with a B-Spline. 
 * 
 * The function to approximate is given by 3 separate functions, one for the x values,
 * one for y-values and one for the z-values.
 */
class TIGL_EXPORT CFunctionToBspline
{
public:
    /**
     * @brief CFunctionToBspline - Constructor
     *
     * @param f Function returning the values of the curve
     * @param umin Start parameter of the function to approximate
     * @param umax End parameter of the function to approximate
     * @param degree Degree of the resulting B-Spline
     * @param tolerance Maximum approximation error
     * @param maxDepth Maximum depth of curve splitting -> Influences resulting segment number
     */
    CFunctionToBspline(MathFunc3d& f,
                       double umin, double umax,
                       int degree,
                       double tolerance=1e-5,
                       int maxDepth = 10);

    ~CFunctionToBspline();

    /// Computes the B-Spline approximation
    Handle(Geom_BSplineCurve) Curve();

    /// returns the error made by spline approximation
    double ApproxError();

private:
    class CFunctionToBsplineImpl;
    CFunctionToBsplineImpl* pimpl;
};

} // namespace tigl

#endif // FUNCTIONTOBSPLINE_H
