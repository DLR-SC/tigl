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
    TIGL_EXPORT CCPACSFuselageSection const& GetSection(int index) const;
    TIGL_EXPORT CCPACSFuselageSection& GetSection(int index);
    TIGL_EXPORT CCPACSFuselageSection const& GetSection(const std::string& sectionUID) const;
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

    CCPACSEllipsoidDome const* GetEllipsoidDome() const;
    CCPACSTorisphericalDome const* GetTorisphericalDome() const;
    CCPACSIsotensoidDome const* GetIsotensoidDome() const;

    // Get short name for loft
    std::string GetShortShapeName() const;

    void BuildShapeFromSegments(TopoDS_Shape& loftShape) const;
    void BuildShapeFromSimpleParameters(TopoDS_Shape& loftShape) const;

    void BuildVesselWire(std::vector<TopoDS_Edge>& edges, BRepBuilderAPI_MakeWire& wire) const;
    void BuildVesselWireEllipsoid(BRepBuilderAPI_MakeWire& wire) const;
    void BuildVesselWireTorispherical(BRepBuilderAPI_MakeWire& wire) const;
    void BuildVesselWireIsotensoid(BRepBuilderAPI_MakeWire& wire) const;

    /**
     * @brief  Approximated contour of an isotensoid dome section.
     *
     * This struct computes and stores the axial profile and corresponding radii
     * of an isotensoid dome, starting from a cylindrical base and ending at the
     * polar opening.
     *
     * @details
     * The algorithmic approach is based on patent EP0714753A2. For a more in-depth
     * treatment of the theory and practical applications, see:
     *   Vasiliev, Valery V. Composite pressure vessels: Design, analysis, and
     *   manufacturing. Bull Ridge Corporation, 2009.
     *
     * @param cylinderRadius       Radius of the initial cylindrical base.
     * @param polarOpeningRadius   Radius at the polar opening of the dome.
     * @param nodeNumber           Number of integration steps used to sample
     *                             the contour curve
     */
    struct IsotensoidContour {

        std::vector<double> axialPositions;
        std::vector<double> radii;

        IsotensoidContour(double cylinderRadius, double polarOpeningRadius, int nodeNumber);
    };
};

} // namespace tigl

#endif // CCPACSFUSELAGE_H
