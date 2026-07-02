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
#include "Cache.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSEnginePylon : public generated::CPACSEnginePylon, public CTiglRelativelyPositionedComponent
{
public:
    TIGL_EXPORT CCPACSEnginePylon(CCPACSEnginePylons* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const override;

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override { return TIGL_COMPONENT_ENGINE_PYLON; }
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override { return TIGL_INTENT_PHYSICAL; }

    // Override setters for invalidation
    TIGL_EXPORT void SetSymmetryAxis(const TiglSymmetryAxis& axis) override;
    TIGL_EXPORT void SetTransformation(const CCPACSTransformation& transform) override;
    TIGL_EXPORT void SetSymmetry(const boost::optional<TiglSymmetryAxis>& value) override;

    TIGL_EXPORT CTiglTransformation GetPositioningTransformation(std::string sectionUID);

    TIGL_EXPORT bool HasLoft() const;

protected:
    // Legacy untrimmed loft (default behavior)
    virtual PNamedShape BuildLoft() const override;

    // New API – trimmed/untrimmed caches
    void BuildLoftImpl(PNamedShape& cache, bool trim) const;
    void BuildLoftTrimmed(PNamedShape& cache) const;
    void BuildLoftUntrimmed(PNamedShape& cache) const;

public:
    // Returns the trimmed loft (UV cuts at profile positions)
    TIGL_EXPORT PNamedShape GetTrimmedLoft() const override;
    // Returns the untrimmed loft (delegates to GetLoft())
    TIGL_EXPORT PNamedShape GetUntrimmedLoft() const;

private:
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

    // Caches for trimmed/untrimmed geometry
    mutable Cache<PNamedShape, CCPACSEnginePylon> loftUntrimmed;   /**< Engine pylon surface, untrimmed (without UV cuts at profiles) */
    mutable Cache<PNamedShape, CCPACSEnginePylon> loftTrimmed;     /**< Engine pylon surface, trimmed (with UV cuts at profiles) */

};

} // end namespace tigl

#endif // CCPACSENGINEPYLON_H
