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
#ifndef CCPACSWINGRIBEXPLICITPOSITIONING_H
#define CCPACSWINGRIBEXPLICITPOSITIONING_H

#include <string>

#include <tixi.h>

#include "tigl_internal.h"


namespace tigl 
{

// forward declarations
class CCPACSWingRibsDefinition;

class CCPACSWingRibExplicitPositioning
{
public:
    TIGL_EXPORT CCPACSWingRibExplicitPositioning(CCPACSWingRibsDefinition& parent);
    TIGL_EXPORT virtual ~CCPACSWingRibExplicitPositioning();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & explicitRibPosXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & explicitRibPosXPath) const;

    TIGL_EXPORT const std::string& GetStartReference() const;
    TIGL_EXPORT void SetStartReference(const std::string&);

    TIGL_EXPORT const std::string& GetEndReference() const;
    TIGL_EXPORT void SetEndReference(const std::string&);

    TIGL_EXPORT double GetStartEta() const;
    TIGL_EXPORT void SetStartEta(double);

    TIGL_EXPORT double GetEndEta() const;
    TIGL_EXPORT void SetEndEta(double);

protected:
    void Cleanup();

private:
    CCPACSWingRibExplicitPositioning(const CCPACSWingRibExplicitPositioning& );
    void operator=(const CCPACSWingRibExplicitPositioning& );

private:
    CCPACSWingRibsDefinition& parent;
    std::string startReference;
    std::string endReference;
    double startEta;
    double endEta;
};

} // end namespace tigl

#endif // CCPACSWINGRIBEXPLICITPOSITIONING_H
