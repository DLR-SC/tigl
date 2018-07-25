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
* @file 
* @brief  Calculates from x,y,z coordinates to eta-xsi coordinates.
*/

#include "CTiglPointTranslator.h"
#include "tigl.h"
#include "CTiglOptimizer.h"
#include "CTiglLogging.h"

#include <cassert>
#include <iomanip>
#include <limits>

#include <GeomFill.hxx>
#include <Geom_Line.hxx>

namespace tigl
{

void CTiglPointTranslator::SegmentProjection::setProjectionPoint(const CTiglPoint& p)
{
    _x = p;
}

CTiglPointTranslator::CTiglPointTranslator()
{ 
    initialized = false;
}

CTiglPointTranslator::CTiglPointTranslator(const CTiglPoint& x1, const CTiglPoint& x2, const CTiglPoint& x3, const CTiglPoint& x4)
{
    setQuadriangle(x1, x2, x3, x4);
}

void CTiglPointTranslator::setQuadriangle(const CTiglPoint& x1, const CTiglPoint& x2, const CTiglPoint& x3, const CTiglPoint& x4)
{
    a = x2-x1;
    b = x3-x1;
    c = x1-x2-x3+x4;
    d = x1;

    initialized = true;
}

// Vector between point x and a point on the plane defined by (x1,x2,x3,x4);
void CTiglPointTranslator::calcPoint(double alpha, double beta, CTiglPoint& p) const
{
    p.x = alpha*a.x + beta*b.x + alpha*beta*c.x + d.x;
    p.y = alpha*a.y + beta*b.y + alpha*beta*c.y + d.y;
    p.z = alpha*a.z + beta*b.z + alpha*beta*c.z + d.z;
}


// This defines our minimization problem. The objective function
// measures the vector p(eta,xsi) (distance of x and a point 
// in the plane defined by x1, x2, x3, x4. 
double CTiglPointTranslator::SegmentProjection::getFunctionValue(const double * x) const
{
    CTiglPoint p;
    _t.translate(x[0], x[1], &p);
    p -= _x;

    return p.norm2Sqr();
}


// calculate gradient and hessian of our minimization problem
void CTiglPointTranslator::SegmentProjection::getGradientHessian(const double * x, double * grad, double * hess) const
{
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

void CTiglPointTranslator::SegmentProjection::getGradient(const double * x, double * grad) const
{
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

void CTiglPointTranslator::SegmentProjection::getHessian(const double * x, double * hess) const
{
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
    TIGL_MATRIX2D(hess,2,1,0) = TIGL_MATRIX2D(hess,2,0,1);
}

TiglReturnCode CTiglPointTranslator::translate(const CTiglPoint& p, double* eta, double * xsi) const
{
    if (!eta || !xsi) {
        LOG(ERROR) << "Error in CTiglPointTranslator::translate(): eta and xsi may not be NULL Pointers!" << std::endl;
        return TIGL_NULL_POINTER;
    }
    double etaxsi[2];
    //initial guess
    double minDist = std::numeric_limits<double>::max();
    for (double cureta = 0.; cureta <= 1.0; cureta += 0.2) {
        for (double curxsi = 0.; curxsi <= 1.0; curxsi += 0.2) {
            CTiglPoint curp;
            calcPoint(cureta, curxsi, curp);
            double dist = curp.distance2(p);
            if (dist < minDist) {
                etaxsi[0] = cureta;
                etaxsi[1] = curxsi;
                minDist = dist;
            }
        }
    }
    
    assert(initialized);
    SegmentProjection projector(*this, a, b, c, d);
    projector.setProjectionPoint(p);

    // Scale of the whole problem. Must be
    // Computed to make convergence independent of scale
    double scale = a.norm2() * b.norm2();
    if (scale < 1.0) {
        scale = 1.0;
    }

    TiglReturnCode ret = CTiglOptimizer::optNewton2d(projector, etaxsi, 1e-7*scale, 1e-8);
    if (ret != TIGL_SUCCESS){
        // show some debuggin info
        CTiglPoint x1 = d;
        CTiglPoint x2 = a + x1;
        CTiglPoint x3 = b + x1;
        CTiglPoint x4 = c + x2 + x3 - x1;
        LOG(ERROR) << std::setprecision(20) << "Error in CTiglPointTranslator::translate: "
                   << "p = " << p.x << "," << p.y << "," << p.z << endl
                   << "  x1 = " << x1.x << "," << x1.y << "," << x1.z << endl
                   << "  x2 = " << x2.x << "," << x2.y << "," << x2.z << endl
                   << "  x3 = " << x3.x << "," << x3.y << "," << x3.z << endl
                   << "  x4 = " << x4.x << "," << x4.y << "," << x4.z << endl;
        *eta = etaxsi[0]; *xsi = etaxsi[1];
        LOG(ERROR) << "  Resulting eta/xsi: " << *eta << "," << *xsi << endl;
        return ret;
    }
    else {
        *eta = etaxsi[0]; *xsi = etaxsi[1];
        return TIGL_SUCCESS;
    }
} 

// converts from eta-xsi to spatial coordinates
TiglReturnCode CTiglPointTranslator::translate(double eta, double xsi, CTiglPoint* p) const
{
    if (!p) {
        LOG(ERROR) << "Error in CTiglPointTranslator::translate(): p may not be a NULL Pointer!" << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    assert(initialized);
    
    calcPoint(eta, xsi, *p);
    
    return TIGL_SUCCESS;
}

// converts from eta-xsi to spatial coordinates
TiglReturnCode CTiglPointTranslator::getNormal(double eta, double xsi, CTiglPoint* n) const
{
    if (!n) {
        LOG(ERROR) << "Error in CTiglPointTranslator::getNormal(): n may not be a NULL Pointer!" << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    assert(initialized);

    CTiglPoint acb = a + c*eta;
    CTiglPoint bca = b + c*xsi;

    *n = CTiglPoint::cross_prod(bca, acb);
    
    return TIGL_SUCCESS;
}

    
// projects the point x onto the plane and returns this point
TiglReturnCode CTiglPointTranslator::project(const CTiglPoint& xx, CTiglPoint* pOnSurf)
{
    if (!pOnSurf) {
        LOG(ERROR) << "Error in CTiglPointTranslator::project(): p may not be a NULL Pointer!" << std::endl;
        return TIGL_NULL_POINTER;
    }

    double eta, xsi;
    TiglReturnCode ret = translate(xx, &eta, &xsi);
    if (ret != TIGL_SUCCESS) {
        return ret;
    }
    calcPoint(eta,xsi, *pOnSurf);
    
    return TIGL_SUCCESS;
}

} // end namespace tigl
