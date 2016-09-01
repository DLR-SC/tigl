/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file 
* @brief  Implementation of CPACS wing segments handling routines.
*/

#ifndef CCPACSWINGSEGMENTS_H
#define CCPACSWINGSEGMENTS_H

#include <string>
#include <vector>

#include "tigl_internal.h"
#include "tixi.h"

namespace tigl 
{

class CCPACSWing;
class CCPACSWingSegment;

class CCPACSWingSegments
{
public:
    // Constructor
    TIGL_EXPORT CCPACSWingSegments(CCPACSWing* aWing);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingSegments(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS segments element
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath);

    // Write CPACS segments element
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath) const;

    // Gets a segment by index or UID.
    TIGL_EXPORT CCPACSWingSegment & GetSegment(const int index);
    TIGL_EXPORT CCPACSWingSegment & GetSegment(const std::string& segmentUID);

    // Gets total segment count
    TIGL_EXPORT int GetSegmentCount(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Typedef for a CCPACSWingSegment container to store the segments of a wing.
    typedef std::vector<CCPACSWingSegment*> CCPACSWingSegmentContainer;

    // Copy constructor
    CCPACSWingSegments(const CCPACSWingSegments& );

    // Assignment operator
    void operator=(const CCPACSWingSegments& );

    CCPACSWingSegmentContainer segments;       /**< Segment elements    */
    CCPACSWing*                wing;           /**< Parent wing         */

};

} // end namespace tigl

#endif // CCPACSWINGSEGMENTS_H
