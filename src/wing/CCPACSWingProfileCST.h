/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-17 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS wing profile as a CST profile
*
* The wing profile is defined by the (C)lass function / (S)hape function (T)ransformation
* geometry representation method.
*/

#include "generated/CPACSCst2D.h"
#include "tigl_internal.h"
#include "ITiglWingProfileAlgo.h"
#include "Cache.h"

#include <vector>
#include <TopoDS_Edge.hxx>

#ifndef CCPACSWINGPROFILECST_H
#define CCPACSWINGPROFILECST_H

namespace tigl
{
class CCPACSWingProfile;

class CCPACSWingProfileCST : public generated::CPACSCst2D, public ITiglWingProfileAlgo
{
public:
    // Constructor
    TIGL_EXPORT CCPACSWingProfileCST();

    // Update of wire points ...
    TIGL_EXPORT void Invalidate() OVERRIDE;

    // Returns the profile points as read from TIXI.
    TIGL_EXPORT const std::vector<CTiglPoint>& GetSamplePoints() const OVERRIDE; // TODO: why do we need those anyway, they just return an empty vector?

    // get upper wing profile wire
    TIGL_EXPORT const TopoDS_Edge& GetUpperWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const OVERRIDE;

    // get lower wing profile wire
    TIGL_EXPORT const TopoDS_Edge& GetLowerWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const OVERRIDE;

    // get trailing edge
    TIGL_EXPORT const TopoDS_Edge& GetTrailingEdge(TiglShapeModifier mod = UNMODIFIED_SHAPE) const OVERRIDE;

    // gets the upper and lower wing profile into on edge
    TIGL_EXPORT const TopoDS_Edge& GetUpperLowerWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const OVERRIDE;

    // get leading edge point();
    TIGL_EXPORT const gp_Pnt & GetLEPoint() const OVERRIDE;

    // get trailing edge point();
    TIGL_EXPORT const gp_Pnt & GetTEPoint() const OVERRIDE;

    // CST profiles have always sharp trailing edges
    TIGL_EXPORT bool HasBluntTE() const OVERRIDE;

private:
    struct WireCache {
        TopoDS_Edge               upperWireOpened;      /**< wire of the upper wing profile */
        TopoDS_Edge               lowerWireOpened;      /**< wire of the lower wing profile */
        TopoDS_Edge               upperWireClosed;      /**< wire of the upper wing profile */
        TopoDS_Edge               lowerWireClosed;      /**< wire of the lower wing profile */
        TopoDS_Edge               upperLowerEdgeOpened; /**< edge consisting of upper and lower wing profile */
        TopoDS_Edge               upperLowerEdgeClosed; /**< edge consisting of upper and lower wing profile */ 
        TopoDS_Edge               trailingEdgeOpened;   /**< edge of the trailing edge */
        TopoDS_Edge               trailingEdgeClosed;   /**< edge of the trailing edge */
        gp_Pnt                    lePoint;              /**< Leading edge point */
        gp_Pnt                    tePoint;              /**< Trailing edge point */
    };

    // Builds the wing profile wires.
    void BuildWires(WireCache& cache) const;

private:
    Cache<WireCache, CCPACSWingProfileCST> wireCache;
};

} // end namespace tigl

#endif // CCPACSWINGPROFILECST_H
