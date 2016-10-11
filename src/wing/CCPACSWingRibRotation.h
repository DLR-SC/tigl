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
#ifndef CCPACSWINGRIBROTATION_H
#define CCPACSWINGRIBROTATION_H

#include <string>

#include <tixi.h>

#include "tigl_internal.h"


namespace tigl
{

// forward declarations
class CCPACSWingRibsDefinition;

class CCPACSWingRibRotation
{
public:
    TIGL_EXPORT CCPACSWingRibRotation(CCPACSWingRibsDefinition& parent);
    TIGL_EXPORT virtual ~CCPACSWingRibRotation();

    TIGL_EXPORT void Cleanup();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & ribRotationXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & ribRotationXPath) const;

    TIGL_EXPORT const std::string& GetRibRotationReference() const;
    TIGL_EXPORT void SetRibRotationReference(const std::string&);

    TIGL_EXPORT double GetZRotation() const;
    TIGL_EXPORT void SetZRotation(double);

private:
    CCPACSWingRibRotation(const CCPACSWingRibRotation&);
    void operator=(const CCPACSWingRibRotation&);

private:
    CCPACSWingRibsDefinition& parent;
    std::string ribRotationReference;
    double zRotation;
};

} // end namespace tigl

#endif // CCPACSWINGRIBROTATION_H
