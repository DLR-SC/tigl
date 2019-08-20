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
* @brief  Implementation of CPACS fuselage handling routines.
*/

#ifndef CCPACSFUSELAGE_H
#define CCPACSFUSELAGE_H

#include "generated/CPACSFuselage.h"

#include <string>

#include "tigl_config.h"
#include "CTiglTransformation.h"
#include "CCPACSFuselageSections.h"
#include "CCPACSFuselageSegments.h"
#include "CCPACSPositionings.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CCPACSGuideCurve.h"
#include "CTiglFuselageConnection.h"
#include "Cache.h"
#include "CreateConnectedElementI.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"
#include <gp_Lin.hxx>
#include "CTiglFuselageHelper.h"

namespace tigl
{
class CCPACSConfiguration;
class CCPACSFuselageStringerFramePosition;

class CCPACSFuselage : public generated::CPACSFuselage, public CTiglRelativelyPositionedComponent, public CreateConnectedElementI
{
public:
    // Constructor
    TIGL_EXPORT CCPACSFuselage(CCPACSFuselages* parent, CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT ~CCPACSFuselage() OVERRIDE;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS fuselage elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& fuselageXPath) OVERRIDE;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration & GetConfiguration() const;

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT PNamedShape GetLoft(TiglCoordinateSystem cs = GLOBAL_COORDINATE_SYSTEM) const;

    // Get section count
    TIGL_EXPORT int GetSectionCount() const;

    // Returns the section for a given index
    TIGL_EXPORT CCPACSFuselageSection& GetSection(int index) const;

    // Get segment count
    TIGL_EXPORT int GetSegmentCount() const;

    // Returns the segment for a given index
    TIGL_EXPORT CCPACSFuselageSegment& GetSegment(const int index);
    TIGL_EXPORT const CCPACSFuselageSegment& GetSegment(const int index) const;

    // Returns the segment for a given UID
    TIGL_EXPORT CCPACSFuselageSegment& GetSegment(std::string uid);

    // Get the positioning transformation for a given section index
    TIGL_EXPORT boost::optional<CTiglTransformation> GetPositioningTransformation(const std::string& sectionUID);

    // Gets a point on the given fuselage segment in dependence of a parameters eta and zeta with
    // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
    // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
    // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
    TIGL_EXPORT gp_Pnt GetPoint(int segmentIndex, double eta, double zeta);

    // Gets the volume of this fuselage
    TIGL_EXPORT double GetVolume();

    // Gets the surfade area of this wing
    TIGL_EXPORT double GetSurfaceArea();

    // Returns the circumference of the segment "segmentIndex" at a given eta
    TIGL_EXPORT double GetCircumference(int segmentIndex, double eta);

    // Returns the Component Type TIGL_COMPONENT_FUSELAGE
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE {return TIGL_COMPONENT_FUSELAGE; }
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE {return TIGL_INTENT_PHYSICAL;}

    // Returns the point where the distance between the selected fuselage and the ground is at minimum.
    // The Fuselage could be turned with a given angle at at given axis, specified by a point and a direction.
    TIGL_EXPORT gp_Pnt GetMinumumDistanceToGround(gp_Ax1 RAxis, double angle);

    // Get the guide curve segment(partial guide curve) with a given UID
    TIGL_EXPORT CCPACSGuideCurve& GetGuideCurveSegment(std::string uid);
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurveSegment(std::string uid) const;

    // Returns all guide curve wires as a compound
    TIGL_EXPORT const TopoDS_Compound& GetGuideCurveWires() const;

    // Returns all guide curve points
    TIGL_EXPORT std::vector<gp_Pnt> GetGuideCurvePoints() const;

    // create the line intersecting the fuselage for the stringer/frame profile
    TIGL_EXPORT gp_Lin Intersection(gp_Pnt pRef, double angleRef) const;
    TIGL_EXPORT gp_Lin Intersection(const CCPACSFuselageStringerFramePosition& pos) const;

    // project the edge/wire onto the fuselage loft
    TIGL_EXPORT TopoDS_Wire projectConic(TopoDS_Shape wireOrEdge, gp_Pnt origin) const;
    TIGL_EXPORT TopoDS_Wire projectParallel(TopoDS_Shape wireOrEdge, gp_Dir direction) const;

    /*
     * Creator functions
     */

    // Return the center of the noise airfoil, this correspond to the the beginning of the fuselage.
    TIGL_EXPORT CTiglPoint GetNoseCenter();

    // Set the position of the noise center.
    TIGL_EXPORT void SetNoseCenter(const CTiglPoint &newCenter);

    // Set the rotation of the fuselage transformation (nothing else ;)
    // Todo evalute the possiblity to rotate around the nose (simple fuselage case)
    TIGL_EXPORT void SetRotation(const CTiglPoint& newRotation);

    // Gets the total length of this fuselage
    TIGL_EXPORT double GetLength();

    // Sets the total length of this fuselage. (The noise keeps its position.)
    TIGL_EXPORT void SetLength(double newLength);

    // Return the maximal height of the fuselage
    // The height is computed by reverting the fuselage rotation and building a bounding box around the fuselage
    TIGL_EXPORT double GetMaximalHeight();

    // Set the maximal height of the fuselage by
    // inverting the rotation of the fuselage, bring the nose to the origin and scaling in the Z direction
    TIGL_EXPORT void SetMaxHeight(double newMaxHeight);

    // Return the maximal width of the fuselage
    TIGL_EXPORT double GetMaximalWidth();

    // Set the maximal width of the fuselage by inverting the rotation of the fuselage and scaling in the Y direction
    TIGL_EXPORT void SetMaxWidth(double newMaxWidth);

    /**
     * Create a new section, a new element and connect the element to the "startElement".
     * The new element is placed "after" the start element.
     * If there is already a element after the start element, we split the existing segment and insert the new element
     * between the the two elements.
     *
     * @param startElementUID
     */
    TIGL_EXPORT void CreateNewConnectedElementAfter(std::string startElementUID) override;

    /**
     * Create a new section, a new element and connect the element to the "startElement".
     * The new element is placed "Before" the start element.
     * If there is already a element before the start element, we split the existing segment and insert the new element
     * between the the two elements.
     *
     * @param startElementUID
     */
    TIGL_EXPORT void CreateNewConnectedElementBefore(std::string startElementUID) override;

    /**
      *Create a new section, a new element and place the new element between the startElement and the endElement.
     * @remark The startElement and endElement must be connected by a segment.
     * @param startElementUID
     * @param endElementUID
     */
    TIGL_EXPORT void CreateNewConnectedElementBetween(std::string startElementUID, std::string endElementUID) override;


    TIGL_EXPORT void DeleteConnectedElement(std::string elementUID) override;

    TIGL_EXPORT std::vector<std::string> GetOrderedConnectedElement() override;

protected:
    void BuildGuideCurves(TopoDS_Compound& cache) const;

    void ConnectGuideCurveSegments();

    // Cleanup routine
    void Cleanup();

    // Adds all segments of this fuselage to one shape
    PNamedShape BuildLoft() const OVERRIDE;

    void SetFaceTraits(PNamedShape loft) const;

    void SetFuselageHelper(CTiglFuselageHelper& cache) const ;

private:
    // get short name for loft
    std::string GetShortShapeName() const;

private:
    CCPACSConfiguration*       configuration;        /**< Parent configuration    */
    FusedElementsContainerType fusedElements;        /**< Stores already fused segments */

    TopoDS_Compound            aCompound;
    Cache<TopoDS_Compound, CCPACSFuselage> guideCurves;
    BRep_Builder               aBuilder;
    double                     myVolume;             /**< Volume of this fuselage              */

    Cache<CTiglFuselageHelper, CCPACSFuselage> fuselageHelper;

    friend class CCPACSFuselageSegment;
    friend class CTiglStandardizer;
};

TIGL_EXPORT TopoDS_Shape transformFuselageProfileGeometry(const CTiglTransformation& fuselTransform, const CTiglFuselageConnection& connection, const TopoDS_Shape& shape);

} // end namespace tigl

#endif // CCPACSFUSELAGE_H
