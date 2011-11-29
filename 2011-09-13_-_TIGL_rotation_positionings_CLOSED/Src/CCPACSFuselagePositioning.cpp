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

	// Build end transformation matrix for the positioning
	void CCPACSFuselagePositioning::BuildMatrix(void)
	{
		// Compose the transformation for the end section reference point.
		// Each fuselage positioning will have its own orientation independently from
		// any previous sections (no angle chain).
		// Only the translation is matched to join the positioning length lines.
		// So any change e.g. in sweep angle will affect only this positioning.

		double xEnd = length * sin(CTiglTransformation::DegreeToRadian(sweepangle));
		double yEnd = length * cos(CTiglTransformation::DegreeToRadian(sweepangle));
		double zEnd = 0.0;

		endTransformation.SetIdentity();
		endTransformation.AddTranslation(xEnd, yEnd, zEnd);
		endTransformation.AddRotationX(dihedralangle);
		endTransformation.AddTranslation(startPoint.x, startPoint.y, startPoint.z);

		// Calculate end section point by transforming origin
		gp_Pnt tempPnt = endTransformation.Transform(gp_Pnt(0.0, 0.0, 0.0));
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

		// Get subelement "sweepangle"
		tempString  = positioningXPath + "/sweepangle";
		elementPath = const_cast<char*>(tempString.c_str());
		tixiGetDoubleElement(tixiHandle, elementPath, &sweepangle);

		// Get subelement "dihedralangle"
		tempString  = positioningXPath + "/dihedralangle";
		elementPath = const_cast<char*>(tempString.c_str());
		tixiGetDoubleElement(tixiHandle, elementPath, &dihedralangle);

		// Get subelement "toSectionUID"
		char*		  ptrOuterSection = "";
		tempString  = positioningXPath + "/toSectionUID";
		elementPath = const_cast<char*>(tempString.c_str());
		if (tixiGetTextElement(tixiHandle, elementPath, &ptrOuterSection) != SUCCESS)
			throw CTiglError("Error: Can't read element <toSectionUID/> in CCPACSFuselagePositioning:ReadCPACS", TIGL_XML_ERROR);
		endSection = ptrOuterSection;

		// Get subelement "fromSectionUID"
		char*		  ptrInnerSection = "";
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
