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

#include "tigl.h"
#include "CCPACSPositioning.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include "gp_Pnt.hxx"
#include <iostream>

namespace tigl
{

// Constructor
CCPACSPositioning::CCPACSPositioning(void)
{
    Cleanup();
}

// Destructor
CCPACSPositioning::~CCPACSPositioning(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSPositioning::Invalidate(void)
{
    invalidated = true;
}

// Cleanup routine
void CCPACSPositioning::Cleanup(void)
{
    m_length        = 1.0;
    m_sweepAngle    = 0.0;
    m_dihedralAngle = 0.0;
    innerPoint    = CTiglPoint(0.0, 0.0, 0.0);
    outerPoint    = CTiglPoint(0.0, 0.0, 0.0);
    outerTransformation.SetIdentity();
    childPositionings.clear();
    Invalidate();
}

// Sets the positioning inner point
void CCPACSPositioning::SetInnerPoint(const CTiglPoint& aPoint)
{
    innerPoint = aPoint;
    Invalidate();
}

// Sets the positioning start point
void CCPACSPositioning::SetStartPoint(const CTiglPoint& aPoint) {
    SetInnerPoint(aPoint);
}

// Gets the positioning outer point
CTiglPoint CCPACSPositioning::GetOuterPoint(void)
{
    Update();
    return outerPoint;
}

// Gets the positioning end point
CTiglPoint CCPACSPositioning::GetEndPoint() {
    return GetOuterPoint();
}

// Gets the outer transformation of this segment
CTiglTransformation CCPACSPositioning::GetOuterTransformation(void)
{
    Update();
    return outerTransformation;
}

CTiglTransformation CCPACSPositioning::GetEndTransformation(void) {
    Update();
    return outerTransformation;
}

// Gets the section-uid of the outer section of this positioning
std::string CCPACSPositioning::GetOuterSectionIndex(void)
{
    Update();
    return m_toSectionUID;
}

std::string CCPACSPositioning::GetEndSectionIndex(void) {
    return GetOuterSectionIndex();
}

// Gets the section-uid of the inner section of this positioning
std::string CCPACSPositioning::GetInnerSectionIndex(void)
{
    Update();
    if (m_fromSectionUID.isValid())
        return m_fromSectionUID.get();
    else
        return std::string();
}

std::string CCPACSPositioning::GetStartSectionIndex(void) {
    return GetInnerSectionIndex();
}

// Build outer transformation matrix for the positioning
void CCPACSPositioning::BuildMatrix(void)
{
    // Compose the transformation for the tip section reference point.
    // The positioning transformation is basically a translation in two steps:
    // 1. from the wing origin to the innerPoint (= outerPoint of previous positioning)
    // 2. from the innerPoint to the outerPoint with coordinates given
    //    in a "spherical" coordinate system (length, sweepAngle, dihedralAngle).
    // The original section is neither rotated by sweepAngle nor by dihedralAngle.

    // Calculate the cartesian translation components for step two from "spherical" input coordinates
    CTiglTransformation tempTransformation;
    tempTransformation.SetIdentity();
    tempTransformation.AddRotationZ(-m_sweepAngle);
    tempTransformation.AddRotationX(m_dihedralAngle);
    gp_Pnt tempPnt = tempTransformation.Transform(gp_Pnt(0.0, m_length, 0.0));

    // Setup transformation combining both steps
    outerTransformation.SetIdentity();
    outerTransformation.AddTranslation( innerPoint.x + tempPnt.X() , 
                                        innerPoint.y + tempPnt.Y() , 
                                        innerPoint.z + tempPnt.Z() );

    // calculate outer section point by transforming orign
    tempPnt = outerTransformation.Transform(gp_Pnt(0.0, 0.0, 0.0));
    outerPoint.x = tempPnt.X();
    outerPoint.y = tempPnt.Y();
    outerPoint.z = tempPnt.Z();
}

// Update internal segment data
void CCPACSPositioning::Update(void)
{
    if (!invalidated) {
        return;
    }

    BuildMatrix();
    invalidated = false;
}

// Read CPACS segment elements
void CCPACSPositioning::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& positioningXPath)
{
    Cleanup();
    generated::CPACSPositioning::ReadCPACS(tixiHandle, positioningXPath);
    Update();
}

void CCPACSPositioning::ConnectChildPositioning(CCPACSPositioning* child)
{
    if (!child) {
        return;
    }
    
    if (child->m_fromSectionUID.get() != this->m_toSectionUID) {
        throw CTiglError("Incompatible positioning connection in CCPACSPositioning::ConnectChildPositioning");
    }
    
    childPositionings.push_back(child);
}

const std::vector<CCPACSPositioning*> CCPACSPositioning::GetChilds() const
{
    return childPositionings;
}

void CCPACSPositioning::DisconnectChilds()
{
    childPositionings.clear();
}

} // end namespace tigl
