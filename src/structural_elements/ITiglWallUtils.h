// ITiglWallUtils.h

#pragma once

#include "CCPACSFuselage.h"
#include "CCPACSFuselageStructure.h"
#include "CCPACSHull.h"
#include "CCPACSHullStructure.h"
#include "CCPACSWalls.h"
#include "CTiglError.h"

namespace tigl
{

template <typename T> const CCPACSWalls& GetWalls(const T* wallObject)
{
    const auto* wallSegments = wallObject->GetParent();
    if (!wallSegments) {
        throw CTiglError("Error in GetWalls. Null pointer returned.", TIGL_NULL_POINTER);
    }

    const auto* walls = wallSegments->GetParent();
    if (!walls) {
        throw CTiglError("Error in GetWalls. Null pointer returned.", TIGL_NULL_POINTER);
    }

    return *walls;
}

template <typename T> const CCPACSFuselage& GetFuselage(const T* wallObject)
{
    const CCPACSWalls& walls = GetWalls(wallObject);

    const auto* fuselageStructure = walls.GetParent<CCPACSFuselageStructure>();
    if (!fuselageStructure) {
        throw CTiglError("Cannot get fuselage structure in GetFuselage. Null pointer parent.", TIGL_NULL_POINTER);
    }

    const auto* fuselage = fuselageStructure->GetParent();
    if (!fuselage) {
        throw CTiglError("Cannot get fuselage in GetFuselage. Null pointer parent.", TIGL_NULL_POINTER);
    }

    return *fuselage;
}

template <typename T> const CCPACSHull& GetHull(const T* wallObject)
{
    const CCPACSWalls& walls = GetWalls(wallObject);

    const auto* hullStructure = walls.GetParent<CCPACSHullStructure>();
    if (!hullStructure) {
        throw CTiglError("Cannot get hull structure in GetHull. Null pointer parent.", TIGL_NULL_POINTER);
    }

    const auto* hull = hullStructure->GetParent();
    if (!hull) {
        throw CTiglError("Cannot get hull in GetHull. Null pointer parent.", TIGL_NULL_POINTER);
    }

    return *hull;
}

} // namespace tigl
