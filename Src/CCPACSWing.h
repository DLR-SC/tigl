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
* @brief  Implementation of CPACS wing handling routines.
*/

#ifndef CCPACSWING_H
#define CCPACSWING_H

#include <string>

#include "tixi.h"
#include "CTiglTransformation.h"
#include "CTiglAbstractGeometricComponent.h"
#include "CCPACSWingSections.h"
#include "CCPACSWingSegments.h"
#include "CCPACSWingComponentSegments.h"
#include "CCPACSWingPositionings.h"
#include "ITiglSegment.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"


namespace tigl {

	class CCPACSConfiguration;

    class CCPACSWing : public CTiglAbstractGeometricComponent
	{

	public:
		// Constructor
		CCPACSWing(CCPACSConfiguration* config);

		// Virtual destructor
		virtual ~CCPACSWing(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS wing elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & wingXPath);

		// Returns the name of the wing
		const std::string & GetName(void) const;

		// Returns the uid of the wing
		const char* GetUIDPtr(void);

		// Returns the parent configuration
		CCPACSConfiguration & GetConfiguration(void) const;

		// Get section count
		int GetSectionCount(void) const;

		// Returns the section for a given index
		CCPACSWingSection & GetSection(int index) const;

		// Get segment count
		int GetSegmentCount(void);

		// Returns the segment for a given index
		ITiglSegment & GetSegment(const int index);

		// Get segment count
		int GetComponentSegmentCount(void);

		// Returns the segment for a given index
		ITiglSegment & GetComponentSegment(const int index);

		// Gets the wing transformation
		CTiglTransformation GetWingTransformation(void);

		// Get the positioning transformation for a given section uid
		CTiglTransformation GetPositioningTransformation(std::string index);

        // Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
	    gp_Pnt GetUpperPoint(int segmentIndex, double eta, double xsi);

    	// Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
	    gp_Pnt GetLowerPoint(int segmentIndex, double eta, double xsi);

   		// Gets the loft of the whole wing.
		TopoDS_Shape & GetLoft(void);

        // Gets the volume of this wing
        double GetVolume(void);

        // Get the Transformation object
        CTiglTransformation GetTransformation(void);
        
		// Gets the surfade area of this wing
        double GetSurfaceArea();

        // Returns the Component Type TIGL_COMPONENT_WING.
        TiglGeometricComponentType GetComponentType(void) {return TIGL_COMPONENT_WING;}

        // Returns a unique Hashcode for a specific geometric component
		int GetComponentHashCode(void);

    protected:
		// Cleanup routine
		void Cleanup(void);

		// Build transformation matrix for the wing
		void BuildMatrix(void);

		// Update internal wing data
		void Update(void);

		// Adds all Segments of this wing to one shape
		void BuildFusedSegments(void);


    private:
		// Copy constructor
		CCPACSWing(const CCPACSWing & ) : segments(0), componentSegments(0) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWing & ) { /* Do nothing */ }

	private:
		std::string                	name;                 	/**< Wing name           */
		std::string				   	uid;					/**< Wing UID			  */
		CTiglTransformation        	transformation;       	/**< Wing transformation */
		CTiglPoint                 	translation;          	/**< Wing translation    */
		CTiglPoint                 	scaling;              	/**< Wing scaling        */
		CTiglPoint                 	rotation;             	/**< Wing rotation       */
		CCPACSWingSections         	sections;             	/**< Wing sections       */
		CCPACSWingSegments         	segments;             	/**< Wing segments       */
		CCPACSWingComponentSegments	componentSegments;     	/**< Wing ComponentSegments       */
		CCPACSWingPositionings     	positionings;         	/**< Wing positionings   */
		CCPACSConfiguration*       	configuration;        	/**< Parent configuration*/
		TopoDS_Shape		       	fusedSegments;        	/**< All Segments in one shape */
		bool                       	invalidated;          	/**< Internal state flag */
		bool                       	rebuildFusedSegments; 	/**< Indicates if segmentation fusing need rebuild */
		FusedElementsContainerType 	fusedElements;		 	/**< Stores already fused segments */
		TopoDS_Compound			   	aCompound;
		BRep_Builder			   	aBuilder;
        double                     	myVolume;             	/**< Volume of this Wing                  */
	};

} // end namespace tigl

#endif // CCPACSWING_H
