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
    TIGL_EXPORT CCPACSGuideCurves(CCPACSFuselageSegment* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSGuideCurves(CCPACSWingSegment* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

    // Returns the total count of guide curves in this configuration
    TIGL_EXPORT int GetGuideCurveCount() const;

    // Returns the guide curve for a given index
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurve(int index) const;

    // Returns the guide curve for a given index
    TIGL_EXPORT CCPACSGuideCurve& GetGuideCurve(int index);

    // Returns the guide curve for a given uid
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurve(std::string uid) const;

    // Returns the guide curve for a given uid
    TIGL_EXPORT CCPACSGuideCurve& GetGuideCurve(std::string uid);

    // Check if guide curve with a given UID exists
    TIGL_EXPORT bool GuideCurveExists(std::string uid) const;

    // Given relCirc, find the guide curves that sandwich this parameter,
    // i.e. the guideCurve with the largerst fromRelativeCircumference < relCirc
    // and the guideCurve with the smallest fromRelativeCircumference > relCirc.
    // Outputs relCircStart and relCircEnd of the two guide curves.
    // The output index is the index of the first guide curve in a sorted std::vector,
    // i.e. index starts at 0 and the order is not necessarily the one from the cpacs
    // file.
    //
    // This function is used e.g. in CCPACSWingSegment->GetPoint.
    TIGL_EXPORT void GetRelativeCircumferenceRange(double relCirc,
                                                   double& relCircStart,
                                                   double& relCircEnd,
                                                   int& index) const;
};

} // end namespace tigl

#endif // CCPACSGUIDECURVES_H


