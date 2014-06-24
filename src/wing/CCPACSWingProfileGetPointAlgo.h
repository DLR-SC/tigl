/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-12 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
 * @file   CCPACSWingProfileGetPointAlgo.h
 * @brief  Get point and tangent depending on the parameter on a fuselage guide curves
 *
 * This class takes the concatenated lower and upper wing profile wire (in this order)
 * and provides the point and the tangent for a given parameter alpha along the curve.
 * The curve starts at the trailing edge (alpha=-1), runs through the upper fuselage profile towards
 * the leading edge (alpha=0) and returns to the trailing edge (alpha=1) via the lower fuselage profile
 */

#include "tigl_internal.h"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Edge.hxx"
#include "TopTools_SequenceOfShape.hxx"
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"

#ifndef CCPACSWINGPROFILEGETPOINTALGO_H
#define CCPACSWINGPROFILEGETPOINTALGO_H

namespace tigl
{

class CCPACSWingProfileGetPointAlgo
{

public:
    /**
     * \brief Constructor which expects a wire of to concatenated upper and lower wing edges as input
     *
     *
     * \param wireContainer Containes exactly two wires: The upper and lower wing profile wires
     */
    TIGL_EXPORT CCPACSWingProfileGetPointAlgo (const TopTools_SequenceOfShape& wireContainer);

    /**
     * \brief Get point and the tangent on wing profile at curve parameter alpha
     *
     * \param alpha The curve parameter which starts at the trailing edge (alpha=-1), runs through the upper wing profile towards
     *              the leading edge (alpha=0) and returns to the trailing edge (alpha=1) via the lower wing profile
     *
     * \param point Point on the profile corresponding to the parameter alpha
     * \param tangent Tangent on the profile corresponding to the parameter alpha
     */
    TIGL_EXPORT void GetPointTangent(const double& alpha, gp_Pnt& point, gp_Vec& tangent);

private:
    TopoDS_Edge upperWire;         /**< Upper wire of wing profile */
    TopoDS_Edge lowerWire;         /**< Lower wire of wing profile */
    Standard_Real lowerWireLength; /**< Circumfence of the lower wing profile */
    Standard_Real upperWireLength; /**< Circumfence of the upper wing profile */
};

} // end namespace tigl
#endif // CCPACSWINGPROFILEGETPOINTALGO_H


