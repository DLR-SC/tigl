/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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

#ifndef WIRETOCURVE_H
#define WIRETOCURVE_H

#include <TopoDS_Wire.hxx>

#include "tigl_internal.h"

namespace tigl
{

/**
 * @brief This class converts a TopoDS_Wire objects to a bspline curve
 * by converting all edges to b-spline concatenating all splines to
 * a resulting b-spline
 */
class CWireToCurve
{
public:
    /**
     * @brief CWireToCurve constructor
     * @param parByLength If true, the resulting B-Spline parameter sub-ranges will be proportional
     *        to the sub-curve lengths. I.e. a long edge will have a larger parameter range than a short
     *        one. If false, the sub-parameter ranges will be the same as before, i.e. a curve with a large
     *        parameter range will result in a large range in the resulting curve.
     * @param tolerance Maximum distance of the connection point of two edges. If distance is larger than 
     *        tolerance, the class returns a NULL Handle.
     */
    TIGL_EXPORT CWireToCurve(const TopoDS_Wire&, bool parByLength=true, double tolerance = 1e-6);

    /// Returns the resulting spline
    /// Returns NULL Handle in case of an error
    TIGL_EXPORT operator Handle(Geom_BSplineCurve) ();
    TIGL_EXPORT Handle(Geom_BSplineCurve) curve();

private:
    Handle(Geom_BSplineCurve) ShiftCurveRange(Handle(Geom_BSplineCurve) curve, double umin, double umax);

    TopoDS_Wire _wire;
    double _tolerance;
    bool _parByLength;
};

} // namespacae tigl

#endif // WIRETOCURVE_H
