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
#ifndef CCPACSWINGSPARPOSITION_H
#define CCPACSWINGSPARPOSITION_H

#include <string>

#include <tixi.h>

#include "tigl_internal.h"


namespace tigl
{

// forward declarations
class CCPACSWingSpars;


class CCPACSWingSparPosition
{
public:
    enum /* class */ InputType
    {
        ElementUID,
        Eta,
        None
    };

    TIGL_EXPORT CCPACSWingSparPosition(CCPACSWingSpars* spars);

    TIGL_EXPORT virtual ~CCPACSWingSparPosition();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & sparPositionXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & sparPositionXPath) const;

    TIGL_EXPORT const std::string & GetUID() const;

    TIGL_EXPORT InputType GetInputType() const;

    TIGL_EXPORT const std::string& GetElementUID() const;
    TIGL_EXPORT void SetElementUID(const std::string&);

    TIGL_EXPORT double GetEta() const;
    TIGL_EXPORT void  SetEta(double);

    TIGL_EXPORT double GetXsi() const;
    TIGL_EXPORT void SetXsi(double);

protected:
    void Cleanup();

private:
    CCPACSWingSparPosition(const CCPACSWingSparPosition&);
    void operator=(const CCPACSWingSparPosition&);

private:
    CCPACSWingSpars& spars;
    InputType inputType;
    std::string uid;
    std::string elementUID;
    double eta;
    double xsi;
};

} // end namespace tigl

#endif // CCPACSWINGSPARPOSITION_H
