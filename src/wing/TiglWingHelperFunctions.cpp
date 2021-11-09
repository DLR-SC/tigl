#include "TiglWingHelperFunctions.h"

#include "tigl.h"
#include "CCPACSWingSegment.h"

namespace tigl
{

namespace winghelper
{

bool HasShape(const tigl::CCPACSWing& wing)
{
    return wing.GetSegmentCount() > 0;

}

TiglAxis GetDeepDirection(const tigl::CCPACSWing& wing)
{
    if (!HasShape(wing)) {
        LOG(WARNING) << "CTiglWingHelper::GetDeepDirection: This wing has no shape -> impossible to determine the "
                        "direction properly. The default direction will be returned";
        return TIGL_X_AXIS;
    }

    gp_XYZ cumulatedDepthDirection(0, 0, 0);
    for (int i = 1; i <= wing.GetSegmentCount(); ++i) {
        const CCPACSWingSegment& segment = wing.GetSegment(i);
        gp_XYZ dirDepth                  = segment.GetChordPoint(0, 1).XYZ() - segment.GetChordPoint(0, 0).XYZ();
        dirDepth                         = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
        cumulatedDepthDirection += dirDepth;
    }
    const CCPACSWingSegment& outerSegment = wing.GetSegment(wing.GetSegmentCount());
    gp_XYZ dirDepth = outerSegment.GetChordPoint(1, 1).XYZ() - outerSegment.GetChordPoint(1, 0).XYZ();
    dirDepth        = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
    cumulatedDepthDirection += dirDepth;

    switch (GetMajorDirection(wing)) {
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

TiglAxis GetMajorDirection(const CCPACSWing& wing)
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
        gp_XYZ cumulatedSpanDirection(0, 0, 0);
        gp_XYZ cumulatedDepthDirection(0, 0, 0);
        for (int i = 1; i <= wing.GetSegmentCount(); ++i) {
            const CCPACSWingSegment& segment = wing.GetSegment(i);
            gp_XYZ dirSpan                   = segment.GetChordPoint(1, 0).XYZ() - segment.GetChordPoint(0, 0).XYZ();
            gp_XYZ dirDepth                  = segment.GetChordPoint(0, 1).XYZ() - segment.GetChordPoint(0, 0).XYZ();
            dirSpan  = gp_XYZ(fabs(dirSpan.X()), fabs(dirSpan.Y()), fabs(dirSpan.Z())); // why we use abs value?
            dirDepth = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
            cumulatedSpanDirection += dirSpan;
            cumulatedDepthDirection += dirDepth;
        }
        const CCPACSWingSegment& outerSegment = wing.GetSegment(wing.GetSegmentCount());
        gp_XYZ dirDepth = outerSegment.GetChordPoint(1, 1).XYZ() - outerSegment.GetChordPoint(1, 0).XYZ();
        dirDepth        = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
        cumulatedDepthDirection += dirDepth;

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
