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

#include <string>

#include "tigl_internal.h"
#include "tigl_config.h"
#include "tixi.h"
#include "CCPACSFuselageConnection.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSGuideCurves.h"

#include "TopoDS_Shape.hxx"
#include "TopTools_SequenceOfShape.hxx"



namespace tigl
{

class CCPACSFuselage;

class CCPACSFuselageSegment : public CTiglAbstractSegment, public IGuideCurveBuilder
{

public:
    // Constructor
    TIGL_EXPORT CCPACSFuselageSegment(CCPACSFuselage* aFuselage, int aSegmentIndex);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSFuselageSegment(void);

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath);

    // Write CPACS segment elements
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath);

    // Returns the fuselage this segment belongs to
    TIGL_EXPORT CCPACSFuselage& GetFuselage(void) const;

    // Returns the segment index of this segment
    TIGL_EXPORT int GetSegmentIndex(void) const;

    // Returns the start section UID of this segment
    TIGL_EXPORT const std::string& GetStartSectionUID(void);

    // Returns the end section UID of this segment
    TIGL_EXPORT const std::string& GetEndSectionUID(void);

    // Returns the start section index of this segment
    TIGL_EXPORT int GetStartSectionIndex(void);

    // Returns the end section index of this segment
    TIGL_EXPORT int GetEndSectionIndex(void);

    // Returns the starting Segement Connection
    TIGL_EXPORT CCPACSFuselageConnection& GetStartConnection(void);

    // Return the end Segment Connection
    TIGL_EXPORT CCPACSFuselageConnection& GetEndConnection(void);

    // Returns the start section element UID of this segment
    TIGL_EXPORT const std::string& GetStartSectionElementUID(void);

    // Returns the end section element UID of this segment
    TIGL_EXPORT const std::string& GetEndSectionElementUID(void);

    // Returns the start section element index of this segment
    TIGL_EXPORT int GetStartSectionElementIndex(void);

    // Returns the end section element index of this segment
    TIGL_EXPORT int GetEndSectionElementIndex(void);

    // Gets the count of segments connected to the start section of this segment
    TIGL_EXPORT int GetStartConnectedSegmentCount(void);

    // Gets the count of segments connected to the end section of this segment
    TIGL_EXPORT int GetEndConnectedSegmentCount(void);

    // Gets the index (number) of the n-th segment connected to the start section
    // of this segment. n starts at 1.
    TIGL_EXPORT int GetStartConnectedSegmentIndex(int n);

    // Gets the index (number) of the n-th segment connected to the end section
    // of this segment. n starts at 1.
    TIGL_EXPORT int GetEndConnectedSegmentIndex(int n);

    // helper function to get the wire of the start section
    TIGL_EXPORT TopoDS_Wire GetStartWire(void);

    // helper function to get the wire of the end section
    TIGL_EXPORT TopoDS_Wire GetEndWire(void);

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

    TIGL_EXPORT TiglGeometricComponentType GetComponentType(){return TIGL_COMPONENT_FUSELSEGMENT | TIGL_COMPONENT_SEGMENT | TIGL_COMPONENT_LOGICAL;}

    TIGL_EXPORT CCPACSGuideCurves& GetGuideCurveSegments();
    
    // builds all guide curve segments
    TIGL_EXPORT void BuildGuideCurve(CCPACSGuideCurve*);

protected:
    // Cleanup routine
    void Cleanup(void);

    // Update internal segment data
    void Update(void);

    // Builds the loft between the two segment sections
    PNamedShape BuildLoft(void);

    void SetFaceTraits(PNamedShape loft, bool hasSymmetryPlane);
private:
    // get short name for loft
    std::string GetShortShapeName(void);

    // Copy constructor
    CCPACSFuselageSegment(const CCPACSFuselageSegment& );

    // Assignment operator
    void operator=(const CCPACSFuselageSegment& );

    std::string              name;                 /**< Segment name                            */
    CCPACSFuselageConnection startConnection;      /**< Start segment connection                */
    CCPACSFuselageConnection endConnection;        /**< End segment connection                  */
    CCPACSFuselage*          fuselage;             /**< Parent fuselage                         */
    CCPACSGuideCurves        guideCurves;          /**< Guide curve container                   */
    double                   myVolume;             /**< Volume of this segment                  */
    double                   mySurfaceArea;        /**< Surface Area of this segment            */
    double                   myWireLength;         /**< Wire length of this segment for a given zeta */
    bool                     guideCurvesPresent;   /**< If guide curves are not present, lofted surface is possible */
    bool                     guideCurvesBuilt;     /**< True, if guide curves are already built                     */

};

} // end namespace tigl

#endif // CCPACSFUSELAGESEGMENT_H
