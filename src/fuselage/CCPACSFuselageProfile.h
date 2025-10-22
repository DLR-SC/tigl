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
    TIGL_EXPORT CCPACSFuselageProfile(CCPACSFuselageProfiles* parent, CTiglUIDManager* uidMgr);

    // Destructor
    TIGL_EXPORT ~CCPACSFuselageProfile() override;

    // Read CPACS fuselage profile file
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

    // Returns the name of the fuselage profile
    TIGL_EXPORT const int GetNumPoints() const;

    // Returns the flag for the mirror symmetry with respect to the x-z-plane in the fuselage profile
    TIGL_EXPORT bool GetMirrorSymmetry() const;

    // Returns the fuselage profile wire. The profile is not transformed.
    TIGL_EXPORT TopoDS_Wire GetWire(bool forceClosed = false) const;

    // Gets a point on the fuselage profile wire in dependence of a parameter zeta with
    // 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the wire start point,
    // for zeta = 1.0 the last wire point.
    TIGL_EXPORT gp_Pnt GetPoint(double zeta) const;

    // Returns the "diameter" line as a wire
    // (The diameter is the line between the start point and the most distant point)
    TIGL_EXPORT TopoDS_Wire GetDiameterWire() const;

    // Returns the width of the profile (y distance).
    TIGL_EXPORT double GetWidth() const;

    // Returns the height of the profile, (z distance).
    TIGL_EXPORT double GetHeight() const;

private:
    struct WireCache {
        TopoDS_Wire original;
        TopoDS_Wire closed;
    };

    struct DiameterPointsCache {
        gp_Pnt start;
        gp_Pnt end;
    };

    struct SizeCache {
        double width;
        double height;
    };

    // Transforms a point by the fuselage profile transformation //todo is it working ? where is trh transformation
    gp_Pnt TransformPoint(const gp_Pnt& aPoint) const;

    // Builds the fuselage profile wires.
    void BuildWires(WireCache& cache) const;

    //Builds the fuselage profile wires from point list
    void BuildWiresPointList(WireCache& cache) const;

    //Builds the fuselage profile wires from height to width ratio and corner radius
    void BuildWiresRectangle(WireCache& cache) const;

    //Builds the fuselage profile wires from lowerHeightFraction and exponents m,n for lower and upper semi-ellipse
    void BuildWiresSuperEllipse(WireCache& cache) const;

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

    void BuildSize(SizeCache& cache) const;

private:
    // Invalidates internal wing profile state
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

private:
    bool mirrorSymmetry; /**< Mirror symmetry with respect to the x-z plane */
    Cache<WireCache, CCPACSFuselageProfile> wireCache; /**< Original and force closed fuselage profile wire */
    Cache<DiameterPointsCache, CCPACSFuselageProfile> diameterPointsCache;
    Cache<SizeCache, CCPACSFuselageProfile> sizeCache;
    std::unique_ptr<ITiglWireAlgorithm> profileWireAlgo;
};

} // end namespace tigl

#endif // CCPACSFUSELAGEPROFILE_H
