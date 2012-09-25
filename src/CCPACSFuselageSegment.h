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
* @brief  Implementation of CPACS fuselage segment handling routines.
*/

#ifndef CCPACSFUSELAGESEGMENT_H
#define CCPACSFUSELAGESEGMENT_H

#include <string>

#include "tixi.h"
#include "CCPACSFuselageConnection.h"
#include "CTiglTransformation.h"
#include "CTiglPoint.h"
#include "ITiglSegment.h"

#include "TopoDS_Shape.hxx"


namespace tigl {

	class CCPACSFuselage;

	class CCPACSFuselageSegment : public ITiglSegment
	{

	public:
		// Constructor
		CCPACSFuselageSegment(CCPACSFuselage* aFuselage, int aSegmentIndex);

		// Virtual Destructor
		virtual ~CCPACSFuselageSegment(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS segment elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath);

		// Returns the fuselage this segment belongs to
		CCPACSFuselage& GetFuselage(void) const;

        // Returns the segment index of this segment
        int GetSegmentIndex(void) const;

		// Gets the loft between the two segment sections
		TopoDS_Shape GetLoft(void);

        // Returns the start section UID of this segment
		std::string GetStartSectionUID(void);

        // Returns the end section UID of this segment
		std::string GetEndSectionUID(void);

        // Returns the start section index of this segment
		int GetStartSectionIndex(void);

        // Returns the end section index of this segment
		int GetEndSectionIndex(void);

		// Returns the starting Segement Connection
		CCPACSFuselageConnection& GetStartConnection(void);

		// Return the end Segment Connection
		CCPACSFuselageConnection& GetEndConnection(void);

        // Returns the start section element UID of this segment
		std::string GetStartSectionElementUID(void);

        // Returns the end section element UID of this segment
		std::string GetEndSectionElementUID(void);

        // Returns the start section element index of this segment
		int GetStartSectionElementIndex(void);

        // Returns the end section element index of this segment
		int GetEndSectionElementIndex(void);

        // Gets the count of segments connected to the start section of this segment
        int GetStartConnectedSegmentCount(void);

        // Gets the count of segments connected to the end section of this segment
        int GetEndConnectedSegmentCount(void);

        // Gets the index (number) of the n-th segment connected to the start section
        // of this segment. n starts at 1.
        int GetStartConnectedSegmentIndex(int n);

        // Gets the index (number) of the n-th segment connected to the end section
        // of this segment. n starts at 1.
        int GetEndConnectedSegmentIndex(int n);

        // Gets a point on the fuselage segment in dependence of parameters eta and zeta with
        // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
        // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
        // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
        gp_Pnt GetPoint(double eta, double zeta);

        int GetNumPointsOnYPlane(double eta, double ypos);
        gp_Pnt GetPointOnYPlane(double eta, double ypos, int pointIndex);

        int GetNumPointsOnXPlane(double eta, double xpos);
        gp_Pnt GetPointOnXPlane(double eta, double xpos, int pointIndex);

        // Gets a point on the fuselage segment in dependence of an angle alpha (degree).
        // The origin of the angle could be set via the parameters y_cs and z_cs,
        // but in most cases y_cs and z_cs will be zero get the get center line of the profile.
        gp_Pnt GetPointAngle(double eta, double alpha, double y_cs, double z_cs);

        // Gets the volume of this segment
        double GetVolume();
		
		// Gets the surface area of this segment
        double GetSurfaceArea();

        // Gets the uid of this segment
        const std::string & GetUID(void);

        // Gets the C pointer of the UID string
        const char* GetUIDPtr(void);

        // Gets the wire on the loft at a given eta
        TopoDS_Shape getWireOnLoft(double eta);

        // Returns the circumference if the segment at a given eta
        double GetCircumference(const double eta);

        // Returns the inner profile points as read from TIXI. The points are already transformed.
        std::vector<CTiglPoint*> GetRawStartProfilePoints();

        // Returns the outer profile points as read from TIXI. The points are already transformed.
        std::vector<CTiglPoint*> GetRawEndProfilePoints();

    protected:
		// Cleanup routine
		void Cleanup(void);

		// Update internal segment data
		void Update(void);

		// Builds the loft between the two segment sections
		void BuildLoft(void);

    private:
		// Copy constructor
		CCPACSFuselageSegment(const CCPACSFuselageSegment& ) : startConnection(0), endConnection(0) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSFuselageSegment& ) { /* Do nothing */ }

		std::string              name;                 /**< Segment name                            */
        std::string              uid;                  /**< Segment uid                             */
		CCPACSFuselageConnection startConnection;      /**< Start segment connection                */
		CCPACSFuselageConnection endConnection;        /**< End segment connection                  */
		CCPACSFuselage*          fuselage;             /**< Parent fuselage                         */
		TopoDS_Shape             loft;                 /**< The loft between two sections           */
		bool                     invalidated;          /**< Internal state flag                     */
        int                      mySegmentIndex;       /**< Index of this segment                   */
        double                   myVolume;             /**< Volume of this segment                  */
        double                   mySurfaceArea;        /**< Surface Area of this segment            */
        double                   myWireLength;         /**< Wire length of this segment for a given zeta */

	};

} // end namespace tigl

#endif // CCPACSFUSELAGESEGMENT_H
