/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-15 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief  Implementation of CPACS hull handling routines.
*/

#pragma once

#include "generated/CPACSHull.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CCPACSConfiguration.h"
#include "generated/CPACSHulls.h"

namespace tigl
{

class CCPACSHull : public generated::CPACSHull, public CTiglRelativelyPositionedComponent
{
public:

    TIGL_EXPORT explicit CCPACSHull(CCPACSHulls* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    TIGL_EXPORT std::string GetDefaultedUID() const override;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

    // //Any DuctAssembly that references this duct element, can register its Invalidation
    // //as a callback.
    // TIGL_EXPORT void RegisterInvalidationCallback(std::function<void()> const&);

protected:
    PNamedShape BuildLoft() const override;

private:

    // virtual void InvalidateImpl(const boost::optional<std::string>&) const override;

    // // get short name for loft
    std::string GetShortShapeName() const;

    void SetFaceTraits (PNamedShape loft) const;

    // std::vector<std::function<void()>> invalidationCallbacks;

};

}
