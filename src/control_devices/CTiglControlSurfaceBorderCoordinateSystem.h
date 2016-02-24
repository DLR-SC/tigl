/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-24 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef CTIGLCONTROLSURFACEBORDERCOORDINATESYSTEM_H
#define CTIGLCONTROLSURFACEBORDERCOORDINATESYSTEM_H

#include <gp_Vec.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pln.hxx>

namespace tigl
{

/**
 * @brief Defines are coordiante system for the wing flap
 * border.
 * 
 * The x direction of the coordinate system points from the
 * leading edge to the trailing edge of the flap border.
 */
class CTiglControlSurfaceBorderCoordinateSystem
{
public:
    /**
     * @brief CTiglControlSurfaceBorderCoordinateSystem constructor
     * 
     * @param le Leading edge point of the flap border
     * @param te Trailing edge point of the flap border
     * @param upDir Direction, that should be normal to the wing chord surface
     */
    CTiglControlSurfaceBorderCoordinateSystem(gp_Pnt le, gp_Pnt te, gp_Vec upDir);

    gp_Vec getNormal() const;
    gp_Pnt getLe() const;
    gp_Pnt getTe() const;
    gp_Vec getXDir() const;
    gp_Vec getYDir() const;

    gp_Pln getPlane() const;

private:
    gp_Pnt _le, _te;
    gp_Vec _ydir;
};

} // namespace tigl

#endif // CTIGLCONTROLSURFACEBORDERCOORDINATESYSTEM_H
