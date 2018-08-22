/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
#include "generated/UniquePtr.h"
#include "tigl_internal.h"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Edge.hxx"
#include "PTiglWingProfileAlgo.h"
#include "Cache.h"

#include <gp_Pnt.hxx>

#include <Geom2d_TrimmedCurve.hxx>

namespace tigl 
{
class CTiglWingProfilePointList;

class CCPACSWingProfile : public generated::CPACSProfileGeometry
{

public:
    TIGL_EXPORT CCPACSWingProfile(CTiglUIDManager* uidMgr);

    TIGL_EXPORT ~CCPACSWingProfile() OVERRIDE;

    // Read CPACS wing profile file
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) OVERRIDE;

    // Returns whether the profile is a rotor profile
    TIGL_EXPORT bool IsRotorProfile() const;

    // Invalidates internal wing profile state
    TIGL_EXPORT void Invalidate();

    // Returns the wing profile wire, splitted at the leading edge
    TIGL_EXPORT TopoDS_Wire GetSplitWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const;

    // Returns the wing profile wire
    TIGL_EXPORT TopoDS_Wire GetWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const;

    // Returns ths wing upper and lower profile wire
    TIGL_EXPORT TopoDS_Edge GetUpperWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const;
    TIGL_EXPORT TopoDS_Edge GetLowerWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const;
    TIGL_EXPORT TopoDS_Edge GetTrailingEdge(TiglShapeModifier mod = UNMODIFIED_SHAPE) const;

    // Returns the leading edge point of the wing profile wire. The leading edge point
    // is already transformed by the wing profile element transformation.
    TIGL_EXPORT gp_Pnt GetLEPoint() const;

    // Returns the trailing edge point of the wing profile wire. The trailing edge point
    // is already transformed by the wing profile element transformation.
    TIGL_EXPORT gp_Pnt GetTEPoint() const;

    // Returns the chord line as a wire
    TIGL_EXPORT TopoDS_Wire GetChordLineWire() const;

    // Returns a point on the chord line between leading and trailing
    // edge as function of parameter xsi, which ranges from 0.0 to 1.0.
    // For xsi = 0.0 chord point is equal to leading edge, for xsi = 1.0
    // chord point is equal to trailing edge.
    TIGL_EXPORT gp_Pnt GetChordPoint(double xsi) const;

    // Returns a point on the upper wing profile as function of
    // parameter xsi, which ranges from 0.0 to 1.0.
    // For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
    // point is equal to trailing edge.
    TIGL_EXPORT gp_Pnt GetUpperPoint(double xsi) const;

    // Returns a point on the lower wing profile as function of
    // parameter xsi, which ranges from 0.0 to 1.0.
    // For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
    // point is equal to trailing edge.
    TIGL_EXPORT gp_Pnt GetLowerPoint(double xsi) const;

    // get profile algorithm type
    TIGL_EXPORT ITiglWingProfileAlgo* GetProfileAlgo();
    TIGL_EXPORT const ITiglWingProfileAlgo* GetProfileAlgo() const;

    // Checks, whether the trailing edge is blunt or not
    TIGL_EXPORT bool HasBluntTE() const;

protected:
    // Cleanup routine
    void Cleanup();

    // Returns an upper or lower point on the wing profile in
    // dependence of parameter xsi, which ranges from 0.0 to 1.0.
    // For xsi = 0.0 point is equal to leading edge, for xsi = 1.0
    // point is equal to trailing edge. If fromUpper is true, a point
    // on the upper profile is returned, otherwise from the lower.
    gp_Pnt GetPoint(double xsi, bool fromUpper) const;

    // Helper function to determine the chord line between leading and trailing edge in the profile plane
    Handle(Geom2d_TrimmedCurve) GetChordLine() const;

    void buildPointListAlgo(unique_ptr<CTiglWingProfilePointList>& cache) const;

private:
    // Copy constructor
    CCPACSWingProfile(const CCPACSWingProfile& );

    // Assignment operator
    void operator=(const CCPACSWingProfile& );

private:
    bool                                  isRotorProfile; /**< Indicates if this profile is a rotor profile */
    Cache<unique_ptr<CTiglWingProfilePointList>, CCPACSWingProfile> pointListAlgo;  // is created in case the wing profile alg is a point list, otherwise cst2d constructed in the base class is used

}; // class CCPACSWingProfile

} // end namespace tigl

#endif // CCPACSWINGPROFILE_H
