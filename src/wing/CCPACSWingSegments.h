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

    // TODO: support reordering of the segement

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

    // Get the segment that get form element uid to element uid, if there is no such segment the function raise an error
    TIGL_EXPORT CCPACSWingSegment & GetSegmentFromTo(const std::string &fromElemUID, const std::string toElementUID);


    // Gets total segment count
    TIGL_EXPORT int GetSegmentCount() const;

    const CTiglRelativelyPositionedComponent* GetParentComponent() const
    {
        return m_parentVariant;
    }


    // return the elements uids in order from root to tip
    // It assume that the element are already order in m_segments !
    TIGL_EXPORT std::vector<std::string> GetElementUIDsInOrder() const;


    /**
        * Split the segment into two segments.
        * The splitter element will be used as the junction between the two segments.
        *
        * @remark Only the segment is split we do not care about the position of the splitter.
        * @param segmentToSplit: the uid of the segment to split
        * @param splitterElement: the uid of the element to be used to connect the two segments
        * @return the new created segment
        */
    TIGL_EXPORT CCPACSWingSegment& SplitSegment(const std::string& segmentToSplit, const std::string& splitterElement);



    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath) OVERRIDE;

private:
    CTiglRelativelyPositionedComponent* m_parentVariant;

    void ReorderSegments();

    // check order of segments - each segment must start with the element of the previous segment
    bool NeedReordering();

};

} // end namespace tigl

#endif // CCPACSWINGSEGMENTS_H
