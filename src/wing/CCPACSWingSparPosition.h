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

#include "generated/CPACSSparPosition.h"
#include <gp_Vec.hxx>

namespace tigl
{

// forward declarations
namespace generated
{
  class CPACSWingRibPoint;
  class CPACSEtaXsiPoint;
}

class CCPACSWingCSStructure;

class CCPACSWingSparPosition : public generated::CPACSSparPosition
{
public:
    TIGL_EXPORT CCPACSWingSparPosition(CCPACSWingSparPositions* sparPositions, CTiglUIDManager* uidMgr);

    TIGL_EXPORT bool isOnInnerSectionElement() const;
    TIGL_EXPORT bool isOnOuterSectionElement() const;
    TIGL_EXPORT bool isOnSectionElement() const;
    TIGL_EXPORT bool isOnRib() const;

    //TIGL_EXPORT InputType GetInputType() const;
    
    TIGL_EXPORT const std::string& GetReferenceUID() const;

    TIGL_EXPORT double GetEta() const;
    TIGL_EXPORT double GetXsi() const;
    
    TIGL_EXPORT const CCPACSWingRibPoint& GetRibPoint() const;
    TIGL_EXPORT const CCPACSEtaXsiPoint &GetEtaXsiPoint() const;

    TIGL_EXPORT gp_Vec GetUpVector(const CCPACSWingCSStructure& structure, gp_Pnt midplanePnt) const;
};

TIGL_EXPORT int WingRibPointGetRibNumber(const generated::CPACSWingRibPoint&);

/// If the spar position is on a section element, the element uid is returned
TIGL_EXPORT std::string WingSparPosGetElementUID(const CCPACSWingSparPosition&);

} // end namespace tigl

#endif // CCPACSWINGSPARPOSITION_H
