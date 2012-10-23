/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
* @brief  Implementation of CPACS fuselage positioning handling routines.
*/

#include "tigl.h"
#include "CCPACSFuselagePositioning.h"
#include "CTiglError.h"
#include "gp_Pnt.hxx"
#include <iostream>

namespace tigl {

	// Constructor
	CCPACSFuselagePositioning::CCPACSFuselagePositioning(void)
	{
		Cleanup();
	}

	// Destructor
	CCPACSFuselagePositioning::~CCPACSFuselagePositioning(void)
	{
		Cleanup();
	}

	// Invalidates internal state
	void CCPACSFuselagePositioning::Invalidate(void)
	{
		invalidated = true;
	}

	// Cleanup routine
	void CCPACSFuselagePositioning::Cleanup(void)
	{
		length        = 1.0;
		sweepangle    = 0.0;
		dihedralangle = 0.0;
		startPoint    = CTiglPoint(0.0, 0.0, 0.0);
		endPoint      = CTiglPoint(0.0, 0.0, 0.0);
		endTransformation.SetIdentity();
		startSection  = -1;
		endSection    = -1;
		Invalidate();
	}

	// Sets the positioning start point
	void CCPACSFuselagePositioning::SetStartPoint(const CTiglPoint& aPoint)
	{
		startPoint = aPoint;
		Invalidate();
	}

	// Gets the positioning end point
	CTiglPoint CCPACSFuselagePositioning::GetEndPoint(void)
	{
		Update();
		return endPoint;
	}

	// Gets the end transformation of this segment
	CTiglTransformation CCPACSFuselagePositioning::GetEndTransformation(void)
	{
		Update();
		return endTransformation;
	}

	// Gets the section index of the end section of this positioning
	std::string CCPACSFuselagePositioning::GetEndSectionIndex(void)
	{
		Update();
		return endSection;
	}

	// Gets the section index of the start section of this positioning
	std::string CCPACSFuselagePositioning::GetStartSectionIndex(void)
	{
		Update();
		return startSection;
	}

	// Build outer transformation matrix for the positioning
	void CCPACSFuselagePositioning::BuildMatrix(void)
	{
		// Compose the transformation for the tip section reference point.
		// The positioning transformation is basically a translation in two steps:
		// 1. from the fuselage origin to the startPoint (= endPoint of previous positioning)
		// 2. from the innerPoint to the endPoint with coordinates given
		//    in a "spherical" coordinate system (length, sweepAngle, dihedralAngle).
		// The original section is neither rotated by sweepAngle nor by dihedralAngle.

		// Calculate the cartesian translation components for step two from "spherical" input coordinates
		CTiglTransformation tempTransformation;
		tempTransformation.SetIdentity();
		tempTransformation.AddRotationZ(-sweepangle);
		tempTransformation.AddRotationX(dihedralangle);
		gp_Pnt tempPnt = tempTransformation.Transform(gp_Pnt(0.0, length, 0.0));

		// Setup transformation combining both steps
		endTransformation.SetIdentity();
		endTransformation.AddTranslation( startPoint.x + tempPnt.X() , 
                                          startPoint.y + tempPnt.Y() , 
                                          startPoint.z + tempPnt.Z() );

		// calculate outer section point by transforming origin
		tempPnt = endTransformation.Transform(gp_Pnt(0.0, 0.0, 0.0));
		endPoint.x = tempPnt.X();
		endPoint.y = tempPnt.Y();
		endPoint.z = tempPnt.Z();
	}

	// Update internal segment data
	void CCPACSFuselagePositioning::Update(void)
	{
		if (!invalidated)
			return;

		BuildMatrix();
		invalidated = false;
	}

	// Read CPACS segment elements
	void CCPACSFuselagePositioning::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& positioningXPath)
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

		// Get subelement "toSectionUID"
		char*		  ptrOuterSection = NULL;
		tempString  = positioningXPath + "/toSectionUID";
		elementPath = const_cast<char*>(tempString.c_str());
		if (tixiGetTextElement(tixiHandle, elementPath, &ptrOuterSection) != SUCCESS)
			throw CTiglError("Error: Can't read element <toSectionUID/> in CCPACSFuselagePositioning:ReadCPACS", TIGL_XML_ERROR);
		endSection = ptrOuterSection;

		// Get subelement "fromSectionUID"
		char*		  ptrInnerSection = NULL;
		tempString  = positioningXPath + "/fromSectionUID";
		elementPath = const_cast<char*>(tempString.c_str());
		if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
			tixiGetTextElement(tixiHandle, elementPath, &ptrInnerSection);
			startSection = ptrInnerSection;
		} else {
			startSection = endSection;
		}

		Update();
	}

} // end namespace tigl
