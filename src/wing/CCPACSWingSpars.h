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
#ifndef CCPACSWINGSPARS_H
#define CCPACSWINGSPARS_H

#include <string>

#include <tixi.h>

#include "CCPACSWingSparPositions.h"
#include "CCPACSWingSparSegments.h"
#include "tigl_internal.h"


namespace tigl
{

// forward declarations
class CCPACSWingCSStructure;


class CCPACSWingSpars
{
public:
    TIGL_EXPORT CCPACSWingSpars(CCPACSWingCSStructure& structure);
    TIGL_EXPORT virtual ~CCPACSWingSpars();

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & sparsXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & sparsXPath) const;

    TIGL_EXPORT const CCPACSWingCSStructure& GetStructure() const;
    TIGL_EXPORT CCPACSWingCSStructure& GetStructure();

    TIGL_EXPORT const CCPACSWingSparSegments& GetSparSegments() const;
    TIGL_EXPORT CCPACSWingSparSegments& GetSparSegments();

    TIGL_EXPORT const CCPACSWingSparPositions& GetSparPositions() const;
    TIGL_EXPORT CCPACSWingSparPositions& GetSparPositions();

protected:
    void Cleanup();

private:
    CCPACSWingSpars(const CCPACSWingSpars&);
    void operator=(const CCPACSWingSpars&);

private:
    CCPACSWingCSStructure& structure;

    CCPACSWingSparPositions sparPositions;
    CCPACSWingSparSegments sparSegments;

};

} // end namespace tigl

#endif // CCPACSWINGSPARS_H
