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

#ifndef CCPACSFUSELAGEPOSITIONING_H
#define CCPACSFUSELAGEPOSITIONING_H

#include "tixi.h"
#include "CTiglTransformation.h"
#include "CTiglPoint.h"
#include <string>
#include <vector>

namespace tigl {

	class CCPACSFuselagePositioning
	{

	public:
		// Constructor
		CCPACSFuselagePositioning(void);

		// Virtual Destructor
		virtual ~CCPACSFuselagePositioning(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS segment elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& positioningXPath);

		// Sets the positioning of the start point
		void SetStartPoint(const CTiglPoint& aPoint);

		// Gets the positioning of the end point
		CTiglPoint GetEndPoint(void);

		// Gets the end transformation of this positioning
		CTiglTransformation GetEndTransformation(void);

		// Gets the section index of the end section of this positioning
		std::string GetEndSectionIndex(void);

		// Gets the section index of the start section of this positioning
		std::string GetStartSectionIndex(void);

	protected:
		// Cleanup routine
		void Cleanup(void);

		// Build transformation matrix for the positioning
		void BuildMatrix(void);

		// Update internal positioning data
		void Update(void);

	private:
		// Copy constructor
		CCPACSFuselagePositioning(const CCPACSFuselagePositioning& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSFuselagePositioning& ) { /* Do nothing */ }

	private:
		double               length;               /**< Positioning length                      */
		double               sweepangle;           /**< Positioning sweep angle                 */
		double               dihedralangle;        /**< Positioning dihedral angle              */
		CTiglPoint           startPoint;           /**< Positioning start point                 */
		CTiglPoint           endPoint;             /**< Positioning end point                   */
		CTiglTransformation  endTransformation;    /**< Transformation for the end section      */
		std::string 		 startSection;         /**< uid of start section                    */
		std::string          endSection;           /**< uid of end section                      */
		bool                 invalidated;          /**< Internal state flag                     */

	};

} // end namespace tigl

#endif // CCPACSFUSELAGEPOSITIONING_H
