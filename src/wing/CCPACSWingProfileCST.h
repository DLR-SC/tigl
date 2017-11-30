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

    // Destructor
    TIGL_EXPORT virtual ~CCPACSWingProfileCST();

    // Cleanup routine
    TIGL_EXPORT void Cleanup();

    // Update of wire points ...
    TIGL_EXPORT virtual void Update() OVERRIDE;

    // Returns the profile points as read from TIXI.
    TIGL_EXPORT virtual std::vector<CTiglPoint>& GetSamplePoints() OVERRIDE; // TODO: why do we need those anyway, they just return an empty vector?
    TIGL_EXPORT virtual const std::vector<CTiglPoint>& GetSamplePoints() const OVERRIDE; // TODO: why do we need those anyway, they just return an empty vector?

    // get upper wing profile wire
    TIGL_EXPORT virtual const TopoDS_Edge & GetUpperWire() const OVERRIDE;

    // get lower wing profile wire
    TIGL_EXPORT virtual const TopoDS_Edge & GetLowerWire() const OVERRIDE;

    // get trailing edge
    TIGL_EXPORT const TopoDS_Edge & GetTrailingEdge() const;

    // get trailing edge for opened profile
    TIGL_EXPORT virtual const TopoDS_Edge & GetTrailingEdgeOpened() const OVERRIDE;

    // gets the upper and lower wing profile into on edge
    TIGL_EXPORT virtual const TopoDS_Edge & GetUpperLowerWire() const OVERRIDE;

    // Getter for upper wire of closed profile
    TIGL_EXPORT const TopoDS_Edge & GetUpperWireClosed() const;

    // Getter for lower wire of closed profile
    TIGL_EXPORT const TopoDS_Edge & GetLowerWireClosed() const;

    // Getter for upper wire of opened profile
    TIGL_EXPORT const TopoDS_Edge & GetUpperWireOpened() const;

    // Getter for lower wire of opened profile
    TIGL_EXPORT const TopoDS_Edge & GetLowerWireOpened() const;

    // get leading edge point();
    TIGL_EXPORT virtual const gp_Pnt & GetLEPoint() const OVERRIDE;

    // get trailing edge point();
    TIGL_EXPORT virtual const gp_Pnt & GetTEPoint() const OVERRIDE;

    // CST profiles have always sharp trailing edges
    TIGL_EXPORT virtual bool HasBluntTE() const OVERRIDE { return false;}

protected:
    // Builds the wing profile wires.
    void BuildWires();

private:
    TopoDS_Edge               upperWire;          /**< wire of the upper wing profile */
    TopoDS_Edge               lowerWire;          /**< wire of the lower wing profile */
    TopoDS_Edge               upperLowerEdge;     /**< edge consisting of upper and lower wing profile */
    TopoDS_Edge               trailingEdge;       /**< wire of the trailing edge */
    gp_Pnt                    lePoint;            /**< Leading edge point */
    gp_Pnt                    tePoint;            /**< Trailing edge point */
};

} // end namespace tigl

#endif // CCPACSWINGPROFILECST_H
