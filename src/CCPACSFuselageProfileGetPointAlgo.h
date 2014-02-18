/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-17 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
 * @file   CCPACSFuselageProfileGetPointAlgo.h
 * @brief  Get point and tangent depending on the parameter on a fuselage guide curves
 *
 * This class takes a fuselage profile wire and provides the point and the tangent
 * for a given parameter along the curve, as it is needed by
 * The curve starts at alpha=0 and ends at alpha=1
 */

#include "tigl_internal.h"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Edge.hxx"
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"

#ifndef CCPACSFUSELAGEPROFILEGETPOINTALGO_H
#define CCPACSFUSELAGEPROFILEGETPOINTALGO_H

namespace tigl
{

class CCPACSFuselageProfileGetPointAlgo
{

public:
    /**
     * \brief Constructor which expects a fuselage profile wire
     *
     *
     * \param wire Fuselage profile wire
     */
    CCPACSFuselageProfileGetPointAlgo (TopoDS_Wire& wire);

    /**
     * \brief Get point and the tangent on fuselage profile at curve parameter alpha
     *
     * \param alpha The curve parameter which starts at alpha=0 and ends at alpha=1
     *
     * \param point Point on the profile corresponding to the parameter alpha
     * \param tangent Tangent on the profile corresponding to the parameter alpha
     */
    void GetPointTangent(const double& alpha, gp_Pnt& point, gp_Vec& tangent);

private:
    TopoDS_Wire wire;    /**< Wire of the fuselage profile */
    Standard_Real wireLength; /**< Circumfence of the wing profile */
};

} // end namespace tigl
#endif // CCPACSFUSELAGEPROFILEGETPOINTALGO_H




