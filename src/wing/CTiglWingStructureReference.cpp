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
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingCSStructure.h"
#include "CCPACSWing.h"
#include "CNamedShape.h"
#include <BRepTools.hxx>


namespace tigl
{
//CTiglWingStructureReference::CTiglWingStructureReference(const CCPACSTrailingEdgeDevice& parent)
//: type(TrailingEdgeDeviceType), trailingEdgeDevice(&parent)
//{
//}

CTiglWingStructureReference::CTiglWingStructureReference(const CCPACSWingComponentSegment& parent)
: type(ComponentSegmentType), componentSegment(&parent)
{
}

CTiglWingStructureReference::CTiglWingStructureReference(const CCPACSWingCSStructure& structure)
{
    if (structure.IsParent<CCPACSWingComponentSegment>()) {
        type = ComponentSegmentType;
        componentSegment = structure.GetParent<CCPACSWingComponentSegment>();
    }
    else if (structure.IsParent<CCPACSTrailingEdgeDevice>()) {
        throw CTiglError("Trailing edge devices are not yet supported by CTiglWingStructureReference");
        //type = TrailingEdgeDeviceType;
        //trailingEdgeDevice = structure.GetParent<CCPACSTrailingEdgeDevice>();
    }
    else {
        throw CTiglError("Unrecognized parent of CCPACSWingCSStructure");
    }
}

#define DISPATCH(call)                                                                                                 \
    switch (type) {                                                                                                    \
    case ComponentSegmentType:                                                                                         \
        return componentSegment->call;                                                                                 \
    /*case TrailingEdgeDeviceType:                                                                                       \
        return trailingEdgeDevice->call;*/                                                                               \
    default:                                                                                                           \
        throw CTiglError("Internal Error in CTiglWingStructureReference: unknown type passed to DISPATCH macro!");     \
    }

CTiglWingStructureReference::Type CTiglWingStructureReference::GetType() const
{
    return type;
}

const CCPACSWing& CTiglWingStructureReference::GetWing() const
{
    DISPATCH(GetWing())
}

const boost::optional<CCPACSWingCSStructure>& CTiglWingStructureReference::GetStructure() const
{
    DISPATCH(GetStructure())
}

PNamedShape CTiglWingStructureReference::GetLoft(TiglCoordinateSystem reference) const
{
    PNamedShape loft;
    switch (type) {
        case ComponentSegmentType:
            loft = componentSegment->GetLoft()->DeepCopy();
            break;
/*        case TrailingEdgeDeviceType:
            loft = trailingEdgeDevice->GetLoft()->DeepCopy();
            break;*/
        default: throw CTiglError("Internal Error in CTiglWingStructureReference: unknown type passed to GetLoft method!");
    }

    // apply inverse wing transform to go to local coordinates
    CTiglTransformation transform;

    switch(reference) {
    case WING_COORDINATE_SYSTEM:
        // apply inverse wing transform to go to local coordinates
        transform = GetWing().GetTransformation().getTransformationMatrix().Inverted();
        loft->SetShape(transform.Transform(loft->Shape()));
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        break;
    default:
        throw CTiglError("Wrong coordinate system given in CTiglWingStructureReference::GetLoft");
    }

    return loft;
}

gp_Pnt CTiglWingStructureReference::GetPoint(double eta, double xsi, TiglCoordinateSystem reference) const
{
    DISPATCH(GetPoint(eta, xsi, reference));
}

double CTiglWingStructureReference::GetLeadingEdgeLength() const
{
    DISPATCH(GetLeadingEdgeLength());
}

double CTiglWingStructureReference::GetTrailingEdgeLength() const
{
    DISPATCH(GetTrailingEdgeLength());
}

gp_Pnt CTiglWingStructureReference::GetLeadingEdgePoint(double eta) const
{
    DISPATCH(GetLeadingEdgePoint(eta));
}

gp_Pnt CTiglWingStructureReference::GetTrailingEdgePoint(double eta) const
{
    DISPATCH(GetTrailingEdgePoint(eta));
}

gp_Vec CTiglWingStructureReference::GetLeadingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    DISPATCH(GetLeadingEdgeDirection(point, defaultSegmentUID));
}

gp_Vec CTiglWingStructureReference::GetTrailingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    DISPATCH(GetTrailingEdgeDirection(point, defaultSegmentUID));
}

TopoDS_Wire CTiglWingStructureReference::GetLeadingEdgeLine() const
{
    DISPATCH(GetLeadingEdgeLine());
}

TopoDS_Wire CTiglWingStructureReference::GetTrailingEdgeLine() const
{
    DISPATCH(GetTrailingEdgeLine())
}

void CTiglWingStructureReference::GetEtaXsiLocal(const gp_Pnt& p, double& eta, double& xsi) const
{
    DISPATCH(GetEtaXsiLocal(p, eta, xsi));
}

gp_Vec CTiglWingStructureReference::GetMidplaneEtaDir(double eta) const
{
    DISPATCH(GetMidplaneEtaDir(eta));
}

gp_Vec CTiglWingStructureReference::GetMidplaneNormal(double eta) const
{
    DISPATCH(GetMidplaneNormal(eta));
}

TopoDS_Shape CTiglWingStructureReference::GetMidplaneShape() const
{
    DISPATCH(GetMidplaneShape());
}

TopoDS_Shape CTiglWingStructureReference::GetUpperShape(TiglCoordinateSystem reference) const
{
    TopoDS_Shape loft;
    switch (type) {
        case ComponentSegmentType:
            loft = componentSegment->GetUpperShape()->Shape();
            break;
/*        case TrailingEdgeDeviceType:
            loft = trailingEdgeDevice->GetUpperShape();
            break;*/
        default:
            throw CTiglError("Internal Error in CTiglWingStructureReference: unknown type passed to GetUpperShape method!");
    }

    CTiglTransformation transform;

    switch(reference) {
    case WING_COORDINATE_SYSTEM:
        // apply inverse wing transform to go to local coordinates
        transform = GetWing().GetTransformation().getTransformationMatrix().Inverted();
        loft = transform.Transform(loft);
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        break;
    default:
        throw CTiglError("Wrong coordinate system given in CTiglWingStructureReference::GetUpperShape");
    }

    return loft;
}

TopoDS_Shape CTiglWingStructureReference::GetLowerShape(TiglCoordinateSystem reference) const
{
    TopoDS_Shape loft;
    switch (type) {
        case ComponentSegmentType:
            loft = componentSegment->GetLowerShape()->Shape();
            break;
/*        case TrailingEdgeDeviceType:
            loft = trailingEdgeDevice->GetLowerShape();
            break;*/
        default:
            throw CTiglError("Internal Error in CTiglWingStructureReference: unknown type passed to GetLowerShape method!");
    }

    CTiglTransformation transform;

    switch(reference) {
    case WING_COORDINATE_SYSTEM:
        // apply inverse wing transform to go to local coordinates
        transform = GetWing().GetTransformation().getTransformationMatrix().Inverted();
        loft = transform.Transform(loft);
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        break;
    default:
        throw CTiglError("Wrong coordinate system given in CTiglWingStructureReference::GetLowerShape");
    }

    return loft;
}

TopoDS_Face CTiglWingStructureReference::GetInnerFace() const
{
    DISPATCH(GetInnerFace());
}

TopoDS_Face CTiglWingStructureReference::GetOuterFace() const
{
    DISPATCH(GetOuterFace());
}

TopoDS_Wire CTiglWingStructureReference::GetMidplaneLine(const gp_Pnt& startPoint, const gp_Pnt& endPoint) const
{
    DISPATCH(GetMidplaneLine(startPoint, endPoint))
}

const std::string& CTiglWingStructureReference::GetUID() const
{
    DISPATCH(GetUID());
}

const CCPACSWingComponentSegment& CTiglWingStructureReference::GetWingComponentSegment() const
{
    if (type != ComponentSegmentType) {
        LOG(ERROR) << "CTiglWingStructureReference is not a CCPACSWingComponentSegment";
        throw CTiglError("CTiglWingStructureReference is not a CCPACSWingComponentSegment");
    }
    return *componentSegment;
}

//const CCPACSTrailingEdgeDevice& CTiglWingStructureReference::GetTrailingEdgeDevice() const
//{
//    if (type != TrailingEdgeDeviceType) {
//        LOG(ERROR) << "CTiglWingStructureReference is not a CCPACSTrailingEdgeDevice";
//        throw CTiglError("CTiglWingStructureReference is not a CCPACSTrailingEdgeDevice");
//    }
//    return *trailingEdgeDevice;
//}

}
