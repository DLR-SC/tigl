/* 
* Copyright (C) 2016 Airbus Defence and Space
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
#ifndef CCPACSWINGSPARSEGMENTS_H
#define CCPACSWINGSPARSEGMENTS_H

#include "generated/CPACSSparSegments.h"
#include "tigl_internal.h"


namespace tigl
{

// forward declarations
class CCPACSWingSparSegment;
class CCPACSWingSpars;

class CCPACSWingSparSegments : public generated::CPACSSparSegments
{
public:
    TIGL_EXPORT CCPACSWingSparSegments(CCPACSWingSpars* parent);

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT int GetSparSegmentCount() const;
    TIGL_EXPORT CCPACSWingSparSegment& GetSparSegment(int index) const;
    TIGL_EXPORT CCPACSWingSparSegment& GetSparSegment(const std::string& uid) const;
};

} // end namespace tigl

#endif // CCPACSWINGSPARSEGMENTS_H
