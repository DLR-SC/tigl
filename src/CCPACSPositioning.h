/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
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

#pragma once

#include <vector>
#include "generated/CPACSPositioning.h"
#include "CTiglTransformation.h"
#include "CTiglPoint.h"

namespace tigl
{
class CCPACSPositioning : public generated::CPACSPositioning
{
public:
    // Constructor
    TIGL_EXPORT CCPACSPositioning(CCPACSPositionings* parent, CTiglUIDManager* uidMgr);

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& positioningXPath) override;

    TIGL_EXPORT void SetLength(const double& value) override;
    TIGL_EXPORT void SetSweepAngle(const double& value) override;
    TIGL_EXPORT void SetDihedralAngle(const double& value) override;
    TIGL_EXPORT void SetFromSectionUID(const boost::optional<std::string>& value) override;
    TIGL_EXPORT void SetToSectionUID(const std::string& value) override;

    TIGL_EXPORT void SetFromPoint(const CTiglPoint& aPoint);
    TIGL_EXPORT const CTiglPoint& GetFromPoint();

    TIGL_EXPORT void SetToPoint(const CTiglPoint& aPoint);
    TIGL_EXPORT const CTiglPoint& GetToPoint();

    TIGL_EXPORT CTiglTransformation GetToTransformation();

    // Adds child to childs. To be successful the following condition must be met:
    // child.startSectionIndex == this.endSectionIndex
    // no ownership is taken of child
    TIGL_EXPORT void AddDependentPositioning(CCPACSPositioning* child);
    TIGL_EXPORT void DisconnectDependentPositionings();
    TIGL_EXPORT const std::vector<CCPACSPositioning*> GetDependentPositionings() const;

protected:
    // Build transformation matrix for the positioning
    void BuildMatrix();

    // Update internal positioning data
    void Update();

private:
    // Invalidates internal state
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

    CTiglPoint           _fromPoint;        //< Positioning inner/start point
    CTiglPoint           _toPoint;          //< Positioning outer/end point
    CTiglTransformation  _toTransformation; //< Transformation for the outer/end section
    mutable bool         invalidated;       //< Internal state flag
    std::vector<CCPACSPositioning*> _dependentPositionings;
};
}
