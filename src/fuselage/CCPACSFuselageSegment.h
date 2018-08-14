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
* @brief  Implementation of CPACS fuselage segment handling routines.
*/

#ifndef CCPACSFUSELAGESEGMENT_H
#define CCPACSFUSELAGESEGMENT_H

#include "generated/CPACSFuselageSegment.h"

#include "tigl_internal.h"
#include "tigl_config.h"
#include "tixi.h"
#include "CTiglFuselageConnection.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSGuideCurve.h"
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
    TIGL_EXPORT CCPACSFuselageSegment(CCPACSFuselageSegments* parent, CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT ~CCPACSFuselageSegment() OVERRIDE;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& segmentXPath) OVERRIDE;

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT void SetFromElementUID(const std::string& value) OVERRIDE;
    TIGL_EXPORT void SetToElementUID(const std::string& value) OVERRIDE;

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
    TIGL_EXPORT CTiglFuselageConnection& GetStartConnection();

    // Return the end Segment Connection
    TIGL_EXPORT CTiglFuselageConnection& GetEndConnection();

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
    TIGL_EXPORT TopoDS_Wire GetStartWire(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // helper function to get the wire of the end section
    TIGL_EXPORT TopoDS_Wire GetEndWire(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Gets a point on the fuselage segment in dependence of parameters eta and zeta with
    // 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
    // profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
    // the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
    TIGL_EXPORT gp_Pnt GetPoint(double eta, double zeta);

    // Gets the origin (0, 0, 0) of the inner & outer profiles after trafo
    // These should be a good approximation for the center point
    TIGL_EXPORT gp_Pnt GetTransformedProfileOriginStart() const;
    TIGL_EXPORT gp_Pnt GetTransformedProfileOriginEnd() const;

    TIGL_EXPORT int GetNumPointsOnYPlane(double eta, double ypos);
    TIGL_EXPORT gp_Pnt GetPointOnYPlane(double eta, double ypos, int pointIndex);

    TIGL_EXPORT int GetNumPointsOnXPlane(double eta, double xpos);
    TIGL_EXPORT gp_Pnt GetPointOnXPlane(double eta, double xpos, int pointIndex);

    // Gets a point on the fuselage segment in dependence of an angle alpha (degree).
    // The origin of the angle could be set via the parameters y_cs and z_cs.
    // y_cs and z_cs are assumed to be offsets from the cross section center. Set absolute = true
    // if the values should be interpreted as absolute coordinates
    TIGL_EXPORT gp_Pnt GetPointAngle(double eta, double alpha, double y_cs = 0.0, double z_cs=0.0, bool absolute = false);

    // Gets the volume of this segment
    TIGL_EXPORT double GetVolume();

    // Gets the surface area of this segment
    TIGL_EXPORT double GetSurfaceArea();

    // Gets the wire on the loft at a given eta
    TIGL_EXPORT TopoDS_Shape getWireOnLoft(double eta);

    // Returns the circumference if the segment at a given eta
    TIGL_EXPORT double GetCircumference(const double eta);

    // Returns the inner profile points as read from TIXI. The points are already transformed.
    TIGL_EXPORT std::vector<CTiglPoint> GetRawStartProfilePoints();

    // Returns the outer profile points as read from TIXI. The points are already transformed.
    TIGL_EXPORT std::vector<CTiglPoint> GetRawEndProfilePoints();

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE { return TIGL_COMPONENT_FUSELSEGMENT | TIGL_COMPONENT_SEGMENT | TIGL_COMPONENT_LOGICAL; }

    // Returns the number of faces in the loft. This depends on the number of guide curves as well as if the fuselage has a symmetry plane.
    TIGL_EXPORT int GetNumberOfLoftFaces() const;

private:
    struct SurfacePropertiesCache {
        double myVolume;      ///< Volume of this segment
        double mySurfaceArea; ///< Surface Area of this segment
    };

    // Cleanup routine
    void Cleanup();

    // Builds the loft between the two segment sections
    PNamedShape BuildLoft() const OVERRIDE;

    void SetFaceTraits(PNamedShape loft) const;

    void UpdateSurfaceProperties(SurfacePropertiesCache& cache) const;

private:
    // get short name for loft
    std::string GetShortShapeName() const;

    CTiglFuselageConnection startConnection;      /**< Start segment connection                */
    CTiglFuselageConnection endConnection;        /**< End segment connection                  */
    CCPACSFuselage*         fuselage;             /**< Parent fuselage                         */
    Cache<SurfacePropertiesCache, CCPACSFuselageSegment> surfacePropertiesCache;
    bool                    loftLinearly = false; /**< Set to true to speed up lofting of the
                                                    * segment. This removes the dependency on
                                                    * the fuselage loft at the price of a
                                                    * nonsmooth fuselage                       */

    unique_ptr<IGuideCurveBuilder> m_guideCurveBuilder;
};

} // end namespace tigl

#endif // CCPACSFUSELAGESEGMENT_H
