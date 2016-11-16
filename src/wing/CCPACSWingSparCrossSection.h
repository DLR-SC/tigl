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
#ifndef CCPACSWINGSPARCROSSSECTION_H
#define CCPACSWINGSPARCROSSSECTION_H

#include <string>

#include <tixi.h>

#include "CCPACSWingSparWeb.h"
#include "tigl_internal.h"


namespace tigl
{

// forward declarations
class CCPACSCapType;
class CCPACSWingCSStructure;

class CCPACSWingSparCrossSection
{

public:
    TIGL_EXPORT CCPACSWingSparCrossSection(CCPACSWingCSStructure& structure);

    TIGL_EXPORT virtual ~CCPACSWingSparCrossSection();

    TIGL_EXPORT void Cleanup();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & crossSectionXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & crossSectionXPath) const;

    TIGL_EXPORT double GetRotation(void) const;

    TIGL_EXPORT bool HasUpperCap() const;
    TIGL_EXPORT bool HasLowerCap() const;
    TIGL_EXPORT bool HasWeb2() const;

    TIGL_EXPORT const CCPACSCapType& GetLowerCap() const;
    TIGL_EXPORT CCPACSCapType& GetLowerCap();
        
    TIGL_EXPORT const CCPACSCapType& GetUpperCap() const;
    TIGL_EXPORT CCPACSCapType& GetUpperCap();
        
    TIGL_EXPORT const CCPACSWingSparWeb& GetWeb1() const;
    TIGL_EXPORT CCPACSWingSparWeb& GetWeb1();
        
    TIGL_EXPORT const CCPACSWingSparWeb& GetWeb2() const;
    TIGL_EXPORT CCPACSWingSparWeb& GetWeb2();

private:
    CCPACSWingSparCrossSection(const CCPACSWingSparCrossSection&);
    void operator=(const CCPACSWingSparCrossSection&);

private:
    CCPACSWingCSStructure& structure;
    double rotation;
    CCPACSCapType* lowerCap;
    CCPACSCapType* upperCap;
    CCPACSWingSparWeb web1;
    CCPACSWingSparWeb* web2;

    // TODO: sparCells not interpreted yet
};

} // end namespace tigl

#endif // CCPACSWINGSPARCROSSSECTION_H
