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
#include "CCPACSWingSections.h"
#include "CCPACSWingSegments.h"
#include "CCPACSWingComponentSegments.h"
#include "CCPACSPositionings.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSGuideCurve.h"
#include "Cache.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Compound.hxx"

namespace tigl
{
class CCPACSConfiguration;

class CCPACSWing : public generated::CPACSWing, public CTiglRelativelyPositionedComponent
{
friend class CTiglWingBuilder;

public:
    // Constructor
    TIGL_EXPORT CCPACSWing(CCPACSWings* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSWing(CCPACSRotorBlades* parent, CTiglUIDManager* uidMgr);

    // Virtual destructor
    TIGL_EXPORT ~CCPACSWing() OVERRIDE;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS wing elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& wingXPath) OVERRIDE;

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    // Returns whether this wing is a rotor blade
    TIGL_EXPORT bool IsRotorBlade() const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration & GetConfiguration() const;

    // Get section count
    TIGL_EXPORT int GetSectionCount() const;

    // Returns the section for a given index
    TIGL_EXPORT CCPACSWingSection& GetSection(int index);
    TIGL_EXPORT const CCPACSWingSection& GetSection(int index) const;

    // Getter of the number of segments of the wing
    TIGL_EXPORT int GetSegmentCount() const;

    // Returns the segment for a given index or uid
    TIGL_EXPORT CCPACSWingSegment& GetSegment(const int index);
    TIGL_EXPORT const CCPACSWingSegment& GetSegment(const int index) const;
    TIGL_EXPORT CCPACSWingSegment& GetSegment(std::string uid);
    TIGL_EXPORT const CCPACSWingSegment& GetSegment(std::string uid) const;

    // Getter of the number of component segments of the wing
    TIGL_EXPORT int GetComponentSegmentCount() const;

    // Returns the segment for a given index or uid
    TIGL_EXPORT CCPACSWingComponentSegment& GetComponentSegment(const int index);
    TIGL_EXPORT const CCPACSWingComponentSegment& GetComponentSegment(const int index) const;
    TIGL_EXPORT CCPACSWingComponentSegment& GetComponentSegment(const std::string& uid);

    // Get the positioning transformation for a given section uid
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation(std::string sectionUID);

    // Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
    TIGL_EXPORT gp_Pnt GetUpperPoint(int segmentIndex, double eta, double xsi);

    // Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
    TIGL_EXPORT gp_Pnt GetLowerPoint(int segmentIndex, double eta, double xsi);

    // Gets a point on the chord surface in absolute (world) coordinates for a given segment, eta, xsi
    TIGL_EXPORT gp_Pnt GetChordPoint(int segmentIndex, double eta, double xsi);

    // Gets the loft of the whole wing
    TIGL_EXPORT TopoDS_Shape & GetLoftWithLeadingEdge();
        
    TIGL_EXPORT TopoDS_Shape & GetUpperShape();
    TIGL_EXPORT TopoDS_Shape & GetLowerShape();

    // Gets the volume of this wing
    TIGL_EXPORT double GetVolume();

    // Gets the surfade area of this wing
    TIGL_EXPORT double GetSurfaceArea();

    // Returns the reference area of the wing by taking account the drilateral portions
    // of each wing segment by projecting the wing segments into the plane defined by the user
    TIGL_EXPORT double GetReferenceArea(TiglSymmetryAxis symPlane);

    // Returns wetted Area
    TIGL_EXPORT double GetWettedArea(TopoDS_Shape parent);

    // Returns the wingspan of the wing
    TIGL_EXPORT double GetWingspan();

    // Returns the aspect ratio of the wing
    TIGL_EXPORT double GetAspectRatio();

    // Returns the mean aerodynamic chord of the wing
    TIGL_EXPORT void  GetWingMAC(double& mac_chord, double& mac_x, double& mac_y, double& mac_z);

    // Calculates the segment coordinates from global (x,y,z) coordinates
    // Returns the segment index of the according segment
    // If x,y,z does not belong to any segment, -1 is returned
    TIGL_EXPORT int GetSegmentEtaXsi(const gp_Pnt& xyz, double& eta, double& xsi, bool &onTop);

    // Returns the Component Type TIGL_COMPONENT_WING.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE
    {
        return !IsRotorBlade() ? TIGL_COMPONENT_WING : TIGL_COMPONENT_ROTORBLADE;
    }

    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE {return TIGL_INTENT_PHYSICAL; }

    // Returns the lower Surface of a Segment
    TIGL_EXPORT Handle(Geom_Surface) GetLowerSegmentSurface(int index);

    // Returns the upper Surface of a Segment
    TIGL_EXPORT Handle(Geom_Surface) GetUpperSegmentSurface(int index);

    // Get the guide curve segment (partial guide curve) with a given UID
    TIGL_EXPORT CCPACSGuideCurve& GetGuideCurveSegment(std::string uid);

    // Returns all points that define the guide curves
    TIGL_EXPORT std::vector<gp_Pnt> GetGuideCurvePoints();

    // Returns all guide curve wires as a compound
    TIGL_EXPORT TopoDS_Compound GetGuideCurveWires() const;

protected:
    void BuildGuideCurveWires(TopoDS_Compound& cache) const;

    // Cleanup routine
    void Cleanup();

    void ConnectGuideCurveSegments(void);

    // Update internal wing data
    void Update();

    // Adds all Segments of this wing to one shape
    PNamedShape BuildFusedSegments(bool splitWingInUpperAndLower) const;
        
    PNamedShape BuildLoft() const OVERRIDE;
        
    void BuildUpperLowerShells();

private:
    // get short name for loft
    std::string GetShortShapeName() const;

private:
    bool                           isRotorBlade;             /**< Indicates if this wing is a rotor blade */
    CCPACSConfiguration*           configuration;            /**< Parent configuration*/
    TopoDS_Shape                   fusedSegmentWithEdge;     /**< All Segments in one shape plus modelled leading edge */ 
    TopoDS_Shape                   upperShape;
    TopoDS_Shape                   lowerShape;
    Cache<TopoDS_Compound, CCPACSWing> guideCurves;
    bool                           invalidated;              /**< Internal state flag */
    bool                           rebuildFusedSegments;     /**< Indicates if segmentation fusing need rebuild */
    bool                           rebuildFusedSegWEdge;     /**< Indicates if segmentation fusing need rebuild */
    bool                           rebuildShells;
    FusedElementsContainerType     fusedElements;            /**< Stores already fused segments */
    double                         myVolume;                 /**< Volume of this Wing           */

    friend class CCPACSWingSegment;
    friend class CCPACSWingComponentSegment;
};

/// Transforms a shape in profile coordinates to world coordinates
TIGL_EXPORT TopoDS_Shape transformWingProfileGeometry(const CTiglTransformation& wingTransform, const CTiglWingConnection& connection, const TopoDS_Shape& wire);

} // end namespace tigl

#endif // CCPACSWING_H
