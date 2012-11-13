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

#include <cassert>

// uncomment, if you want to see the function working
//#define DEBUG

namespace tigl{

namespace {
    inline double max(double a, double b) { return a > b ? a : b; }
}

CTiglPointTranslator::CTiglPointTranslator(){
    grad[0] = 0.; grad[1] = 0.;
    hess[0][0] = hess[0][1] = hess[1][0] = hess[1][1] = 0.;
    
    initialized = false;
}

CTiglPointTranslator::CTiglPointTranslator(const CTiglPoint& x1, const CTiglPoint& x2, const CTiglPoint& x3, const CTiglPoint& x4){
    setQuadriangle(x1, x2, x3, x4);
}

void CTiglPointTranslator::setQuadriangle(const CTiglPoint& x1, const CTiglPoint& x2, const CTiglPoint& x3, const CTiglPoint& x4){
    a = x2-x1;
    b = x3-x1;
    c = x1-x2-x3+x4;
    d = x1;

    grad[0] = 0.; grad[1] = 0.;
    hess[0][0] = hess[0][1] = hess[1][0] = hess[1][1] = 0.;
    
    initialized = true;
}

// Vector between point x and a point on the plane defined by (x1,x2,x3,x4);
void CTiglPointTranslator::calcP(double alpha, double beta, CTiglPoint& p) const{
    p.x = alpha*a.x + beta*b.x + alpha*beta*c.x + d.x - x.x;
    p.y = alpha*a.y + beta*b.y + alpha*beta*c.y + d.y - x.y;
    p.z = alpha*a.z + beta*b.z + alpha*beta*c.z + d.z - x.z;
}


// This defines our minimization problem. The objective function
// measures the vector p(eta,xsi) (distance of x and a point 
// in the plane defined by x1, x2, x3, x4. 
double CTiglPointTranslator::calc_obj(double eta, double xsi){
    CTiglPoint p;
    calcP(eta, xsi, p);

    return p.norm2Sqr();
}


// calculate gradient and hessian of our minimization problem
int CTiglPointTranslator::calc_grad_hess(double alpha, double beta){
    CTiglPoint p;
    calcP(alpha,beta, p);

    CTiglPoint acb = a + c*beta;
    CTiglPoint bca = b + c*alpha;

    grad[0] = 2.*CTiglPoint::inner_prod(p, acb);
    grad[1] = 2.*CTiglPoint::inner_prod(p, bca);

    hess[0][0] = 2.*acb.norm2Sqr();
    hess[1][1] = 2.*bca.norm2Sqr();
    hess[0][1] = 2.*CTiglPoint::inner_prod(bca, acb) + 2.*CTiglPoint::inner_prod(p, c);
    hess[1][0] = hess[0][1];

    return 0;
}

// We use newtons optimization method for fast convergence
int CTiglPointTranslator::optimize(double& eta, double& xsi){
    eta = 0.;
    xsi = 0.;

    double of = calc_obj(eta,xsi);
    double of_old = of + 1.;

    calc_grad_hess(eta, xsi);

    // iterate
    int iter      = 0;
    int numOfIter = 100;
    double prec   = 1e-5;

    while ( iter < numOfIter && 
            (of_old - of)/max(fabs(of),1.) > prec && 
            sqrt(grad[0]*grad[0]+grad[1]*grad[1]) > prec ) 
    {
        // calc direction
        double det = hess[0][0]*hess[1][1] - hess[0][1]*hess[1][0];
        if ( fabs(det) < 1e-12 ){
            std::cerr << "Error: Determinant too small in CTiglPointTranslator::optimize!" << std::endl;
            return 1;
        }

        // calculate inverse hessian
        double inv_hess[2][2];
        double invdet = 1./det;
        inv_hess[0][0] =  invdet * ( hess[1][1] );
        inv_hess[1][1] =  invdet * ( hess[0][0] );
        inv_hess[0][1] = -invdet * ( hess[0][1] );
        inv_hess[1][0] = -invdet * ( hess[1][0] );

        // calculate search direction
        double dir[2];
        dir[0] = - inv_hess[0][0]*grad[0] - inv_hess[0][1]*grad[1];
        dir[1] = - inv_hess[1][0]*grad[0] - inv_hess[1][1]*grad[1];


        double alpha = 1;
        eta += alpha*dir[0];
        xsi += alpha*dir[1];

        // calculate new values
        of_old = of;
        of = calc_obj(eta,xsi);
        calc_grad_hess(eta,xsi);

#ifdef DEBUG
        std::cout << "Iteration=" << iter+1 << " f=" << of << " norm(grad)=" << grad[0]*grad[0]+grad[1]*grad[1] << std::endl;
#endif

        iter++;
    }

    return 0;
}

TiglReturnCode CTiglPointTranslator::translate(const CTiglPoint& xx, double* eta, double * xsi){
    if(!eta || !xsi){
        std::cerr << "Error in CTiglPointTranslator::translate(): eta and xsi may not be NULL Pointers!" << std::endl;
        return TIGL_NULL_POINTER;
    }
    
    assert(initialized);

    grad[0] = 0.; grad[1] = 0.;
    hess[0][0] = hess[0][1] = hess[1][0] = hess[1][1] = 0.;

    this->x = xx;
    if(optimize(*eta, *xsi) != 0){
        return TIGL_ERROR;
    }
    else 
        return TIGL_SUCCESS;
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
    
    assert(initialized);
    
    double eta = 0., xsi = 0.;
    
    grad[0] = 0.; grad[1] = 0.;
    hess[0][0] = hess[0][1] = hess[1][0] = hess[1][1] = 0.;

    this->x = xx;
    if(optimize(eta, xsi) != 0){
        return TIGL_ERROR;
    }
    
    calcP(eta,xsi,*px);
    return TIGL_SUCCESS;
}

} // end namespace tigl
