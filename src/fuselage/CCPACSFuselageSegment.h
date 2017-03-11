/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
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
* @brief  Implementation of CPACS fuselage segment handling routines.
*/

#ifndef CCPACSFUSELAGESEGMENT_H
#define CCPACSFUSELAGESEGMENT_H

#include "generated/CPACSFuselageSegment.h"

#include "tigl_internal.h"
#include "tigl_config.h"
#include "tixi.h"
#include "CCPACSFuselageConnection.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSGuideCurves.h"
#include "CCPACSTransformation.h"

#include "TopoDS_Shape.hxx"
#include "TopTools_SequenceOfShape.hxx"

namespace tigl
{
class CCPACSFuselage;

class CCPACSFuselageSegment : public generated::CPACSFuselageSegment, public CTiglAbstractSegment<CCPACSFuselageSegment>
{

public:
    // Constructor
    TIGL_EXPORT CCPACSFuselageSegment(CCPACSFuselageSegments* parent);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSFuselageSegment();

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath);

    TIGL_EXPORT virtual const std::string& GetUID() const OVERRIDE;

    // Returns the fuselage this segment belongs to
    TIGL_EXPORT CCPACSFuselage& GetFuselage() const;

    // Returns the start section UID of this segment
    TIGL_EXPORT const std::string& GetStartSectionUID();

    // Returns the end section UID of this segment
    TIGL_EXPORT const std::string& GetEndSectionUID();

    // Returns the start section index of this segment
    TIGL_EXPORT int GetStartSectionIndex();

    // Returns the end section index of this segment
    TIGL_EXPORT int GetEndSectionIndex();

    // Returns the starting Segement Connection
    TIGL_EXPORT CCPACSFuselageConnection& GetStartConnection();

    // Return the end Segment Connection
    TIGL_EXPORT CCPACSFuselageConnection& GetEndConnection();

    // Returns the start section element UID of this segment
    TIGL_EXPORT const std::string& GetStartSectionElementUID();

    // Returns the end section element UID of this segment
    TIGL_EXPORT const std::string& GetEndSectionElementUID();

    // Returns the start section element index of this segment
    TIGL_EXPORT int GetStartSectionElementIndex();

    // Returns the end section element index of this segment
    TIGL_EXPORT int GetEndSectionElementIndex();

    // Gets the count of segments connected to the start section of this segment
    TIGL_EXPORT int GetStartConnectedSegmentCount();

    // Gets the count of segments connected to the end section of this segment
    TIGL_EXPORT int GetEndConnectedSegmentCount();

    // Gets the index (number) of the n-th segment connected to the start section
    // of this segment. n starts at 1.
    TIGL_EXPORT int GetStartConnectedSegmentIndex(int n);

    // Gets the index (number) of the n-th segment connected to the end section
    // of this segment. n starts at 1.
    TIGL_EXPORT int GetEndConnectedSegmentIndex(int n);

    // helper function to get the wire of the start section
    TIGL_EXPORT TopoDS_Wire GetStartWire();

    // helper function to get the wire of the end section
    TIGL_EXPORT TopoDS_Wire GetEndWire();

    // Gets a point on the fuselage segment in dependence of parameters eta and zeta with
    // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
    // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
    // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
    TIGL_EXPORT gp_Pnt GetPoint(double eta, double zeta);

    TIGL_EXPORT int GetNumPointsOnYPlane(double eta, double ypos);
    TIGL_EXPORT gp_Pnt GetPointOnYPlane(double eta, double ypos, int pointIndex);

    TIGL_EXPORT int GetNumPointsOnXPlane(double eta, double xpos);
    TIGL_EXPORT gp_Pnt GetPointOnXPlane(double eta, double xpos, int pointIndex);

    // Gets a point on the fuselage segment in dependence of an angle alpha (degree).
    // The origin of the angle could be set via the parameters y_cs and z_cs,
    // but in most cases y_cs and z_cs will be zero get the get center line of the profile.
    TIGL_EXPORT gp_Pnt GetPointAngle(double eta, double alpha, double y_cs, double z_cs);

    // Gets the volume of this segment
    TIGL_EXPORT double GetVolume();
        
    // Gets the surface area of this segment
    TIGL_EXPORT double GetSurfaceArea();

    // Gets the wire on the loft at a given eta
    TIGL_EXPORT TopoDS_Shape getWireOnLoft(double eta);

    // Returns the circumference if the segment at a given eta
    TIGL_EXPORT double GetCircumference(const double eta);

    // Returns the inner profile points as read from TIXI. The points are already transformed.
    TIGL_EXPORT std::vector<CTiglPoint*> GetRawStartProfilePoints();

    // Returns the outer profile points as read from TIXI. The points are already transformed.
    TIGL_EXPORT std::vector<CTiglPoint*> GetRawEndProfilePoints();

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const { return TIGL_COMPONENT_FUSELSEGMENT | TIGL_COMPONENT_SEGMENT | TIGL_COMPONENT_LOGICAL; }

    // builds all guide curve wires
    TIGL_EXPORT TopTools_SequenceOfShape& BuildGuideCurves();

    // get guide curve for given UID
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurve(std::string UID);

    // check if guide curve with a given UID exists
    TIGL_EXPORT bool GuideCurveExists(std::string UID);

protected:
    // Cleanup routine
    void Cleanup();

    // Builds the loft between the two segment sections
    PNamedShape BuildLoft();

private:
    // get short name for loft
    std::string GetShortShapeName();

    CCPACSFuselageConnection startConnection;      /**< Start segment connection                */
    CCPACSFuselageConnection endConnection;        /**< End segment connection                  */
    CCPACSFuselage*          fuselage;             /**< Parent fuselage                         */
    TopTools_SequenceOfShape guideCurveWires;      /**< container for the guide curve wires     */
    double                   myVolume;             /**< Volume of this segment                  */
    double                   mySurfaceArea;        /**< Surface Area of this segment            */
    double                   myWireLength;         /**< Wire length of this segment for a given zeta */
};

} // end namespace tigl

#endif // CCPACSFUSELAGESEGMENT_H
