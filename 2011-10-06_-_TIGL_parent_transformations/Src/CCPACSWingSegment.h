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
* @brief  Implementation of CPACS wing segment handling routines.
*/

#ifndef CCPACSWINGSEGMENT_H
#define CCPACSWINGSEGMENT_H

#include <string>

#include "tixi.h"
#include "CCPACSWingConnection.h"
#include "CTiglPoint.h"
#include "ITiglSegment.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Wire.hxx"
#include "Geom_BSplineSurface.hxx"


namespace tigl {

	class CCPACSWing;

	class CCPACSWingSegment : public ITiglSegment
	{

	public:
		// Constructor
		CCPACSWingSegment(CCPACSWing* aWing, int aSegmentIndex);

		// Virtual Destructor
		virtual ~CCPACSWingSegment(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS segment elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & segmentXPath);

		// Returns the wing this segment belongs to
		CCPACSWing& GetWing(void) const;

        // Returns the segment index of this segment
		int GetSegmentIndex(void) const;

		// Gets the loft between the two segment sections
		TopoDS_Shape GetLoft(void);

		// Gets the upper point in relative wing coordinates for a given eta and xsi
		gp_Pnt GetUpperPoint(double eta, double xsi);

		// Gets the lower point in relative wing coordinates for a given eta and xsi
		gp_Pnt GetLowerPoint(double eta, double xsi);

        // Returns the inner section UID of this segment
		std::string GetInnerSectionUID(void);

        // Returns the outer section UID of this segment
		std::string GetOuterSectionUID(void);

        // Returns the inner section element UID of this segment
		std::string GetInnerSectionElementUID(void);

        // Returns the outer section element UID of this segment
		std::string GetOuterSectionElementUID(void);

        // Returns the inner section index of this segment
		int GetInnerSectionIndex(void);

        // Returns the outer section index of this segment
		int GetOuterSectionIndex(void);

        // Returns the inner section element index of this segment
		int GetInnerSectionElementIndex(void);

        // Returns the outer section element index of this segment
		int GetOuterSectionElementIndex(void);

		// Returns the starting(inner) Segment Connection
		CCPACSWingConnection& GetInnerConnection(void);

		// Return the end(outer) Segment Connection
		CCPACSWingConnection& GetOuterConnection(void);

        // Gets the count of segments connected to the inner section of this segment
        int GetInnerConnectedSegmentCount(void);

        // Gets the count of segments connected to the outer section of this segment
        int GetOuterConnectedSegmentCount(void);

        // Gets the index (number) of the n-th segment connected to the inner section
        // of this segment. n starts at 1.
        int GetInnerConnectedSegmentIndex(int n);

        // Gets the index (number) of the n-th segment connected to the outer section
        // of this segment. n starts at 1.
        int GetOuterConnectedSegmentIndex(int n);

        // Gets the volume of this segment
        double GetVolume();

		// Gets the surface area of this segment
        double GetSurfaceArea();

        // Gets the uid of this segment
        const std::string & GetUID(void);

        // Gets the C pointer of the UID string
        const char* GetUIDPtr(void);

        // helper function to get the inner transformed chord line wire, used in GetLoft and when determining triangulation midpoints projection on segments in VtkExport
        TopoDS_Wire GetInnerWire(void);

        // helper function to get the outer transformed chord line wire, used in GetLoft and when determining triangulation midpoints projection on segments in VtkExport
        TopoDS_Wire GetOuterWire(void);

        // Returns eta as parametric distance from a given point on the surface
        // Get information about a point beeing on upper/lower side with "GetIsOnTop"
        double GetEta(gp_Pnt pnt, bool isUpper);

        // Returns zeta as parametric distance from a given point on the surface
        // Get information about a point beeing on upper/lower side with "GetIsOnTop"
        double GetXsi(gp_Pnt pnt, bool isUpper);

        // Returns the inner profile points as read from TIXI. The points are already transformed.
        std::vector<CTiglPoint*> GetRawInnerProfilePoints();

        // Returns the outer profile points as read from TIXI. The points are already transformed.
        std::vector<CTiglPoint*> GetRawOuterProfilePoints();

        // Returns if the given point is ont the Top of the wing or on the lower side.
        bool GetIsOnTop(gp_Pnt pnt);


    protected:
		// Cleanup routine
		void Cleanup(void);

		// Update internal segment data
		void Update(void);

		// Builds the loft between the two segment sections
		void BuildLoft(void);

        // Returns an upper or lower point on the segment surface in
        // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
        // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
        // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
        // edge on the outer wing profile. If fromUpper is true, a point
        // on the upper surface is returned, otherwise from the lower.
		gp_Pnt GetPoint(double eta, double xsi, bool fromUpper);


    private:
		// Copy constructor
		CCPACSWingSegment(const CCPACSWingSegment& ) : innerConnection(0), outerConnection(0) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWingSegment& ) { /* Do nothing */ }

		// Builds upper and lower surfaces
		void MakeSurfaces();


	private:
		std::string          name;                 /**< Segment name                            */
		std::string          uid;                  /**< Segment uid                             */
		CCPACSWingConnection innerConnection;      /**< Inner segment connection (root)         */
		CCPACSWingConnection outerConnection;      /**< Outer segment connection (tip)          */
		CCPACSWing*          wing;                 /**< Parent wing                             */
		TopoDS_Shape         loft;                 /**< The loft between two sections           */
		bool                 invalidated;          /**< Internal state flag                     */
        int                  mySegmentIndex;       /**< Index of this segment                   */
        double               myVolume;             /**< Volume of this segment                  */
        double               mySurfaceArea;        /**< Surface area of this segment            */
		TopoDS_Shape		 upperShape;		   /**< Upper shape of this segment				*/
		TopoDS_Shape		 lowerShape;
		Handle(Geom_Surface) upperSurface;
		Handle(Geom_Surface) lowerSurface;
		bool                 surfacesAreValid;

	};

} // end namespace tigl

#endif // CCPACSWINGSEGMENT_H
