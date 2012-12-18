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
#include "ITiglObjectiveFunction.h"

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
    class SegmentProjection : public tigl::ITiglObjectiveFunction {
    public:
        SegmentProjection(CTiglPointTranslator& t, CTiglPoint & a, CTiglPoint& b, CTiglPoint& c, CTiglPoint& d) 
          : ITiglObjectiveFunction(), _t(t), _a(a), _b(b), _c(c), _d(d), _x(0,0,0) {
        }

        virtual ~SegmentProjection(void){}

        void setProjectionPoint(const CTiglPoint& p);

        virtual double getFunctionValue(const double * x) const ;
        void   getGradient     (const double * x, double * dx) const  ;
        void   getHessian      (const double * x, double * H)  const  ;
        void   getGradientHessian(const double * x, double * dx, double * H) const;

        virtual int getParameterCount() const { return 2; }
        virtual bool hasAnalyticGradient() const { return true; }
        virtual bool hasAnalyticHessian () const { return true; }

    private:
        CTiglPointTranslator& _t;
        CTiglPoint &_a, &_b, &_c, &_d;
        CTiglPoint _x;
    };

    void   calcP(double eta, double xsi, CTiglPoint& p) const;


    SegmentProjection projector;
    CTiglPoint a, b, c, d;

    bool initialized;
};

} // end namespace tigl

#endif // CTIGLPOINTTRANSLATOR_H
