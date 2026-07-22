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
* @brief  Implementation of CPACS duct handling routines.
*/

#pragma once

#include "generated/CPACSDuct.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CCPACSConfiguration.h"
#include "CCPACSDucts.h"

namespace tigl
{

class CCPACSDuct : public generated::CPACSDuct, public CTiglRelativelyPositionedComponent
{
public:

    TIGL_EXPORT explicit CCPACSDuct(CCPACSDucts* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    TIGL_EXPORT std::string GetDefaultedUID() const override;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

    //Any DuctAssembly that references this duct element, can register its Invalidation
    //as a callback.
    TIGL_EXPORT void RegisterInvalidationCallback(std::function<void()> const&);

    TIGL_EXPORT PNamedShape GetTrimmedLoft() const override;
    TIGL_EXPORT PNamedShape GetUntrimmedLoft() const;

protected:
    PNamedShape BuildLoft() const override;

private:

    virtual void InvalidateImpl(const boost::optional<std::string>&) const override;

    // get short name for loft
    std::string GetShortShapeName() const;

    // Names the loft's faces. nSegments controls whether the aerodynamic faces
    // are grouped per segment (trimmed loft) or as a single group (untrimmed loft).
    void SetFaceTraits (PNamedShape loft, int nSegments) const;

    std::vector<std::function<void()>> invalidationCallbacks;

    mutable Cache<PNamedShape, CCPACSDuct> loftUntrimmed;   /**< Duct surface, untrimmed (without UV cuts at profiles) */
    mutable Cache<PNamedShape, CCPACSDuct> loftTrimmed;     /**< Duct surface, trimmed (with UV cuts at profiles) */

    void BuildLoftTrimmed(PNamedShape& cache) const;
    void BuildLoftUntrimmed(PNamedShape& cache) const;
    void BuildLoftImpl(PNamedShape& cache, bool trim) const;
};

}
