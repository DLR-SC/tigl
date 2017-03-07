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
/**
* @file
* @brief  Implementation of CPACS wing positioning handling routines.
*/

#ifndef CCPACSPositioning_H
#define CCPACSPositioning_H

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
    TIGL_EXPORT CCPACSPositioning();

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSPositioning();

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& positioningXPath);

    // Sets the positioning of the inner point
    TIGL_EXPORT void SetInnerPoint(const CTiglPoint& aPoint);

    // Gets the positioning of the outer point
    TIGL_EXPORT CTiglPoint GetOuterPoint();

    // Gets the outer transformation of this positioning
    TIGL_EXPORT CTiglTransformation GetOuterTransformation();

    // Gets the section-uid of the outer section of this positioning
    DEPRECATED TIGL_EXPORT std::string GetOuterSectionIndex();

    // Gets the section-uid of the inner section of this positioning
    DEPRECATED TIGL_EXPORT std::string GetInnerSectionIndex();

    // Sets the positioning of the start point
    TIGL_EXPORT void SetStartPoint(const CTiglPoint& aPoint);

    // Gets the positioning of the end point
    TIGL_EXPORT CTiglPoint GetEndPoint();

    // Gets the end transformation of this positioning
    TIGL_EXPORT CTiglTransformation GetEndTransformation();

    // Gets the section index of the end section of this positioning
    DEPRECATED TIGL_EXPORT std::string GetEndSectionIndex();

    // Gets the section index of the start section of this positioning
    DEPRECATED TIGL_EXPORT std::string GetStartSectionIndex();

    // Adds child to childs. To be successful the following condition must be met:
    // child.startSectionIndex == this.endSectionIndex
    TIGL_EXPORT void ConnectChildPositioning(CCPACSPositioning* child);
    TIGL_EXPORT void DisconnectChilds();

    TIGL_EXPORT const std::vector<CCPACSPositioning*> GetChilds() const;

    // Cleanup routine
    TIGL_EXPORT void Cleanup();

protected:
    // Build transformation matrix for the positioning
    void BuildMatrix();

    // Update internal positioning data
    void Update();

private:
    CTiglPoint           innerPoint;           /**< Positioning inner point                 */
    CTiglPoint           outerPoint;           /**< Positioning outer point                 */
    CTiglTransformation  outerTransformation;  /**< Transformation for the outer section    */
    bool                 invalidated;          /**< Internal state flag                     */

    std::vector<CCPACSPositioning*> childPositionings;
};

} // end namespace tigl

#endif // CCPACSPositioning_H
