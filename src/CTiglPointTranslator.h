/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2012-11-13 Martin Siggel <Martin.Siggel@dlr.de>
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
* @file 
* @brief  Calculates from x,y,z coordinates to eta-xsi coordinates.
*/


#ifndef CTIGLPOINTTRANSLATOR_H
#define CTIGLPOINTTRANSLATOR_H

#include "CTiglPoint.h"
#include "tigl.h"

namespace tigl {

class CTiglPointTranslator{
public:
    CTiglPointTranslator();

    /**
     * @brief The class is initialized with the corners of a quadriangle. 
     * @param xfl (in) Coordinate of front left point
     * @param xfr (in) Coordinate of front right point
     * @param xbl (in) Coordinate of back left point
     * @param xbr (in) Coordinate of back right point
     */
    CTiglPointTranslator(const CTiglPoint& xfl, const CTiglPoint& xfr, const CTiglPoint& xbl, const CTiglPoint& xbr);
    
    /**
     * @brief Defines the quadriangle by the given for spatial points.
     * @param xfl (in) Coordinate of front left point
     * @param xfr (in) Coordinate of front right point
     * @param xbl (in) Coordinate of back left point
     * @param xbr (in) Coordinate of back right point
     */
    void setQuadriangle(const CTiglPoint& xfl, const CTiglPoint& xfr, const CTiglPoint& xbl, const CTiglPoint& xbr);

    /// Finds an eta-xsi coordinate that minimizes the distance to point x.
    /// The function is not reentrant. If you want to parallelize it, use 
    /// multiple instances of CTiglPointTranslator.
    TiglReturnCode translate(const CTiglPoint& x, double* eta, double* xsi);
    
    /// Converts from eta-xsi to spatial coordinates. Reentrant.
    TiglReturnCode translate(double eta, double xsi, CTiglPoint* x) const;
    
    /// Projects the point x onto the plane and returns that point p.
    /// The function is not reentrant. If you want to parallelize it, use 
    /// multiple instances of CTiglPointTranslator.
    TiglReturnCode project(const CTiglPoint& x, CTiglPoint* p);

private:
    int optimize(double& eta, double& xsi);
    double calc_obj (double eta, double xsi);
    int    calc_grad_hess(double eta, double xsi);
    void   calcP(double eta, double xsi, CTiglPoint& p) const;

    CTiglPoint a, b, c, d;
    CTiglPoint x;
    double hess[2][2];
    double grad[2];
    bool initialized;
};

} // end namespace tigl

#endif // CTIGLPOINTTRANSLATOR_H
