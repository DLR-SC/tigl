/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#include <boost/optional.hpp>

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

#include "generated/CPACSStringerFramePosition.h"
#include "Cache.h"

namespace tigl
{
class CCPACSFuselage;

class CCPACSFuselageStringerFramePosition : public generated::CPACSStringerFramePosition
{
public:
    TIGL_EXPORT CCPACSFuselageStringerFramePosition(CCPACSFrame* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSFuselageStringerFramePosition(CCPACSFuselageStringer* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT gp_Pnt GetRefPoint() const;

    TIGL_EXPORT virtual void SetPositionX(const double& value) override;
    TIGL_EXPORT virtual void SetReferenceY(const double& value) override;
    TIGL_EXPORT virtual void SetReferenceZ(const double& value) override;

    TIGL_EXPORT double GetPositionXRel() const;
    TIGL_EXPORT double GetReferenceYRel() const;
    TIGL_EXPORT double GetReferenceZRel() const;

    TIGL_EXPORT void SetPositionXRel(double positionXRel);
    TIGL_EXPORT void SetReferenceYRel(double referenceYRel);
    TIGL_EXPORT void SetReferenceZRel(double referenceZRel);

    TIGL_EXPORT void GetXBorders(double& ymin, double& ymax);
    TIGL_EXPORT void GetYBorders(double& ymin, double& ymax);
    TIGL_EXPORT void GetZBorders(double& zmin, double& zmax);
private:
    const CCPACSFuselage& GetFuselage() const;

    struct RelativePositionCache {
        double positionXRel;
        double xmin;
        double xmax;

        double referenceYRel;
        double ymin;
        double ymax;

        double referenceZRel;
        double zmin;
        double zmax;
    };

    void Invalidate();
    void UpdateRelativePositioning(RelativePositionCache& cache) const;

    Cache<RelativePositionCache, CCPACSFuselageStringerFramePosition> m_relCache;
};
} // namespace tigl
