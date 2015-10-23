/* 
* Copyright (C) 2007-2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-13 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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

#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <string>

/**
 * \brief write BSpline curve parameter in SplineLib format
 *
 * \param curve 
 * \param filename
 */
void exportCurveToSplineLib(Handle_Geom_BSplineCurve curve, const std::string& filename);

/**
 * \brief write BSpline surface parameter in SplineLib format
 *
 * \param surf
 * \param filename
 */
void exportSurfaceToSplineLib(Handle_Geom_BSplineSurface surf, const std::string& filename);
