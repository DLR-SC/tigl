/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-01-05 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef OCCT_COMPAT_H
#define OCCT_COMPAT_H

#include "tigl_internal.h"
#include <Standard_Version.hxx>

#if OCC_VERSION_HEX < 0x070000
  #define DEFINE_STANDARD_RTTI_OVERRIDE(C1) \
     Standard_EXPORT const Handle(Standard_Type)& DynamicType() const OVERRIDE;
  #define DEFINE_STANDARD_RTTIEXT(C1,C2) DEFINE_STANDARD_RTTI_OVERRIDE(C1)
  #define DEFINE_STANDARD_RTTI_INLINE(C1,C2) DEFINE_STANDARD_RTTI_OVERRIDE(C1)
#endif

#endif // OCCT_COMPAT_H

