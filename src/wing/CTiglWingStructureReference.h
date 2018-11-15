/* 
* Copyright (C) 2014 Airbus Defense and Space
*
* Created: 2014-06-13 Roland Landertshamer
*
* Changed: $Id $ 
*
* Version: $Revision $
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
#ifndef CTIGLWINGSTRUCTUREREFERENCE_H
#define CTIGLWINGSTRUCTUREREFERENCE_H

#include <boost/optional.hpp>

#include "tigl_internal.h"
#include "tigl.h"

#include "PNamedShape.h"
#include "TopoDS_Shape.hxx"
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Face.hxx"

namespace tigl
{

class CCPACSWing;
class CCPACSWingSparSegment;
class CCPACSWingRibsDefinition;
class CCPACSWingCSStructure;
//class CCPACSTrailingEdgeDevice;
class CCPACSWingComponentSegment;
class CCPACSWingStructure;

// TODO(bgruber): rename to WingStructureParent, this class is NOT a reference to a WingStructure
// this is a set type, encapsulating a reference to either a CCPACSTrailingEdgeDevice or a CCPACSWingComponentSegment
class CTiglWingStructureReference
{
public:
    // Enumeration for stored object type
    enum Type
    {
        ComponentSegmentType,
        TrailingEdgeDeviceType
    };

//    TIGL_EXPORT CTiglWingStructureReference(const CCPACSTrailingEdgeDevice& parent);
    TIGL_EXPORT CTiglWingStructureReference(const CCPACSWingComponentSegment& parent);
    TIGL_EXPORT CTiglWingStructureReference(const CCPACSWingCSStructure& structure);

    TIGL_EXPORT Type GetType() const;
    TIGL_EXPORT const CCPACSWing& GetWing() const;
    TIGL_EXPORT const boost::optional<CCPACSWingCSStructure>& GetStructure() const;
    TIGL_EXPORT PNamedShape GetLoft(TiglCoordinateSystem reference = WING_COORDINATE_SYSTEM) const;
    TIGL_EXPORT gp_Pnt GetPoint(double eta, double xsi, TiglCoordinateSystem reference) const;
    TIGL_EXPORT double GetLeadingEdgeLength() const;
    TIGL_EXPORT double GetTrailingEdgeLength() const;
    TIGL_EXPORT gp_Pnt GetLeadingEdgePoint(double relativePos) const;
    TIGL_EXPORT gp_Pnt GetTrailingEdgePoint(double relativePos) const;
    TIGL_EXPORT gp_Vec GetLeadingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID = "") const;
    TIGL_EXPORT gp_Vec GetTrailingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID = "") const;
    TIGL_EXPORT TopoDS_Wire GetLeadingEdgeLine() const;
    TIGL_EXPORT TopoDS_Wire GetTrailingEdgeLine() const;
    TIGL_EXPORT void GetEtaXsiLocal(const gp_Pnt& p, double& eta, double& xsi) const;
    TIGL_EXPORT gp_Vec GetMidplaneEtaDir(double eta) const;
    TIGL_EXPORT gp_Vec GetMidplaneNormal(double eta) const;
    TIGL_EXPORT TopoDS_Shape GetMidplaneShape() const;
    TIGL_EXPORT TopoDS_Shape GetUpperShape(TiglCoordinateSystem reference = WING_COORDINATE_SYSTEM) const;
    TIGL_EXPORT TopoDS_Shape GetLowerShape(TiglCoordinateSystem reference = WING_COORDINATE_SYSTEM) const;
    TIGL_EXPORT TopoDS_Face GetInnerFace() const;
    TIGL_EXPORT TopoDS_Face GetOuterFace() const;
    TIGL_EXPORT TopoDS_Wire GetMidplaneLine(const gp_Pnt& startPoint, const gp_Pnt& endPoint) const;
    TIGL_EXPORT const std::string& GetUID() const;

    TIGL_EXPORT const CCPACSWingComponentSegment& GetWingComponentSegment() const; // throws if not a component segment
//    TIGL_EXPORT const CCPACSTrailingEdgeDevice& GetTrailingEdgeDevice() const;

private:
    Type type;

    // pointer to target object
    union
    {
        const CCPACSWingComponentSegment* componentSegment;
//        const CCPACSTrailingEdgeDevice* trailingEdgeDevice;
    };
};

} // end namespace tigl

#endif // CTIGLWINGSTRUCTUREREFERENCE_H
