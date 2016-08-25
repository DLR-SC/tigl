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
#include "TiglWingStructureReference.h"

#include "CTiglLogging.h"
//#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"


namespace tigl
{
//TiglWingStructureReference::TiglWingStructureReference(CCPACSTrailingEdgeDevice& parent)
//: type(TrailingEdgeDeviceType), trailingEdgeDevice(&parent) { }

TiglWingStructureReference::TiglWingStructureReference(CCPACSWingComponentSegment& parent)
: type(ComponentSegmentType), componentSegment(&parent) { }

#define DISPATCH(call) \
    switch (type) { \
        case ComponentSegmentType: return componentSegment->call; \
/*        case TrailingEdgeDeviceType: return trailingEdgeDevice->call; */ \
        default: std::terminate(); \
    }

CCPACSWing& TiglWingStructureReference::GetWing() const
{
    DISPATCH(GetWing())
}

bool TiglWingStructureReference::HasStructure() const
{
    DISPATCH(HasStructure())
}

CCPACSWingCSStructure& TiglWingStructureReference::GetStructure()
{
    DISPATCH(GetStructure())
}

const CCPACSWingCSStructure& TiglWingStructureReference::GetStructure() const
{
    DISPATCH(GetStructure())
}


PNamedShape TiglWingStructureReference::GetLoft() const
{
    DISPATCH(GetLoft())
}

gp_Pnt TiglWingStructureReference::GetPoint(double eta, double xsi) const
{
    DISPATCH(GetPoint(eta, xsi))
}

double TiglWingStructureReference::GetLeadingEdgeLength() const
{
    DISPATCH(GetLeadingEdgeLength())
}

double TiglWingStructureReference::GetTrailingEdgeLength() const
{
    DISPATCH(GetTrailingEdgeLength())
}

gp_Pnt TiglWingStructureReference::GetLeadingEdgePoint(double eta) const
{
    DISPATCH(GetLeadingEdgePoint(eta))
}

gp_Pnt TiglWingStructureReference::GetTrailingEdgePoint(double eta) const
{
    DISPATCH(GetTrailingEdgePoint(eta))
}

gp_Vec TiglWingStructureReference::GetLeadingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    DISPATCH(GetLeadingEdgeDirection(point, defaultSegmentUID))
}

gp_Vec TiglWingStructureReference::GetTrailingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    DISPATCH(GetTrailingEdgeDirection(point, defaultSegmentUID))
}

TopoDS_Wire TiglWingStructureReference::GetLeadingEdgeLine() const
{
    DISPATCH(GetLeadingEdgeLine())
}

TopoDS_Wire TiglWingStructureReference::GetTrailingEdgeLine() const
{
    DISPATCH(GetTrailingEdgeLine())
}

void TiglWingStructureReference::GetMidplaneEtaXsi(const gp_Pnt& p, double& eta, double& xsi) const
{
    DISPATCH(GetMidplaneEtaXsi(p, eta, xsi));
}

gp_Vec TiglWingStructureReference::GetMidplaneEtaDir(double eta) const
{
    DISPATCH(GetMidplaneEtaDir(eta))
}

gp_Vec TiglWingStructureReference::GetMidplaneNormal(double eta) const
{
    DISPATCH(GetMidplaneNormal(eta))
}

gp_Pnt TiglWingStructureReference::GetMidplaneOrChordlinePoint(double eta, double xsi) const
{
    DISPATCH(GetMidplaneOrChordlinePoint(eta, xsi))
}

TopoDS_Shape TiglWingStructureReference::GetMidplaneShape() const
{
    DISPATCH(GetMidplaneShape())
}

TopoDS_Shape TiglWingStructureReference::GetUpperShape() const
{
    DISPATCH(GetUpperShape())
}

TopoDS_Shape TiglWingStructureReference::GetLowerShape() const
{
    DISPATCH(GetLowerShape())
}

TopoDS_Face TiglWingStructureReference::GetInnerFace() const
{
    DISPATCH(GetInnerFace())
}

TopoDS_Face TiglWingStructureReference::GetOuterFace() const
{
    DISPATCH(GetOuterFace())
}

TopoDS_Wire TiglWingStructureReference::GetMidplaneLine(const gp_Pnt& startPoint, const gp_Pnt& endPoint) const
{
    DISPATCH(GetMidplaneLine(startPoint, endPoint))
}

const std::string& TiglWingStructureReference::GetUID() const
{
    DISPATCH(GetUID())
}

CCPACSWingComponentSegment& TiglWingStructureReference::GetWingComponentSegment() const
{
    if (type != ComponentSegmentType) {
        LOG(ERROR) << "TiglWingStructureReference is not a CCPACSWingComponentSegment";
        throw CTiglError("TiglWingStructureReference is not a CCPACSWingComponentSegment");
    }
    return *componentSegment;
}

//CCPACSTrailingEdgeDevice& TiglWingStructureReference::GetTrailingEdgeDevice() const
//{
//    if (type != TrailingEdgeDeviceType) {
//        LOG(ERROR) << "TiglWingStructureReference is not a CCPACSTrailingEdgeDevice";
//        throw CTiglError("TiglWingStructureReference is not a CCPACSTrailingEdgeDevice");
//    }
//    return *trailingEdgeDevice;
//}

}
