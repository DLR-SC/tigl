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
* @brief  Implementation of CPACS vessel handling routines.
*/

#ifndef CCPACSVessel_H
#define CCPACSVessel_H

#include "generated/CPACSVessel.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CCPACSConfiguration.h"

#include <BRepBuilderAPI_MakeWire.hxx>

namespace tigl
{
class CCPACSVessel : public generated::CPACSVessel, public CTiglRelativelyPositionedComponent
{
public:
    // Constructor
    TIGL_EXPORT explicit CCPACSVessel(CCPACSVessels* parent, CTiglUIDManager* uidMgr);

    // Get the parent configuration
    TIGL_EXPORT CCPACSConfiguration const& GetConfiguration() const;

    // Get the default uID
    TIGL_EXPORT std::string GetDefaultedUID() const override;

    // Get the component type: TIGL_COMPONENT_TANK_HULL
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;
    // Get the component intent: TIGL_INTENT_PHYSICAL
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

    // Get number of sections
    TIGL_EXPORT int GetSectionCount() const;
    // Get a specific section
    TIGL_EXPORT CCPACSFuselageSection& GetSection(int index) const;
    TIGL_EXPORT CCPACSFuselageSection& GetSection(const std::string& sectionUID);
    // Get a section face
    TIGL_EXPORT TopoDS_Shape GetSectionFace(const std::string sectionUID) const;

    // Get number of segments
    TIGL_EXPORT int GetSegmentCount() const;
    // Get a specific segment via its index
    TIGL_EXPORT CCPACSFuselageSegment& GetSegment(const int index);
    TIGL_EXPORT const CCPACSFuselageSegment& GetSegment(const int index) const;
    // Get a specific segment via its uID
    TIGL_EXPORT CCPACSFuselageSegment& GetSegment(std::string uid);

    // Get the geometric volume of the vessel
    TIGL_EXPORT double GetGeometricVolume();

    // Get the surface area of the vessel
    TIGL_EXPORT double GetSurfaceArea();

    // Get the circumference of the vessel
    TIGL_EXPORT double GetCircumference(int segmentIndex, double eta);

    // Get a point on the given vessel segment in dependence of a parameters eta and zeta with
    // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
    // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
    // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
    TIGL_EXPORT gp_Pnt GetPoint(int segmentIndex, double eta, double zeta);
    // Set the _getPointBehavior to asParameterOnSurface or onLinearLoft
    TIGL_EXPORT void SetGetPointBehavior(TiglGetPointBehavior behavior = asParameterOnSurface);
    // Get the _getPointBehavior
    TIGL_EXPORT TiglGetPointBehavior GetGetPointBehavior() const;

    // Get the guide curve segment (partial guide curve) with a given uID
    TIGL_EXPORT CCPACSGuideCurve& GetGuideCurveSegment(std::string uid);
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurveSegment(std::string uid) const;
    // Returns all guide curve points
    TIGL_EXPORT std::vector<gp_Pnt> GetGuideCurvePoints() const;

    // Check whether the vessel is defined using segments
    TIGL_EXPORT bool IsVesselViaSegments() const;
    // Check whether the vessel is defined using design parameters
    TIGL_EXPORT bool IsVesselViaDesignParameters() const;
    // Check whether the vessel has spherical dome
    TIGL_EXPORT bool HasSphericalDome() const;
    // Check whether the vessel has ellipsoid dome
    TIGL_EXPORT bool HasEllipsoidDome() const;
    // Check whether the vessel has spherical dome
    TIGL_EXPORT bool HasTorisphericalDome() const;
    // Check whether the vessel has isotensoid dome
    TIGL_EXPORT bool HasIsotensoidDome() const;

protected:
    // Build the loft
    PNamedShape BuildLoft() const override;

    // Set the face traits
    void SetFaceTraits(PNamedShape loft) const;

private:
    const std::string _vesselTypeException =
        "This method is only available for vessels with segments. No segment found.";

    mutable const tigl::generated::CPACSEllipsoidDome* _ellipsoidPtr         = NULL;
    mutable const tigl::generated::CPACSTorisphericalDome* _torisphericalPtr = NULL;
    mutable const tigl::generated::CPACSIsotensoidDome* _isotensoidPtr       = NULL;
    mutable bool _isEvaluated                                                = false;
    void EvaluateDome() const;

    // Get short name for loft
    std::string GetShortShapeName() const;

    TiglGetPointBehavior _getPointBehavior{asParameterOnSurface};
    void IsotensoidContour(double rCyl, double rPolarOpening, int nodeNumber, std::vector<double>& x,
                           std::vector<double>& r) const;

    void BuildShapeFromSegments(TopoDS_Shape& loftShape) const;
    void BuildShapeFromSimpleParameters(TopoDS_Shape& loftShape) const;

    void BuildVesselWire(std::vector<TopoDS_Edge>& edges, BRepBuilderAPI_MakeWire& wire) const;
    void BuildVesselWireEllipsoid(BRepBuilderAPI_MakeWire& wire) const;
    void BuildVesselWireTorispherical(BRepBuilderAPI_MakeWire& wire) const;
    void BuildVesselWireIsotensoid(BRepBuilderAPI_MakeWire& wire) const;
};

} // namespace tigl

#endif // CCPACSFUSELAGE_H
