/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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

#include "generated/UniquePtr.h"
#include "generated/CPACSProfileGeometry.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "TopoDS_Wire.hxx"
#include "Cache.h"

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

    // Destructor
    TIGL_EXPORT ~CCPACSFuselageProfile() OVERRIDE;

    // Read CPACS fuselage profile file
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) OVERRIDE;

    // Returns the name of the fuselage profile
    TIGL_EXPORT const int GetNumPoints() const;

    // Returns the flag for the mirror symmetry with respect to the x-z-plane in the fuselage profile
    TIGL_EXPORT bool GetMirrorSymmetry() const;

    // Invalidates internal fuselage profile state
    TIGL_EXPORT void Invalidate();

    // Returns the fuselage profile wire. The wire is already transformed by the
    // fuselage profile element transformation.
    TIGL_EXPORT TopoDS_Wire GetWire(bool forceClosed = false) const;

    // Gets a point on the fuselage profile wire in dependence of a parameter zeta with
    // 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the wire start point,
    // for zeta = 1.0 the last wire point.
    TIGL_EXPORT gp_Pnt GetPoint(double zeta) const;

    // Returns the "diameter" line as a wire
    TIGL_EXPORT TopoDS_Wire GetDiameterWire() const;

private:
    struct WireCache {
        TopoDS_Wire original;
        TopoDS_Wire closed;
    };

    struct DiameterPointsCache {
        gp_Pnt start;
        gp_Pnt end;
    };

    // Transforms a point by the fuselage profile transformation
    gp_Pnt TransformPoint(const gp_Pnt& aPoint) const;

    // Builds the fuselage profile wires. The wires are already transformed by the
    // fuselage profile transformation.
    void BuildWires(WireCache& cache) const;

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
    void BuildDiameterPoints(DiameterPointsCache& cache) const;

private:
    // Checks is two point are the same, or nearly the same.
    bool checkSamePoints(gp_Pnt pointA, gp_Pnt pointB) const;

private:
    bool mirrorSymmetry; /**< Mirror symmetry with repect to the x-z plane */
    Cache<WireCache, CCPACSFuselageProfile> wireCache;   /**< Original and force closed fuselage profile wire */
    Cache<DiameterPointsCache, CCPACSFuselageProfile> diameterPointsCache;
    unique_ptr<ITiglWireAlgorithm> profileWireAlgo;

};

} // end namespace tigl

#endif // CCPACSFUSELAGEPROFILE_H

