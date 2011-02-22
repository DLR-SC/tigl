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
* @brief  Implementation of CPACS fuselage handling routines.
*/

#ifndef CCPACSFUSELAGE_H
#define CCPACSFUSELAGE_H

#include <string>

#include "tixi.h"
#include "CTiglTransformation.h"
#include "ITiglSegment.h"
#include "CCPACSFuselageSections.h"
#include "CCPACSFuselageSegments.h"
#include "CCPACSFuselagePositionings.h"
#include "CTiglAbstractGeometricComponent.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"


namespace tigl {

	class CCPACSConfiguration;

    class CCPACSFuselage : public CTiglAbstractGeometricComponent
	{

	public:
		// Constructor
		CCPACSFuselage(CCPACSConfiguration* config);

		// Virtual Destructor
		virtual ~CCPACSFuselage(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS fuselage elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath);

		// Returns the name of the fuselage
		std::string GetName(void) const;

        // Returns the uid of the fuselage
        const char* GetUIDPtr(void);

		// Returns the parent configuration
		CCPACSConfiguration & GetConfiguration(void) const;

		// Get section count
		int GetSectionCount(void) const;

		// Returns the section for a given index
		CCPACSFuselageSection& GetSection(int index) const;

		// Get segment count
		int GetSegmentCount(void);

		// Returns the segment for a given index
		ITiglSegment & GetSegment(const int index);

		// Gets the fuselage transformation
		CTiglTransformation GetFuselageTransformation(void);

		// Get the positioning transformation for a given section index
		CTiglTransformation GetPositioningTransformation(std::string index);

        // Gets a point on the given fuselage segment in dependence of a parameters eta and zeta with
        // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
        // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
        // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
        gp_Pnt GetPoint(int segmentIndex, double eta, double zeta);

   		// Gets the loft of the whole fuselage.
		TopoDS_Shape & GetLoft(void);

        // Gets the volume of this fuselage
        double GetVolume();

        // get the Transformation object
        CTiglTransformation GetTransformation(void);

        // Gets the surfade area of this wing
        double GetSurfaceArea();

        // Returns the circumference of the segment "segmentIndex" at a given eta
        double GetCircumference(int segmentIndex, double eta);

        // Returns the Component Type TIGL_COMPONENT_FUSELAGE
        TiglGeometricComponentType GetComponentType(void) {return TIGL_COMPONENT_FUSELAGE;}

        // Returns a unique Hashcode for a specific geometric component
		int GetComponentHashCode(void);

		// Returns the point where the distance between the selected fuselage and the ground is at minimum.
		// The Fuselage could be turned with a given angle at at given axis, specified by a point and a direction.
		gp_Pnt GetMinumumDistanceToGround(gp_Ax1 RAxis, double angle);

    protected:
		// Cleanup routine
		void Cleanup(void);

		// Build transformation matrix for the fuselage
		void BuildMatrix(void);

		// Update internal fuselage data
		void Update(void);

		// Adds all segments of this fuselage to one shape
		void BuildFusedSegments(void);

    private:
		// Copy constructor
		CCPACSFuselage(const CCPACSFuselage & ) : segments(0) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSFuselage & ) { /* Do nothing */ }

	private:
		std::string                name;                 /**< Fuselage name           */
        std::string                uid;                  /**< Fuselage UID            */
		CTiglTransformation        transformation;       /**< Fuselage transformation */
		CTiglPoint                 translation;          /**< Fuselage translation    */
		CTiglPoint                 scaling;              /**< Fuselage scaling        */
		CTiglPoint                 rotation;             /**< Fuselage rotation       */
		CCPACSFuselageSections     sections;             /**< Fuselage sections       */
		CCPACSFuselageSegments     segments;             /**< Fuselage segments       */
		CCPACSFuselagePositionings positionings;         /**< Fuselage positionings   */
		CCPACSConfiguration*       configuration;        /**< Parent configuration    */
		TopoDS_Shape			   fusedSegments;        /**< All Segments in one shape */
		bool                       invalidated;          /**< Internal state flag     */
		bool					   rebuildFusedSegments; /**< Indicates if segmentation fusing need rebuild */
		FusedElementsContainerType fusedElements;		 /**< Stores already fused segments */

		TopoDS_Compound			   aCompound;
		BRep_Builder			   aBuilder;
        double                     myVolume;             /**< Volume of this fuselage              */
	};

} // end namespace tigl

#endif // CCPACSFUSELAGE_H
