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
#ifndef CCPACSWINGSPARWEB_H
#define CCPACSWINGSPARWEB_H

#include <string>

#include <tixi.h>

#include "CCPACSMaterial.h"
#include "tigl_internal.h"


namespace tigl
{

class CCPACSWingSparWeb
{
public:
    TIGL_EXPORT CCPACSWingSparWeb();
    TIGL_EXPORT virtual ~CCPACSWingSparWeb();

    TIGL_EXPORT void Cleanup();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & sparWebXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & sparWebXPath) const;

    TIGL_EXPORT const CCPACSMaterial& GetMaterial() const;
    TIGL_EXPORT CCPACSMaterial& GetMaterial();

    TIGL_EXPORT double GetRelPos() const;

private:
    CCPACSWingSparWeb(const CCPACSWingSparWeb&);
    void operator=(const CCPACSWingSparWeb&);

private:
    double relPos;
    CCPACSMaterial material;
};

} // end namespace tigl

#endif // CCPACSWINGSPARWEB_H
