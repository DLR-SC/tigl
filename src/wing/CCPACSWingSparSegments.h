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

#include <string>
#include <vector>

#include <tixi.h>

#include "tigl_internal.h"


namespace tigl
{

// forward declarations
class CCPACSWingSparSegment;
class CCPACSWingSpars;

class CCPACSWingSparSegments
{
private:
    typedef std::vector<CCPACSWingSparSegment*> CCPACSWingSparSegmentContainer;
    
public:
    TIGL_EXPORT CCPACSWingSparSegments(CCPACSWingSpars& parent);
    TIGL_EXPORT virtual ~CCPACSWingSparSegments(void);

    TIGL_EXPORT void Cleanup(void);
    TIGL_EXPORT void Invalidate(void);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & sparSegmentsXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & sparSegmentsXPath) const;

    TIGL_EXPORT int GetSparSegmentCount() const;
    TIGL_EXPORT CCPACSWingSparSegment& GetSparSegment(int index) const;
    TIGL_EXPORT CCPACSWingSparSegment& GetSparSegment(const std::string& uid) const;

private:
    CCPACSWingSparSegments(const CCPACSWingSparSegments&);
    void operator=(const CCPACSWingSparSegments&);

private:
    CCPACSWingSparSegmentContainer sparSegments;
    CCPACSWingSpars& parent;
};

} // end namespace tigl

#endif // CCPACSWINGSPARSEGMENTS_H
