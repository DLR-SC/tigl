/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
 * Author: Malo Drougard
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


#include "TiglWingHelperFunctions.h"

#include "tigl.h"
#include "CCPACSWingSegment.h"

namespace
{
    struct CumulatedLength
    {
        gp_XYZ depth{0., 0., 0.};
        gp_XYZ span{0., 0., 0.};
    };


    // Helper function to compute accumulated depth and span of the wing
    CumulatedLength GetCumulatedLength(const tigl::CCPACSWing& wing)
    {
        CumulatedLength length;
        for (int i = 1; i <= wing.GetSegmentCount(); ++i) {
            const auto& segment = wing.GetSegment(i);
            gp_XYZ dirSpan  = segment.GetChordPoint(1, 0).XYZ() - segment.GetChordPoint(0, 0).XYZ();
            gp_XYZ dirDepth = segment.GetChordPoint(0, 1).XYZ() - segment.GetChordPoint(0, 0).XYZ();
            length.span  += gp_XYZ(fabs(dirSpan.X()), fabs(dirSpan.Y()), fabs(dirSpan.Z()));
            length.depth += gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));

        }
        const auto& outerSegment = wing.GetSegment(wing.GetSegmentCount());
        gp_XYZ dirDepth = outerSegment.GetChordPoint(1, 1).XYZ() - outerSegment.GetChordPoint(1, 0).XYZ();
        length.depth    += gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));

        return length;
    }
}

namespace tigl
{

namespace winghelper
{

bool HasShape(const tigl::CCPACSWing& wing)
{
    return wing.GetSegmentCount() > 0;

}

TiglAxis GetWingDepthAxis(const tigl::CCPACSWing& wing)
{
    if (!HasShape(wing)) {
        LOG(WARNING) << "CTiglWingHelper::GetDeepDirection: This wing has no shape -> impossible to determine the "
                        "direction properly. The default direction will be returned";
        return TIGL_X_AXIS;
    }

    gp_XYZ cumulatedDepthDirection = GetCumulatedLength(wing).depth;

    switch (GetWingSpanAxis(wing)) {
    case TIGL_Y_AXIS:
        return cumulatedDepthDirection.X() >= cumulatedDepthDirection.Z() ? TiglAxis::TIGL_X_AXIS
                                                                          : TiglAxis::TIGL_Z_AXIS;
    case TIGL_Z_AXIS:
        return cumulatedDepthDirection.X() >= cumulatedDepthDirection.Y() ? TiglAxis::TIGL_X_AXIS
                                                                          : TiglAxis::TIGL_Y_AXIS;
    case TIGL_X_AXIS:
        return cumulatedDepthDirection.Z() >= cumulatedDepthDirection.Y() ? TiglAxis::TIGL_Z_AXIS
                                                                          : TiglAxis::TIGL_Y_AXIS;
    default:
        return TiglAxis ::TIGL_X_AXIS;
    }
}

TiglAxis GetWingSpanAxis(const CCPACSWing& wing)
{
    if (!HasShape(wing)) {
        LOG(WARNING) << "CTiglWingHelper::GetMajorDirection: This wing has no shape -> impossible to determine the "
                        "direction properly. The default direction will be returned";
        return TIGL_Y_AXIS;
    }

    switch (wing.GetSymmetryAxis()) {
    case TIGL_X_Y_PLANE:
        return TiglAxis::TIGL_Z_AXIS;
    case TIGL_X_Z_PLANE:
        return TiglAxis::TIGL_Y_AXIS;
    case TIGL_Y_Z_PLANE:
        return TiglAxis ::TIGL_X_AXIS;
    default:
        // heuristic to find the best major axis
        // first find the deep axis , then chose the major axis between the two left axis
        auto length = GetCumulatedLength(wing);
        gp_XYZ cumulatedSpanDirection = length.span;
        gp_XYZ cumulatedDepthDirection = length.depth;

        int depthIndex = 0;
        if (cumulatedDepthDirection.X() >= cumulatedDepthDirection.Y() &&
            cumulatedDepthDirection.X() >= cumulatedDepthDirection.Z()) {
            depthIndex = 0;
        }
        else if (cumulatedDepthDirection.Y() >= cumulatedDepthDirection.X() &&
                 cumulatedDepthDirection.Y() >= cumulatedDepthDirection.Z()) {
            depthIndex = 1;
        }
        else {
            depthIndex = 2;
        }

        switch (depthIndex) {
        case 0:
            return cumulatedSpanDirection.Y() >= cumulatedSpanDirection.Z() ? TiglAxis::TIGL_Y_AXIS
                                                                            : TiglAxis::TIGL_Z_AXIS;
        case 1:
            return cumulatedSpanDirection.X() >= cumulatedSpanDirection.Z() ? TiglAxis::TIGL_X_AXIS
                                                                            : TiglAxis::TIGL_Z_AXIS;
        case 2:
            return cumulatedSpanDirection.X() >= cumulatedSpanDirection.Y() ? TiglAxis::TIGL_X_AXIS
                                                                            : TiglAxis::TIGL_Y_AXIS;
        default:
            return TiglAxis ::TIGL_Y_AXIS;
        }
    }
}

} // namespace winghelper

} // namespace tigl
