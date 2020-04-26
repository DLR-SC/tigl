/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2019-01-31 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "generated/CPACSEnginePosition.h"
#include "CTiglRelativelyPositionedComponent.h"

namespace tigl {

class CCPACSEnginePosition : public generated::CPACSEnginePosition, public CTiglRelativelyPositionedComponent
{
public:
    TIGL_EXPORT CCPACSEnginePosition(CCPACSEnginePositions* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const override;

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override { return TIGL_COMPONENT_ENGINE_NACELLE; }
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override { return TIGL_INTENT_PHYSICAL; }

    // Override setters for invalidation
    TIGL_EXPORT void SetSymmetryAxis(const TiglSymmetryAxis& axis) override;
    TIGL_EXPORT void SetTransformation(const CCPACSTransformation& transform) override;
    TIGL_EXPORT void SetSymmetry(const boost::optional<TiglSymmetryAxis>& value) override;
    TIGL_EXPORT void SetEngineUID(const std::string& value) override;

    TIGL_EXPORT bool HasLoft() const;

protected:
    virtual PNamedShape BuildLoft() const override;

private:
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

};

} // namespace tigl
