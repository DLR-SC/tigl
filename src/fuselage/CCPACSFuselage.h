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

#include "TopoDS_Shape.hxx"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"
#include <gp_Lin.hxx>

namespace tigl
{
class CCPACSConfiguration;
class CCPACSFuselageStringerFramePosition;

class CCPACSFuselage : public generated::CPACSFuselage, public CTiglRelativelyPositionedComponent
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

    // return the element uid that is considered as the noise element
    TIGL_EXPORT std::string GetNoseUID();

    // Return the center of the noise airfoil, this correspond to the the beginning of the fuselage.
    TIGL_EXPORT CTiglPoint GetNoseCenter();

    // Set the position of the noise center.
    TIGL_EXPORT void SetNoseCenter(const CTiglPoint &newCenter);

    // Set the rotation of the fuselage transformation (nothing else ;)
    TIGL_EXPORT void SetRotation(const CTiglPoint& newRotation);

    // return the element uid that is considered as the tail element
    TIGL_EXPORT std::string GetTailUID();

    // Return the elements that form the connections in this fuselage
    TIGL_EXPORT std::vector<std::string> GetElementUIDsInOrder();

    // Gets the total length of this fuselage
    TIGL_EXPORT double GetLength();

    // Gets the length between the two fuselage elements given as arguments
    TIGL_EXPORT double GetLengthBetween(const std::string& startElementUID, const std::string& endElementUID);

    // Sets the total length of this fuselage. (The noise keeps its position.)
    TIGL_EXPORT void SetLength(double newLength);

    // Sets the total length of this fuselage. (The noise keeps its position.)
    TIGL_EXPORT void SetLengthBetween(const std::string& startElementUID, const std::string& endElementUID,
                                      double newPartialLength);

    // Get the biggest section element circumference of the fuselage
    TIGL_EXPORT double GetMaximalCircumference();

    // Get the biggest section element circumference between two section element of the fuselage
    TIGL_EXPORT double GetMaximalCircumferenceBetween(const std::string& startElementUID,
                                                      const std::string& endElementUID);

    // Return the maximal height of the fuselage
    // The height is computed in the plane formed by the wire (not in the world CS ZY plane)
    TIGL_EXPORT double GetMaximalHeight();

    TIGL_EXPORT double GetMaximalHeightBetween(const std::string& startElementUID, const std::string& endElementUID);

    // Return the maximal width of the fuselage
    // The width is computed in the plane formed by the wire (not in the world CS ZY plane)
    TIGL_EXPORT double GetMaximalWidth();

    TIGL_EXPORT double GetMaximalWidthBetween(const std::string& startElementUID, const std::string& endElementUID);

    // Return the maximal area of the fuselage
    // The area is computed in the plane formed by the wire (not in the world CS ZY plane)
    TIGL_EXPORT double GetMaximalWireArea();

    TIGL_EXPORT double GetMaximalWireAreaBetween(const std::string& startElementUID, const std::string& endElementUID);


    // Scale the wires uniformly.
    // This can be used to set the max height or max area
    TIGL_EXPORT void ScaleWiresUniformly(double scaleFactor);

    // Scale the wires between start and end uid uniformly (start and end included)
    TIGL_EXPORT void ScaleWiresUniformlyBetween(double scaleFactor, const std::string& startElementUID, const std::string& endElementUID);

    TIGL_EXPORT void SetMaxHeight(double newMaxHeight);

    TIGL_EXPORT void SetMaxHeightBetween(double newMaxHeight, const std::string& startUID, const std::string& endUID );


    TIGL_EXPORT void SetMaxWidth(double newMaxWidth);

    TIGL_EXPORT void SetMaxWidthBetween(double newMaxWidth, const std::string& startUID, const std::string& endUID );


    TIGL_EXPORT void SetMaxArea(double newMaxArea);

    TIGL_EXPORT void SetMaxAreaBetween(double newMaxArea, const std::string& startUID, const std::string& endUID );


    // Set the biggest circumference of the fuselage to the given value,
    // the other circumferences are proportionally scaled
    TIGL_EXPORT void SetMaximalCircumference(double newMaximalCircumference);

    // Set the biggest circumference of the fuselage between two given section elements to the given value,
    // the other circumferences are proportionally scaled
    TIGL_EXPORT void SetMaximalCircumferenceBetween(const std::string& startElementUID,
                                                    const std::string& endElementUID, double newMaximalCircumference);

protected:
    void BuildGuideCurves(TopoDS_Compound& cache) const;

    void ConnectGuideCurveSegments();

    // Cleanup routine
    void Cleanup();

    // Adds all segments of this fuselage to one shape
    PNamedShape BuildLoft() const OVERRIDE;

    void SetFaceTraits(PNamedShape loft) const;


    typedef double (CTiglFuselageSectionElement::*pGetProperty)(TiglCoordinateSystem referenceCS) const;

    // functional programing: apply func on each element in between and return the max double
    double GetMaxBetween(pGetProperty func, const std::string& startElementUID, const std::string& endElementUID);


    typedef void (CTiglFuselageSectionElement::*pSetProperty)(double, TiglCoordinateSystem referenceCS);

    // functional programing: apply the "set" func on each element using the value "value" as parameter
    double ApplyFunctionBetween(pSetProperty func, double value, const std::string& startElementUID, const std::string& endElementUID);

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

    friend class CCPACSFuselageSegment;
};

TIGL_EXPORT TopoDS_Shape transformFuselageProfileGeometry(const CTiglTransformation& fuselTransform, const CTiglFuselageConnection& connection, const TopoDS_Shape& shape);

} // end namespace tigl

#endif // CCPACSFUSELAGE_H
