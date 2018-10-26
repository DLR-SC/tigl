/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-12 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>

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

#include "generated/UniquePtr.h"
#include "tixi.h"
#include "tigl_internal.h"
#include "ITiglWireAlgorithm.h"
#include "ITiglWingProfileAlgo.h"
#include "PTiglWingProfileAlgo.h"
#include "Geom_TrimmedCurve.hxx"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Edge.hxx"
#include "CCPACSPointListXYZ.h"
#include "Cache.h"

namespace tigl
{

class CCPACSWingProfile;

class CTiglWingProfilePointList : public ITiglWingProfileAlgo
{

public:
    // Constructor
    TIGL_EXPORT CTiglWingProfilePointList(const CCPACSWingProfile& profile, const CCPACSPointListXYZ& cpacsPointlist);

    TIGL_EXPORT void Invalidate() OVERRIDE;

    // Returns the profile points as read from TIXI.
    TIGL_EXPORT const std::vector<CTiglPoint>& GetSamplePoints() const OVERRIDE;

    // get upper wing profile wire
    TIGL_EXPORT const TopoDS_Edge& GetUpperWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const OVERRIDE;

    // get lower wing profile wire
    TIGL_EXPORT const TopoDS_Edge& GetLowerWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const OVERRIDE;

    // get the upper and lower wing profile combined into one edge
    TIGL_EXPORT const TopoDS_Edge& GetUpperLowerWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const OVERRIDE;

    // get trailing edge if existing in definition
    TIGL_EXPORT const TopoDS_Edge& GetTrailingEdge(TiglShapeModifier mod = UNMODIFIED_SHAPE) const OVERRIDE;

    // get leading edge point();
    TIGL_EXPORT const gp_Pnt& GetLEPoint() const OVERRIDE;

    // get trailing edge point();
    TIGL_EXPORT const gp_Pnt& GetTEPoint() const OVERRIDE;

    // Checks, whether the trailing edge is blunt or
    // not by comparing first and last point.
    TIGL_EXPORT bool HasBluntTE() const OVERRIDE;

protected:
    struct WireCache {
        bool        profileIsClosed;        ///< stores whether the defined profile is closed or has a trailing edge
        TopoDS_Edge upperWireOpened;        ///< wire of upper wing profile from open profile
        TopoDS_Edge lowerWireOpened;        ///< wire of lower wing profile from open profile
        TopoDS_Edge upperWireClosed;        ///< wire of the upper wing profile
        TopoDS_Edge lowerWireClosed;        ///< wire of the lower wing profile
        TopoDS_Edge upperLowerEdgeOpened;   ///< edge of the upper and lower wing profile combined
        TopoDS_Edge upperLowerEdgeClosed;
        TopoDS_Edge trailingEdgeOpened;     ///< wire of the trailing edge
        TopoDS_Edge trailingEdgeClosed;     ///< always null, but required for consistency
        gp_Pnt      lePoint;                ///< Leading edge point
        gp_Pnt      tePoint;                ///< Trailing edge point
    };

    // Builds the wing profile wires.
    void BuildWires(WireCache& cache) const;

    // Builds leading and trailing edge points of the wing profile wire.
    void BuildLETEPoints(WireCache& cache) const;

    // Helper method for closing profile at trailing edge
    void closeProfilePoints(ITiglWireAlgorithm::CPointContainer& points) const;

    // Helper method for opening profile at trailing edge
    void openProfilePoints(ITiglWireAlgorithm::CPointContainer& points) const;

private:
    // Copy constructor
    CTiglWingProfilePointList(const CTiglWingProfilePointList&);

    // Assignment operator
    void operator=(const CTiglWingProfilePointList&);

    // Helper method for trimming upper an lower curve
    void trimUpperLowerCurve(WireCache& cache, Handle(Geom_TrimmedCurve) lowerCurve, Handle(Geom_TrimmedCurve) upperCurve, Handle_Geom_Curve curve) const;

private:
    // constant for opening profile
    static const double       c_trailingEdgeRelGap;
    // constant blending distance for opening/closing trailing edge
    static const double       c_blendingDistance;

    const std::vector<CTiglPoint>& coordinates;    /**< Coordinates of a wing profile element */
    unique_ptr<ITiglWireAlgorithm> profileWireAlgo;/**< Pointer to wire algorithm (e.g. CTiglInterpolateBsplineWire) */
    const CCPACSWingProfile&            profileRef;     /**< Reference to the wing profile */

    Cache<WireCache, CTiglWingProfilePointList> wireCache;
};

} // end namespace tigl

#endif // CCPACSWINGPROFILEPOINTLIST_H

