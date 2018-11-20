/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Definition of the interface which describes a geometric component.
*/

#ifndef ITIGLGEOMETRICCOMPONENT_H
#define ITIGLGEOMETRICCOMPONENT_H

#include <string>

#include "tigl.h"
#include "tigl_internal.h"
#include "PNamedShape.h"

namespace tigl 
{
class ITiglGeometricComponent
{
public:
    // Returns the unique id of this component or an empty string if the component does not have a uid
    TIGL_EXPORT virtual std::string GetDefaultedUID() const = 0;

    // Gets the loft of a geometric component
    TIGL_EXPORT virtual PNamedShape GetLoft() const = 0;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT virtual TiglGeometricComponentType GetComponentType() const = 0;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT virtual TiglGeometricComponentIntent GetComponentIntent() const = 0;
};

} // end namespace tigl

#endif // ITIGLGEOMETRICCOMPONENT_H
