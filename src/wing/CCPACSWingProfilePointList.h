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
#include "ITiglWireAlgorithm.h"
#include "ITiglWingProfileAlgo.h"
#include "PTiglWingProfileAlgo.h"
#include "Geom_TrimmedCurve.hxx"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Edge.hxx"
#include "CCPACSPointListXYZ.h"


namespace tigl
{

class CCPACSWingProfile;

class CCPACSWingProfilePointList : public ITiglWingProfileAlgo // TODO: this is NOT a CPACS class, rename to e.g. WingProfilePointListAlgo
{

public:
    // Constructor
    TIGL_EXPORT CCPACSWingProfilePointList(const CCPACSWingProfile& profile, const CCPACSPointListXYZ& cpacsPointlist, const std::string& xpath);

    DEPRECATED TIGL_EXPORT static std::string CPACSID();

    // Cleanup routine
    TIGL_EXPORT void Cleanup();

    // Update interna
    TIGL_EXPORT void Update();

    // Returns the profile points as read from TIXI.
    TIGL_EXPORT virtual const std::vector<CTiglPoint>& GetSamplePoints() const override;

    // get profiles CPACS XML path
    DEPRECATED TIGL_EXPORT const std::string& GetProfileDataXPath() const;

    // get upper wing profile wire
    TIGL_EXPORT const TopoDS_Edge& GetUpperWire() const;

    // get lower wing profile wire
    TIGL_EXPORT const TopoDS_Edge& GetLowerWire() const;

    // get the upper and lower wing profile combined into one edge
    TIGL_EXPORT const TopoDS_Edge & GetUpperLowerWire() const;

    // get trailing edge if existing in definition
    TIGL_EXPORT const TopoDS_Edge& GetTrailingEdge() const;

    // returns the trailing edge for the opened wing profile
    TIGL_EXPORT const TopoDS_Edge& GetTrailingEdgeOpened() const;

    // getter for upper wire of closed profile
    TIGL_EXPORT const TopoDS_Edge& GetUpperWireClosed() const;

    // getter for lower wire of closed profile
    TIGL_EXPORT const TopoDS_Edge& GetLowerWireClosed() const;

    // getter for upper wire of opened profile
    TIGL_EXPORT const TopoDS_Edge& GetUpperWireOpened() const;

    // getter for lower wire of opened profile
    TIGL_EXPORT const TopoDS_Edge& GetLowerWireOpened() const;

    // get leading edge point();
    TIGL_EXPORT const gp_Pnt& GetLEPoint() const;

    // get trailing edge point();
    TIGL_EXPORT const gp_Pnt& GetTEPoint() const;

protected:
    // Builds the wing profile wires.
    void BuildWires();

    // Builds leading and trailing edge points of the wing profile wire.
    void BuildLETEPoints();

    // Helper method for closing profile at trailing edge
    void closeProfilePoints(ITiglWireAlgorithm::CPointContainer& points);

    // Helper method for opening profile at trailing edge
    void openProfilePoints(ITiglWireAlgorithm::CPointContainer& points);

private:
    // Copy constructor
    CCPACSWingProfilePointList(const CCPACSWingProfilePointList&);

    // Assignment operator
    void operator=(const CCPACSWingProfilePointList&);

    // Helper method for trimming upper an lower curve
    void trimUpperLowerCurve(Handle(Geom_TrimmedCurve) lowerCurve, Handle(Geom_TrimmedCurve) upperCurve, Handle_Geom_Curve curve);

private:
    // constant for opening profile
    static const double       c_trailingEdgeRelGap;
    // constant blending distance for opening/closing trailing edge
    static const double       c_blendingDistance;
    // stores whether the defined profile is closed or has a trailing edge
    bool                      profileIsClosed;

    std::vector<CTiglPoint>             coordinates;    /**< Coordinates of a wing profile element */
    std::unique_ptr<ITiglWireAlgorithm> profileWireAlgo;/**< Pointer to wire algorithm (e.g. CTiglInterpolateBsplineWire) */
    const CCPACSWingProfile&            profileRef;     /**< Reference to the wing profile */

    std::string                         ProfileDataXPath; /**< CPACS path to profile data (pointList or cst2D) */
    TopoDS_Edge                         upperWireOpened;  /**< wire of upper wing profile from open profile */
    TopoDS_Edge                         lowerWireOpened;  /**< wire of lower wing profile from open profile */
    TopoDS_Edge                         upperWireClosed;        /**< wire of the upper wing profile */
    TopoDS_Edge                         lowerWireClosed;        /**< wire of the lower wing profile */
    TopoDS_Edge                         upperLowerEdgeOpened;   /**< edge of the upper and lower wing profile combined */
    TopoDS_Edge                         upperLowerEdgeClosed;
    TopoDS_Edge                         trailingEdgeOpened;     /**< wire of the trailing edge */
    TopoDS_Edge                         trailingEdgeClosed;     /**< always null, but required for consistency */

    gp_Pnt                              lePoint;          /**< Leading edge point */
    gp_Pnt                              tePoint;          /**< Trailing edge point */
};

} // end namespace tigl

#endif // CCPACSWINGPROFILEPOINTLIST_H

