/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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
* @file CTiglPointTranslator.h
* @brief  Translates between spatial coordinates (x,y,z) and eta-xsi coordinates on a bilinear surface.
*
* The theory behind the calculation is given in the document http://tigl.googlecode.com/files/PointTranslatorTheory.pdf
* Information about the newton solver can be found here: http://en.wikipedia.org/wiki/Newton%27s_method_in_optimization
*/


#ifndef CTIGLPOINTTRANSLATOR_H
#define CTIGLPOINTTRANSLATOR_H

#include "CTiglPoint.h"
#include "tigl.h"
#include "tigl_internal.h"
#include "ITiglObjectiveFunction.h"

namespace tigl 
{

class CTiglPointTranslator
{
public:
    TIGL_EXPORT CTiglPointTranslator();

    /**
     * @brief The class is initialized with the corners of a quadriangle. 
     * @param xfl (in) Coordinate of front left point
     * @param xfr (in) Coordinate of front right point
     * @param xbl (in) Coordinate of back left point
     * @param xbr (in) Coordinate of back right point
     */
    TIGL_EXPORT CTiglPointTranslator(const CTiglPoint& xfl, const CTiglPoint& xfr, const CTiglPoint& xbl, const CTiglPoint& xbr);
    
    /**
     * @brief Defines the quadriangle by the given for spatial points.
     * @param xfl (in) Coordinate of front left point
     * @param xfr (in) Coordinate of front right point
     * @param xbl (in) Coordinate of back left point
     * @param xbr (in) Coordinate of back right point
     */
    TIGL_EXPORT void setQuadriangle(const CTiglPoint& xfl, const CTiglPoint& xfr, const CTiglPoint& xbl, const CTiglPoint& xbr);

    /// Finds an eta-xsi coordinate that minimizes the distance to point p.
    /// The function is not reentrant. If you want to parallelize it, use 
    /// multiple instances of CTiglPointTranslator.
    TIGL_EXPORT TiglReturnCode translate(const CTiglPoint& p, double* eta, double* xsi);
    
    /// Converts from eta-xsi to spatial coordinates. Reentrant.
    TIGL_EXPORT TiglReturnCode translate(double eta, double xsi, CTiglPoint* p) const;

    TIGL_EXPORT TiglReturnCode getNormal(double eta, double xsi, CTiglPoint* n) const;
    
    /// Projects the point p onto the plane and returns that point pOnSurf.
    /// The function is not reentrant. If you want to parallelize it, use 
    /// multiple instances of CTiglPointTranslator.
    TIGL_EXPORT TiglReturnCode project(const CTiglPoint& p, CTiglPoint* pOnSurf);

private:
    class SegmentProjection : public tigl::ITiglObjectiveFunction 
    {
    public:
        TIGL_EXPORT SegmentProjection(CTiglPointTranslator& t, CTiglPoint & a, CTiglPoint& b, CTiglPoint& c, CTiglPoint& d)
          : ITiglObjectiveFunction(), _t(t), _a(a), _b(b), _c(c), _x(0,0,0)
        {
        }

        TIGL_EXPORT ~SegmentProjection() OVERRIDE{}

        TIGL_EXPORT void setProjectionPoint(const CTiglPoint& p);

        TIGL_EXPORT double getFunctionValue(const double * x) const OVERRIDE;
        TIGL_EXPORT void   getGradient     (const double * x, double * dx) const OVERRIDE;
        TIGL_EXPORT void   getHessian      (const double * x, double * H)  const OVERRIDE;
        TIGL_EXPORT void   getGradientHessian(const double * x, double * dx, double * H) const OVERRIDE;

        TIGL_EXPORT int getParameterCount() const OVERRIDE { return 2; }
        TIGL_EXPORT bool hasAnalyticGradient() const OVERRIDE { return true; }
        TIGL_EXPORT bool hasAnalyticHessian () const OVERRIDE { return true; }

    private:
        CTiglPointTranslator& _t;
        CTiglPoint &_a, &_b, &_c;
        CTiglPoint _x;
    };

    void   calcPoint(double eta, double xsi, CTiglPoint& p) const;


    SegmentProjection projector;
    CTiglPoint a, b, c, d;

    bool initialized;
};

} // end namespace tigl

#endif // CTIGLPOINTTRANSLATOR_H
