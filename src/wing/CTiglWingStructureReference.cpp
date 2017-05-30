/*
* Copyright (C) 2016 Airbus Defense and Space
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
#include "CTiglWingStructureReference.h"

#include "CTiglLogging.h"
//#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"


namespace tigl
{
//CTiglWingStructureReference::CTiglWingStructureReference(CCPACSTrailingEdgeDevice& parent)
//: type(TrailingEdgeDeviceType), trailingEdgeDevice(&parent) { }

CTiglWingStructureReference::CTiglWingStructureReference(CCPACSWingComponentSegment& parent)
: type(ComponentSegmentType), componentSegment(&parent) { }

#define DISPATCH(call) \
    switch (type) { \
        case ComponentSegmentType: return componentSegment->call; \
/*        case TrailingEdgeDeviceType: return trailingEdgeDevice->call; */ \
        default: throw CTiglError("Internal Error in CTiglWingStructureReference: unknown type passed to DISPATCH macro!"); \
    }

CCPACSWing& CTiglWingStructureReference::GetWing() const
{
    DISPATCH(GetWing())
}

boost::optional<CCPACSWingCSStructure>& CTiglWingStructureReference::GetStructure()
{
    DISPATCH(GetStructure())
}

const boost::optional<CCPACSWingCSStructure>& CTiglWingStructureReference::GetStructure() const
{
    DISPATCH(GetStructure())
}

PNamedShape CTiglWingStructureReference::GetLoft() const
{
    DISPATCH(GetLoft())
}

gp_Pnt CTiglWingStructureReference::GetPoint(double eta, double xsi, TiglCoordinateSystem reference) const
{
    DISPATCH(GetPoint(eta, xsi, reference))
}

double CTiglWingStructureReference::GetLeadingEdgeLength() const
{
    DISPATCH(GetLeadingEdgeLength())
}

double CTiglWingStructureReference::GetTrailingEdgeLength() const
{
    DISPATCH(GetTrailingEdgeLength())
}

gp_Pnt CTiglWingStructureReference::GetLeadingEdgePoint(double eta) const
{
    DISPATCH(GetLeadingEdgePoint(eta))
}

gp_Pnt CTiglWingStructureReference::GetTrailingEdgePoint(double eta) const
{
    DISPATCH(GetTrailingEdgePoint(eta))
}

gp_Vec CTiglWingStructureReference::GetLeadingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    DISPATCH(GetLeadingEdgeDirection(point, defaultSegmentUID))
}

gp_Vec CTiglWingStructureReference::GetTrailingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    DISPATCH(GetTrailingEdgeDirection(point, defaultSegmentUID))
}

TopoDS_Wire CTiglWingStructureReference::GetLeadingEdgeLine() const
{
    DISPATCH(GetLeadingEdgeLine())
}

TopoDS_Wire CTiglWingStructureReference::GetTrailingEdgeLine() const
{
    DISPATCH(GetTrailingEdgeLine())
}

void CTiglWingStructureReference::GetMidplaneEtaXsi(const gp_Pnt& p, double& eta, double& xsi) const
{
    DISPATCH(GetMidplaneEtaXsi(p, eta, xsi));
}

gp_Vec CTiglWingStructureReference::GetMidplaneEtaDir(double eta) const
{
    DISPATCH(GetMidplaneEtaDir(eta))
}

gp_Vec CTiglWingStructureReference::GetMidplaneNormal(double eta) const
{
    DISPATCH(GetMidplaneNormal(eta))
}

TopoDS_Shape CTiglWingStructureReference::GetMidplaneShape() const
{
    DISPATCH(GetMidplaneShape())
}

TopoDS_Shape CTiglWingStructureReference::GetUpperShape() const
{
    DISPATCH(GetUpperShape())
}

TopoDS_Shape CTiglWingStructureReference::GetLowerShape() const
{
    DISPATCH(GetLowerShape())
}

TopoDS_Face CTiglWingStructureReference::GetInnerFace() const
{
    DISPATCH(GetInnerFace())
}

TopoDS_Face CTiglWingStructureReference::GetOuterFace() const
{
    DISPATCH(GetOuterFace())
}

TopoDS_Wire CTiglWingStructureReference::GetMidplaneLine(const gp_Pnt& startPoint, const gp_Pnt& endPoint) const
{
    DISPATCH(GetMidplaneLine(startPoint, endPoint))
}

const std::string& CTiglWingStructureReference::GetUID() const
{
    DISPATCH(GetUID())
}

CCPACSWingComponentSegment& CTiglWingStructureReference::GetWingComponentSegment() const
{
    if (type != ComponentSegmentType) {
        LOG(ERROR) << "CTiglWingStructureReference is not a CCPACSWingComponentSegment";
        throw CTiglError("CTiglWingStructureReference is not a CCPACSWingComponentSegment");
    }
    return *componentSegment;
}

//CCPACSTrailingEdgeDevice& CTiglWingStructureReference::GetTrailingEdgeDevice() const
//{
//    if (type != TrailingEdgeDeviceType) {
//        LOG(ERROR) << "CTiglWingStructureReference is not a CCPACSTrailingEdgeDevice";
//        throw CTiglError("CTiglWingStructureReference is not a CCPACSTrailingEdgeDevice");
//    }
//    return *trailingEdgeDevice;
//}

}
