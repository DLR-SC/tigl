/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-11-20 Martin Siggel <Martin.Siggel@dlr.de>
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

%module (package="tigl3", directors="1") tmath

%include common.i


%{
#include <math_Vector.hxx>
#include <math_Matrix.hxx>
%}

%import math.i
%import core.i
%import occtaddons.i

%{
// All includes that are required for the wrapper compilation
#include "tiglmathfunctions.h"
#include "CTiglOptimizer.h"
#include "CTiglPointTranslator.h"
#include "ITiglObjectiveFunction.h"
#include "CTiglError.h"
%}


%feature("autodoc", "3");

// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

%feature("director") MathFunc3d;
%feature("director") MathFunc1d;

%include "tiglmathfunctions.h"
%include "ITiglObjectiveFunction.h"
%include "CTiglOptimizer.h"
%include "CTiglPointTranslator.h"
