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

#include "CTiglPointTranslator.h"
#include "tigl.h"
#include "CTiglOptimizer.h"

#include <cassert>

// uncomment, if you want to see the function working
//#define DEBUG


#define C1    0.1
#define BTFAC 0.5


namespace tigl{

namespace {
    inline double max(double a, double b) { return a > b ? a : b; }
}

void CTiglPointTranslator::SegmentProjection::setProjectionPoint(const CTiglPoint& p){
    _x = p;
}

CTiglPointTranslator::CTiglPointTranslator() : projector(*this, a,b,c,d){ 
    initialized = false;
}

CTiglPointTranslator::CTiglPointTranslator(const CTiglPoint& x1, const CTiglPoint& x2, const CTiglPoint& x3, const CTiglPoint& x4) : projector(*this, a,b,c,d) {
    setQuadriangle(x1, x2, x3, x4);
}

void CTiglPointTranslator::setQuadriangle(const CTiglPoint& x1, const CTiglPoint& x2, const CTiglPoint& x3, const CTiglPoint& x4){
    a = x2-x1;
    b = x3-x1;
    c = x1-x2-x3+x4;
    d = x1;

    initialized = true;
}

// Vector between point x and a point on the plane defined by (x1,x2,x3,x4);
void CTiglPointTranslator::calcP(double alpha, double beta, CTiglPoint& p) const{
    p.x = alpha*a.x + beta*b.x + alpha*beta*c.x + d.x;
    p.y = alpha*a.y + beta*b.y + alpha*beta*c.y + d.y;
    p.z = alpha*a.z + beta*b.z + alpha*beta*c.z + d.z;
}


// This defines our minimization problem. The objective function
// measures the vector p(eta,xsi) (distance of x and a point 
// in the plane defined by x1, x2, x3, x4. 
double CTiglPointTranslator::SegmentProjection::getFunctionValue(const double * x) const{
    CTiglPoint p;
    _t.translate(x[0], x[1], &p);
    p -= _x;

    return p.norm2Sqr();
}


// calculate gradient and hessian of our minimization problem
void CTiglPointTranslator::SegmentProjection::getGradientHessian(const double * x, double * grad, double * hess) const{
    CTiglPoint p;
    _t.translate(x[0], x[1], &p);
    p -= _x;

    double alpha = x[0];
    double beta  = x[1];

    CTiglPoint acb = _a + _c*beta;
    CTiglPoint bca = _b + _c*alpha;

    grad[0] = 2.*CTiglPoint::inner_prod(p, acb);
    grad[1] = 2.*CTiglPoint::inner_prod(p, bca);

    TIGL_MATRIX2D(hess,2,0,0) = 2.*acb.norm2Sqr();
    TIGL_MATRIX2D(hess,2,1,1) = 2.*bca.norm2Sqr();
    TIGL_MATRIX2D(hess,2,0,1) = 2.*CTiglPoint::inner_prod(bca, acb) + 2.*CTiglPoint::inner_prod(p, _c);
    TIGL_MATRIX2D(hess,2,1,0) = TIGL_MATRIX2D(hess,2,0,1);
}

void CTiglPointTranslator::SegmentProjection::getGradient(const double * x, double * grad) const{
    CTiglPoint p;
    _t.translate(x[0], x[1], &p);
    p -= _x;

    double alpha = x[0];
    double beta  = x[1];

    CTiglPoint acb = _a + _c*beta;
    CTiglPoint bca = _b + _c*alpha;

    grad[0] = 2.*CTiglPoint::inner_prod(p, acb);
    grad[1] = 2.*CTiglPoint::inner_prod(p, bca);
}

void CTiglPointTranslator::SegmentProjection::getHessian(const double * x, double * hess) const{
    CTiglPoint p;
    _t.translate(x[0], x[1], &p);
    p -= _x;

    double alpha = x[0];
    double beta  = x[1];

    CTiglPoint acb = _a + _c*beta;
    CTiglPoint bca = _b + _c*alpha;

    TIGL_MATRIX2D(hess,2,0,0) = 2.*acb.norm2Sqr();
    TIGL_MATRIX2D(hess,2,1,1) = 2.*bca.norm2Sqr();
    TIGL_MATRIX2D(hess,2,0,1) = 2.*CTiglPoint::inner_prod(bca, acb) + 2.*CTiglPoint::inner_prod(p, _c);
}

TiglReturnCode CTiglPointTranslator::translate(const CTiglPoint& xx, double* eta, double * xsi){
    if(!eta || !xsi){
        std::cerr << "Error in CTiglPointTranslator::translate(): eta and xsi may not be NULL Pointers!" << std::endl;
        return TIGL_NULL_POINTER;
    }
    double x[2];
    x[0] = 0; x[1] = 0;

    assert(initialized);

    projector.setProjectionPoint(xx);
    projector.setNumericalStepSize(1.e-5);

    if( CTiglOptimizer::optNewton2d(projector, x) != TIGL_SUCCESS){
        return TIGL_ERROR;
    }
    else {
        *eta = x[0]; *xsi = x[1];
        return TIGL_SUCCESS;
    }
} 

// converts from eta-xsi to spatial coordinates
TiglReturnCode CTiglPointTranslator::translate(double eta, double xsi, CTiglPoint* px) const{
    if(!px){
        std::cerr << "Error in CTiglPointTranslator::translate(): x may not be a NULL Pointer!" << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    assert(initialized);
    
    calcP(eta, xsi, *px);
    
    return TIGL_SUCCESS;
}
    
// projects the point x onto the plane and returns this point
TiglReturnCode CTiglPointTranslator::project(const CTiglPoint& xx, CTiglPoint* px){
    if(!px){
        std::cerr << "Error in CTiglPointTranslator::project(): p may not be a NULL Pointer!" << std::endl;
        return TIGL_NULL_POINTER;
    }
    double x[2];
    x[0] = 0; x[1] = 0;

    assert(initialized);

    projector.setProjectionPoint(xx);

    if(CTiglOptimizer::optNewton2d(projector, x) != TIGL_SUCCESS){
        return TIGL_ERROR;
    }
    
    calcP(x[0],x[1],*px);

    return TIGL_SUCCESS;
}

} // end namespace tigl
