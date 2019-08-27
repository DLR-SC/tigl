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
    TIGL_EXPORT CCPACSPositioning(CTiglUIDManager* uidMgr);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& positioningXPath) OVERRIDE;

    TIGL_EXPORT void SetFromPoint(const CTiglPoint& aPoint);
    TIGL_EXPORT const CTiglPoint& GetFromPoint();


    /**
     * Set the toPoint of this positioning.
     * By the the "toPoint", we mean the total position obtain by the section from this positioning and its parent positionings.
     *
     * @param aPoint
     * @param moveDependentPositionings: If true, the dependent positionings will also move from the delta between the newPosition
     * and the old position. If false, the dependent positioning will keep their positions (but, their parameters (sweep,...)
     * need to be updated).
     */
    TIGL_EXPORT void SetToPoint(const CTiglPoint& newPosition, bool moveDependentPositionings = false);
    TIGL_EXPORT const CTiglPoint& GetToPoint();

    TIGL_EXPORT CTiglTransformation GetToTransformation();

    // Adds child to childs. To be successful the following condition must be met:
    // child.startSectionIndex == this.endSectionIndex
    // no ownership is taken of child
    TIGL_EXPORT void AddDependentPositioning(CCPACSPositioning* child);
    TIGL_EXPORT void DisconnectDependentPositionings();
    TIGL_EXPORT const std::vector<CCPACSPositioning*> GetDependentPositionings() const;

    /**
     * Set the length, sweep and dihedral parameter from the given positioning vector.
     * @remark The fromSectionUID nor the toSectionUID are changed.
     * @param delta
     * @param rounding, if rounding is set to true angle and length will be rounding near common value as 0 ,90,180,..
     */
    TIGL_EXPORT void SetParametersFromVector(const CTiglPoint& delta, bool rounding = true );

protected:
    // Build transformation matrix for the positioning
    void BuildMatrix();

    // Update internal positioning data
    void Update();

    /**
     * Set the positioning vector (length, sweep, dihedral) such that the "toPoint" does not move
     * if the "fromPoint" take the new position "newFromPosition"
     * @remark This method is useful to update a positioning without changing its dependencies.
     * @param  newFromPosition: the new position of the from point.
     */
    void SetFromPointKeepingToPoint(const CTiglPoint& newFromPosition);


private:
    CTiglPoint           _fromPoint;        //< Positioning inner/start point
    CTiglPoint           _toPoint;          //< Positioning outer/end point
    CTiglTransformation  _toTransformation; //< Transformation for the outer/end section
    bool                 invalidated;       //< Internal state flag
    std::vector<CCPACSPositioning*> _dependentPositionings;
};
}
