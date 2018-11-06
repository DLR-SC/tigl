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
* @brief  Implementation of CPACS wing segments handling routines.
*/

#ifndef CCPACSWINGSEGMENTS_H
#define CCPACSWINGSEGMENTS_H

#include "generated/CPACSWingSegments.h"
#include "tigl_internal.h"

namespace tigl
{
class CCPACSWing;
class CTiglRelativelyPositionedComponent;

class CCPACSWingSegments : public generated::CPACSWingSegments
{
public:
    // Constructor
    TIGL_EXPORT CCPACSWingSegments(CCPACSWing* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSWingSegments(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Gets a segment by index or UID.
    TIGL_EXPORT CCPACSWingSegment& GetSegment(const int index);
    TIGL_EXPORT const CCPACSWingSegment& GetSegment(const int index) const;
    TIGL_EXPORT CCPACSWingSegment& GetSegment(const std::string& segmentUID);
    TIGL_EXPORT const CCPACSWingSegment& GetSegment(const std::string& segmentUID) const;

    // Gets total segment count
    TIGL_EXPORT int GetSegmentCount() const;

    const CTiglRelativelyPositionedComponent* GetParentComponent() const
    {
        return m_parentVariant;
    }

private:
    CTiglRelativelyPositionedComponent* m_parentVariant;
};

} // end namespace tigl

#endif // CCPACSWINGSEGMENTS_H
