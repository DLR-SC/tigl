/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-15 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief  Implementation of CPACS hull handling routines.
*/

#pragma once

#include "generated/CPACSHull.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CCPACSConfiguration.h"

#include <BRepBuilderAPI_MakeWire.hxx>

namespace tigl
{

class CCPACSHull : public generated::CPACSHull, public CTiglRelativelyPositionedComponent
{
public:
    // Constructor
    TIGL_EXPORT explicit CCPACSHull(CCPACSHulls* parent, CTiglUIDManager* uidMgr);

    // Get the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    // Get the default uID
    TIGL_EXPORT std::string GetDefaultedUID() const override;

    // Get the component type: TIGL_COMPONENT_FUSELAGE_TANK_HULL
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;
    // Get the component intent: TIGL_INTENT_PHYSICAL
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

    // Get number of sections
    TIGL_EXPORT int GetSectionCount() const;
    // Get a specific section
    TIGL_EXPORT CCPACSFuselageSection& GetSection(int index) const;
    // Get a section face
    TIGL_EXPORT TopoDS_Shape GetSectionFace(const std::string section_uid) const;

    // Get number of segments
    TIGL_EXPORT int GetSegmentCount() const;
    // Get a specific segment via its index
    TIGL_EXPORT CCPACSFuselageSegment& GetSegment(const int index);
    TIGL_EXPORT const CCPACSFuselageSegment& GetSegment(const int index) const;
    // Get a specific segment via its uID
    TIGL_EXPORT CCPACSFuselageSegment& GetSegment(std::string uid);

    // Get the volume of the hull
    TIGL_EXPORT double GetVolume();

    // Get the surface area of the hull
    TIGL_EXPORT double GetSurfaceArea();

    // Get the circumference of the hull
    TIGL_EXPORT double GetCircumference(int segmentIndex, double eta);

    // Get a point on the given hull segment in dependence of a parameters eta and zeta with
    // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
    // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
    // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
    TIGL_EXPORT gp_Pnt GetPoint(int segmentIndex, double eta, double zeta);
    // Set the getPointBehavior to asParameterOnSurface or onLinearLoft
    TIGL_EXPORT void SetGetPointBehavior(TiglGetPointBehavior behavior = asParameterOnSurface);
    // Get the getPointBehavior
    TIGL_EXPORT TiglGetPointBehavior GetGetPointBehavior() const;

    // Get the guide curve segment (partial guide curve) with a given uID
    TIGL_EXPORT CCPACSGuideCurve& GetGuideCurveSegment(std::string uid);
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurveSegment(std::string uid) const;
    // Returns all guide curve points
    TIGL_EXPORT std::vector<gp_Pnt> GetGuideCurvePoints() const;

protected:
    // Build the loft
    PNamedShape BuildLoft() const override;

    // Set the face traits
    void SetFaceTraits(PNamedShape loft) const;

private:
    // Get short name for loft
    std::string GetShortShapeName() const;

    TiglGetPointBehavior getPointBehavior{asParameterOnSurface};
    void IsotensoidContour(double rCyl, double rPolarOpening, int nodeNumber, std::vector<double>& x,
                           std::vector<double>& r) const;

    void BuildShapeFromSegments(TopoDS_Shape& loftShape) const;
    void BuildShapeFromSimpleParameters(TopoDS_Shape& loftShape) const;

    void BuildTankWire(std::vector<TopoDS_Edge>& edges, BRepBuilderAPI_MakeWire& wire) const;
    void BuildTankWireEllipsoid(BRepBuilderAPI_MakeWire& wire) const;
    void BuildTankWireTorispherical(BRepBuilderAPI_MakeWire& wire) const;
    void BuildTankWireIsotensoid(BRepBuilderAPI_MakeWire& wire) const;
};

} // namespace tigl
