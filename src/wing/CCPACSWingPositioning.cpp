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
#include "CCPACSWingPositioning.h"
#include "CTiglError.h"
#include "gp_Pnt.hxx"
#include <iostream>

namespace tigl
{

// Constructor
CCPACSWingPositioning::CCPACSWingPositioning(void)
{
    Cleanup();
}

// Destructor
CCPACSWingPositioning::~CCPACSWingPositioning(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWingPositioning::Invalidate(void)
{
    invalidated = true;
}

// Cleanup routine
void CCPACSWingPositioning::Cleanup(void)
{
    length        = 1.0;
    sweepangle    = 0.0;
    dihedralangle = 0.0;
    innerPoint    = CTiglPoint(0.0, 0.0, 0.0);
    outerPoint    = CTiglPoint(0.0, 0.0, 0.0);
    outerTransformation.SetIdentity();
    innerSection  = -1;
    outerSection  = -1;
    childPositionings.clear();
    Invalidate();
}

// Sets the positioning inner point
void CCPACSWingPositioning::SetInnerPoint(const CTiglPoint& aPoint)
{
    innerPoint = aPoint;
    Invalidate();
}

// Gets the positioning outer point
CTiglPoint CCPACSWingPositioning::GetOuterPoint(void)
{
    Update();
    return outerPoint;
}

// Gets the outer transformation of this segment
CTiglTransformation CCPACSWingPositioning::GetOuterTransformation(void)
{
    Update();
    return outerTransformation;
}

// Gets the section-uid of the outer section of this positioning
std::string CCPACSWingPositioning::GetOuterSectionIndex(void)
{
    Update();
    return outerSection;
}

// Gets the section-uid of the inner section of this positioning
std::string CCPACSWingPositioning::GetInnerSectionIndex(void)
{
    Update();
    return innerSection;
}

// Build outer transformation matrix for the positioning
void CCPACSWingPositioning::BuildMatrix(void)
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
    tempTransformation.AddRotationZ(-sweepangle);
    tempTransformation.AddRotationX(dihedralangle);
    gp_Pnt tempPnt = tempTransformation.Transform(gp_Pnt(0.0, length, 0.0));

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
void CCPACSWingPositioning::Update(void)
{
    if (!invalidated) {
        return;
    }

    BuildMatrix();
    invalidated = false;
}

// Read CPACS segment elements
void CCPACSWingPositioning::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& positioningXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get subelement "length"
    tempString  = positioningXPath + "/length";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiGetDoubleElement(tixiHandle, elementPath, &length);

    // Get subelement "sweepAngle"
    tempString  = positioningXPath + "/sweepAngle";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiGetDoubleElement(tixiHandle, elementPath, &sweepangle);

    // Get subelement "dihedralAngle"
    tempString  = positioningXPath + "/dihedralAngle";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiGetDoubleElement(tixiHandle, elementPath, &dihedralangle);

    // Get subelement "toSectionUID" - the outer section
    char*          ptrOuterSection = NULL;
    tempString  = positioningXPath + "/toSectionUID";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrOuterSection) != SUCCESS) {
        throw CTiglError("Error: Can't read element <section[2]/> in CCPACSWingPositioning:ReadCPACS", TIGL_XML_ERROR);
    }
    outerSection = ptrOuterSection;

    // Get subelement "fromSectionUID" - the inner section
    char*          ptrInnerSection = NULL;
    tempString  = positioningXPath + "/fromSectionUID";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS && 
        tixiGetTextElement(tixiHandle, elementPath, &ptrInnerSection) == SUCCESS) {

        innerSection = ptrInnerSection;
    }
    else {
        innerSection = "";
    }

    Update();
}

void CCPACSWingPositioning::ConnectChildPositioning(CCPACSWingPositioning* child)
{
    if (!child) {
        return;
    }
    
    if (child->innerSection != this->outerSection) {
        throw CTiglError("Incompatible positioning connection in CCPACSWingPositioning::ConnectChildPositioning");
    }
    
    childPositionings.push_back(child);
}

const std::vector<CCPACSWingPositioning*> CCPACSWingPositioning::GetChilds() const
{
    return childPositionings;
}

void CCPACSWingPositioning::DisconnectChilds()
{
    childPositionings.clear();
}

} // end namespace tigl
