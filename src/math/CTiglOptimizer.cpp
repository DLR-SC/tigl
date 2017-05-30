/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Martin Siggel <Martin.Siggel@dlr.de>
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
*/
#include "CTiglOptimizer.h"

#include <iostream>
#include <cassert>
#include <cmath>

#include "ITiglObjectiveFunction.h"
#include "CTiglError.h"
#include "CTiglLogging.h"

#define C1    0.1
#define BTFAC 0.5

namespace 
{
    inline double max(double a, double b) { return a > b ? a : b; }
}

//#define DEBUG

namespace tigl
{

// more infos here: http://en.wikipedia.org/wiki/Wolfe_conditions
inline double CTiglOptimizer::armijoBacktrack2d(const class ITiglObjectiveFunction& f, const double * x, 
                                                const double * grad, double * dir, 
                                                double alpha, double& of)
{
    double slope = grad[0]*dir[0]+grad[1]*dir[1];
    // the hessian might not be positive definite
    if (slope >= -1.e-15){
#ifdef DEBUG
        DLOG(WARNING) << "Warning: Hessian not pos. definite. Switch back to gradient.";
#endif
        dir[0] = -grad[0];
        dir[1] = -grad[1];
        slope = grad[0]*dir[0]+grad[1]*dir[1];
    }

    assert(slope < 0);
    
    double xnew[2];
    xnew[0] = x[0] + alpha*dir[0];
    xnew[1] = x[1] + alpha*dir[1];

    int iter = 0;
    double of_init=of;

    while ( (of=f.getFunctionValue(xnew)) > of_init + C1*alpha*slope ){
        alpha *= BTFAC;
        if (iter++ > 20) {
            //normally we should not get here (except from pathological functions)
            LOG(ERROR) << "Line search cannot find sufficient decrease, abort" << std::endl;
            alpha = 0.;
            of = of_init;
            break;
        }
        xnew[0] = x[0] + alpha*dir[0];
        xnew[1] = x[1] + alpha*dir[1];
    }

    return alpha;
}

TiglReturnCode CTiglOptimizer::optNewton2d(const class ITiglObjectiveFunction& f, double * x, double gradTol, double ofTol)
{
    
    if (f.getParameterCount() != 2) {
        LOG(ERROR) << "Number of optimization parameters != 2 in CTiglPointTranslator::optimize!" << std::endl;
        return TIGL_MATH_ERROR;
    }

    double grad[2];
    double hess[4];

    double of = f.getFunctionValue(x);
    double of_old = 2.*of;

#ifdef DEBUG_
    LOG(INFO) << "Initial guess eta=" << x[0] << " xsi=" << x[1] << " f=" << of;
#endif

    f.getGradientHessian(x, grad, hess);

    // iterate
    int iter      = 0;
    int numOfIter = 100;

    while ( iter < numOfIter && 
            (of_old - of)/max(of, 1.) > ofTol && 
            sqrt(grad[0]*grad[0]+grad[1]*grad[1]) > gradTol ) {

        of_old = of;

        // calculate determinant of hessian
        double det = TIGL_MATRIX2D(hess,2,0,0)*TIGL_MATRIX2D(hess,2,1,1) - TIGL_MATRIX2D(hess,2,0,1)*TIGL_MATRIX2D(hess,2,1,0);
        if ( fabs(det) < 1e-12 ){
            LOG(ERROR) << "Determinant too small in CTiglPointTranslator::optimize!" << std::endl;
            return TIGL_MATH_ERROR;
        }

        // calculate inverse hessian
        double inv_hess[4];
        double invdet = 1./det;
        TIGL_MATRIX2D(inv_hess,2,0,0)  =  invdet * ( TIGL_MATRIX2D(hess,2,1,1) );
        TIGL_MATRIX2D(inv_hess,2,1,1)  =  invdet * ( TIGL_MATRIX2D(hess,2,0,0) );
        TIGL_MATRIX2D(inv_hess,2,0,1)  = -invdet * ( TIGL_MATRIX2D(hess,2,0,1) );
        TIGL_MATRIX2D(inv_hess,2,1,0)  = -invdet * ( TIGL_MATRIX2D(hess,2,1,0) );

        // calculate newton search direction
        double dir[2];
        dir[0] = - TIGL_MATRIX2D(inv_hess,2,0,0) *grad[0] - TIGL_MATRIX2D(inv_hess,2,0,1) *grad[1];
        dir[1] = - TIGL_MATRIX2D(inv_hess,2,1,0) *grad[0] - TIGL_MATRIX2D(inv_hess,2,1,1) *grad[1];
        
        double alpha = 1.;
        // we need a line search to ensure convergence, lets take backtracking approach
        alpha = armijoBacktrack2d(f,x, grad, dir, alpha, of);
        if (alpha <= 0.) {
            return TIGL_MATH_ERROR;
        }

        x[0] += alpha*dir[0];
        x[1] += alpha*dir[1];

        // calculate new gradient and hessian, of is already calculated in backtracking
        f.getGradientHessian(x, grad, hess);

#ifdef DEBUG
        DLOG(INFO) << "Iter=" << iter+1 << " eta;xi=(" << x[0] << ";" << x[1] << ") f=" << of
                   << " norm(grad)=" << sqrt(grad[0]*grad[0]+grad[1]*grad[1]) <<  " alpha=" << alpha << std::endl;
#endif

        iter++;
    }

    return TIGL_SUCCESS;
}

}

