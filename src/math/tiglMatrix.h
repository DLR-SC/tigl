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

#pragma once

#include "tigl_internal.h"

#include <math_Matrix.hxx>

namespace tigl
{
typedef math_Matrix tiglMatrix;
typedef math_Vector tiglVector;

/// Stores the field to a file in binary form
/// Note: this is not portable. The result will
/// be different on varying platforms.
///
/// The corresponding read function should be used only on the same system
TIGL_EXPORT void writeMatrix(const tiglMatrix& x, const std::string& filename);


TIGL_EXPORT tiglMatrix readMatrix(const std::string& filename);

}
