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
* @brief  Implementation of CPACS fuselage section handling routines.
*/

#ifndef CCPACSFUSELAGESECTION_H
#define CCPACSFUSELAGESECTION_H

#include "tixi.h"
#include "CCPACSFuselageSectionElements.h"
#include "CTiglTransformation.h"
#include <string>

namespace tigl {

	class CCPACSFuselageSection
	{

	public:
		// Constructor
		CCPACSFuselageSection();

		// Virtual Destructor
		virtual ~CCPACSFuselageSection(void);

		// Read CPACS section elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath);

		// Get element count for this section
		int GetSectionElementCount(void) const;

		// Get element for a given index
		CCPACSFuselageSectionElement& GetSectionElement(int index) const;
		
		// Get the UID of this FuselageSection
		std::string GetUID(void) const;

		// Gets the section transformation
		CTiglTransformation GetSectionTransformation(void) const;

	protected:
		// Cleanup routine
		void Cleanup(void);

		// Build transformation matrix for the section
		void BuildMatrix(void);

		// Update internal section data
		void Update(void);

	private:
		// Copy constructor
		CCPACSFuselageSection(const CCPACSFuselageSection& )  { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSFuselageSection& ) { /* Do nothing */ }

	private:
		std::string                   name;           /**< Section name             */
		std::string                   uid;            /**< Section uid              */
		CTiglTransformation           transformation; /**< Section transfromation   */
		CTiglPoint                    translation;    /**< Section translation      */
		CTiglPoint                    scaling;        /**< Section scaling          */
		CTiglPoint                    rotation;       /**< Section rotation         */
		CCPACSFuselageSectionElements elements;       /**< Section elements         */

	};

} // end namespace tigl

#endif // CCPACSFUSELAGESECTION_H
