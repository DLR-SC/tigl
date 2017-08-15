/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSMATERIAL_H
#define CCPACSMATERIAL_H

#include <iostream>
#include "tixi.h"
#include "tigl_internal.h"
#include "CTiglPoint.h"

/**
 * The orthropy direction changed to the 2.3.0
 * release from a x,y,t vector to a double,
 * which contains now a rotation angle.
 */
#define CPACS_VERSION VERSION_HEX_CODE(2,3,0)

namespace tigl
{

class CCPACSMaterial
{
public:
    TIGL_EXPORT CCPACSMaterial();
    
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &materialXPath);
    
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string&) const;
    
    TIGL_EXPORT void Invalidate();
    
    // returns true, if the material could be read from CPACS file
    TIGL_EXPORT bool IsValid() const;
    
    TIGL_EXPORT bool isComposite() const;
    
    TIGL_EXPORT void SetComposite(bool composite);

    TIGL_EXPORT const std::string& GetUID() const;
    TIGL_EXPORT double GetThickness() const;

    TIGL_EXPORT double GetThicknessScaling() const;

#if CPACS_VERSION >= VERSION_HEX_CODE(2,3,0)
    TIGL_EXPORT void SetOrthotropyDirection(double);
#else
    TIGL_EXPORT void SetOrthotropyDirection(CTiglPoint);
#endif

#if CPACS_VERSION >= VERSION_HEX_CODE(2,3,0)
    TIGL_EXPORT double GetOrthotropyDirection() const;
#else
    TIGL_EXPORT const CTiglPoint& GetOrthotropyDirection() const;
#endif

    TIGL_EXPORT void SetUID(const std::string& uid);

    TIGL_EXPORT void SetThickness(double thickness);

    TIGL_EXPORT void SetThicknessScaling(double thicknessScaling);

    TIGL_EXPORT void Cleanup();
private:
    std::string uid;
    double thickness;
    double thicknessScaling;
#if CPACS_VERSION >= VERSION_HEX_CODE(2,3,0)
    double orthotropyDirection;
#else
    CTiglPoint orthotropyDirection;
#endif
    
    bool isvalid;
    bool is_composite; // whether the material is a composite
};

} // namespace tigl

#endif // CCPACSMATERIAL_H
