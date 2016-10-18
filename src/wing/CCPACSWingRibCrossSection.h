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
#ifndef CCPACSWINGRIBCROSSSECTION_H
#define CCPACSWINGRIBCROSSSECTION_H

#include <string>

#include <tixi.h>

#include "CCPACSMaterial.h"
#include "tigl_internal.h"


namespace tigl
{

// forward declarations
class CCPACSCapType;
class CCPACSWingRibsDefinition;

class CCPACSWingRibCrossSection
{
public:
    TIGL_EXPORT CCPACSWingRibCrossSection(CCPACSWingRibsDefinition& parent);
    TIGL_EXPORT virtual ~CCPACSWingRibCrossSection();

    TIGL_EXPORT void Cleanup();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & ribCrossSectionXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & ribCrossSectionXPath) const;

    TIGL_EXPORT const CCPACSMaterial& GetMaterial() const;
    TIGL_EXPORT CCPACSMaterial& GetMaterial();

    TIGL_EXPORT double GetXRotation() const;
    TIGL_EXPORT void SetXRotation(double);

    TIGL_EXPORT bool HasUpperCap() const;
    TIGL_EXPORT const CCPACSCapType& GetUpperCap() const;
    TIGL_EXPORT CCPACSCapType& GetUpperCap();

    TIGL_EXPORT bool HasLowerCap() const;
    TIGL_EXPORT const CCPACSCapType& GetLowerCap() const;
    TIGL_EXPORT CCPACSCapType& GetLowerCap();

private:
    CCPACSWingRibCrossSection(const CCPACSWingRibCrossSection&);
    void operator=(const CCPACSWingRibCrossSection&);

private:
    CCPACSWingRibsDefinition& parent;
    CCPACSMaterial material;
    double xRotation;
    CCPACSCapType* lowerCap;
    CCPACSCapType* upperCap;

    // TODO: support for RibCell not implemented yet!
};

} // end namespace tigl

#endif // CCPACSWINGRIBCROSSSECTION_H
