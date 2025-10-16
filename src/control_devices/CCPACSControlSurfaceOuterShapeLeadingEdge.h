/*
 * Copyright (C) 2025 German Aerospace Center (DLR/SC)
 *
 * Created: 2025 Ole Albers <ole.albers@dlr.de>
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

#include "tigl_internal.h"
#include "generated/CPACSControlSurfaceOuterShapeLeadingEdge.h"
#include "PNamedShape.h"

#include <string>

#include <gp_Vec.hxx>

namespace tigl
{

class CCPACSControlSurfaceOuterShapeLeadingEdge : public generated::CPACSControlSurfaceOuterShapeLeadingEdge
{
public:
    CCPACSControlSurfaceOuterShapeLeadingEdge(CCPACSLeadingEdgeDevice* parent, CTiglUIDManager* uidMgr);

    /**
      * Builds and returns the outer flap shape.
      *
      * @param wingCleanShape Shape of the wing without the flaps, required for modeling
      * @param upDir Up direction of the component segment
      */
    TIGL_EXPORT PNamedShape GetLoft(PNamedShape wingCleanShape, gp_Vec upDir) const;

    /**
      * Returns the cutout shape, which can be used when no wing cutout is defined in the
      * control surface device
      *
      * The return value can be zero, in case the flap shape is build up from flap profiles
      */
    TIGL_EXPORT PNamedShape CutoutShape(PNamedShape wingCleanShape, gp_Vec upDir) const;

    TIGL_EXPORT void SetUID(const std::string& uid);

private:
    bool NeedsWingIntersection() const;

    std::string _uid;
};

} // namespace tigl
