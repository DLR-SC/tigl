/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
/**
* @file
* @brief  Implementation of CPACS guide curve container handling routines
*/

#ifndef CCPACSGUIDECURVES_H
#define CCPACSGUIDECURVES_H

#include "generated/CPACSGuideCurves.h"

namespace tigl
{
class CCPACSGuideCurves : public generated::CPACSGuideCurves
{
public:
    TIGL_EXPORT CCPACSGuideCurves(CTiglUIDManager* uidMgr);

    TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) OVERRIDE;

    // Returns the total count of guide curves in this configuration
    TIGL_EXPORT int GetGuideCurveCount() const;

    // Returns the guide curve for a given index
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurve(int index) const;

    // Returns the guide curve for a given uid
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurve(std::string uid) const;

    // Check if guide curve with a given UID exists
    TIGL_EXPORT bool GuideCurveExists(std::string uid) const;
};

} // end namespace tigl

#endif // CCPACSGUIDECURVES_H


