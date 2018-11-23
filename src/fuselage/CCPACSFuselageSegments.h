/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
/**
* @file 
* @brief  Implementation of CPACS fuselage segments handling routines.
*/

#ifndef CCPACSFUSELAGESEGMENTS_H
#define CCPACSFUSELAGESEGMENTS_H

#include "generated/CPACSFuselageSegments.h"

namespace tigl
{
class CCPACSFuselageSegment;

class CCPACSFuselageSegments : public generated::CPACSFuselageSegments
{
public:
    TIGL_EXPORT CCPACSFuselageSegments(CCPACSFuselage* parent, CTiglUIDManager* uidMgr);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Gets a segment by index.
    TIGL_EXPORT CCPACSFuselageSegment& GetSegment(int index);
    TIGL_EXPORT const CCPACSFuselageSegment& GetSegment(int index) const;

    // Gets a segment by uid 
    TIGL_EXPORT CCPACSFuselageSegment & GetSegment(const std::string& segmentUID);

    // Gets total segment count
    TIGL_EXPORT int GetSegmentCount() const;

    // CPACSFuselageSegments interface
public:
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath) OVERRIDE;

private:
    void ReorderSegments();
};

} // end namespace tigl

#endif // CCPACSFUSELAGESEGMENTS_H
