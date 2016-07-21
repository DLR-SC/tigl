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
* @brief  TODO: Describe!.
*/
#ifndef ITIGL_OBJECTIVE_FUNCTION
#define ITIGL_OBJECTIVE_FUNCTION

#include "tigl_internal.h"

#define TIGL_MATRIX2D(A,n,i,j) ((A)[(i) + (j)*(n)])

namespace tigl 
{

class ITiglObjectiveFunction
{
public:
    TIGL_EXPORT ITiglObjectiveFunction();
    TIGL_EXPORT virtual ~ITiglObjectiveFunction();

    // set step size h for finite differences
    TIGL_EXPORT void setNumericalStepSize(double h);

    TIGL_EXPORT virtual int getParameterCount() const = 0;

    TIGL_EXPORT virtual bool hasAnalyticGradient() const = 0;
    TIGL_EXPORT virtual bool hasAnalyticHessian()  const = 0;

    /// returns the objective function for the parameter vector x
    TIGL_EXPORT virtual double getFunctionValue     (const double * x) const = 0;
    
    /// returns the gradient dx for the parameter vector x
    TIGL_EXPORT virtual void   getGradient (const double * x, double * dx) const;

    /// provides a routine for the numerical gradient calculation
    TIGL_EXPORT virtual void   getNumericalGradient(const double * x, double * dx) const;

    /// returns the hessian matrix for the parameter vector x
    TIGL_EXPORT virtual void   getHessian  (const double * x, double * H) const;

    /// provides a routine for the numerical hessian calculation
    TIGL_EXPORT virtual void   getNumericalHessian(const double * x, double * H) const;

    /// Sometimes it is more efficient to calculate gradient and hessian at once
    /// for these cases this function is provided.
    /// In the default implementation, it calls getGradient and getHessian succesively
    TIGL_EXPORT virtual void   getGradientHessian(const double * x, double * dx, double * H) const;

private:
    double h;


};


} // end namespace tigl

#endif // ITIGL_OBJECTIVE_FUNCTION
