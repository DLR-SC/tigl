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

#include "TopoDS_Shape.hxx"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"

namespace tigl
{
class CCPACSConfiguration;

class CCPACSFuselage : public generated::CPACSFuselage, public CTiglRelativelyPositionedComponent
{
public:
    // Constructor
    TIGL_EXPORT CCPACSFuselage(CCPACSFuselages* parent, CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSFuselage();

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS fuselage elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath);

    TIGL_EXPORT virtual void SetUID(const std::string& uid) OVERRIDE;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration & GetConfiguration() const;

    TIGL_EXPORT virtual std::string GetDefaultedUID() const OVERRIDE;

    // Get section count
    TIGL_EXPORT int GetSectionCount() const;

    // Returns the section for a given index
    TIGL_EXPORT CCPACSFuselageSection& GetSection(int index) const;

    // Get segment count
    TIGL_EXPORT int GetSegmentCount() const;

    // Returns the segment for a given index
    TIGL_EXPORT CCPACSFuselageSegment& GetSegment(const int index);

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
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const {return TIGL_COMPONENT_FUSELAGE | TIGL_COMPONENT_PHYSICAL;}

    // Returns the point where the distance between the selected fuselage and the ground is at minimum.
    // The Fuselage could be turned with a given angle at at given axis, specified by a point and a direction.
    TIGL_EXPORT gp_Pnt GetMinumumDistanceToGround(gp_Ax1 RAxis, double angle);

    // Get the guide curve segment(partial guide curve) with a given UID
    TIGL_EXPORT CCPACSGuideCurve& GetGuideCurveSegment(std::string uid);

    // Returns all guide curve wires as a compound
    TIGL_EXPORT TopoDS_Compound& GetGuideCurveWires();

protected:
    void BuildGuideCurves();

    void ConnectGuideCurveSegments();

    // Cleanup routine
    void Cleanup();

    // Adds all segments of this fuselage to one shape
    PNamedShape BuildLoft();
    
    void SetFaceTraits(PNamedShape loft, bool hasSymmetryPlane, bool smoothSurface);

private:
    // get short name for loft
    std::string GetShortShapeName();

private:
    CCPACSConfiguration*       configuration;        /**< Parent configuration    */
    FusedElementsContainerType fusedElements;        /**< Stores already fused segments */

    TopoDS_Compound            aCompound;
    TopoDS_Compound            guideCurves;
    BRep_Builder               aBuilder;
    double                     myVolume;             /**< Volume of this fuselage              */

    friend class CCPACSFuselageSegment;
};

TIGL_EXPORT TopoDS_Shape transformFuselageProfileGeometry(const CTiglTransformation& fuselTransform, const CTiglFuselageConnection& connection, const TopoDS_Shape& shape);

} // end namespace tigl

#endif // CCPACSFUSELAGE_H
