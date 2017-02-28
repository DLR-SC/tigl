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

#include "tigl_internal.h"

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

// this is a set type, encapsulating a reference to either a CCPACSTrailingEdgeDevice or a CCPACSWingComponentSegment
class CTiglWingStructureReference
{
public:
//    TIGL_EXPORT CTiglWingStructureReference(CCPACSTrailingEdgeDevice& parent);
    TIGL_EXPORT CTiglWingStructureReference(CCPACSWingComponentSegment& parent);

    TIGL_EXPORT CCPACSWing& GetWing() const;
    TIGL_EXPORT bool HasStructure() const;
    TIGL_EXPORT CCPACSWingCSStructure& GetStructure();
    TIGL_EXPORT const CCPACSWingCSStructure& GetStructure() const;
    TIGL_EXPORT PNamedShape GetLoft() const;
    TIGL_EXPORT gp_Pnt GetPoint(double eta, double xsi) const;
    TIGL_EXPORT double GetLeadingEdgeLength() const;
    TIGL_EXPORT double GetTrailingEdgeLength() const;
    TIGL_EXPORT gp_Pnt GetLeadingEdgePoint(double relativePos) const;
    TIGL_EXPORT gp_Pnt GetTrailingEdgePoint(double relativePos) const;
    TIGL_EXPORT gp_Vec GetLeadingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID = "") const;
    TIGL_EXPORT gp_Vec GetTrailingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID = "") const;
    TIGL_EXPORT TopoDS_Wire GetLeadingEdgeLine() const;
    TIGL_EXPORT TopoDS_Wire GetTrailingEdgeLine() const;
    TIGL_EXPORT void GetMidplaneEtaXsi(const gp_Pnt& p, double& eta, double& xsi) const;
    TIGL_EXPORT gp_Vec GetMidplaneEtaDir(double eta) const;
    TIGL_EXPORT gp_Vec GetMidplaneNormal(double eta) const;
    TIGL_EXPORT gp_Pnt GetMidplaneOrChordlinePoint(double eta, double xsi) const;
    TIGL_EXPORT TopoDS_Shape GetMidplaneShape() const;
    TIGL_EXPORT TopoDS_Shape GetUpperShape() const;
    TIGL_EXPORT TopoDS_Shape GetLowerShape() const;
    TIGL_EXPORT TopoDS_Face GetInnerFace() const;
    TIGL_EXPORT TopoDS_Face GetOuterFace() const;
    TIGL_EXPORT TopoDS_Wire GetMidplaneLine(const gp_Pnt& startPoint, const gp_Pnt& endPoint) const;
    TIGL_EXPORT const std::string& GetUID() const;

    TIGL_EXPORT CCPACSWingComponentSegment& GetWingComponentSegment() const;
//    TIGL_EXPORT CCPACSTrailingEdgeDevice& GetTrailingEdgeDevice() const;

private:
    // Enumeration for stored object type
    enum Type
    {
        ComponentSegmentType,
//        TrailingEdgeDeviceType
    };
    Type type;

    // pointer to target object
    union
    {
        CCPACSWingComponentSegment* componentSegment;
//        CCPACSTrailingEdgeDevice* trailingEdgeDevice;
    };
};

} // end namespace tigl

#endif // CTIGLWINGSTRUCTUREREFERENCE_H
