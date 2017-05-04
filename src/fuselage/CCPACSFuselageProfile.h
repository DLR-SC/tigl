/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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

#include "generated/CPACSProfileGeometry.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "CTiglArcLengthReparameterization.h"
#include "TopoDS_Wire.hxx"

#include <gp_Pnt.hxx>

#include <vector>
#include <string>
#include <memory>

namespace tigl
{

class CTiglPoint;
class ITiglWireAlgorithm;

class CCPACSFuselageProfile : public generated::CPACSProfileGeometry
{
public:
    // Constructor
    TIGL_EXPORT CCPACSFuselageProfile(CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSFuselageProfile();

    // Read CPACS fuselage profile file
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

    // Write CPACS fuselage profile file
    TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

    // Returns the name of the fuselage profile
    TIGL_EXPORT const int GetNumPoints() const;

    // Returns the flag for the mirror symmetry with respect to the x-z-plane in the fuselage profile
    TIGL_EXPORT bool GetMirrorSymmetry() const;

    // Invalidates internal fuselage profile state
    TIGL_EXPORT void Invalidate();

    // Returns the fuselage profile wire. The wire is already transformed by the
    // fuselage profile element transformation.
    TIGL_EXPORT TopoDS_Wire GetWire(bool forceClosed = false);

    // Gets a point on the fuselage profile wire in dependence of a parameter zeta with
    // 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the wire start point,
    // for zeta = 1.0 the last wire point.
    TIGL_EXPORT gp_Pnt GetPoint(double zeta);

    // Returns the profile points as read from TIXI.
    TIGL_EXPORT std::vector<CTiglPoint*> GetCoordinateContainer();

    // Returns the "diameter" line as a wire
    TIGL_EXPORT TopoDS_Wire GetDiameterWire();

protected:
    // Cleanup routine
    void Cleanup();

    // Update the internal state, i.g. recalculates wire
    void Update();

    // Transforms a point by the fuselage profile transformation
    gp_Pnt TransformPoint(const gp_Pnt& aPoint) const;

    // Builds the fuselage profile wires. The wires are already transformed by the
    // fuselage profile transformation.
    void BuildWires();

    // Helper function to determine the "diameter" (the wing profile chord line equivalent) 
    // which is defined as the line intersecting Point1 and Point2
    // 
    // In the case of a non-mirror symmetric profile we have
    // Point1: The middle point between first and last point of the profile point list
    // Point2: The profile point list point with the largest distance to Point1
    // 
    // In the case of a mirror symmetric profile we have
    // Point1: First point in the profile point list
    // Point2: Last point in the profile point list
    void BuildDiameterPoints();

private:
    // Checks is two point are the same, or nearly the same.
    bool checkSamePoints(gp_Pnt pointA, gp_Pnt pointB);

private:
    bool                                mirrorSymmetry; /**< Mirror symmetry with repect to the x-z plane */
    std::vector<CTiglPoint>             coordinates;    /**< Coordinates of a fuselage profile element */
    bool                                invalidated;    /**< Flag if element is invalid */
    TopoDS_Wire                         wireOriginal;   /**< Original fuselage profile wire */
    TopoDS_Wire                         wireClosed;     /**< Forced closed fuselage profile wire */
    double                              wireLength;     /**< Length of fuselage profile wire */
    unique_ptr<ITiglWireAlgorithm> profileWireAlgo;
    gp_Pnt                              startDiameterPoint;
    gp_Pnt                              endDiameterPoint;
    CTiglArcLengthReparameterization reparOriginal;

};

} // end namespace tigl

#endif // CCPACSFUSELAGEPROFILE_H

