/*
* Copyright (C) 2026 German Aerospace Center (DLR/SC)
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

#ifndef CTIGLROUNDEDSEGMENTSHELPER_H
#define CTIGLROUNDEDSEGMENTSHELPER_H

#include "CTiglMakeLoft.h"
#include "CTiglError.h"

#include <boost/optional.hpp>
#include <vector>
#include <string>

namespace tigl
{

/**
 * @brief Configures a lofter for the rounded segments lofting algorithm, using the
 * rounding distances stored in a CPACS segments container (wing, fuselage or duct segments).
 *
 * If roundedSegments is false, the lofter is left untouched and the default lofting
 * behavior applies. If it is true, the lofter is switched to the rounded segments algorithm.
 * In this case, the rounding distances at all inner sections are required, i.e. the
 * outerRoundingDistance of segment i and the innerRoundingDistance of segment i+1 must be
 * given, must be greater than zero and must not exceed 1, as they are relative to the segment.
 * For each segment, the sum of both distances must not exceed 1 either, otherwise the surface
 * would fold back onto itself. The innerRoundingDistance of the first segment and the
 * outerRoundingDistance of the last segment lie at the ends of the loft and are not used.
 *
 * @throws CTiglError if a required rounding distance is missing or invalid.
 */
template <class SegmentsT>
void ConfigureRoundedSegmentsLoft(CTiglMakeLoft& lofter, const SegmentsT& segments, bool roundedSegments)
{
    lofter.setRoundedSegments(roundedSegments);
    if (!roundedSegments) {
        return;
    }

    const int nSegments = segments.GetSegmentCount();
    std::vector<double> innerRoundingDistance;
    std::vector<double> outerRoundingDistance;
    innerRoundingDistance.reserve(nSegments);
    outerRoundingDistance.reserve(nSegments);

    auto requireDistance = [](const boost::optional<double>& value, const std::string& segmentUID,
                              const std::string& name) {
        if (!value) {
            throw CTiglError("Rounded segments are enabled, but the " + name + " of segment \"" + segmentUID +
                             "\" is missing. It is required at all inner sections.");
        }
        if (*value <= 0. || *value > 1.) {
            throw CTiglError("Rounded segments are enabled, but the " + name + " of segment \"" + segmentUID +
                             "\" is " + std::to_string(*value) + ". At all inner sections, it must be greater than "
                             "zero and must not exceed 1 (it is relative to the segment).");
        }
        return *value;
    };

    for (int i = 1; i <= nSegments; ++i) {
        const auto& segment = segments.GetSegment(i);
        // the ends of the loft are not rounded
        double inner = i == 1 ? 0. : requireDistance(segment.GetInnerRoundingDistance(), segment.GetUID(), "innerRoundingDistance");
        double outer = i == nSegments ? 0. : requireDistance(segment.GetOuterRoundingDistance(), segment.GetUID(), "outerRoundingDistance");

        // otherwise the blending regions of the segment overlap and the surface folds back onto itself
        if (inner + outer > 1.) {
            throw CTiglError("The innerRoundingDistance and outerRoundingDistance of segment \"" + segment.GetUID() +
                             "\" sum up to " + std::to_string(inner + outer) + ". The sum must not exceed 1, "
                             "as the distances are relative to the segment.");
        }
        innerRoundingDistance.push_back(inner);
        outerRoundingDistance.push_back(outer);
    }

    lofter.setInnerRoundingDistance(innerRoundingDistance);
    lofter.setOuterRoundingDistance(outerRoundingDistance);
}

} // namespace tigl

#endif // CTIGLROUNDEDSEGMENTSHELPER_H
