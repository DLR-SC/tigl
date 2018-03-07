/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-19 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef CCONTROLSURFACEBORDERBUILDER_H
#define CCONTROLSURFACEBORDERBUILDER_H

#include "CTiglControlSurfaceBorderCoordinateSystem.h"

#include <gp_Pln.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

namespace tigl
{

class CControlSurfaceBorderBuilder
{
public:
    CControlSurfaceBorderBuilder(const CTiglControlSurfaceBorderCoordinateSystem& coords, TopoDS_Shape wingShape);
    ~CControlSurfaceBorderBuilder();

    /**
     * @brief Computes the cutout wire with a defined leading edge shape according to 
     * the CPACS definition.
     * 
     * @param rLEHeight Relative height of the nose point
     * @param xsiNode Depth coordinate of the nose point
     * @param xsiUpper Upper skin point, where xsi=1 is the flap leading edge, and xsi=0 the trailing edge
     * @param xsiLower Lower skin point, where xsi=1 is the flap leading edge, and xsi=0 the trailing edge
     */
    TopoDS_Wire borderWithLEShape(double rLEHeight, double xsiNose, double xsiUpper, double xsiLower);

    /**
     * @brief Computes the cutout wire with a defined trailing edge shape according to 
     * the CPACS definition of a leading edge device.
     * 
     * @param rTEHeight Relative height of the center point of the edge
     * @param xsiTail   Depth coordinate of the center point
     * @param xsiTEUpper Upper skin point of the edge, where xsi=1 is the flap leading edge, and xsi=0 the trailing edge
     * @param xsiTELower Lower skin point of the edge, where xsi=1 is the flap leading edge, and xsi=0 the trailing edge
     * @return 
     */
    TopoDS_Wire borderWithInnerShape(double rTEHeight, double xsiTail, double xsiTEUpper, double xsiTELower);

    /**
     * @brief Simplified cutout without any center point definition
     * 
     * @param xsiUpper Upper skin point o the edge, where xsi=1 is the flap leading edge, and xsi=0 the trailing edge
     * @param xsiLower Lower skin point o the edge, where xsi=1 is the flap leading edge, and xsi=0 the trailing edge
     * @return 
     */
    TopoDS_Wire borderSimple(double xsiUpper, double xsiLower);

    gp_Pnt2d upperPoint();
    gp_Pnt2d lowerPoint();

    gp_Vec2d upperTangent();
    gp_Vec2d lowerTangent();

private:
    CControlSurfaceBorderBuilder();
    
    /// Computes the points on the wing skin and their tangents
    TopoDS_Wire boarderWithInnerShapeImpl(double rTEHeight, double xsiTail, double xsiTEUpper, double xsiTELower, double offset);
    void computeSkinPointsImpl(double xsi, gp_Pnt2d& pntUp, gp_Vec2d& tangentUp, gp_Pnt2d& pntLo, gp_Vec2d& tanLo);
    void computeSkinPoints(double xsiUpper, double xsiLower);
    
    TopoDS_Shape _wingShape;
    const CTiglControlSurfaceBorderCoordinateSystem& _coords;
    
    gp_Pnt2d _up2d, _lp2d;
    gp_Vec2d _upTan2d, _loTan2d;
};


}
#endif // CCONTROLSURFACEBORDERBUILDER_H
