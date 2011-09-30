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
* @brief  Implementation of routines for managing CPACS configurations by handle.
*         Implements the singleton design pattern.
*/

#ifndef CCPACSCONFIGURATIONMANAGER_H
#define CCPACSCONFIGURATIONMANAGER_H

#include "tigl.h"
#include "CCPACSConfiguration.h"
#include <map>

namespace tigl {

	class CCPACSConfigurationManager
	{

	private:
		// Typedef for a CCPACSConfiguration container to store multiple CPACS configurations by a handle.
		typedef std::map<TiglCPACSConfigurationHandle, CCPACSConfiguration*> CCPACSConfigContainer;
		typedef CCPACSConfigContainer::iterator                              CCPACSConfigIterator;
		typedef CCPACSConfigContainer::const_iterator                        CCPACSConfigConstIterator;

	public:
		// Returns a reference to the only instance of this class
		static CCPACSConfigurationManager& GetInstance(void);

		// Adds a configuration to the configuration container and returns a handle for it.
		TiglCPACSConfigurationHandle AddConfiguration(CCPACSConfiguration* config);

		// Removes and deletes a configuration from the configuration container
		void DeleteConfiguration(TiglCPACSConfigurationHandle handle);

		// Returns the configuration for a given handle
		CCPACSConfiguration& GetConfiguration(TiglCPACSConfigurationHandle handle) const;

        // Tests if a given configuration handle is valid
        bool IsValid(TiglCPACSConfigurationHandle handle) const;

        // Invalidates all configurations and forces recalculation of wires/points etc.
        void Invalidate(void);

		// Destructor
		~CCPACSConfigurationManager(void);

	private:
		// Constructor
		CCPACSConfigurationManager(void);

		// Copy constructor
		CCPACSConfigurationManager(const CCPACSConfigurationManager& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSConfigurationManager& )             { /* Do nothing */ }

	private:
		CCPACSConfigContainer        configurations; // Container to store the configurations by a handle
		TiglCPACSConfigurationHandle handleCounter;  // Used to generate new handles

	};

} // end namespace tigl

#endif // CCPACSCONFIGURATIONMANAGER_H
