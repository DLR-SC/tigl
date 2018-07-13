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

%module (package="tigl3") occtaddons

%include std_string.i

%{
// includes are required to for compilation
#include<Standard_ErrorHandler.hxx>
#include<Standard_Failure.hxx>
#include <sstream>
%}

%include math_headers.i
%import math.i

// enable printing matrix (python)
%extend math_Matrix {
    std::string __str__() {
        std::stringstream oss;

        oss << "[";
        for (int j = self->LowerCol(); j <= self->UpperCol(); ++j) {
            if (j != self->LowerCol()) {
                oss << " ";
            }
            oss << "[ ";
            for (int i = self->LowerRow(); i <= self->UpperRow(); ++i) {
                oss << self->Value(i,j) << "\t";
            }
            oss << "]";
            if (j != self->UpperCol()) {
                oss << std::endl;
            }
        }
        oss << "]" << std::endl;
        return oss.str();
    }
}

