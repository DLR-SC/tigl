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

#include "CTiglWingHelper.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "tiglcommonfunctions.h"
#include "CCPACSWingSectionElement.h"
#include "CCPACSWingSection.h"
#include "CCPACSWing.h"

tigl::CTiglWingHelper::CTiglWingHelper()
    : tipUidCache(*this, &tigl::CTiglWingHelper::SetTipUid)
{
    wing = nullptr;
}

tigl::CTiglWingHelper::CTiglWingHelper(tigl::CCPACSWing* associatedWing)
    : tipUidCache(*this, &tigl::CTiglWingHelper::SetTipUid)
{
    SetWing(associatedWing);
}

void tigl::CTiglWingHelper::SetWing(CCPACSWing* associatedWing)
{
    tipUidCache.clear();
    wing = associatedWing;
    if (wing) {
        elementUIDs      = wing->GetSegments().GetElementUIDsInOrder();
        cTiglElementsMap = BuildCTiglElementsMap();
    }
    else {
        elementUIDs.clear();
        cTiglElementsMap.clear();
    }
}

std::map<std::string, tigl::CTiglWingSectionElement*> tigl::CTiglWingHelper::BuildCTiglElementsMap()
{

    std::map<std::string, CTiglWingSectionElement*> map;
    if (!wing) {
        LOG(WARNING) << "CTiglWingHelper::BuildCTiglElementsMap: Associated wing is not set -> Impossible to build the "
                        "map, call SetWing first. ";
    }

    for (int s = 0; s < wing->GetSections().GetSectionCount(); s++) {
        for (int e = 1; e <= wing->GetSection(s + 1).GetSectionElementCount(); e++) {
            CCPACSWingSectionElement& tempElement = wing->GetSection(s + 1).GetSectionElement(e);
            map[tempElement.GetUID()]             = tempElement.GetCTiglSectionElement();
        }
    }
    return map;
}

bool tigl::CTiglWingHelper::HasShape() const
{
    if (elementUIDs.size() >= 2) {
        return true;
    }
    return false;
}

void tigl::CTiglWingHelper::SetTipUid(std::string& cache) const
{

    /* Use the distance in the major wing direction between the root and the center of each element
     * to determine which element is more suited to be considered as the tip.
     */

    cache = "";

    if (!HasShape()) {
        LOG(WARNING) << "TiglWingHelper::SetTipUid:  This wing has no segments, impossible to determine a root";
        return;
    }

    CTiglPoint rootCenter = cTiglElementsMap.at(GetRootUID())->GetCenter();

    CTiglPoint delta;
    TiglAxis majorDir = GetMajorDirection();
    double maxD       = -1;

    for (size_t i = 0; i < elementUIDs.size(); i++) {

        delta = cTiglElementsMap.at(elementUIDs[i])->GetCenter() - rootCenter;

        if (majorDir == TIGL_Y_AXIS && fabs(delta.y) > maxD) {
            maxD  = delta.y;
            cache = elementUIDs[i];
        }
        else if (majorDir == TIGL_Z_AXIS && fabs(delta.z) > maxD) {
            maxD  = delta.z;
            cache = elementUIDs[i];
        }
        else if (majorDir == TIGL_X_AXIS && fabs(delta.x) > maxD) {
            maxD  = delta.x;
            cache = elementUIDs[i];
        }
        else {
            LOG(ERROR) << "CTiglWingHelper::SetTipCElement: Unexpected case!";
        }
    }
}

TiglAxis tigl::CTiglWingHelper::GetMajorDirection() const
{
    if (!HasShape()) {
        LOG(WARNING) << "CTiglWingHelper::GetMajorDirection: This wing has no shape -> impossible to determine the "
                        "direction properly. The default direction will be returned";
        return TIGL_Y_AXIS;
    }

    switch (wing->GetSymmetryAxis()) {
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
        for (int i = 1; i <= wing->GetSegmentCount(); ++i) {
            const CCPACSWingSegment& segment = wing->GetSegment(i);
            gp_XYZ dirSpan                   = segment.GetChordPoint(1, 0).XYZ() - segment.GetChordPoint(0, 0).XYZ();
            gp_XYZ dirDepth                  = segment.GetChordPoint(0, 1).XYZ() - segment.GetChordPoint(0, 0).XYZ();
            dirSpan  = gp_XYZ(fabs(dirSpan.X()), fabs(dirSpan.Y()), fabs(dirSpan.Z())); // why we use abs value?
            dirDepth = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
            cumulatedSpanDirection += dirSpan;
            cumulatedDepthDirection += dirDepth;
        }
        const CCPACSWingSegment& outerSegment = wing->GetSegment(wing->GetSegmentCount());
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

TiglAxis tigl::CTiglWingHelper::GetDeepDirection() const
{
    if (!HasShape()) {
        LOG(WARNING) << "CTiglWingHelper::GetDeepDirection: This wing has no shape -> impossible to determine the "
                        "direction properly. The default direction will be returned";
        return TIGL_X_AXIS;
    }

    gp_XYZ cumulatedDepthDirection(0, 0, 0);
    for (int i = 1; i <= wing->GetSegmentCount(); ++i) {
        const CCPACSWingSegment& segment = wing->GetSegment(i);
        gp_XYZ dirDepth                  = segment.GetChordPoint(0, 1).XYZ() - segment.GetChordPoint(0, 0).XYZ();
        dirDepth                         = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
        cumulatedDepthDirection += dirDepth;
    }
    const CCPACSWingSegment& outerSegment = wing->GetSegment(wing->GetSegmentCount());
    gp_XYZ dirDepth = outerSegment.GetChordPoint(1, 1).XYZ() - outerSegment.GetChordPoint(1, 0).XYZ();
    dirDepth        = gp_XYZ(fabs(dirDepth.X()), fabs(dirDepth.Y()), fabs(dirDepth.Z()));
    cumulatedDepthDirection += dirDepth;

    switch (GetMajorDirection()) {
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

TiglAxis tigl::CTiglWingHelper::GetThirdDirection() const
{
    if (!HasShape()) {
        LOG(WARNING) << "CTiglWingHelper::GetThirdDirection: This wing has no shape -> impossible to determine the "
                        "direction properly. The default direction will be returned";
        return TIGL_Z_AXIS;
    }

    std::list<TiglAxis> allAxis = {TIGL_X_AXIS, TIGL_Y_AXIS, TIGL_Z_AXIS};
    allAxis.remove(GetMajorDirection());
    allAxis.remove(GetDeepDirection());
    return allAxis.front();
}

std::string tigl::CTiglWingHelper::GetTipUID() const
{
    return tipUidCache.value();
}

std::string tigl::CTiglWingHelper::GetRootUID() const
{
    if (HasShape()) {
        return elementUIDs[0];
    }
    else {
        LOG(WARNING) << "CTiglWingHelper::GetRootUID: This wing has no segments, impossible to determine a root";
        return "";
    }
}

tigl::CTiglWingSectionElement* tigl::CTiglWingHelper::GetCTiglElementOfWing(const std::string& elementUID) const
{
    try {
        return cTiglElementsMap.at(elementUID);
    }
    catch (const std::out_of_range&) {
        LOG(ERROR) << "CTiglWingSectionElement::GetCTiglElementOfWing: The given element UID:  " + elementUID +
                          " seems not to be present in this wing.";
        return nullptr;
    }
}
