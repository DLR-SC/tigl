/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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
* @brief  Implementation of CPACS wing ComponentSegment handling routines.
*/

#ifndef CCPACSWINGCOMPONENTSEGMENT_H
#define CCPACSWINGCOMPONENTSEGMENT_H

#include <string>

#include "tixi.h"
#include "CCPACSWingConnection.h"
#include "CTiglPoint.h"
#include "CTiglAbstractSegment.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Wire.hxx"
#include "Geom_BSplineSurface.hxx"


namespace tigl {

	class CCPACSWing;

	class CCPACSWingComponentSegment : public CTiglAbstractSegment
	{

	public:
		// Constructor
		CCPACSWingComponentSegment(CCPACSWing* aWing, int aSegmentIndex);

		// Virtual Destructor
		virtual ~CCPACSWingComponentSegment(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS segment elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & segmentXPath);

		// Returns the wing this segment belongs to
		CCPACSWing& GetWing(void) const;

		// Gets the loft between the two segment sections
		TopoDS_Shape& GetLoft(void);

		// Gets a point in relative wing coordinates for a given eta and xsi
		gp_Pnt GetPoint(double eta, double xsi);

        // Gets the volume of this segment
        double GetVolume();

		// Gets the surface area of this segment
        double GetSurfaceArea();

        // Gets the fromElementUID of this segment
		const std::string & GetFromElementUID(void);

		// Gets the toElementUID of this segment
		const std::string & GetToElementUID(void);

		// Returns the segment to a given point on the componentSegment.
		// Returns null if the point is not an that wing!
		const std::string findSegment(double x, double y, double z);

        TiglGeometricComponentType GetComponentType(){ return TIGL_COMPONENT_WINGCOMPSEGMENT | TIGL_COMPONENT_SEGMENT | TIGL_COMPONENT_LOGICAL; }

        // builds data structure for a TDocStd_Application
        // mostly used for export
        TDF_Label ExportDataStructure(Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label);

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
//		gp_Pnt GetPoint(double eta, double xsi, bool fromUpper);

    private:
		// Copy constructor
		CCPACSWingComponentSegment(const CCPACSWingComponentSegment& )  { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWingComponentSegment& ) { /* Do nothing */ }


	private:
		std::string          name;                 /**< Segment name                            */
		std::string          fromElementUID;       /**< Inner segment uid (root			        */
		std::string          toElementUID;         /**< Outer segment uid (tip)			        */
		CCPACSWing*          wing;                 /**< Parent wing                             */
        double               myVolume;             /**< Volume of this segment                  */
        double               mySurfaceArea;        /**< Surface area of this segment            */
		TopoDS_Shape		 upperShape;		   /**< Upper shape of this componentSegment	*/
		TopoDS_Shape		 lowerShape;		   /**< Lower shape of this componentSegment	*/
		Handle(Geom_Surface) upperSurface;
		Handle(Geom_Surface) lowerSurface;
		bool                 surfacesAreValid;

	};

} // end namespace tigl

#endif // CCPACSWINGCOMPONENTSEGMENT_H
