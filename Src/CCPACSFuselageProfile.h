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
* @brief  Implementation of CPACS fuselage profile handling routines.
*/

#ifndef CCPACSFUSELAGEPROFILE_H
#define CCPACSFUSELAGEPROFILE_H

#include "tixi.h"
#include "CTiglPoint.h"
#include "CTiglTransformation.h"
#include "TopoDS_Wire.hxx"
#include <vector>
#include <string>

namespace tigl {

    class CCPACSFuselageProfile
    {

    private:
        // Typedef for a container to store the coordinates of a fuselage profile element.
        typedef std::vector<CTiglPoint*> CCPACSCoordinateContainer;

    public:
        // Constructor
        CCPACSFuselageProfile(const std::string& path);

        // Virtual Destructor
        virtual ~CCPACSFuselageProfile(void);

        // Read CPACS fuselage profile file
        void ReadCPACS(TixiDocumentHandle tixiHandle);

        // Returns the filename of the fuselage profile file
        std::string GetFileName(void) const;

        // Returns the name of the fuselage profile
        std::string GetName(void) const;

        // Returns the UID of the fuselage profile
        std::string GetUID(void) const;

        // Invalidates internal fuselage profile state
        void Invalidate(void);

        // Returns the fuselage profile wire. The wire is already transformed by the
        // fuselage profile element transformation.
        TopoDS_Wire GetWire(bool forceClosed = false);

        // Get length of fuselage profile wire
        double GetWireLength(void);

        // Gets a point on the fuselage profile wire in dependence of a parameter zeta with
        // 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the wire start point,
        // for zeta = 1.0 the last wire point.
        gp_Pnt GetPoint(double zeta);

        // Returns the profile points as read from TIXI.
        std::vector<CTiglPoint*> GetCoordinateContainer();

    protected:
        // Cleanup routine
        void Cleanup(void);

        // Update the internal state, i.g. recalculates wire
        void Update(void);

        // Transforms a point by the fuselage profile transformation
        gp_Pnt TransformPoint(const gp_Pnt& aPoint) const;

        // Builds the fuselage profile wires. The wires are already transformed by the
        // fuselage profile transformation.
        void BuildWires(void);

        // Computes the length of the fuselage profile wire
        void ComputeWireLength(void);

    private:
        // Copy constructor
        CCPACSFuselageProfile(const CCPACSFuselageProfile& )  { /* Do nothing */ }

        // Assignment operator
        void operator=(const CCPACSFuselageProfile& ) { /* Do nothing */ }

        // Checks is two point are the same, or nearly the same.
        bool checkSamePoints(gp_Pnt pointA, gp_Pnt pointB);

    private:
        std::string               ProfileXPath;   /**< The XPath to this profile in cpacs */
        std::string               name;			  /**< The Name of the profile */
        std::string				  description;	  /**< The description of the profile */
        std::string				  uid;			  /**< The UID of this profile */
        CCPACSCoordinateContainer coordinates;    /**< Coordinates of a fuselage profile element */
        bool                      invalidated;    /**< Flag if element is invalid */
        TopoDS_Wire               wireOriginal;   /**< Original fuselage profile wire */
        TopoDS_Wire               wireClosed;     /**< Forced closed fuselage profile wire */
        double                    wireLength;     /**< Length of fuselage profile wire */

    };

} // end namespace tigl

#endif // CCPACSFUSELAGEPROFILE_H
