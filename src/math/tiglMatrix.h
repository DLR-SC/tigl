/*
* Copyright (C) 2007-2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-01-17 Jan Kleinert <Jan.Kleinert@dlr.de>
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
* @brief  A function that calculates the polar decomposition of a 3x3 matrix A.
*
* PolarDecomposition(A,U,P) calculates the polar decomposition U,P of the input
* matrix A, such that A = U x P, where U is a unitary matrix and P is a positive
* semi-definite Hermitian matrix. U can be interpreted as a rotation and P as a
* spatial scaling, possibly including off-diaogonal shearing terms
*/

#pragma once

#include <math_Matrix.hxx>

namespace tigl
{

typedef math_Matrix tiglMatrix;

}
