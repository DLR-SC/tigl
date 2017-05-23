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
#include <vector>
#include "ITiglObjectiveFunction.h"
#include "CTiglError.h"

namespace tigl 
{

ITiglObjectiveFunction::ITiglObjectiveFunction()
{
    h = 1.e-10;
}


ITiglObjectiveFunction::~ITiglObjectiveFunction()
{
}

void ITiglObjectiveFunction::setNumericalStepSize(double h) 
{
    if ( h <= 0) {
        throw CTiglError("Step size for finite differences must be larger than zero!");
    }

    this->h = h;
}

// by default, we calculate the gradient with finite differences if no gradient is given
void ITiglObjectiveFunction::getGradient(const double * x, double * dx) const 
{
    getNumericalGradient(x,dx);
}

void ITiglObjectiveFunction::getNumericalGradient(const double * x, double * dx) const 
{
    if (!dx) {
        throw CTiglError("gradient argument must not be a zero pointer in  ITiglObjectiveFunction::getNumericalGradient!");
    }

    const double curOf = getFunctionValue(x);
    std::vector<double> xnew(getParameterCount());

    for (int i = 0; i < getParameterCount(); ++i) {
        xnew[i] = x[i];
    }

    for (int i = 0; i < getParameterCount(); ++i) {
        xnew[i] = x[i] + h;

        double ofi = getFunctionValue(&xnew.front());
        dx[i] = (ofi - curOf)/h;

        xnew[i] = x[i];
    }
}

// by default, we calculate the hessian matrix with finite differences if no gradient is given
void ITiglObjectiveFunction::getHessian(const double * x, double * H) const 
{
    getNumericalHessian(x,H);
}

// calculates the numerical hessian based on gradient calls. this is not very accurate but
// should suffice for the moment.
void ITiglObjectiveFunction::getNumericalHessian(const double * x, double * H) const 
{
    if (!H) {
        throw CTiglError("hessian argument must not be a zero pointer in  ITiglObjectiveFunction::getNumericalHessian!");
    }

    int n = getParameterCount();

    // calculate current gradient
    double * dxcur = new double[n];
    getGradient(x, dxcur);

    double * xnew  = new double[n];
    double * dxnew = new double[n];

    for (int i = 0; i <n; ++i) {
        xnew[i] = x[i];
    }

    for (int i = 0; i < n; ++i) {
        xnew[i] = x[i] + h;

        getGradient(xnew, dxnew);
        for (int j = 0; j < n; ++j) {
            TIGL_MATRIX2D(H,n,i,j) = (dxnew[j]-dxcur[j])/h;
        }

        xnew[i] = x[i];
    }

    delete[] dxcur;
    delete[] xnew;
    delete[] dxnew;

}


void ITiglObjectiveFunction::getGradientHessian(const double * x, double * dx, double * H) const 
{
    getGradient(x,dx);
    getHessian(x,H);
}

} // namespace tigl

