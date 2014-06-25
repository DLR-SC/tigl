/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-12 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS wing profile as a point list.
*
* The wing profile is defined by a list of points. The list starts at the
* trailing edge moves around the leading edge and goes back to the trailing
* edge. Currently it is assumed, that profile moves alomg the x,z plane.
*/

#ifndef CCPACSWINGPROFILEPOINTLIST_H
#define CCPACSWINGPROFILEPOINTLIST_H

#include <vector>
#include <string>

#include "tixi.h"
#include "tigl_internal.h"
#include "ITiglWingProfileAlgo.h"
#include "PTiglWingProfileAlgo.h"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Edge.hxx"


namespace tigl
{

class CCPACSWingProfile;
class ITiglWireAlgorithm;
typedef ITiglWireAlgorithm* WireAlgoPointer;

class CCPACSWingProfilePointList : public ITiglWingProfileAlgo
{

private:
    // Typedef for a container to store the coordinates of a wing profile element.
    typedef std::vector<CTiglPoint*> CCPACSCoordinateContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSWingProfilePointList(const CCPACSWingProfile& profile, const std::string& cpacsPath);

    // Destructor
    TIGL_EXPORT ~CCPACSWingProfilePointList(void);

    TIGL_EXPORT static std::string CPACSID();

    // Cleanup routine
    TIGL_EXPORT void Cleanup(void);

    // Update interna
    TIGL_EXPORT void Update(void);

    // Read CPACS wing profile file
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Returns the profile points as read from TIXI.
    TIGL_EXPORT std::vector<CTiglPoint*> GetSamplePoints() const;

    // get profiles CPACS XML path
    TIGL_EXPORT const std::string& GetProfileDataXPath() const;

    // get upper wing profile wire
    TIGL_EXPORT const TopoDS_Edge& GetUpperWire() const;

    // get lower wing profile wire
    TIGL_EXPORT const TopoDS_Edge& GetLowerWire() const;

    // get the upper and lower wing profile combined into one edge
    TIGL_EXPORT const TopoDS_Edge & GetUpperLowerWire() const;

    // get trailing edge
    TIGL_EXPORT const TopoDS_Edge& GetTrailingEdge() const;

    // get leading edge point();
    TIGL_EXPORT const gp_Pnt& GetLEPoint() const;

    // get trailing edge point();
    TIGL_EXPORT const gp_Pnt& GetTEPoint() const;

protected:
    // Builds the wing profile wires.
    void BuildWires();

    // Builds leading and trailing edge points of the wing profile wire.
    void BuildLETEPoints(void);

private:
    // Copy constructor
    CCPACSWingProfilePointList(const CCPACSWingProfilePointList&);

    // Assignment operator
    void operator=(const CCPACSWingProfilePointList&);

private:
    CCPACSCoordinateContainer coordinates;    /**< Coordinates of a wing profile element */
    WireAlgoPointer           profileWireAlgo;/**< Pointer to wire algorithm (e.g. CTiglInterpolateBsplineWire) */
    const CCPACSWingProfile&  profileRef;     /**< Reference to the wing profile */


    std::string               ProfileDataXPath; /**< CPACS path to profile data (pointList or cst2D) */
    TopoDS_Edge               upperWire;        /**< wire of the upper wing profile */
    TopoDS_Edge               lowerWire;        /**< wire of the lower wing profile */
    TopoDS_Edge               upperLowerEdge;   /**< edge of the upper and lower wing profile combined */
    TopoDS_Edge               trailingEdge;     /**< wire of the trailing edge */
    gp_Pnt                    lePoint;          /**< Leading edge point */
    gp_Pnt                    tePoint;          /**< Trailing edge point */
};

} // end namespace tigl

#endif // CCPACSWINGPROFILEPOINTLIST_H

