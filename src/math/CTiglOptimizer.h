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

#include "tigl.h"
#include "tigl_internal.h"


namespace tigl
{

class CTiglOptimizer 
{
public:
    /**
    * @brief Optimizes the objective function f. 
    *
    * @param f (in) Objective function 
    * @param x (in/out) The function is initialized with the initial guess x and returns the solution x.
    * @param gradTol Norm of the gradient df/dx, when to stop optimizing.
    * @param ofTol Relative change of the objective function, when to stop optimizing.
    */
    TIGL_EXPORT static TiglReturnCode optNewton2d(const class ITiglObjectiveFunction& f, double * x, double gradTol = 1e-5, double ofTol = 1e-5);

private:
    static double armijoBacktrack2d  (const class ITiglObjectiveFunction& f, const double * x, const double * grad, double * dir, double alpha, double& ofval);

};


}

