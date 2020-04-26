/*
* Copyright (c) 2018 RISC Software GmbH
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

#pragma once

#include "generated/CPACSDoorAssemblyPosition.h"
#include "CTiglStringerFrameBorderedObject.h"
#include "ITiglGeometricComponent.h"

namespace tigl
{
class CCPACSDoorAssemblyPosition : public generated::CPACSDoorAssemblyPosition, public CTiglStringerFrameBorderedObject, public ITiglGeometricComponent
{
public:
    TIGL_EXPORT CCPACSDoorAssemblyPosition(CCPACSCargoDoorsAssembly* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const override;
    TIGL_EXPORT PNamedShape GetLoft() const override;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

    TIGL_EXPORT virtual void SetStartFrameUID(const std::string& value) override;
    TIGL_EXPORT virtual void SetEndFrameUID(const std::string& value) override;
    TIGL_EXPORT virtual void SetStartStringerUID(const std::string& value) override;
    TIGL_EXPORT virtual void SetEndStringerUID(const std::string& value) override;
private:
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

};

} // namespace tigl
