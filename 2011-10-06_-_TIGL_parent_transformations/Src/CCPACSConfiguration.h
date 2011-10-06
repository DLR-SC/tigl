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
* @brief  Implementation of CPACS configuration handling routines.
*/

#ifndef CCPACSCONFIGURATION_H
#define CCPACSCONFIGURATION_H

#include "CTiglUIDManager.h"
#include "CCPACSHeader.h"
#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"

namespace tigl {

	class CCPACSConfiguration
	{

	public:
		// Constructor
		CCPACSConfiguration(TixiDocumentHandle tixiHandle);

		// Virtual Destructor
		virtual ~CCPACSConfiguration(void);

		// Invalidates the internal state of the configuration and forces
		// recalculation of wires, lofts etc.
		void Invalidate(void);

		// Read CPACS configuration
		void ReadCPACS(char* configurationUID);

		// Returns the underlying tixi document handle used by a CPACS configuration
		TixiDocumentHandle GetTixiDocumentHandle(void) const;

		// Returns the total count of wing profiles in this configuration
		int GetWingProfileCount(void) const;

		// Returns the wing profile for a given index - TODO: depricated!
		CCPACSWingProfile& GetWingProfile(int index) const;

		// Returns the wing profile for a given uid.
		CCPACSWingProfile& GetWingProfile(std::string uid) const;

		// Returns the total count of wings in a configuration
		int GetWingCount(void) const;

		// Returns the wing for a given index.
		CCPACSWing& GetWing(int index) const;

        // Returns the wing for a given UID.
        CCPACSWing& GetWing(const std::string UID) const;

		// Returns the total count of fuselage profiles in this configuration
		int GetFuselageProfileCount(void) const;

		// Returns the fuselage profile for a given index.
		CCPACSFuselageProfile& GetFuselageProfile(int index) const;

		// Returns the fuselage profile for a given uid.
	    CCPACSFuselageProfile& GetFuselageProfile(std::string uid) const;

		// Returns the total count of fuselages in a configuration
		int GetFuselageCount(void) const;

		// Returns the fuselage for a given index.
		CCPACSFuselage& GetFuselage(int index) const;

        // Returns the fuselage for a given UID.
        CCPACSFuselage& GetFuselage(std::string UID) const;

        // Returns the uid manager
        CTiglUIDManager& GetUIDManager(void);

		// Returns the bolean fused airplane as TopoDS_Shape
		TopoDS_Shape GetFusedAirplane(void);

    protected:
        void OutputComponentTree(ITiglGeometricComponent* parent);

    private:
		// Copy constructor
		CCPACSConfiguration(const CCPACSConfiguration& ) : wings(0), fuselages(0) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSConfiguration& ) { /* Do nothing */ }

	private:
		TixiDocumentHandle           tixiDocumentHandle;   /**< Handle for internal TixiDocument */
		CCPACSHeader                 header;               /**< Configuration header element */
		CCPACSWings                  wings;                /**< Configuration wings element */
		CCPACSFuselages              fuselages;            /**< Configuration fuselages element */
		CTiglUIDManager              uidManager;           /**< Stores the unique ids of the components */
		TopoDS_Shape				 fusedAirplane;		   /**< The complete airplaine as one fused shape */
	};

} // end namespace tigl

#endif // CCPACSCONFIGURATION_H
