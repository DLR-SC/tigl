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
* @brief  Implementation of CPACS wing profile handling routines.
*
* The wing profile is defined by a list of points. The list starts at the
* trailing edge moves around the leading edge and goes back to the trailing
* edge. Currently it is assumed, that profile moves alomg the x,z plane.
*/

#ifndef CCPACSWINGPROFILE_H
#define CCPACSWINGPROFILE_H

#include <memory>
#include "generated/CPACSProfileGeometry.h"
#include "generated/Optional.hpp"
#include "tigl_internal.h"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Edge.hxx"
#include "PTiglWingProfileAlgo.h"

#include <gp_Pnt.hxx>
#include <Handle_Geom2d_TrimmedCurve.hxx>


namespace tigl 
{
class CCPACSWingProfilePointList;

class CCPACSWingProfile : public generated::CPACSProfileGeometry
{

public:
    // Algo
    TIGL_EXPORT CCPACSWingProfile();

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingProfile();

    // Read CPACS wing profile file
    TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;


    // Returns the name of the wing profile
    TIGL_EXPORT const std::string& GetName(void) const;

    // Returns the description of the wing profile
    TIGL_EXPORT const std::string& GetDescription(void) const;

    // Returns the uid of the wing profile
    TIGL_EXPORT const std::string& GetUID(void) const;

    // Invalidates internal wing profile state
    TIGL_EXPORT void Invalidate();

    // Returns the wing profile wire, splitted at the leading edge
    TIGL_EXPORT TopoDS_Wire GetSplitWire();
    
    // Returns the wing profile wire
    TIGL_EXPORT TopoDS_Wire GetWire();
    
    // Returns the wing profile wire with flat trailing edge
    TIGL_EXPORT TopoDS_Wire GetWireOpened();

    // Returns the wing profile wire with pointed trailing edge
    TIGL_EXPORT TopoDS_Wire GetWireClosed();

    // Returns ths wing upper and lower profile wire
    TIGL_EXPORT TopoDS_Edge GetUpperWire();
    TIGL_EXPORT TopoDS_Edge GetLowerWire();
    TIGL_EXPORT TopoDS_Edge GetTrailingEdge();

    // Returns the wing upper and lower profile wire for opened profile
    TIGL_EXPORT TopoDS_Edge GetUpperWireOpened();
    TIGL_EXPORT TopoDS_Edge GetLowerWireOpened();

    // Returns the wing upper and lower profile wire for closed profile
    TIGL_EXPORT TopoDS_Edge GetUpperWireClosed();
    TIGL_EXPORT TopoDS_Edge GetLowerWireClosed();

    // Returns the trailing edge for the opened profile
    TIGL_EXPORT TopoDS_Edge GetTrailingEdgeOpened();

    // Returns the leading edge point of the wing profile wire. The leading edge point
    // is already transformed by the wing profile element transformation.
    TIGL_EXPORT gp_Pnt GetLEPoint();

    // Returns the trailing edge point of the wing profile wire. The trailing edge point
    // is already transformed by the wing profile element transformation.
    TIGL_EXPORT gp_Pnt GetTEPoint();

    // Returns the chord line as a wire
    TIGL_EXPORT TopoDS_Wire GetChordLineWire();

    // Returns a point on the chord line between leading and trailing
    // edge as function of parameter xsi, which ranges from 0.0 to 1.0.
    // For xsi = 0.0 chord point is equal to leading edge, for xsi = 1.0
    // chord point is equal to trailing edge.
    TIGL_EXPORT gp_Pnt GetChordPoint(double xsi);

    // Returns a point on the upper wing profile as function of
    // parameter xsi, which ranges from 0.0 to 1.0.
    // For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
    // point is equal to trailing edge.
    TIGL_EXPORT gp_Pnt GetUpperPoint(double xsi);

    // Returns a point on the lower wing profile as function of
    // parameter xsi, which ranges from 0.0 to 1.0.
    // For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
    // point is equal to trailing edge.
    TIGL_EXPORT gp_Pnt GetLowerPoint(double xsi);

    // get profile algorithm type
    TIGL_EXPORT ITiglWingProfileAlgo* GetProfileAlgo();
    TIGL_EXPORT const ITiglWingProfileAlgo* GetProfileAlgo() const;

protected:
    // Cleanup routine
    void Cleanup();

    // Update the internal state, i.g. recalculates wire and le, te points
    void Update();

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
    CCPACSWingProfile(const CCPACSWingProfile& );

    // Assignment operator
    void operator=(const CCPACSWingProfile& );

private:
    std::string               name;           /**< CPACS wing profile name */
    std::string               description;    /**< CPACS wing profile description */
    std::string               uid;            /**< CPACS wing profile UID */
    bool                                        invalidated;    /**< Flag if element is invalid */
    ITiglWingProfileAlgo*                       profileAlgo; // points to the current profile algo (non-owning)
    std::unique_ptr<CCPACSWingProfilePointList> pointListAlgo; // is created in case the wing profile alg is a point list, otherwise cst2d constructed in the base class is used

}; // class CCPACSWingProfile

} // end namespace tigl

#endif // CCPACSWINGPROFILE_H
