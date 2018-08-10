/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-08-21 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLWINGSEGMENTGUIDECURVEBUILDER_H
#define CTIGLWINGSEGMENTGUIDECURVEBUILDER_H

#include "CCPACSGuideCurve.h"

namespace tigl
{

/**
 * @brief The CTiglWingSegmentGuidecurveBuilder can build several
 * guide curves for one wing segment
 */
class CTiglWingSegmentGuidecurveBuilder : public IGuideCurveBuilder
{

public:
    CTiglWingSegmentGuidecurveBuilder(class CCPACSWingSegment& segment);

    virtual ~CTiglWingSegmentGuidecurveBuilder();

    // IGuideCurveBuilder interface
public:
    std::vector<gp_Pnt> BuildGuideCurvePnts(const CCPACSGuideCurve *) const OVERRIDE;

private:
    class CCPACSWingSegment& m_segment;
};

} // namespace tigl

#endif // CTIGLWINGSEGMENTGUIDECURVEBUILDER_H
