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
* @brief  Implementation of CPACS wing profile handling routines.
*/

#ifndef CCPACSWINGPROFILE_H
#define CCPACSWINGPROFILE_H

#include <vector>
#include <string>

#include "tixi.h"
#include "CTiglPoint.h"
#include "CTiglTransformation.h"

#include "TopoDS_Wire.hxx"
#include "Geom2d_TrimmedCurve.hxx"

namespace tigl {

    class CCPACSWingProfile
    {

    private:
        // Typedef for a container to store the coordinates of a wing profile element.
        typedef std::vector<CTiglPoint*> CCPACSCoordinateContainer;

    public:
        // Constructor
        CCPACSWingProfile(const std::string& aFilename);

        // Virtual Destructor
        virtual ~CCPACSWingProfile(void);

        // Read CPACS wing profile file
        void ReadCPACS(TixiDocumentHandle tixiHandle);

        // Returns the name of the wing profile
        std::string GetName(void) const;

		// Returns the description of the wing profile
        std::string GetDescription(void) const;

		// Returns the uid of the wing profile
        std::string GetUID(void) const;

        // Invalidates internal wing profile state
        void Invalidate(void);

        // Returns the wing profile wire.
        TopoDS_Wire GetWire(bool forceClosed = true);

        // Returns the leading edge point of the wing profile wire. The leading edge point
        // is already transformed by the wing profile element transformation.
        gp_Pnt GetLEPoint(void);

        // Returns the trailing edge point of the wing profile wire. The trailing edge point
        // is already transformed by the wing profile element transformation.
        gp_Pnt GetTEPoint(void);

        // Returns a point on the chord line between leading and trailing
        // edge as function of parameter xsi, which ranges from 0.0 to 1.0.
        // For xsi = 0.0 chord point is equal to leading edge, for xsi = 1.0
        // chord point is equal to trailing edge.
        gp_Pnt GetChordPoint(double xsi);

        // Returns a point on the upper wing profile as function of
        // parameter xsi, which ranges from 0.0 to 1.0.
        // For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
        // point is equal to trailing edge.
        gp_Pnt GetUpperPoint(double xsi);

        // Returns a point on the lower wing profile as function of
        // parameter xsi, which ranges from 0.0 to 1.0.
        // For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
        // point is equal to trailing edge.
        gp_Pnt GetLowerPoint(double xsi);

        // Returns the profile points as read from TIXI.
        std::vector<CTiglPoint*> GetCoordinateContainer();

    protected:
        // Cleanup routine
        void Cleanup(void);

        // Transforms a point by the wing profile transformation
        gp_Pnt TransformPoint(const gp_Pnt& aPoint) const;

        // Update the internal state, i.g. recalculates wire and le, te points
        void Update(void);

        // Builds the wing profile wires.
        void BuildWires(void);

        // Builds leading and trailing edge points
        void BuildLETEPoints(void);

        // Returns an upper or lower point on the wing profile in
        // dependence of parameter xsi, which ranges from 0.0 to 1.0.
        // For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
        // point is equal to trailing edge. If fromUpper is true, a point
        // on the upper profile is returned, otherwise from the lower.
        gp_Pnt GetPoint(double xsi, bool fromUpper);

        // Helper function to determine the chord line between leading and trailing edge in the profile plane
        Handle(Geom2d_TrimmedCurve) GetChordLine();

    private:
        // Copy constructor
        CCPACSWingProfile(const CCPACSWingProfile& )  { /* Do nothing */ }

        // Assignment operator
        void operator=(const CCPACSWingProfile& ) { /* Do nothing */ }

    private:
        std::string               ProfileXPath;
        std::string               name;
		std::string				  description;
		std::string				  uid;
        CCPACSCoordinateContainer coordinates;    /**< Coordinates of a wing profile element */
        bool                      invalidated;    /**< Flag if element is invalid */
        TopoDS_Wire               wireClosed;     /**< Forced closed wing profile wire */
        TopoDS_Wire               wireOriginal;   /**< Original wing profile wire */
        gp_Pnt                    lePoint;        /**< Leading edge point */
        gp_Pnt                    tePoint;        /**< Trailing edge point */

    };

} // end namespace tigl

#endif // CCPACSWINGPROFILE_H
