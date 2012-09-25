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
* @brief  Implementation of CPACS wing positioning handling routines.
*/

#ifndef CCPACSWINGPOSITIONING_H
#define CCPACSWINGPOSITIONING_H

#include "tixi.h"
#include "CTiglTransformation.h"
#include "CTiglPoint.h"
#include <string>
#include <vector>

namespace tigl {

	class CCPACSWingPositioning
	{

	public:
		// Constructor
		CCPACSWingPositioning(void);

		// Virtual Destructor
		virtual ~CCPACSWingPositioning(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS segment elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& positioningXPath);

		// Sets the positioning of the inner point
		void SetInnerPoint(const CTiglPoint& aPoint);

		// Gets the positioning of the outer point
		CTiglPoint GetOuterPoint(void);

		// Gets the outer transformation of this positioning
		CTiglTransformation GetOuterTransformation(void);

		// Gets the section-uid of the outer section of this positioning
		std::string GetOuterSectionIndex(void);

		// Gets the section-uid of the inner section of this positioning
		std::string GetInnerSectionIndex(void);

	protected:
		// Cleanup routine
		void Cleanup(void);

		// Build transformation matrix for the positioning
		void BuildMatrix(void);

		// Update internal positioning data
		void Update(void);

	private:
		// Copy constructor
		CCPACSWingPositioning(const CCPACSWingPositioning& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWingPositioning& ) { /* Do nothing */ }

	private:
		double               length;               /**< Positioning length                      */
		double               sweepangle;           /**< Positioning sweep angle                 */
		double               dihedralangle;        /**< Positioning dihedral angle              */
		CTiglPoint           innerPoint;           /**< Positioning inner point                 */
		CTiglPoint           outerPoint;           /**< Positioning outer point                 */
		CTiglTransformation  outerTransformation;  /**< Transformation for the outer section    */
		std::string          innerSection;         /**< UID of the inner section                */
		std::string			 outerSection;         /**< UID of the outer section                */
		bool                 invalidated;          /**< Internal state flag                     */

	};

} // end namespace tigl

#endif // CCPACSWINGPOSITIONING_H
