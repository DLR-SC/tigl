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
#ifndef CCPACSCAPTYPE_H
#define CCPACSCAPTYPE_H

#include <string>

#include <tixi.h>

#include "CCPACSMaterial.h"
#include "tigl_internal.h"


namespace tigl
{

class CCPACSCapType
{
public:
    TIGL_EXPORT CCPACSCapType();
    TIGL_EXPORT virtual ~CCPACSCapType();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & sparCapXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & sparCapXPath) const;

    TIGL_EXPORT double GetCapArea() const;
    TIGL_EXPORT void SetCapArea(double);

    TIGL_EXPORT const CCPACSMaterial& GetMaterial() const;
    TIGL_EXPORT CCPACSMaterial& GetMaterial();

protected:
    void Cleanup();

private:
    CCPACSCapType(const CCPACSCapType&);
    void operator=(const CCPACSCapType&);

private:
    double area;
    CCPACSMaterial material;
};

} // end namespace tigl

#endif // CCPACSCAPTYPE_H
