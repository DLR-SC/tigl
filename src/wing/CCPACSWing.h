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
* @brief  Implementation of CPACS wing handling routines.
*/

#ifndef CCPACSWING_H
#define CCPACSWING_H

#include <string>

#include "generated/CPACSWing.h"
#include "tigl_config.h"
#include "CTiglTransformation.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "generated/CPACSWingSections.h"
#include "CCPACSWingSegments.h"
#include "CCPACSWingComponentSegments.h"
#include "CCPACSPositionings.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSGuideCurve.h"
#include "Cache.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Compound.hxx"

#include "CTiglWingSectionElement.h"
#include "CTiglWingHelper.h"

namespace tigl
{
class CCPACSConfiguration;

class CCPACSWing : public generated::CPACSWing, public CTiglRelativelyPositionedComponent
{
friend class CTiglWingBuilder;

public:
    /**
     *  Constructor of Class CCPACSWing
     *  Constructor is automatically called during reading of a CPACS-file.
     *  Not intended to be called manually.
     *  @param parent
     *  @param uidMgr
     */
    TIGL_EXPORT CCPACSWing(CCPACSWings* parent, CTiglUIDManager* uidMgr);

    /**
     *  Constructor of Class CCPACSWing
     *  Constructor is automatically called during reading of a CPACS-file.
     *  Not intended to be called manually.
     *  @param parent
     *  @param uidMgr
     */
    TIGL_EXPORT CCPACSWing(CCPACSRotorBlades* parent, CTiglUIDManager* uidMgr);

    // Virtual destructor
    TIGL_EXPORT ~CCPACSWing() override;

    // Read CPACS wing elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& wingXPath) override;

    TIGL_EXPORT std::string GetDefaultedUID() const override;

    // Override setters for invalidation
    TIGL_EXPORT void SetSymmetryAxis(const TiglSymmetryAxis& axis) override;
    TIGL_EXPORT void SetParentUID(const boost::optional<std::string>& value) override;

    /**
     * @brief Returns whether this wing is a rotor blade
     * @return bool
     */
    TIGL_EXPORT bool IsRotorBlade() const;

    /**
     *  @brief Returns the parent configuration
     *  @return CCPACSConfiguration&
     */
    TIGL_EXPORT CCPACSConfiguration & GetConfiguration() const;

    /**
     * @brief Get section count
     * @return int
     */
    TIGL_EXPORT int GetSectionCount() const;

    /**
     * @brief Returns the section for a given index
     * @return CCPACSWingSection&
     */
    TIGL_EXPORT CCPACSWingSection& GetSection(int index);

    /**
     * @brief Returns the section for a given index
     * @return CCPACSWingSection&
     */
    TIGL_EXPORT const CCPACSWingSection& GetSection(int index) const;


    /**
     *  @brief Returns the number of segments of the wing
     *  @param index
     *  @return int
     */
    TIGL_EXPORT int GetSegmentCount() const;

    /**
     * @brief Returns the segment for a given index
     * @param index Segment index
     * @return CCPACSWingSegment&
     */
    TIGL_EXPORT CCPACSWingSegment& GetSegment(const int index);

    /**
     * @brief Returns the segment for a given index
     * @param index Segment index
     * @return CCPACSWingSegment&
     */
    TIGL_EXPORT const CCPACSWingSegment& GetSegment(const int index) const;

    /**
     * @brief Returns the segment for a given uid
     * @param uid Segment UID
     * @return CCPACSWingSegment&
     */
    TIGL_EXPORT CCPACSWingSegment& GetSegment(std::string uid);

    /**
     * @brief Returns the segment for a given uid
     * @param uid Segment UID
     * @return CCPACSWingSegment&
     */
    TIGL_EXPORT const CCPACSWingSegment& GetSegment(std::string uid) const;

    /**
     * @brief Returns the number of component segments of the wing
     * @return int
     */
    TIGL_EXPORT int GetComponentSegmentCount() const;

    /**
     * @brief Returns the segment for a given index
     * @param index Segment index
     * @return CCPACSWingComponentSegment&
     */
    TIGL_EXPORT CCPACSWingComponentSegment& GetComponentSegment(const int index);

    /**
     * @brief Returns the segment for a given index
     * @param index Segment index
     * @return CCPACSWingComponentSegment&
     */
    TIGL_EXPORT const CCPACSWingComponentSegment& GetComponentSegment(const int index) const;

    /**
     * @brief Returns the segment for a given uid
     * @param uid Segment UID
     * @return CCPACSWingComponentSegment&
     */
    TIGL_EXPORT CCPACSWingComponentSegment& GetComponentSegment(const std::string& uid);

    /**
     * @brief Returns the positioning transformation for a given section uid
     * @param sectionUID
     * @return CTiglTransformation
     */
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation(std::string sectionUID);

    /**
     * @brief Returns the upper point in absolute (world) coordinates for a given segment,
     * eta, xsi (calculated output may be influenced by setting different value for Enum getPointBehavior)
     * @param segmentIndex
     * @param eta
     * @param xsi
     * @return gp_nP
     */
    TIGL_EXPORT gp_Pnt GetUpperPoint(int segmentIndex, double eta, double xsi);

    /**
     * @brief Returns the lower point in absolute (world) coordinates for a given segment, eta, xsi
     * (calculated output may be influenced by setting different value for Enum getPointBehavior)
     * @param segmentIndex
     * @param eta
     * @param xsi
     * @returns gp_nP
     */
    TIGL_EXPORT gp_Pnt GetLowerPoint(int segmentIndex, double eta, double xsi);

    /**
     * @brief Returns a point on the chord surface in absolute (world) coordinates for a given segment, eta, xsi,
     * if fourth parameter is ommited, otherwise it will give you the point in the chosen coordinate system (e.g.  WING_COORDINATE_SYSTEM)
     * @param segmentIndex
     * @param eta
     * @param xsi
     * @param referenceCS
     * @returns gp_nP
     */
    TIGL_EXPORT gp_Pnt GetChordPoint(int segmentIndex, double eta, double xsi, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    /**
     * @brief Returns the rebuilt loft of the whole wing with modeled leading edge, if wing shape rebuild is indicated
     * @return TopoDS_Shape& Returns the wing shape
     */
    TIGL_EXPORT TopoDS_Shape & GetLoftWithLeadingEdge();

    /**
     * @brief Returns the wing loft with cut out control surface
     * @return TopoDS_Shape Returns the wing shape with cutouts
     */
    TIGL_EXPORT TopoDS_Shape GetLoftWithCutouts();

    /**
     * @brief Returns upper loft of the wing and builds it if shells require rebuild
     * @return TopoDS_Shape
     */
    TIGL_EXPORT TopoDS_Shape & GetUpperShape();

    /**
     * @brief Returns lower loft of the wing and builds it if shells require rebuild
     * @return TooDS_Shape
     */
    TIGL_EXPORT TopoDS_Shape & GetLowerShape();

    /**
     * @brief Returns the volume of this wing (corresponding to CPACS conventions the result will be in SI-units)
     * @return double
     */
    TIGL_EXPORT double GetVolume();

    /**
     *  @brief Returns the surface area of this wing
     *  @return double
     */
    TIGL_EXPORT double GetSurfaceArea();

    /**
     * @brief Returns the reference area of the wing by taking account the quadrilateral portions
     *  of each wing segment by projecting the wing segments into the plane defined by the user
     * @param symPlane The TiglSymmetryAxis is defined as follows:
     * TIGL_NO_SYMMETRY      = 0,
     * TIGL_X_Y_PLANE        = 1,
     * TIGL_X_Z_PLANE        = 2,
     * TIGL_Y_Z_PLANE        = 3,
     * TIGL_INHERIT_SYMMETRY = 4
     * @return double
     */
    TIGL_EXPORT double GetReferenceArea(TiglSymmetryAxis symPlane) const;

    /**
     * @brief Returns the reference area of the wing in the plane normal to the major direction
     * @return double
     */
    TIGL_EXPORT double GetReferenceArea() const;

    /**
     * @brief Returns wetted Area
     * @param parent
     * @return double
     */
    TIGL_EXPORT double GetWettedArea(TopoDS_Shape parent) const;

    // Returns the wingspan of the wing
    // The span is the width of the wing in the major wing direction.
    // Remark, if the wing is mirrored (with a symmetry axis), the two wings are taken into account.
    // Otherwise, the width of the unique wing is returned.
    // Remark: The span is computed using a bounding box. So, the result may no be completely accurate.
    TIGL_EXPORT double GetWingspan() const;

    // Returns the half span of the wing
    // If the wing is not mirrored, return the whole width.
    // Remark: The span is computed using a bounding box. So, the result may no be completely accurate.

    /**
     * Returns the half span of the wing
     * If the wing is not mirrored, return the whole width.
     * @remark The span is computed using a bounding box. So, the result may no be completely accurate.
     * @return half span of the wing
     */
    TIGL_EXPORT double GetWingHalfSpan();

    /**
     * @brief Returns the aspect ratio of the wing
     * @return double
     */
    TIGL_EXPORT double GetAspectRatio() const;

    /**
     * @brief Returns the mean aerodynamic chord of the wing
     * @param mac_chord
     * @param mac_x
     * @param may_y
     * @param mac_z
     */
    TIGL_EXPORT void  GetWingMAC(double& mac_chord, double& mac_x, double& mac_y, double& mac_z) const;

    /**
     * @brief Calculates the segment coordinates from global (x,y,z) coordinates
     * If x,y,z does not belong to any segment, -1 is returned
     * @param xyz Global (x,y,z) coordinates
     * @param[in] eta
     * @param[in] xsi
     * @param[out] eta
     * @param[out] xsi
     * @param onTop
     * @return int Returns -1 if x,y,z does not belong to any segment
     */
    TIGL_EXPORT int GetSegmentEtaXsi(const gp_Pnt& xyz, double& eta, double& xsi, bool &onTop);

    /**
     * @brief Returns the Component Type TIGL_COMPONENT_WING or TIGL_COMPONENT_ROTORBLADE.
     */
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override
    {
        return !IsRotorBlade() ? TIGL_COMPONENT_WING : TIGL_COMPONENT_ROTORBLADE;
    }

    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override {return TIGL_INTENT_PHYSICAL; }

    /**
     * @brief Returns the lower Surface of a Segment
     * @param index Index of the segment
     * @return Handle(Geom_Surface) Returns Handle to the lower surface of a segment
     */
    TIGL_EXPORT Handle(Geom_Surface) GetLowerSegmentSurface(int index);

    /**
     * @brief Returns the upper Surface of a Segment
     * @param index Index of the segment
     * @return Handle(Geom_Surface) Returns Handle to the upper surface of a segment
     */
    TIGL_EXPORT Handle(Geom_Surface) GetUpperSegmentSurface(int index);

    /**
     * @brief Get the guide curve segment (partial guide curve) with a given UID
     * @param uid UID of the segment
     * @return CCPACSGuideCurve&
     */
    TIGL_EXPORT CCPACSGuideCurve& GetGuideCurveSegment(std::string uid);

    /**
     * @brief Returns all points that define the guide curves
     * @return std::vector<gp_Pnt> Returns a vector containing all points definig the guide curves
     */
    TIGL_EXPORT std::vector<gp_Pnt> GetGuideCurvePoints();

    /**
     * @brief Returns a wire consisting of all guide curves as a compound
     * @return TopoDS_Compound
     */
    TIGL_EXPORT TopoDS_Compound GetGuideCurveWires() const;

    /**
     * @brief Returns the relative circumference parameters (as defined in CPACS)
     * (These parameters are calculated to scale the guide curve wire and construct the
     * guide curves)
     * @return std::vector<double>
     */
    TIGL_EXPORT std::vector<double> GetGuideCurveStartParameters() const;

    /**
     * @brief Determines, whether the wing should be modeled with the flaps or not
     * @param enabled Set 'True' if wing should be built with flags
     */
    TIGL_EXPORT void SetBuildFlaps(bool enabled);

    /**
     * @brief Returns the wing shape without flaps cut out
     * @return PNamedShape
     */
    TIGL_EXPORT PNamedShape GetWingCleanShape() const;

    TiglGetPointBehavior getPointBehavior {asParameterOnSurface};       /**< sets behavior of the GetPoint-function (default: asParameterOnSurface)      */

    // CREATOR FUNCTIONS

    TIGL_EXPORT double GetSweep(double chordPercentage = 0.25) const;

    TIGL_EXPORT double GetDihedral(double chordPercentage = 0.25) const;

    // Return the root leading edge position in world coordinate system
    // We use this position has the wing position.
    TIGL_EXPORT CTiglPoint GetRootLEPosition() const;

    // Set the root leading edge position in world coordinate system
    TIGL_EXPORT void SetRootLEPosition(CTiglPoint newRootPosition);

    // Set the rotation of the wing transformation and invalidate the wing
    TIGL_EXPORT void SetRotation(CTiglPoint newRot);

    // Set the sweep of the wing
    TIGL_EXPORT void SetSweep(double sweepAngle, double chordPercentage = 0.25);

    // Set the dihedral of the wing
    TIGL_EXPORT void SetDihedral(double dihedralAngle, double chordPercentage = 0.25);


    /**
     * Scale the wing uniformly.
     * The root leading point stays at the same position.
     * @param scaleF the scale factor
     */
    TIGL_EXPORT void Scale(double scaleF);


    /**
     * Set the wing reference area while keeping the aspect ratio constant.
     * The area is set scaling the wing uniformly.
     * @remark The span will change.
     * @param newArea
     */
    TIGL_EXPORT void SetAreaKeepAR(double newArea);

     /**
     * Set the wing reference area while keeping the span constant.
     * To obtain the wanted area each airfoil is scaled by the same factor.
     * The leading point of each airfoil stays at the same position.
     * @remark The aspect ratio will change.
     * @remark The tip airfoil is also scaled, so it is possible that the the span change slighly if
     * the tip airfoil is not parallel to the deep axis.
     * @param newArea
     */
    TIGL_EXPORT void SetAreaKeepSpan(double newArea);




    /**
     * Set the wing half span while keeping the aspect ratio constant.
     * The span is set scaling the wing uniformly.
     * @remark The area will change.
     * @param newArea
     */
    TIGL_EXPORT void SetHalfSpanKeepAR(double newHalfSpan);


    /**
     * Set the wing half span while keeping the area constant.
     * The span is set by first scaling the wing uniformly,
     * then resetting the area while keeping the span constant.
     * @remark The aspect ratio will change.
     * @param newArea
     */
    TIGL_EXPORT void SetHalfSpanKeepArea(double newHalfSpan);


    /**
     * Set the wing aspect ratio (AR) while keeping the span constant.
     * The area will change so that the AR is reached.
     * @param newAR
     */
    TIGL_EXPORT void SetARKeepSpan(double newAR);



    /**
     * Set the wing aspect ratio (AR) while keeping the area constant.
     * The span will change so that the AR is reached.
     * @param newAR
     */
    TIGL_EXPORT void SetARKeepArea(double newAR);




    /**
     * Create a new section, a new element and connect the element to the "startElement".
     * The new element is placed "after" the start element.
     * If there is already an element after the start element, we split the existing segment and insert the new element
     * between the two elements.
     *
     * @param startElementUID
     */
    TIGL_EXPORT void CreateNewConnectedElementAfter(std::string startElementUID);

    /**
     * Create a new section, a new element and connect the element to the "startElement".
     * The new element is placed "Before" the start element.
     * If there is already an element before the start element, we split the existing segment and insert the new element
     * between the two elements.
     *
     * @param startElementUID
     */
    TIGL_EXPORT void CreateNewConnectedElementBefore(std::string startElementUID);

    /**
      *Create a new section, a new element and place the new element between the startElement and the endElement.
     * @remark The startElement and endElement must be connected by a segment.
     * @param startElementUID
     * @param endElementUID
     */
    TIGL_EXPORT void CreateNewConnectedElementBetween(std::string startElementUID, std::string endElementUID);


    /**
     * Delete the connected element.
     * This means that the section containing the element is deleted and the segments associated with this element
     * are either deleted or updated.
     * @param ElementUID
     */
    TIGL_EXPORT void DeleteConnectedElement(std::string ElementUID);


    TIGL_EXPORT std::vector<std::string> GetOrderedConnectedElement();

    TIGL_EXPORT  std::vector<CTiglSectionElement* > GetCTiglElements() const;

    /**
     *
     * @return Return all the uid of the airfoils used by this wing
     */
    TIGL_EXPORT std::vector<std::string> GetAllUsedAirfoils();

    /**
     * Set the airfoil uid of all the section elements of this wing.
     * @param profileUID ; the airfoil UID to use
     */
    TIGL_EXPORT void SetAllAirfoils(const std::string& profileUID);


protected:

    struct LocatedGuideCurves
    {
        struct LocatedGuideCurve
        {
            TopoDS_Wire wire;
            double fromRelCircumference;
        };

        TopoDS_Compound wiresAsCompound;
        std::vector<LocatedGuideCurve> curves;
    };

    void BuildGuideCurveWires(LocatedGuideCurves& cache) const;

    // Cleanup routine
    void Cleanup();

    // Update internal wing data
    void Update();

    // Adds all Segments of this wing to one shape
    void BuildFusedSegments(PNamedShape& ) const;
        
    PNamedShape BuildLoft() const override;
        
    void BuildUpperLowerShells();

    void SetWingHelper(CTiglWingHelper& cache) const;

private:
    // Invalidates internal state
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

    // get short name for loft
    std::string GetShortShapeName() const;
    void BuildWingWithCutouts(PNamedShape&) const;
    // Adds all Segments of this wing and flaps to one shape
    PNamedShape GroupedFlapsAndWingShapes() const;

    bool                           isRotorBlade;             /**< Indicates if this wing is a rotor blade */
    CCPACSConfiguration*           configuration;            /**< Parent configuration*/
    TopoDS_Shape                   fusedSegmentWithEdge;     /**< All Segments in one shape plus modelled leading edge */ 
    TopoDS_Shape                   upperShape;
    TopoDS_Shape                   lowerShape;

    Cache<LocatedGuideCurves, CCPACSWing> guideCurves;

    Cache<PNamedShape, CCPACSWing> wingShapeWithCutouts;     /**< Wing without flaps / flaps removed */
    Cache<PNamedShape, CCPACSWing> wingCleanShape;           /**< Clean wing surface without flaps cutout*/
    mutable bool                   rebuildFusedSegWEdge;     /**< Indicates if segmentation fusing need rebuild */
    mutable bool                   rebuildShells;
    bool                           buildFlaps;               /**< Indicates if the wing's loft shall include flaps */
    FusedElementsContainerType     fusedElements;            /**< Stores already fused segments */
    double                         myVolume;                 /**< Volume of this Wing           */

    Cache<CTiglWingHelper, CCPACSWing> wingHelper;



    friend class CCPACSWingSegment;
    friend class CCPACSWingComponentSegment;
    friend class CTiglStandardizer;
};

/// Transforms a shape in profile coordinates to world coordinates
TIGL_EXPORT TopoDS_Shape transformWingProfileGeometry(const CTiglTransformation& wingTransform, const CTiglWingConnection& connection, const TopoDS_Shape& wire);

} // end namespace tigl

#endif // CCPACSWING_H
