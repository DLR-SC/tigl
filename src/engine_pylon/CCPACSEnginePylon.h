/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-10-31 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef CCPACSENGINEPYLON_H
#define CCPACSENGINEPYLON_H

#include "generated/CPACSEnginePylon.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSEnginePylon : public generated::CPACSEnginePylon, public CTiglRelativelyPositionedComponent
{
public:
    TIGL_EXPORT CCPACSEnginePylon(CCPACSEnginePylons* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE { return TIGL_COMPONENT_ENGINE_PYLON | TIGL_COMPONENT_PHYSICAL; }

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT CTiglTransformation GetPositioningTransformation(std::string sectionUID);

    TIGL_EXPORT bool HasLoft() const;

protected:
    virtual PNamedShape BuildLoft() const OVERRIDE;
};

} // end namespace tigl

#endif // CCPACSENGINEPYLON_H
