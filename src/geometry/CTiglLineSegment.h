/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-09-30 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLLINESEGMENT_H
#define CTIGLLINESEGMENT_H

#include "CTiglPoint.h"

namespace tigl
{

/**
 * @brief A simple line segment
 */
class CTiglLineSegment
{
public:
    CTiglLineSegment(const CTiglPoint& p1, const CTiglPoint& p2);
    
    /**
     * @brief Returns a point on the line segment (u = 0 ... 1)
     * @param u Line parameter
     * @return The point on the line
     */
    CTiglPoint value(double u) const;
    
    /**
     * @brief Computes the minimal distance to another line segment
     *
     * We use the algorithm from Christer Ericson (Real Time Collision Detection).
     * It is robust, also in the near parallel case.
     */
    double distance(const CTiglLineSegment& other) const;

private:
    CTiglPoint p;
    CTiglPoint d;
};

} // namespace tigl

#endif // CTIGLLINESEGMENT_H
