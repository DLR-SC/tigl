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
#ifndef CCPACSWINGSPARPOSITIONUIDS_H
#define CCPACSWINGSPARPOSITIONUIDS_H

#include <string>
#include <vector>

#include <tixi.h>

#include "tigl_internal.h"


namespace tigl
{

class CCPACSWingSparPositionUIDs
{
public:
    TIGL_EXPORT CCPACSWingSparPositionUIDs();
    TIGL_EXPORT virtual ~CCPACSWingSparPositionUIDs();

    TIGL_EXPORT void Cleanup(void);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & positionUIDsXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & positionUIDsXPath) const;

    TIGL_EXPORT int GetSparPositionUIDCount() const;
    TIGL_EXPORT const std::string& GetSparPositionUID(int index) const;

private:
    CCPACSWingSparPositionUIDs(const CCPACSWingSparPositionUIDs&);
    void operator=(const CCPACSWingSparPositionUIDs&);

private:
    std::vector<std::string> positionUIDs;
};

} // end namespace tigl

#endif // CCPACSWINGSPARPOSITIONUIDS_H
